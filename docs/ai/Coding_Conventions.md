# Coding Conventions

## Overview
This document establishes coding standards and conventions for the TradingSystem project to ensure consistency, maintainability, and high code quality across all components.

## General Principles

### 1. Code Quality Standards
- **Readability First**: Code should be self-documenting and easy to understand
- **Performance Critical**: Optimize for low latency and high throughput
- **Reliability**: Write robust code with proper error handling
- **Maintainability**: Design for long-term maintenance and evolution

### 2. Architecture Adherence
- Follow the multi-process architecture principles from CLAUDE.md
- Maintain clear separation of concerns between modules
- Use well-defined interfaces for inter-component communication
- Design for scalability and fault tolerance

## C++ Coding Standards

### 1. Naming Conventions

#### Classes and Structures
```cpp
// PascalCase for classes and structures
class TradingEngine {
    // Implementation
};

struct MarketData {
    // Members
};
```

#### Functions and Methods
```cpp
// snake_case for functions and methods
void process_market_data();
double calculate_portfolio_value();
bool validate_order_parameters();
```

#### Variables
```cpp
// snake_case for variables
int order_count = 0;
double bid_price = 0.0;
std::string symbol_name;

// Prefix member variables with underscore
class OrderManager {
private:
    int order_count_;
    double total_value_;
    std::unique_ptr<Database> database_;
};
```

#### Constants
```cpp
// SCREAMING_SNAKE_CASE for constants
const int MAX_ORDER_SIZE = 10000;
const double MIN_PRICE_INCREMENT = 0.01;
constexpr uint64_t NANOSECONDS_PER_SECOND = 1000000000ULL;
```

#### Enums
```cpp
// PascalCase for enum class names, SCREAMING_SNAKE_CASE for values
enum class OrderSide {
    BUY,
    SELL
};

enum class OrderStatus {
    PENDING,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED,
    REJECTED
};
```

### 2. File Organization

#### Header Files (.hpp)
```cpp
#pragma once

#include <system_headers>
#include "project_headers.hpp"

namespace trading {

class ComponentName {
public:
    // Public interface
    
private:
    // Private implementation
};

} // namespace trading
```

#### Source Files (.cpp)
```cpp
#include "header_file.hpp"

#include <system_headers>
#include "other_project_headers.hpp"

namespace trading {

// Implementation

} // namespace trading
```

### 3. Code Formatting

#### Indentation and Spacing
- Use 4 spaces for indentation (no tabs)
- Maximum line length: 100 characters
- Use blank lines to separate logical sections

#### Braces and Brackets
```cpp
// Opening brace on same line for functions and control structures
if (condition) {
    // code
} else {
    // code
}

class ClassName {
public:
    void method_name() {
        // implementation
    }
};
```

#### Function Declarations
```cpp
// Parameter alignment for long parameter lists
void long_function_name(const std::string& symbol,
                       double price,
                       uint64_t quantity,
                       OrderType type);
```

### 4. Memory Management

#### Smart Pointers
```cpp
// Prefer smart pointers over raw pointers
std::unique_ptr<OrderManager> order_manager_;
std::shared_ptr<MarketDataFeed> market_feed_;

// Use raw pointers only for non-owning references
void process_order(const Order* order);
```

#### RAII Pattern
```cpp
// Use RAII for resource management
class DatabaseConnection {
public:
    DatabaseConnection(const std::string& connection_string);
    ~DatabaseConnection(); // Automatic cleanup
    
    // Non-copyable, movable
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    DatabaseConnection(DatabaseConnection&&) = default;
    DatabaseConnection& operator=(DatabaseConnection&&) = default;
};
```

### 5. Error Handling

#### Exception Handling
```cpp
// Use exceptions for exceptional conditions
class TradingException : public std::exception {
public:
    explicit TradingException(const std::string& message) 
        : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
private:
    std::string message_;
};

// Specific exception types
class OrderValidationException : public TradingException {
    // Implementation
};
```

#### Error Codes for Performance-Critical Paths
```cpp
// Use error codes in hot paths to avoid exception overhead
enum class ErrorCode {
    SUCCESS = 0,
    INVALID_SYMBOL,
    INSUFFICIENT_BALANCE,
    MARKET_CLOSED,
    SYSTEM_ERROR
};

ErrorCode validate_order(const Order& order);
```

### 6. Performance Guidelines

#### Lock-Free Programming
```cpp
// Use atomic operations for lock-free data structures
class LockFreeQueue {
private:
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
};

// Memory ordering for performance
void update_price(double new_price) {
    price_.store(new_price, std::memory_order_release);
}
```

#### Minimize Allocations
```cpp
// Pre-allocate containers when size is known
std::vector<Order> orders;
orders.reserve(expected_order_count);

// Use object pools for frequently allocated objects
ObjectPool<Order> order_pool_;
auto order = order_pool_.acquire();
```

## Python Coding Standards (if applicable)

### 1. PEP 8 Compliance
- Follow PEP 8 style guidelines
- Use snake_case for functions and variables
- Use PascalCase for classes
- Maximum line length: 88 characters (Black formatter)

### 2. Type Hints
```python
from typing import List, Dict, Optional, Union

def calculate_returns(prices: List[float], 
                     periods: int = 1) -> List[float]:
    """Calculate price returns over specified periods."""
    return [/* implementation */]
```

## Documentation Standards

### 1. Code Comments
```cpp
// Single-line comments for brief explanations
int order_count = 0; // Track number of active orders

/**
 * Multi-line comments for detailed explanations
 * of complex algorithms or business logic
 */
class ComplexAlgorithm {
    /**
     * Process market signals and generate trading decisions
     * @param market_data Current market state
     * @param portfolio Current portfolio positions
     * @return Trading signals for execution
     */
    std::vector<Signal> process_signals(const MarketData& market_data,
                                       const Portfolio& portfolio);
};
```

### 2. Function Documentation
```cpp
/**
 * Calculate portfolio value based on current positions and market prices
 * 
 * @param positions Map of symbol to position size
 * @param prices Current market prices for symbols
 * @param base_currency Currency for value calculation
 * @return Total portfolio value in base currency
 * 
 * @throws std::invalid_argument If positions contain unknown symbols
 * @throws MarketDataException If price data is stale or invalid
 */
double calculate_portfolio_value(
    const std::unordered_map<std::string, int64_t>& positions,
    const std::unordered_map<std::string, double>& prices,
    const std::string& base_currency = "USD");
```

## Testing Standards

### 1. Unit Test Structure
```cpp
// Use descriptive test names
TEST(OrderManagerTest, ShouldRejectOrderWithInvalidSymbol) {
    // Arrange
    OrderManager manager;
    Order invalid_order{};
    invalid_order.symbol = "INVALID";
    
    // Act & Assert
    EXPECT_THROW(manager.submit_order(invalid_order), 
                 OrderValidationException);
}
```

### 2. Test Coverage Requirements
- Minimum 80% code coverage for new code
- 100% coverage for critical trading logic
- Performance tests for latency-sensitive components
- Integration tests for external API interactions

## Performance Standards

### 1. Latency Requirements
- Order processing: < 100 microseconds
- Market data processing: < 10 microseconds
- Risk checks: < 50 microseconds

### 2. Memory Usage
- Minimize dynamic allocations in hot paths
- Use memory pools for frequently allocated objects
- Monitor memory fragmentation
- Set maximum memory usage limits

### 3. Profiling and Optimization
- Profile code regularly to identify bottlenecks
- Use CPU performance counters for detailed analysis
- Optimize for cache locality
- Consider SIMD instructions for mathematical operations

## Security Guidelines

### 1. Secure Coding Practices
- Validate all inputs from external sources
- Use secure random number generation
- Implement proper authentication and authorization
- Log security-relevant events

### 2. Configuration Security
```cpp
// Never hardcode sensitive information
class Configuration {
private:
    std::string api_key_;  // Load from secure storage
    std::string database_password_;  // Never log or print
    
public:
    void load_from_environment();
    void load_from_encrypted_file(const std::string& filename);
};
```

## Build and Deployment Standards

### 1. CMake Configuration
- Use modern CMake (3.16+)
- Separate debug and release configurations
- Enable appropriate compiler warnings
- Support multiple compilers (GCC, Clang)

### 2. Continuous Integration
- All code must pass automated tests
- Static analysis tools must pass
- Code coverage requirements must be met
- Performance regression tests must pass

## Version Control Guidelines

### 1. Commit Messages
```
feat: add order validation for maximum position size

- Implement position size limits per symbol
- Add configuration for maximum exposure
- Include unit tests for validation logic
- Update documentation for new limits

Fixes #123
```

### 2. Branch Strategy
- Use feature branches for new development
- Require code review for all changes
- Maintain stable main branch
- Tag releases with semantic versioning

## Code Review Checklist

### 1. Functionality
- [ ] Code implements requirements correctly
- [ ] Edge cases are handled appropriately
- [ ] Error conditions are managed properly
- [ ] Performance requirements are met

### 2. Code Quality
- [ ] Follows established coding conventions
- [ ] Is well-documented and readable
- [ ] Has appropriate test coverage
- [ ] Doesn't introduce security vulnerabilities

### 3. Architecture
- [ ] Follows multi-process architecture principles
- [ ] Maintains proper separation of concerns
- [ ] Uses appropriate design patterns
- [ ] Integrates well with existing components