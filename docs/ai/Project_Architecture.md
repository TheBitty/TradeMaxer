# Project Architecture

## Overview
TradingSystem is a high-performance financial trading application built with C++ and Python. This document describes the technical architecture, component relationships, and design decisions following the principles outlined in CLAUDE.md.

## How TradingSystem Works

### Core Concept
TradingSystem operates as a multi-process financial trading platform that processes market data, makes trading decisions, and executes orders while maintaining strict risk management and performance requirements.

### Execution Flow
1. **Market Data Ingestion** → Real-time market data collection
2. **Signal Processing** → Analysis and pattern recognition
3. **Trading Logic** → Decision making based on signals and risk parameters
4. **Order Execution** → Trade execution through broker APIs
5. **Risk Management** → Continuous monitoring and position management
6. **Portfolio Management** → Position tracking and P&L calculation

## Project Structure
```
TradingSystem/
├── src/                          # Core application modules
│   ├── main.cpp                  # Entry point (minimal coordinator)
│   ├── core/                     # Core trading engine
│   │   ├── engine.hpp            # Main trading engine
│   │   ├── market_data.hpp       # Market data structures
│   │   ├── order_manager.hpp     # Order management
│   │   └── portfolio.hpp         # Portfolio management
│   ├── data/                     # Data handling
│   │   ├── feed_handler.hpp      # Market data feed processing
│   │   ├── database.hpp          # Data persistence
│   │   └── cache.hpp             # High-speed data caching
│   ├── risk/                     # Risk management
│   │   ├── risk_manager.hpp      # Risk assessment and controls
│   │   ├── position_manager.hpp  # Position tracking
│   │   └── limits.hpp            # Trading limits and constraints
│   ├── strategy/                 # Trading strategies
│   │   ├── strategy_base.hpp     # Base strategy interface
│   │   ├── signals.hpp           # Signal generation
│   │   └── indicators.hpp        # Technical indicators
│   ├── connectivity/             # External connections
│   │   ├── broker_api.hpp        # Broker API interfaces
│   │   ├── market_api.hpp        # Market data API interfaces
│   │   └── ipc.hpp               # Inter-process communication
│   └── utils/                    # Utilities
│       ├── config.hpp            # Configuration management
│       ├── logger.hpp            # Logging system
│       ├── math.hpp              # Mathematical utilities
│       └── time.hpp              # Time and date utilities
├── tests/                        # Test suite
│   ├── unit/                     # Unit tests
│   ├── integration/              # Integration tests
│   └── performance/              # Performance tests
├── docs/                         # Documentation
│   ├── ai/                       # AI context and documentation
│   ├── technical/                # Technical documentation
│   ├── planning/                 # Project planning documents
│   └── logging/                  # Development tracking
├── specs/                        # Feature specifications
├── config/                       # Configuration files
├── scripts/                      # Build and deployment scripts
└── build/                        # Build artifacts
```

## Architecture Principles

### 1. Multi-Process Design
Following CLAUDE.md principles, the system is designed for multi-process architecture:

```
┌─────────────────────────────────────────┐
│         Market Data Process            │
│    (Real-time data ingestion)          │
├─────────────────────────────────────────┤
│         Trading Engine Process         │
│    (Signal processing & decisions)     │
├─────────────────────────────────────────┤
│         Order Management Process       │
│    (Order execution & tracking)        │
├─────────────────────────────────────────┤
│         Risk Management Process        │
│    (Risk monitoring & controls)        │
└─────────────────────────────────────────┘
```

### 2. Component Architecture

#### Core Components

**Main Function (`main.cpp`)**
- Serves solely as entry point per CLAUDE.md requirements
- Delegates all logic to other modules
- Initializes processes and coordinates startup
- Handles graceful shutdown and cleanup

**Trading Engine (`src/core/engine.hpp`)**
- Central coordinator for trading operations
- Processes market signals and generates trading decisions
- Interfaces with all major subsystems
- Maintains trading state and session management

**Market Data Handler (`src/data/feed_handler.hpp`)**
- Real-time market data ingestion from multiple sources
- Data normalization and validation
- High-frequency data processing with minimal latency
- Market data distribution to trading components

**Order Manager (`src/core/order_manager.hpp`)**
- Order lifecycle management (creation, execution, cancellation)
- Integration with broker APIs for order submission
- Order status tracking and reporting
- Fill processing and trade confirmation

**Risk Manager (`src/risk/risk_manager.hpp`)**
- Real-time risk monitoring and control
- Position limits and exposure management
- Pre-trade and post-trade risk checks
- Emergency stop mechanisms

**Portfolio Manager (`src/core/portfolio.hpp`)**
- Position tracking and P&L calculation
- Portfolio analytics and reporting
- Cash management and margin calculations
- Performance attribution analysis

### 3. Inter-Process Communication

Following the multi-process design principle from CLAUDE.md:

```cpp
// IPC mechanisms for process communication
namespace IPC {
    class SharedMemoryQueue {
        // High-performance message passing
    };
    
    class NamedPipeChannel {
        // Reliable process communication
    };
    
    class SocketConnector {
        // Network-based communication
    };
}
```

### 4. Data Flow Architecture

```
Market Data Sources → Data Normalization → Signal Processing
                                               ↓
Risk Controls ← Trading Decisions ← Strategy Engine
      ↓                              ↓
  Risk Check → Order Generation → Broker APIs
      ↓                              ↓
Portfolio Update ← Trade Execution ← Order Fills
      ↓
  Reporting & Analytics
```

## Technical Architecture

### 1. Performance Requirements
- **Latency**: Sub-millisecond order processing
- **Throughput**: 100k+ market updates per second
- **Memory**: Efficient memory usage with lock-free structures
- **CPU**: Multi-core utilization with thread affinity

### 2. Data Structures
```cpp
// Core market data structure
struct MarketData {
    uint64_t timestamp;
    uint32_t symbol_id;
    double bid_price;
    double ask_price;
    uint64_t bid_size;
    uint64_t ask_size;
    double last_price;
    uint64_t volume;
};

// Order structure
struct Order {
    uint64_t order_id;
    uint32_t symbol_id;
    OrderSide side;
    OrderType type;
    double price;
    uint64_t quantity;
    uint64_t timestamp;
    OrderStatus status;
};
```

### 3. Threading Model
```cpp
class TradingEngine {
private:
    std::thread market_data_thread_;
    std::thread signal_processing_thread_;
    std::thread order_management_thread_;
    std::thread risk_monitoring_thread_;
    
    // Lock-free queues for inter-thread communication
    LockFreeQueue<MarketData> market_data_queue_;
    LockFreeQueue<Order> order_queue_;
};
```

### 4. Memory Management
- **Zero-copy**: Minimize data copying in critical paths
- **Pool allocation**: Pre-allocated memory pools for objects
- **NUMA awareness**: Memory locality optimization
- **Cache optimization**: Data structure alignment for cache efficiency

## Module Responsibilities

### `main.cpp`
- Initialize system components
- Setup process coordination
- Handle system-wide configuration
- Coordinate graceful shutdown
- Monitor process health

### `src/core/`
- Core trading logic and decision making
- Order lifecycle management
- Portfolio state management
- Trading session coordination

### `src/data/`
- Market data ingestion and processing
- Data validation and normalization
- Historical data management
- Data distribution to consumers

### `src/risk/`
- Real-time risk monitoring
- Position and exposure tracking
- Limit enforcement
- Risk reporting and alerts

### `src/strategy/`
- Trading signal generation
- Technical indicator calculations
- Strategy backtesting framework
- Strategy parameter optimization

### `src/connectivity/`
- External API integrations
- Message serialization/deserialization
- Connection management and failover
- Protocol implementations

## Security Architecture

### 1. Data Protection
- Encrypted configuration files
- Secure API key management
- Data integrity validation
- Audit trail maintenance

### 2. Access Control
- Role-based permissions
- API rate limiting
- Session management
- Authentication mechanisms

### 3. Operational Security
- Process isolation
- Resource limits
- Error containment
- Security monitoring

## Performance Considerations

### 1. Latency Optimization
- Kernel bypass networking
- CPU affinity for critical threads
- Lock-free programming patterns
- Memory pre-allocation

### 2. Scalability Design
- Horizontal scaling capabilities
- Load balancing mechanisms
- Distributed processing support
- Cloud deployment ready

## Architecture Decision Records (ADRs)

### ADR-001: Multi-Process Architecture
**Status**: Accepted
**Context**: Need for fault isolation and scalability
**Decision**: Implement multi-process design as specified in CLAUDE.md
**Consequences**: Better fault isolation, more complex IPC requirements

### ADR-002: C++ as Primary Language
**Status**: Accepted
**Context**: Performance requirements for trading systems
**Decision**: Use C++17/20 for core components
**Consequences**: Maximum performance, steeper learning curve

### ADR-003: Modular Component Design
**Status**: Accepted
**Context**: Need for maintainable and testable code
**Decision**: Separate components into distinct modules
**Consequences**: Better testability, clear interfaces, potential overhead

### ADR-004: Lock-Free Programming
**Status**: Accepted
**Context**: Low-latency requirements
**Decision**: Use lock-free data structures in critical paths
**Consequences**: Better performance, increased complexity