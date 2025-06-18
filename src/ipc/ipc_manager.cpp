#include "ipc_manager.h"
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <cstring>

namespace TradingSystem {

IPCManager::IPCManager(const std::string& pipe_name) 
    : pipe_name(pipe_name), write_fd(-1), read_fd(-1), 
      connected(false), running(false) {
}

IPCManager::~IPCManager() {
    stop();
    cleanup();
}

bool IPCManager::initialize() {
    return createNamedPipe();
}

bool IPCManager::createNamedPipe() {
    // Create two pipes for bidirectional communication
    std::string write_pipe = pipe_name + "_to_python";
    std::string read_pipe = pipe_name + "_to_cpp";
    
    // Remove existing pipes
    unlink(write_pipe.c_str());
    unlink(read_pipe.c_str());
    
    // Create new pipes
    if (mkfifo(write_pipe.c_str(), 0666) == -1) {
        std::cerr << "Failed to create write pipe: " << strerror(errno) << std::endl;
        return false;
    }
    
    if (mkfifo(read_pipe.c_str(), 0666) == -1) {
        std::cerr << "Failed to create read pipe: " << strerror(errno) << std::endl;
        unlink(write_pipe.c_str());
        return false;
    }
    
    connected = true;
    return true;
}

void IPCManager::start() {
    if (running) return;
    
    running = true;
    reader_thread = std::thread(&IPCManager::readerLoop, this);
}

void IPCManager::stop() {
    running = false;
    
    if (reader_thread.joinable()) {
        reader_thread.join();
    }
}

bool IPCManager::sendMessage(const std::string& message) {
    if (!connected) return false;
    
    std::string write_pipe = pipe_name + "_to_python";
    
    // Open pipe for writing if not already open
    if (write_fd == -1) {
        write_fd = open(write_pipe.c_str(), O_WRONLY | O_NONBLOCK);
        if (write_fd == -1) {
            // Try without non-blocking first time
            write_fd = open(write_pipe.c_str(), O_WRONLY);
            if (write_fd == -1) {
                std::cerr << "Failed to open write pipe: " << strerror(errno) << std::endl;
                return false;
            }
        }
    }
    
    // Add newline for message delimiter
    std::string msg_with_delimiter = message + "\n";
    ssize_t bytes_written = write(write_fd, msg_with_delimiter.c_str(), msg_with_delimiter.length());
    
    if (bytes_written == -1) {
        std::cerr << "Failed to write to pipe: " << strerror(errno) << std::endl;
        close(write_fd);
        write_fd = -1;
        return false;
    }
    
    return true;
}

std::string IPCManager::receiveMessage(int timeout_ms) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    
    if (queue_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), 
                         [this] { return !message_queue.empty(); })) {
        std::string message = message_queue.front();
        message_queue.pop();
        return message;
    }
    
    return "";
}

void IPCManager::setMessageCallback(std::function<void(const std::string&)> callback) {
    message_callback = callback;
}

void IPCManager::readerLoop() {
    std::string read_pipe = pipe_name + "_to_cpp";
    
    while (running) {
        if (read_fd == -1) {
            read_fd = open(read_pipe.c_str(), O_RDONLY);
            if (read_fd == -1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
        }
        
        char buffer[4096];
        ssize_t bytes_read = read(read_fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::string message(buffer);
            
            // Split by newlines in case multiple messages
            size_t pos = 0;
            while ((pos = message.find('\n')) != std::string::npos) {
                std::string msg = message.substr(0, pos);
                
                if (!msg.empty()) {
                    // Add to queue
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex);
                        message_queue.push(msg);
                    }
                    queue_cv.notify_one();
                    
                    // Call callback if set
                    if (message_callback) {
                        message_callback(msg);
                    }
                }
                
                message.erase(0, pos + 1);
            }
        } else if (bytes_read == 0) {
            // Pipe closed, wait and retry
            close(read_fd);
            read_fd = -1;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    if (read_fd != -1) {
        close(read_fd);
        read_fd = -1;
    }
}

void IPCManager::cleanup() {
    if (write_fd != -1) {
        close(write_fd);
        write_fd = -1;
    }
    
    if (read_fd != -1) {
        close(read_fd);
        read_fd = -1;
    }
    
    // Remove pipes
    std::string write_pipe = pipe_name + "_to_python";
    std::string read_pipe = pipe_name + "_to_cpp";
    unlink(write_pipe.c_str());
    unlink(read_pipe.c_str());
    
    connected = false;
}

// PythonProcessManager implementation
PythonProcessManager::PythonProcessManager(const std::string& script_path)
    : script_path(script_path), process_pid(-1), ipc(nullptr), running(false) {
}

PythonProcessManager::~PythonProcessManager() {
    stop();
}

bool PythonProcessManager::start() {
    if (running) return true;
    
    process_pid = fork();
    
    if (process_pid == -1) {
        std::cerr << "Failed to fork process: " << strerror(errno) << std::endl;
        return false;
    }
    
    if (process_pid == 0) {
        // Child process - execute Python script
        execlp("python3", "python3", script_path.c_str(), nullptr);
        
        // If we get here, exec failed
        std::cerr << "Failed to execute Python script: " << strerror(errno) << std::endl;
        exit(1);
    }
    
    // Parent process
    running = true;
    return true;
}

void PythonProcessManager::stop() {
    if (!running || process_pid == -1) return;
    
    // Send SIGTERM to Python process
    kill(process_pid, SIGTERM);
    
    // Wait for process to terminate
    int status;
    waitpid(process_pid, &status, 0);
    
    process_pid = -1;
    running = false;
}

bool PythonProcessManager::isRunning() const {
    if (!running || process_pid == -1) return false;
    
    // Check if process is still alive
    int status;
    pid_t result = waitpid(process_pid, &status, WNOHANG);
    
    return result == 0;
}

} // namespace TradingSystem