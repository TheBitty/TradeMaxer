# User Guide

## Overview
This guide provides comprehensive instructions for using the TradingSystem application, from initial setup through advanced trading operations. It covers both end-user functionality and administrative procedures.

## Getting Started

### 1. System Requirements

#### Minimum Requirements
- **OS**: Linux (Ubuntu 20.04+, CentOS 8+)
- **CPU**: 4 cores, 2.5GHz+
- **Memory**: 8GB RAM
- **Storage**: 100GB available space
- **Network**: Stable internet connection (1Mbps+)

#### Recommended Requirements
- **OS**: Linux with real-time kernel
- **CPU**: 8+ cores, 3.0GHz+, dedicated for trading
- **Memory**: 32GB RAM
- **Storage**: SSD with 500GB+ available space
- **Network**: Low-latency connection (< 10ms to exchanges)

### 2. Installation

#### Quick Installation
```bash
# Download the latest release
wget https://github.com/yourorg/tradingsystem/releases/latest/tradingsystem.tar.gz

# Extract the archive
tar -xzf tradingsystem.tar.gz
cd tradingsystem

# Run the installation script
sudo ./install.sh

# Start the system
./tradingsystem
```

#### Manual Installation
```bash
# Clone the repository
git clone https://github.com/yourorg/tradingsystem.git
cd tradingsystem

# Build from source
mkdir build && cd build
cmake ..
make -j$(nproc)

# Install system-wide
sudo make install
```

### 3. Initial Configuration

#### Basic Configuration File
Create `/etc/tradingsystem/config.json`:
```json
{
    "system": {
        "log_level": "INFO",
        "performance_mode": "balanced"
    },
    "trading": {
        "paper_trading": true,
        "initial_balance": 100000,
        "max_position_size": 1000,
        "default_currency": "USD"
    },
    "market_data": {
        "simulation_mode": true,
        "data_source": "yahoo_finance"
    }
}
```

#### Environment Setup
```bash
# Set up environment variables
export TRADING_SYSTEM_CONFIG_PATH=/etc/tradingsystem/
export TRADING_SYSTEM_LOG_LEVEL=INFO
export TRADING_SYSTEM_DATA_PATH=/var/lib/tradingsystem/

# Add to your shell profile for persistence
echo 'export TRADING_SYSTEM_CONFIG_PATH=/etc/tradingsystem/' >> ~/.bashrc
```

## Basic Operations

### 1. Starting the System

#### Command Line Startup
```bash
# Start with default configuration
./tradingsystem

# Start with specific configuration file
./tradingsystem --config /path/to/config.json

# Start in paper trading mode
./tradingsystem --paper-trading

# Start with debug logging
./tradingsystem --log-level DEBUG
```

#### Service Mode (Linux)
```bash
# Start as a systemd service
sudo systemctl start tradingsystem

# Enable automatic startup
sudo systemctl enable tradingsystem

# Check service status
sudo systemctl status tradingsystem
```

### 2. Basic Trading Workflow

#### Step 1: Monitor Market Data
```bash
# Check market data status
./tradingsystem status --market-data

# List available symbols
./tradingsystem symbols list

# Get current price for a symbol
./tradingsystem price AAPL
```

#### Step 2: Place Orders
```bash
# Place a market buy order
./tradingsystem order buy AAPL 100 --type market

# Place a limit sell order
./tradingsystem order sell AAPL 50 --type limit --price 150.00

# Place a stop-loss order
./tradingsystem order sell AAPL 100 --type stop --stop-price 140.00
```

#### Step 3: Monitor Positions
```bash
# View current positions
./tradingsystem positions

# Check portfolio summary
./tradingsystem portfolio summary

# View P&L
./tradingsystem pnl --period day
```

### 3. Order Management

#### Order Types Supported
1. **Market Orders**: Execute immediately at current market price
2. **Limit Orders**: Execute only at specified price or better
3. **Stop Orders**: Triggered when price reaches stop level
4. **Stop-Limit Orders**: Combination of stop and limit orders

#### Order Commands
```bash
# List all orders
./tradingsystem orders list

# Get order details
./tradingsystem orders get <order_id>

# Cancel an order
./tradingsystem orders cancel <order_id>

# Cancel all orders for a symbol
./tradingsystem orders cancel --symbol AAPL

# Cancel all orders
./tradingsystem orders cancel --all
```

## Advanced Features

### 1. Strategy Management

#### Creating a Strategy
```bash
# Create a new strategy from template
./tradingsystem strategy create --name my_strategy --template momentum

# List available strategy templates
./tradingsystem strategy templates

# Configure strategy parameters
./tradingsystem strategy config my_strategy --set lookback_period=20
```

#### Running Strategies
```bash
# Start a strategy
./tradingsystem strategy start my_strategy

# Stop a strategy
./tradingsystem strategy stop my_strategy

# List running strategies
./tradingsystem strategy list --status running

# View strategy performance
./tradingsystem strategy performance my_strategy --period week
```

### 2. Risk Management

#### Setting Risk Limits
```bash
# Set maximum position size
./tradingsystem risk set-limit max_position_size 10000

# Set daily loss limit
./tradingsystem risk set-limit daily_loss_limit 5000

# Set maximum portfolio exposure
./tradingsystem risk set-limit max_exposure 50000

# View current risk limits
./tradingsystem risk limits
```

#### Risk Monitoring
```bash
# Check current risk metrics
./tradingsystem risk status

# View risk alerts
./tradingsystem risk alerts

# Generate risk report
./tradingsystem risk report --format pdf --output risk_report.pdf
```

### 3. Portfolio Analysis

#### Performance Metrics
```bash
# View portfolio performance
./tradingsystem portfolio performance --period month

# Calculate Sharpe ratio
./tradingsystem portfolio metrics --metric sharpe_ratio

# View drawdown analysis
./tradingsystem portfolio drawdown --period year

# Export performance data
./tradingsystem portfolio export --format csv --output portfolio.csv
```

#### Position Analysis
```bash
# View position details
./tradingsystem positions details AAPL

# Analyze position P&L
./tradingsystem positions pnl --symbol AAPL --breakdown

# View sector allocation
./tradingsystem positions allocation --by sector

# Check position correlations
./tradingsystem positions correlation
```

## Market Data Management

### 1. Data Sources Configuration

#### Adding Data Sources
```bash
# Add a new data source
./tradingsystem data-source add --name bloomberg --type bloomberg \
  --endpoint "bloomberg://localhost:8194"

# Configure data source settings
./tradingsystem data-source config bloomberg --set timeout=5000

# Test data source connectivity
./tradingsystem data-source test bloomberg
```

#### Symbol Management
```bash
# Add symbols to watch list
./tradingsystem symbols add AAPL GOOGL MSFT

# Remove symbols
./tradingsystem symbols remove AMZN

# Update symbol information
./tradingsystem symbols update --source exchange_data
```

### 2. Historical Data

#### Data Download
```bash
# Download historical data
./tradingsystem data download AAPL --start 2023-01-01 --end 2023-12-31

# Download intraday data
./tradingsystem data download AAPL --interval 1min --days 5

# Bulk download for multiple symbols
./tradingsystem data download --symbols-file symbols.txt --period 1year
```

#### Data Analysis
```bash
# View data statistics
./tradingsystem data stats AAPL --period month

# Check data quality
./tradingsystem data quality-check --symbol AAPL

# Export historical data
./tradingsystem data export AAPL --format csv --output aapl_data.csv
```

## Configuration Management

### 1. Runtime Configuration

#### Viewing Configuration
```bash
# Show all configuration
./tradingsystem config show

# Show specific section
./tradingsystem config show trading

# Show specific setting
./tradingsystem config get trading.max_position_size
```

#### Updating Configuration
```bash
# Set a configuration value
./tradingsystem config set trading.max_position_size 5000

# Set multiple values from file
./tradingsystem config load --file updated_config.json

# Reset to default values
./tradingsystem config reset trading.risk_limits
```

### 2. Profile Management

#### Creating Profiles
```bash
# Create a new configuration profile
./tradingsystem profile create production --copy-from default

# List available profiles
./tradingsystem profile list

# Switch to a profile
./tradingsystem profile use production

# Delete a profile
./tradingsystem profile delete old_profile
```

## Monitoring and Logging

### 1. System Monitoring

#### Real-time Monitoring
```bash
# View system status
./tradingsystem status

# Monitor performance metrics
./tradingsystem monitor --metrics latency,throughput,memory

# View active connections
./tradingsystem monitor connections

# Check system health
./tradingsystem health-check
```

#### Log Management
```bash
# View recent logs
./tradingsystem logs tail

# Search logs
./tradingsystem logs search "ERROR" --last 24h

# Export logs
./tradingsystem logs export --start "2023-12-01" --end "2023-12-31" \
  --output logs_december.tar.gz
```

### 2. Alerts and Notifications

#### Setting Up Alerts
```bash
# Configure email alerts
./tradingsystem alerts config email --smtp-server smtp.gmail.com \
  --username your_email@gmail.com --password-file email_password.txt

# Set up Slack notifications
./tradingsystem alerts config slack --webhook-url "https://hooks.slack.com/..."

# Create custom alert rules
./tradingsystem alerts rule create --name "large_loss" \
  --condition "daily_pnl < -1000" --action "email,slack"
```

## API Usage

### 1. REST API

#### Authentication
```bash
# Generate API key
./tradingsystem api-key create --name "my_app" --permissions read,write

# Test API connectivity
curl -H "Authorization: Bearer YOUR_API_KEY" \
  http://localhost:8080/api/v1/status
```

#### Common API Endpoints
```bash
# Get portfolio status
curl -H "Authorization: Bearer YOUR_API_KEY" \
  http://localhost:8080/api/v1/portfolio

# Place an order
curl -X POST -H "Authorization: Bearer YOUR_API_KEY" \
  -H "Content-Type: application/json" \
  -d '{"symbol":"AAPL","side":"buy","quantity":100,"type":"market"}' \
  http://localhost:8080/api/v1/orders

# Get order status
curl -H "Authorization: Bearer YOUR_API_KEY" \
  http://localhost:8080/api/v1/orders/ORDER_ID
```

### 2. WebSocket API

#### Real-time Data Streaming
```javascript
// JavaScript example for WebSocket connection
const ws = new WebSocket('ws://localhost:8080/api/v1/stream');

ws.onopen = function(event) {
    // Subscribe to market data
    ws.send(JSON.stringify({
        type: 'subscribe',
        channel: 'market_data',
        symbols: ['AAPL', 'GOOGL']
    }));
};

ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    console.log('Received:', data);
};
```

## Troubleshooting

### 1. Common Issues

#### System Won't Start
```bash
# Check system requirements
./tradingsystem check-requirements

# Validate configuration
./tradingsystem config validate

# Check log files for errors
./tradingsystem logs tail --level ERROR
```

#### Market Data Issues
```bash
# Test market data connectivity
./tradingsystem data-source test-all

# Check data feed status
./tradingsystem status --market-data

# Restart data feeds
./tradingsystem data-source restart-all
```

#### Order Execution Problems
```bash
# Check broker connectivity
./tradingsystem broker test-connection

# Validate account status
./tradingsystem account status

# Review recent order errors
./tradingsystem orders list --status rejected --limit 10
```

### 2. Getting Help

#### Built-in Help System
```bash
# General help
./tradingsystem help

# Command-specific help
./tradingsystem help orders

# Show command examples
./tradingsystem help orders place --examples
```

#### Support Resources
- **Documentation**: `/usr/share/doc/tradingsystem/`
- **Log Files**: `/var/log/tradingsystem/`
- **Configuration**: `/etc/tradingsystem/`
- **Support Email**: support@tradingsystem.com
- **Community Forum**: https://forum.tradingsystem.com

## Best Practices

### 1. Security Best Practices

#### API Key Management
- Store API keys in secure environment variables
- Rotate API keys regularly (every 90 days)
- Use separate keys for different environments
- Never log or print API keys

#### Network Security
- Use VPN for remote access
- Enable firewall rules to restrict access
- Use SSL/TLS for all external communications
- Monitor for unusual network activity

### 2. Performance Optimization

#### System Tuning
```bash
# Set CPU affinity for better performance
taskset -c 0,1,2,3 ./tradingsystem

# Increase process priority
nice -n -20 ./tradingsystem

# Monitor system performance
htop
iotop
```

#### Configuration Optimization
```json
{
    "system": {
        "performance_mode": "ultra_low_latency",
        "thread_pool_size": 8,
        "memory_limit_mb": 8192
    },
    "market_data": {
        "buffer_size": 50000,
        "batch_processing": true
    }
}
```

### 3. Risk Management Best Practices

#### Position Sizing
- Never risk more than 2% of capital on a single trade
- Diversify across different sectors and asset classes
- Set stop-loss orders for all positions
- Regularly review and adjust position sizes

#### Monitoring
- Check portfolio risk metrics daily
- Set up alerts for risk limit breaches
- Review correlation analysis weekly
- Conduct stress testing monthly

## Appendices

### A. Command Reference

#### Order Commands
| Command | Description | Example |
|---------|-------------|---------|
| `order buy` | Place buy order | `order buy AAPL 100 --type market` |
| `order sell` | Place sell order | `order sell AAPL 50 --type limit --price 150` |
| `orders list` | List all orders | `orders list --status pending` |
| `orders cancel` | Cancel order | `orders cancel 12345` |

#### Portfolio Commands
| Command | Description | Example |
|---------|-------------|---------|
| `portfolio summary` | Portfolio overview | `portfolio summary` |
| `positions` | List positions | `positions --symbol AAPL` |
| `pnl` | Show P&L | `pnl --period week` |

### B. Configuration Schema

#### Complete Configuration Example
```json
{
    "system": {
        "log_level": "INFO",
        "performance_mode": "balanced",
        "max_threads": 8,
        "memory_limit_mb": 4096
    },
    "trading": {
        "paper_trading": false,
        "max_position_size": 10000,
        "max_order_value": 1000000,
        "default_currency": "USD",
        "risk_limits": {
            "daily_loss_limit": 10000,
            "max_exposure": 100000,
            "position_concentration": 0.05
        }
    },
    "market_data": {
        "primary_source": "bloomberg",
        "backup_sources": ["refinitiv", "yahoo"],
        "buffer_size": 10000,
        "timeout_ms": 5000,
        "symbols": ["AAPL", "GOOGL", "MSFT", "AMZN", "TSLA"]
    },
    "broker": {
        "name": "interactive_brokers",
        "endpoint": "localhost:7497",
        "client_id": 1,
        "timeout_ms": 3000
    }
}
```

### C. Error Codes Reference

| Code | Description | Action Required |
|------|-------------|----------------|
| E001 | Market data unavailable | Check data source connectivity |
| E002 | Order validation failed | Review order parameters |
| E003 | Insufficient funds | Check account balance |
| E004 | Risk limit exceeded | Reduce position size |
| E005 | System overloaded | Reduce trading frequency |