# Troubleshooting Guide

## Overview
This document provides solutions to common issues encountered when developing, deploying, and operating the TradingSystem. It includes diagnostic procedures, common error patterns, and resolution strategies.

## System Startup Issues

### 1. Application Won't Start

#### Symptoms
- Process exits immediately
- "Segmentation fault" error
- Missing library errors
- Configuration loading failures

#### Diagnostic Steps
```bash
# Check system resources
free -h
df -h

# Verify executable permissions
ls -la tradingsystem

# Check shared library dependencies
ldd tradingsystem

# Run with debug logging
export TRADING_SYSTEM_LOG_LEVEL=DEBUG
./tradingsystem

# Check system limits
ulimit -a
```

#### Common Causes & Solutions

**Missing Dependencies**
```bash
# Install required libraries
sudo apt-get install libc6-dev libstdc++6-dev
# or for CentOS/RHEL
sudo yum install glibc-devel libstdc++-devel
```

**Insufficient Memory/Resources**
```bash
# Check available memory
cat /proc/meminfo | grep MemAvailable

# Check swap space
swapon -s

# Increase memory limits if needed
echo 'vm.max_map_count=262144' >> /etc/sysctl.conf
sysctl -p
```

**Configuration Issues**
```bash
# Validate configuration file
./tradingsystem --validate-config

# Check configuration file permissions
ls -la config/

# Verify environment variables
env | grep TRADING_SYSTEM
```

### 2. Performance Issues at Startup

#### Symptoms
- Slow application startup (>5 seconds)
- High CPU usage during initialization
- Memory allocation errors

#### Solutions
```cpp
// Optimize initialization order
int main() {
    // Load critical components first
    auto config = load_configuration();
    auto logger = initialize_logging(config);
    
    // Parallel initialization for independent components
    std::vector<std::future<void>> init_futures;
    init_futures.emplace_back(std::async(std::launch::async, 
                                        initialize_market_data));
    init_futures.emplace_back(std::async(std::launch::async, 
                                        initialize_order_manager));
    
    // Wait for all components to initialize
    for (auto& future : init_futures) {
        future.wait();
    }
    
    return 0;
}
```

## Market Data Issues

### 1. No Market Data Received

#### Symptoms
- Empty market data feed
- Stale timestamp warnings
- Connection timeout errors

#### Diagnostic Steps
```bash
# Test network connectivity
ping market-data-provider.com
telnet market-data-provider.com 443

# Check firewall rules
sudo iptables -L
sudo ufw status

# Verify API credentials
curl -H "Authorization: Bearer $API_KEY" \
     https://api.provider.com/v1/status
```

#### Solutions

**Network Connectivity Issues**
```json
{
    "market_data": {
        "connection": {
            "timeout_ms": 10000,
            "retry_attempts": 5,
            "retry_delay_ms": 1000,
            "keep_alive": true
        }
    }
}
```

**Authentication Problems**
```cpp
// Implement robust authentication
class MarketDataAuth {
    bool refresh_token() {
        // Automatic token refresh logic
        auto response = http_client_.post("/auth/refresh", 
                                         {{"refresh_token", refresh_token_}});
        if (response.status_code == 200) {
            access_token_ = response.body["access_token"];
            return true;
        }
        return false;
    }
};
```

### 2. Data Quality Issues

#### Symptoms
- Unrealistic price movements
- Missing data points
- Inconsistent timestamps

#### Data Validation Solutions
```cpp
class DataValidator {
public:
    bool validate_market_data(const MarketData& data) {
        // Price sanity checks
        if (data.price <= 0 || data.price > max_reasonable_price_) {
            LOG(WARNING) << "Invalid price: " << data.price;
            return false;
        }
        
        // Timestamp validation
        auto now = std::chrono::system_clock::now();
        auto data_time = std::chrono::system_clock::from_time_t(data.timestamp);
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - data_time);
        
        if (age.count() > max_data_age_seconds_) {
            LOG(WARNING) << "Stale data detected, age: " << age.count() << "s";
            return false;
        }
        
        return true;
    }
    
private:
    double max_reasonable_price_ = 1000000.0;
    int max_data_age_seconds_ = 60;
};
```

## Trading Execution Issues

### 1. Orders Not Being Executed

#### Symptoms
- Orders stuck in "PENDING" status
- Broker API connection errors
- Order rejection messages

#### Diagnostic Approach
```cpp
class OrderDiagnostics {
public:
    void diagnose_order_failure(const Order& order) {
        LOG(INFO) << "Diagnosing order: " << order.order_id;
        
        // Check order parameters
        if (!validate_order_parameters(order)) {
            LOG(ERROR) << "Invalid order parameters";
            return;
        }
        
        // Check account status
        if (!check_account_status()) {
            LOG(ERROR) << "Account not ready for trading";
            return;
        }
        
        // Check market status
        if (!check_market_hours(order.symbol)) {
            LOG(ERROR) << "Market closed for symbol: " << order.symbol;
            return;
        }
        
        // Check position limits
        if (!check_position_limits(order)) {
            LOG(ERROR) << "Order would exceed position limits";
            return;
        }
    }
    
private:
    bool validate_order_parameters(const Order& order);
    bool check_account_status();
    bool check_market_hours(const std::string& symbol);
    bool check_position_limits(const Order& order);
};
```

### 2. Execution Latency Issues

#### Symptoms
- High order-to-fill latency
- Missed trading opportunities
- Slippage higher than expected

#### Performance Optimization
```cpp
// Pre-allocate order objects
class OrderPool {
public:
    OrderPool(size_t pool_size) {
        orders_.reserve(pool_size);
        for (size_t i = 0; i < pool_size; ++i) {
            orders_.emplace_back(std::make_unique<Order>());
            available_orders_.push(orders_.back().get());
        }
    }
    
    Order* acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (available_orders_.empty()) {
            return nullptr;
        }
        
        auto* order = available_orders_.front();
        available_orders_.pop();
        return order;
    }
    
    void release(Order* order) {
        std::lock_guard<std::mutex> lock(mutex_);
        order->reset();  // Clear order data
        available_orders_.push(order);
    }
    
private:
    std::vector<std::unique_ptr<Order>> orders_;
    std::queue<Order*> available_orders_;
    std::mutex mutex_;
};
```

## Risk Management Issues

### 1. Risk Limits Being Breached

#### Symptoms
- Position limit exceeded warnings
- Automatic trading shutdowns
- Risk alert notifications

#### Monitoring and Prevention
```cpp
class RiskMonitor {
public:
    void check_risk_limits() {
        auto current_exposure = calculate_total_exposure();
        
        if (current_exposure > max_exposure_) {
            LOG(CRITICAL) << "Exposure limit breached: " << current_exposure;
            emergency_shutdown();
        }
        
        auto current_var = calculate_var();
        if (current_var > max_var_) {
            LOG(WARNING) << "VaR limit approached: " << current_var;
            reduce_risk_exposure();
        }
    }
    
private:
    void emergency_shutdown();
    void reduce_risk_exposure();
    double calculate_total_exposure();
    double calculate_var();
    
    double max_exposure_ = 10000000.0;
    double max_var_ = 100000.0;
};
```

### 2. Position Tracking Discrepancies

#### Symptoms
- Position mismatches between systems
- Unexpected P&L calculations
- Reconciliation failures

#### Position Reconciliation
```cpp
class PositionReconciler {
public:
    void reconcile_positions() {
        auto internal_positions = get_internal_positions();
        auto broker_positions = get_broker_positions();
        
        for (const auto& [symbol, internal_pos] : internal_positions) {
            auto broker_it = broker_positions.find(symbol);
            if (broker_it == broker_positions.end()) {
                LOG(ERROR) << "Position mismatch - symbol not found at broker: " 
                          << symbol;
                continue;
            }
            
            if (std::abs(internal_pos - broker_it->second) > position_tolerance_) {
                LOG(ERROR) << "Position mismatch for " << symbol 
                          << " Internal: " << internal_pos 
                          << " Broker: " << broker_it->second;
                
                // Trigger reconciliation process
                schedule_position_sync(symbol);
            }
        }
    }
    
private:
    std::unordered_map<std::string, int64_t> get_internal_positions();
    std::unordered_map<std::string, int64_t> get_broker_positions();
    void schedule_position_sync(const std::string& symbol);
    
    static constexpr int64_t position_tolerance_ = 100;
};
```

## Performance Issues

### 1. High Latency

#### Symptoms
- Order processing > 1ms
- Market data delays
- System unresponsiveness

#### Latency Optimization
```cpp
// Use lock-free data structures
class LockFreeOrderQueue {
public:
    bool enqueue(const Order& order) {
        auto index = tail_.fetch_add(1, std::memory_order_relaxed);
        buffer_[index & (BUFFER_SIZE - 1)] = order;
        return true;
    }
    
    bool dequeue(Order& order) {
        auto tail = tail_.load(std::memory_order_acquire);
        auto head = head_.load(std::memory_order_relaxed);
        
        if (head == tail) {
            return false;  // Queue is empty
        }
        
        order = buffer_[head & (BUFFER_SIZE - 1)];
        head_.store(head + 1, std::memory_order_release);
        return true;
    }
    
private:
    static constexpr size_t BUFFER_SIZE = 1024;
    std::array<Order, BUFFER_SIZE> buffer_;
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};
```

### 2. Memory Issues

#### Symptoms
- Memory leaks
- Out of memory errors
- Excessive memory usage

#### Memory Debugging
```bash
# Use Valgrind for memory leak detection
valgrind --leak-check=full --show-leak-kinds=all ./tradingsystem

# Use AddressSanitizer
g++ -fsanitize=address -g -o tradingsystem main.cpp

# Monitor memory usage
watch -n 1 'ps aux | grep tradingsystem'

# Check for memory fragmentation
cat /proc/buddyinfo
```

## Database and Storage Issues

### 1. Database Connection Problems

#### Symptoms
- Connection timeouts
- "Too many connections" errors
- Slow query performance

#### Database Optimization
```cpp
class DatabaseManager {
public:
    DatabaseManager(const DatabaseConfig& config) 
        : connection_pool_(config.max_connections) {
        
        // Initialize connection pool
        for (int i = 0; i < config.max_connections; ++i) {
            auto conn = create_connection(config);
            connection_pool_.push(std::move(conn));
        }
    }
    
    template<typename Func>
    auto execute_with_retry(Func&& func) -> decltype(func()) {
        int attempts = 0;
        while (attempts < max_retry_attempts_) {
            try {
                return func();
            } catch (const DatabaseException& e) {
                LOG(WARNING) << "Database operation failed, attempt " 
                            << attempts + 1 << ": " << e.what();
                
                if (++attempts >= max_retry_attempts_) {
                    throw;
                }
                
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(retry_delay_ms_ * attempts));
            }
        }
    }
    
private:
    ThreadSafeQueue<std::unique_ptr<DatabaseConnection>> connection_pool_;
    static constexpr int max_retry_attempts_ = 3;
    static constexpr int retry_delay_ms_ = 1000;
};
```

### 2. Storage Space Issues

#### Symptoms
- Disk full errors
- Log rotation failures
- Database growth warnings

#### Storage Management
```bash
# Monitor disk usage
df -h

# Clean up old log files
find /var/log/tradingsystem -name "*.log" -mtime +30 -delete

# Archive old data
tar -czf trades_$(date +%Y%m).tar.gz /data/trades/$(date +%Y-%m)*/

# Set up automated cleanup
cat > /etc/cron.daily/tradingsystem-cleanup << 'EOF'
#!/bin/bash
# Clean up logs older than 30 days
find /var/log/tradingsystem -name "*.log" -mtime +30 -delete

# Archive data older than 90 days
find /data/tradingsystem -name "*.dat" -mtime +90 -exec gzip {} \;
EOF
```

## Configuration Issues

### 1. Invalid Configuration Values

#### Symptoms
- Application startup failures
- Unexpected behavior
- Configuration validation errors

#### Configuration Validation
```cpp
class ConfigValidator {
public:
    std::vector<std::string> validate(const Config& config) {
        std::vector<std::string> errors;
        
        // Validate numeric ranges
        if (config.max_position_size() <= 0) {
            errors.push_back("max_position_size must be positive");
        }
        
        if (config.timeout_ms() < 100 || config.timeout_ms() > 60000) {
            errors.push_back("timeout_ms must be between 100 and 60000");
        }
        
        // Validate dependencies
        if (config.enable_live_trading() && config.paper_trading_mode()) {
            errors.push_back("Cannot enable both live trading and paper trading");
        }
        
        // Validate file paths
        if (!std::filesystem::exists(config.log_directory())) {
            errors.push_back("Log directory does not exist: " + 
                           config.log_directory());
        }
        
        return errors;
    }
};
```

## Monitoring and Alerting

### 1. Health Check Implementation

```cpp
class HealthChecker {
public:
    struct HealthStatus {
        bool overall_healthy = true;
        std::string status_message;
        std::unordered_map<std::string, bool> component_status;
    };
    
    HealthStatus check_system_health() {
        HealthStatus status;
        
        // Check market data feed
        status.component_status["market_data"] = check_market_data_health();
        
        // Check order execution
        status.component_status["order_execution"] = check_order_execution_health();
        
        // Check database connectivity
        status.component_status["database"] = check_database_health();
        
        // Check memory usage
        status.component_status["memory"] = check_memory_health();
        
        // Determine overall health
        status.overall_healthy = std::all_of(
            status.component_status.begin(),
            status.component_status.end(),
            [](const auto& pair) { return pair.second; });
        
        if (!status.overall_healthy) {
            status.status_message = "One or more components unhealthy";
        } else {
            status.status_message = "All systems operational";
        }
        
        return status;
    }
    
private:
    bool check_market_data_health();
    bool check_order_execution_health();
    bool check_database_health();
    bool check_memory_health();
};
```

### 2. Performance Metrics Collection

```cpp
class MetricsCollector {
public:
    void record_latency(const std::string& operation, 
                       std::chrono::microseconds latency) {
        latency_metrics_[operation].push_back(latency.count());
        
        // Keep only recent measurements
        if (latency_metrics_[operation].size() > max_samples_) {
            latency_metrics_[operation].erase(
                latency_metrics_[operation].begin());
        }
    }
    
    double get_average_latency(const std::string& operation) {
        const auto& samples = latency_metrics_[operation];
        if (samples.empty()) return 0.0;
        
        return std::accumulate(samples.begin(), samples.end(), 0.0) / 
               samples.size();
    }
    
    double get_p99_latency(const std::string& operation) {
        auto samples = latency_metrics_[operation];
        if (samples.empty()) return 0.0;
        
        std::sort(samples.begin(), samples.end());
        size_t p99_index = static_cast<size_t>(samples.size() * 0.99);
        return samples[p99_index];
    }
    
private:
    std::unordered_map<std::string, std::vector<double>> latency_metrics_;
    static constexpr size_t max_samples_ = 10000;
};
```

## Emergency Procedures

### 1. Emergency Shutdown

```cpp
class EmergencyShutdown {
public:
    void initiate_emergency_shutdown(const std::string& reason) {
        LOG(CRITICAL) << "EMERGENCY SHUTDOWN INITIATED: " << reason;
        
        // Set global shutdown flag
        emergency_shutdown_flag_.store(true, std::memory_order_release);
        
        // Cancel all pending orders
        order_manager_->cancel_all_orders();
        
        // Close all positions if configured
        if (config_.close_positions_on_emergency_shutdown) {
            portfolio_manager_->close_all_positions();
        }
        
        // Stop market data feeds
        market_data_manager_->stop_all_feeds();
        
        // Flush all logs
        logger_->flush();
        
        // Send emergency notifications
        notification_service_->send_emergency_alert(reason);
        
        LOG(CRITICAL) << "Emergency shutdown completed";
    }
    
    bool is_emergency_shutdown_active() const {
        return emergency_shutdown_flag_.load(std::memory_order_acquire);
    }
    
private:
    std::atomic<bool> emergency_shutdown_flag_{false};
    std::unique_ptr<OrderManager> order_manager_;
    std::unique_ptr<PortfolioManager> portfolio_manager_;
    std::unique_ptr<MarketDataManager> market_data_manager_;
    std::unique_ptr<Logger> logger_;
    std::unique_ptr<NotificationService> notification_service_;
    EmergencyConfig config_;
};
```

## Support and Escalation

### 1. Log Collection for Support

```bash
#!/bin/bash
# collect_logs.sh - Collect diagnostic information

echo "Collecting TradingSystem diagnostic information..."

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
OUTPUT_DIR="/tmp/tradingsystem_diag_${TIMESTAMP}"
ARCHIVE_NAME="tradingsystem_diag_${TIMESTAMP}.tar.gz"

mkdir -p "$OUTPUT_DIR"

# System information
uname -a > "$OUTPUT_DIR/system_info.txt"
cat /proc/cpuinfo > "$OUTPUT_DIR/cpu_info.txt"
free -h > "$OUTPUT_DIR/memory_info.txt"
df -h > "$OUTPUT_DIR/disk_info.txt"

# Application logs
cp -r /var/log/tradingsystem "$OUTPUT_DIR/logs/"

# Configuration files
cp -r /etc/tradingsystem "$OUTPUT_DIR/config/"

# Process information
ps aux | grep tradingsystem > "$OUTPUT_DIR/process_info.txt"

# Network information
netstat -tlnp > "$OUTPUT_DIR/network_info.txt"

# Create archive
tar -czf "$ARCHIVE_NAME" -C /tmp "tradingsystem_diag_${TIMESTAMP}"

echo "Diagnostic information collected: $ARCHIVE_NAME"
echo "Please send this file to the support team"
```

### 2. Common Error Codes and Meanings

| Error Code | Description | Likely Cause | Recommended Action |
|------------|-------------|--------------|-------------------|
| E001 | Market data connection failed | Network/API issue | Check connectivity, verify credentials |
| E002 | Order validation failed | Invalid parameters | Review order parameters |
| E003 | Risk limit exceeded | Position/exposure too high | Reduce positions |
| E004 | Database connection lost | DB server issue | Check database status |
| E005 | Configuration error | Invalid config values | Validate configuration |
| E006 | Memory allocation failed | Insufficient memory | Increase memory limits |
| E007 | Thread creation failed | Resource exhaustion | Check system limits |
| E008 | File I/O error | Disk/permission issue | Check disk space and permissions |