#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <signal.h>
#include <sys/wait.h>

struct SharedData {
    double market_data[100];
    int data_count;
    bool python_ready;
    bool cpp_ready;
};

class TradingApi {
private:
    SharedData* shared_memory;
    int shm_fd;
    pid_t python_pid;
    
public:
    TradingApi() : shared_memory(nullptr), shm_fd(-1), python_pid(-1) {}
    
    bool initialize() {
        const char* shm_name = "/trading_shm";
        
        shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            std::cerr << "Failed to create shared memory" << std::endl;
            return false;
        }
        
        if (ftruncate(shm_fd, sizeof(SharedData)) == -1) {
            std::cerr << "Failed to set shared memory size" << std::endl;
            return false;
        }
        
        shared_memory = static_cast<SharedData*>(
            mmap(0, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)
        );
        
        if (shared_memory == MAP_FAILED) {
            std::cerr << "Failed to map shared memory" << std::endl;
            return false;
        }
        
        memset(shared_memory, 0, sizeof(SharedData));
        shared_memory->cpp_ready = true;
        
        return true;
    }
    
    bool launchPythonScript() {
        python_pid = fork();
        
        if (python_pid == 0) {
            execl("/usr/bin/python3", "python3", "math.py", nullptr);
            std::cerr << "Failed to launch Python script" << std::endl;
            exit(1);
        } else if (python_pid < 0) {
            std::cerr << "Failed to fork process" << std::endl;
            return false;
        }
        
        while (!shared_memory->python_ready) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "Python script launched successfully" << std::endl;
        return true;
    }
    
    void sendMarketData(const double* data, int count) {
        if (count > 100) count = 100;
        
        memcpy(shared_memory->market_data, data, count * sizeof(double));
        shared_memory->data_count = count;
    }
    
    void cleanup() {
        if (python_pid > 0) {
            kill(python_pid, SIGTERM);
            waitpid(python_pid, nullptr, 0);
        }
        
        if (shared_memory != nullptr) {
            munmap(shared_memory, sizeof(SharedData));
        }
        
        if (shm_fd != -1) {
            close(shm_fd);
            shm_unlink("/trading_shm");
        }
    }
    
    ~TradingApi() {
        cleanup();
    }
};

int main() {
    TradingApi api;
    
    if (!api.initialize()) {
        std::cerr << "Failed to initialize trading API" << std::endl;
        return 1;
    }
    
    if (!api.launchPythonScript()) {
        std::cerr << "Failed to launch Python script" << std::endl;
        return 1;
    }
    
    double sample_data[] = {100.5, 101.2, 99.8, 102.1, 98.9};
    api.sendMarketData(sample_data, 5);
    
    std::cout << "Trading system running. Press Ctrl+C to exit." << std::endl;
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
