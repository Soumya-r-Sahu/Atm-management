# Core Banking System (CBS) Project Documentation

## Table of Contents

1. [Project Overview](#project-overview)
2. [Architecture](#architecture)
3. [Core Components](#core-components)
4. [API Reference](#api-reference)
5. [Developer Guide](#developer-guide)
6. [Testing](#testing)
7. [Security](#security)
8. [Performance](#performance)
9. [Build Instructions](#build-instructions)

## Project Overview

The Core Banking System (CBS) project is a comprehensive banking solution featuring:

- **ATM Management**: Card validation, balance operations, and transaction processing
- **Admin Interface**: User management, system configuration, and monitoring
- **Database Layer**: Abstract DAO pattern supporting multiple database backends
- **Security Features**: Password hashing, encryption, and audit logging
- **Logging System**: Compliant transaction and security logging

## Architecture

The CBS project follows a layered architecture:

1. **Presentation Layer**: CLI interfaces, Web interfaces, Mobile apps
2. **Business Logic Layer**: Core banking operations
3. **Data Access Layer**: DAO interfaces and implementations
4. **Infrastructure Layer**: Logging, configuration, and security

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Admin UI   │     │   ATM UI    │     │  Other UIs  │
└─────┬───────┘     └─────┬───────┘     └─────┬───────┘
      │                   │                   │
┌─────▼───────────────────▼───────────────────▼───────┐
│                Business Logic Layer                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │Transaction  │  │ Card Mgmt   │  │ Admin Ops   │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
└─────────────────────────┬─────────────────────────┬─┘
                          │                         │
┌─────────────────────────▼─────────────────────────▼─┐
│                   Data Access Layer                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │MySQL DAO    │  │File-based   │  │Mock DAO     │  │
│  │             │  │DAO          │  │(Testing)    │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
└─────────────────────────┬───────────────────────────┘
                          │
┌─────────────────────────▼───────────────────────────┐
│               Infrastructure Layer                  │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │Logger       │  │Config       │  │Security     │  │
│  │             │  │             │  │             │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
└───────────────────────────────────────────────────┬─┘
                                                    │
┌──────────────────────────────────────────────────▼┐
│                      Database                     │
└───────────────────────────────────────────────────┘
```

For a more detailed interactive architecture diagram, see the [System Architecture Diagram](./CBS_SYSTEM_ARCHITECTURE.md).

## Core Components

### Database Access Objects (DAO)

The DAO interface provides abstract database operations:

```c
typedef struct {
    // Card operations
    bool (*doesCardExist)(int cardNumber);
    bool (*validateCard)(int cardNumber, int pin);
    
    // Account operations
    float (*fetchBalance)(int cardNumber);
    bool (*updateBalance)(int cardNumber, float newBalance);
    
    // Transaction operations
    bool (*logTransaction)(int cardNumber, const char* transactionType, 
                          float amount, bool success);
    // Additional operations...
} DatabaseAccessObject;
```

### Logging System

The CBS Logger provides compliant logging for financial applications:

```c
// Initialize logger
bool cbs_initializeLogger(const char* logDirectory);

// Log operations
void cbs_writeLog(LogCategory category, CBSLogLevel level, const char* format, ...);
void cbs_writeTransactionLog(const char* transactionId, const char* userId, 
                           const char* cardNumber, const char* accountId,
                           const char* transactionType, double amount,
                           double previousBalance, double newBalance,
                           const char* status, const char* details);

// Security logging
void cbs_writeSecurityLog(const char* userId, const char* eventType, 
                        const char* severity, const char* status,
                        const char* targetResource, const char* details,
                        const char* sourceIP);
```

### Security Components

```c
// Password security
bool secure_password_init(void);
char* secure_password_hash(const char* password, HashAlgorithm algorithm, int work_factor);
bool secure_password_verify(const char* password, const char* hash);
bool secure_password_needs_rehash(const char* hash);
char* secure_generate_token(size_t length);
```

### Database Connection Pool

```c
// Create a connection pool
ConnectionPool* connection_pool_create(const ConnectionPoolConfig* config);

// Get and return connections
DatabaseConnection* connection_pool_get(ConnectionPool* pool);
bool connection_pool_return(DatabaseConnection* conn);

// Pool maintenance
void connection_pool_destroy(ConnectionPool* pool);
void connection_pool_stats(ConnectionPool* pool, int* active_connections, 
                         int* idle_connections, int* total_connections,
                         int* connection_requests, double* avg_wait_time_ms);
```

## API Reference

### Card Management API

```c
bool validateCard(int cardNumber, int pin);
bool blockCard(int cardNumber);
bool unblockCard(int cardNumber);
bool updateCardPIN(int cardNumber, int newPIN);
```

### Transaction API

```c
bool performWithdrawal(int cardNumber, float amount);
bool performDeposit(int cardNumber, float amount);
bool performTransfer(int sourceCard, int targetCard, float amount);
bool getMiniStatement(int cardNumber, Transaction* transactions, int* count);
```

### Admin API

```c
bool authenticate_admin(const char* username, const char* password, AdminUser* admin);
bool update_admin_status(AdminUser* admin, const char* target_username, bool status);
bool create_admin_account(AdminUser* admin, const char* new_username, 
                         const char* new_password, const char* roles[], int num_roles);
```

## Developer Guide

### Setting Up Development Environment

1. **Prerequisites**:
   - C Compiler (GCC 7.0+ or equivalent)
   - MySQL development libraries
   - OpenSSL development libraries

2. **Building the Project**:
   ```bash
   # On Linux/macOS
   make all
   
   # On Windows
   ./build.bat
   ```

### Adding a New Feature

1. **Implement business logic** in the appropriate module
2. **Add DAO functions** if database operations are required
3. **Add appropriate logging** using the CBS logger
4. **Write tests** for the new functionality
5. **Update documentation** to reflect the changes

### Coding Standards

- Use camelCase for function names and variables
- Use PascalCase for struct and typedef names
- Include complete documentation for public functions
- Always validate input parameters
- Always log errors and important operations

## Testing

### Test Framework

The project includes a comprehensive test framework:

```c
// Initialize test framework
test_init("Test Suite Name");

// Setup and teardown
test_setup_teardown(setup_function, teardown_function);

// Run tests
test_run("Test Name", test_function);

// Use assertions
test_assert(condition, "Message");
test_assert_int_equal(expected, actual, "Message");
test_assert_str_equal(expected, actual, "Message");
test_assert_float_equal(expected, actual, epsilon, "Message");
```

### Running Tests

```bash
# Run all tests
make test

# Run specific test
bin/test_name
```

## Security

### Authentication

- **Admin Authentication**: Username/password with salted bcrypt hashing
- **Card Authentication**: Card number + PIN validation

### Password Security

- **Bcrypt** as primary hashing algorithm, with fallback to PBKDF2
- **Automatic hash upgrades** when users authenticate with older hash formats
- **Configurable work factors** for future-proofing against hardware improvements

### PII Protection

- Card numbers and personal information are automatically masked in logs
- Sensitive data is encrypted at rest and in transit

## Performance

### Connection Pooling

The database connection pool optimizes performance through:
- Pre-initialized connections
- Connection reuse
- Automatic scaling
- Connection validation

### Optimization Strategies

1. **Prepared Statements**: Used for frequently executed queries
2. **Index Optimization**: Strategic database indexes
3. **Caching**: Frequently accessed data is cached
4. **Batch Processing**: Bulk operations where applicable

## Build Instructions

### Standard Build

```bash
# Build everything
make all

# Build specific component
make atm_system
make admin_system
```

### Windows Build

```powershell
# Build everything
.\build.bat

# Build specific component
.\build.bat atm_system
```

### Running the Application

```bash
# Run ATM system
bin/atm_system

# Run Admin interface
bin/admin_system
```

---

## Navigation

- [Documentation Index](./README.md)
- [Function Reference](./CBS_FUNCTION_REFERENCE.md)
- [Logging Guide](./CBS_LOGGING_GUIDE.md)
- [Build Guide](./CBS_BUILD_GUIDE.md)
- [Testing Plan](./CBS_TESTING_PLAN.md)
