# Configuration Guide

## Overview
This guide provides comprehensive information about configuring the TradingSystem application, including system settings, market data connections, broker integrations, and performance tuning.

## Configuration Architecture

### 1. Configuration Hierarchy
The system uses a hierarchical configuration approach:

```
1. Default Values (hardcoded)
2. Configuration Files (config/*.json)
3. Environment Variables
4. Command Line Arguments (highest priority)
```

### 2. Configuration File Structure
```json
{
    "system": {
        "log_level": "INFO",
        "max_threads": 8,
        "memory_limit_mb": 4096,
        "performance_mode": "high_latency"
    },
    "market_data": {
        "primary_feed": "bloomberg",
        "backup_feeds": ["refinitiv", "iex"],
        "buffer_size": 10000,
        "timeout_ms": 5000
    },
    "trading": {
        "max_position_size": 10000,
        "max_order_value": 1000000,
        "risk_limit_percent": 2.0,
        "default_currency": "USD"
    },
    "connectivity": {
        "broker_api": {
            "endpoint": "https://api.broker.com/v1",
            "timeout_ms": 3000,
            "retry_attempts": 3
        }
    }
}
```

## System Configuration

### 1. Core System Settings

#### Performance Configuration
```json
{
    "system": {
        "performance_mode": "ultra_low_latency|low_latency|balanced|high_throughput",
        "cpu_affinity": [0, 1, 2, 3],
        "memory_limit_mb": 4096,
        "gc_frequency": "low|normal|high",
        "thread_pool_size": 8,
        "numa_node": 0
    }
}
```

#### Logging Configuration
```json
{
    "logging": {
        "level": "TRACE|DEBUG|INFO|WARN|ERROR|FATAL",
        "output": "console|file|both",
        "file_path": "/var/log/tradingsystem/",
        "max_file_size_mb": 100,
        "max_files": 10,
        "async_logging": true,
        "buffer_size": 8192
    }
}
```

### 2. Environment Variables

#### System Environment Variables
```bash
# Core system settings
export TRADING_SYSTEM_LOG_LEVEL=INFO
export TRADING_SYSTEM_CONFIG_PATH=/etc/tradingsystem/
export TRADING_SYSTEM_DATA_PATH=/var/lib/tradingsystem/
export TRADING_SYSTEM_PERFORMANCE_MODE=low_latency

# Memory and threading
export TRADING_SYSTEM_MAX_MEMORY_MB=4096
export TRADING_SYSTEM_THREAD_COUNT=8
export TRADING_SYSTEM_CPU_AFFINITY=0,1,2,3

# Security
export TRADING_SYSTEM_API_KEY_FILE=/etc/tradingsystem/secrets/api_keys
export TRADING_SYSTEM_CERT_PATH=/etc/tradingsystem/certs/
```

## Market Data Configuration

### 1. Data Feed Settings

#### Primary Feed Configuration
```json
{
    "market_data": {
        "feeds": {
            "bloomberg": {
                "enabled": true,
                "priority": 1,
                "endpoint": "bloomberg://localhost:8194",
                "symbols": ["AAPL", "GOOGL", "MSFT", "TSLA"],
                "subscription_type": "real_time",
                "buffer_size": 10000,
                "timeout_ms": 1000
            },
            "refinitiv": {
                "enabled": true,
                "priority": 2,
                "endpoint": "https://api.refinitiv.com/v1",
                "api_key_env": "REFINITIV_API_KEY",
                "rate_limit": 1000,
                "timeout_ms": 2000
            }
        }
    }
}
```

#### Data Processing Settings
```json
{
    "market_data": {
        "processing": {
            "validate_prices": true,
            "outlier_detection": true,
            "outlier_threshold": 3.0,
            "time_sync_tolerance_ms": 100,
            "duplicate_detection": true,
            "data_retention_hours": 24
        }
    }
}
```

### 2. Symbol Configuration

#### Symbol Mapping
```json
{
    "symbols": {
        "AAPL": {
            "exchange": "NASDAQ",
            "currency": "USD",
            "tick_size": 0.01,
            "lot_size": 100,
            "trading_hours": {
                "open": "09:30:00",
                "close": "16:00:00",
                "timezone": "America/New_York"
            }
        }
    }
}
```

## Trading Configuration

### 1. Risk Management Settings

#### Position Limits
```json
{
    "risk": {
        "position_limits": {
            "max_position_size": 10000,
            "max_position_value": 1000000,
            "max_concentration_percent": 5.0,
            "max_sector_exposure_percent": 20.0
        },
        "order_limits": {
            "max_order_size": 1000,
            "max_order_value": 100000,
            "max_orders_per_second": 10,
            "max_orders_per_minute": 100
        },
        "drawdown_limits": {
            "max_daily_loss": 50000,
            "max_weekly_loss": 200000,
            "stop_trading_threshold": 100000
        }
    }
}
```

#### Risk Monitoring
```json
{
    "risk": {
        "monitoring": {
            "real_time_pnl": true,
            "position_delta": true,
            "correlation_tracking": true,
            "var_calculation": true,
            "stress_testing": false,
            "alert_thresholds": {
                "high_correlation": 0.8,
                "large_position": 0.05,
                "unusual_volume": 3.0
            }
        }
    }
}
```

### 2. Strategy Configuration

#### Strategy Parameters
```json
{
    "strategies": {
        "momentum": {
            "enabled": true,
            "lookback_periods": 20,
            "threshold": 0.02,
            "max_positions": 10,
            "position_size": 1000,
            "stop_loss": 0.05,
            "take_profit": 0.10
        },
        "mean_reversion": {
            "enabled": false,
            "lookback_periods": 50,
            "z_score_threshold": 2.0,
            "max_positions": 5
        }
    }
}
```

## Connectivity Configuration

### 1. Broker API Settings

#### Order Execution
```json
{
    "broker": {
        "primary": {
            "name": "interactive_brokers",
            "endpoint": "https://api.interactivebrokers.com/v1",
            "port": 7497,
            "client_id": 1,
            "authentication": {
                "type": "api_key",
                "key_file": "/etc/tradingsystem/secrets/ib_api_key"
            },
            "order_routing": {
                "default_route": "SMART",
                "timeout_ms": 5000,
                "retry_attempts": 3
            }
        }
    }
}
```

#### Connection Management
```json
{
    "connectivity": {
        "connection_pool": {
            "max_connections": 10,
            "idle_timeout_ms": 30000,
            "connect_timeout_ms": 5000,
            "heartbeat_interval_ms": 10000
        },
        "failover": {
            "enabled": true,
            "check_interval_ms": 1000,
            "max_retry_attempts": 5,
            "backoff_multiplier": 2.0
        }
    }
}
```

### 2. Database Configuration

#### Primary Database
```json
{
    "database": {
        "primary": {
            "type": "postgresql",
            "host": "localhost",
            "port": 5432,
            "database": "tradingsystem",
            "username": "trading_user",
            "password_env": "DB_PASSWORD",
            "connection_pool": {
                "min_connections": 5,
                "max_connections": 20,
                "idle_timeout_ms": 60000
            }
        },
        "cache": {
            "type": "redis",
            "host": "localhost",
            "port": 6379,
            "database": 0,
            "password_env": "REDIS_PASSWORD",
            "ttl_seconds": 3600
        }
    }
}
```

## Performance Tuning

### 1. Latency Optimization

#### Network Settings
```json
{
    "network": {
        "tcp_nodelay": true,
        "tcp_quickack": true,
        "socket_buffer_size": 65536,
        "kernel_bypass": false,
        "zero_copy": true,
        "numa_networking": true
    }
}
```

#### Memory Configuration
```json
{
    "memory": {
        "huge_pages": true,
        "memory_mapping": true,
        "lock_memory": true,
        "prefault_memory": true,
        "object_pools": {
            "order_pool_size": 10000,
            "message_pool_size": 50000,
            "buffer_pool_size": 1000
        }
    }
}
```

### 2. Threading Configuration

#### Thread Affinity
```json
{
    "threading": {
        "thread_affinity": {
            "market_data_thread": [0, 1],
            "order_processing_thread": [2, 3],
            "risk_monitoring_thread": [4],
            "strategy_threads": [5, 6, 7]
        },
        "scheduling": {
            "policy": "SCHED_FIFO",
            "priority": 99,
            "rt_priority": true
        }
    }
}
```

## Security Configuration

### 1. Authentication & Authorization

#### API Security
```json
{
    "security": {
        "authentication": {
            "type": "oauth2|api_key|certificate",
            "token_refresh_interval": 3600,
            "certificate_path": "/etc/tradingsystem/certs/",
            "key_rotation_days": 90
        },
        "encryption": {
            "in_transit": true,
            "at_rest": true,
            "algorithm": "AES-256-GCM",
            "key_management": "vault"
        }
    }
}
```

### 2. Audit and Compliance

#### Logging and Monitoring
```json
{
    "compliance": {
        "audit_logging": {
            "enabled": true,
            "log_all_orders": true,
            "log_all_trades": true,
            "log_risk_events": true,
            "retention_days": 2555
        },
        "reporting": {
            "daily_reports": true,
            "regulatory_reports": true,
            "risk_reports": true,
            "output_format": "csv|json|xml"
        }
    }
}
```

## Configuration Management

### 1. Configuration Validation

#### Schema Validation
```cpp
// Example configuration validation
class ConfigValidator {
public:
    bool validate_config(const Config& config) {
        // Validate ranges, dependencies, etc.
        return true;
    }
    
private:
    bool validate_performance_settings(const PerformanceConfig& perf);
    bool validate_risk_limits(const RiskConfig& risk);
    bool validate_connectivity(const ConnectivityConfig& conn);
};
```

### 2. Dynamic Configuration Updates

#### Hot Configuration Reload
```cpp
class ConfigurationManager {
public:
    void reload_config();
    void update_setting(const std::string& path, const std::string& value);
    void register_change_listener(const std::string& path, 
                                 ConfigChangeCallback callback);
    
private:
    void notify_listeners(const std::string& path);
};
```

## Deployment Configurations

### 1. Environment-Specific Configs

#### Development Environment
```json
{
    "environment": "development",
    "logging": {
        "level": "DEBUG",
        "output": "console"
    },
    "market_data": {
        "simulation_mode": true,
        "replay_data": "/data/historical/replay.csv"
    },
    "trading": {
        "paper_trading": true,
        "initial_balance": 1000000
    }
}
```

#### Production Environment
```json
{
    "environment": "production",
    "logging": {
        "level": "INFO",
        "output": "file",
        "file_path": "/var/log/tradingsystem/"
    },
    "market_data": {
        "simulation_mode": false,
        "real_time_feeds": true
    },
    "trading": {
        "paper_trading": false,
        "live_trading": true
    }
}
```

### 2. Container Configuration

#### Docker Configuration
```dockerfile
# Environment variables for container
ENV TRADING_SYSTEM_CONFIG_PATH=/app/config
ENV TRADING_SYSTEM_LOG_LEVEL=INFO
ENV TRADING_SYSTEM_PERFORMANCE_MODE=low_latency

# Volume mounts for configuration
VOLUME ["/app/config", "/app/logs", "/app/data"]
```

#### Kubernetes Configuration
```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: tradingsystem-config
data:
  config.json: |
    {
      "system": {
        "log_level": "INFO",
        "performance_mode": "low_latency"
      }
    }
```

## Troubleshooting Configuration Issues

### 1. Common Configuration Problems

#### Invalid Configuration Values
- Check configuration file syntax (JSON validation)
- Verify environment variable names and values
- Validate configuration value ranges and types
- Check file permissions for configuration files

#### Connection Issues
- Verify network connectivity to external services
- Check firewall rules and security groups
- Validate API keys and authentication credentials
- Test timeout and retry settings

### 2. Configuration Debugging

#### Debug Mode
```bash
# Enable configuration debugging
export TRADING_SYSTEM_DEBUG_CONFIG=true
export TRADING_SYSTEM_LOG_LEVEL=DEBUG

# Run with configuration validation
./tradingsystem --validate-config --config-file=/path/to/config.json
```

#### Configuration Logging
```cpp
// Log all configuration values on startup
void log_configuration(const Config& config) {
    LOG(INFO) << "Configuration loaded:";
    LOG(INFO) << "  Log Level: " << config.log_level();
    LOG(INFO) << "  Performance Mode: " << config.performance_mode();
    LOG(INFO) << "  Max Memory: " << config.max_memory_mb() << "MB";
    // ... log other important settings
}
```

## Configuration Best Practices

### 1. Security Best Practices
- Never store sensitive information in plain text
- Use environment variables for secrets
- Implement configuration encryption for sensitive settings
- Regularly rotate API keys and certificates
- Use minimal permissions for configuration files

### 2. Performance Best Practices
- Profile configuration impact on startup time
- Cache frequently accessed configuration values
- Use appropriate data types for configuration values
- Implement lazy loading for non-critical configurations
- Monitor configuration change impact on performance

### 3. Maintenance Best Practices
- Version control all configuration files
- Document all configuration parameters
- Implement configuration validation and testing
- Use infrastructure as code for deployment configurations
- Maintain separate configurations for different environments