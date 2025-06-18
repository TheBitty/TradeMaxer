#ifndef IPC_MANAGER_H
#define IPC_MANAGER_H

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace TradingSystem {

class IPCManager {
public:
    IPCManager(const std::string& pipe_name = "/tmp/trading_system_pipe");
    ~IPCManager();
    
    // Initialize IPC
    bool initialize();
    
    // Send message to Python process
    bool sendMessage(const std::string& message);
    
    // Receive message from Python process
    std::string receiveMessage(int timeout_ms = 5000);
    
    // Set callback for incoming messages
    void setMessageCallback(std::function<void(const std::string&)> callback);
    
    // Start/stop message processing
    void start();
    void stop();
    
    bool isConnected() const { return connected; }
    
private:
    std::string pipe_name;
    int write_fd;
    int read_fd;
    std::atomic<bool> connected;
    std::atomic<bool> running;
    
    std::thread reader_thread;
    std::queue<std::string> message_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    
    std::function<void(const std::string&)> message_callback;
    
    void readerLoop();
    bool createNamedPipe();
    void cleanup();
};

// Python process manager
class PythonProcessManager {
public:
    PythonProcessManager(const std::string& script_path);
    ~PythonProcessManager();
    
    bool start();
    void stop();
    bool isRunning() const;
    
    void attachIPC(IPCManager* ipc) { this->ipc = ipc; }
    
private:
    std::string script_path;
    pid_t process_pid;
    IPCManager* ipc;
    std::atomic<bool> running;
};

} // namespace TradingSystem

#endif // IPC_MANAGER_H