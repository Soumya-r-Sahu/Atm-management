# Core Banking System (CBS) Compliant Logging Architecture

## Overview

The CBS Logging Architecture is designed to provide a comprehensive, secure, and compliant logging system for the ATM Management Project. It follows banking industry best practices and meets regulatory requirements for transaction auditing, data privacy, and security monitoring.

## Key Features

1. **Enhanced Structured Logging**
   - Multi-level logging with standard severity levels
   - Category-based log organization
   - Consistent timestamp format with millisecond precision
   - Operation IDs for cross-log traceability
   - Sequence numbers for log integrity

2. **Transaction Audit Trails**
   - Before/after state recording for all transactions
   - Complete account balance history
   - Cross-referenced transaction identifiers
   - Event correlation support

3. **Security Compliance**
   - PII (Personal Identifiable Information) data masking
   - PCI DSS compliant card number handling
   - Tamper-evident log design
   - Comprehensive security event recording

4. **Log Management**
   - Automatic log rotation based on size
   - Configurable retention policies
   - Different retention periods for different log types
   - Regulatory compliance with 7-year retention for audit logs

5. **DAO Integration Layer**
   - Seamless integration with existing DAO operations
   - Backward compatible with existing logging systems
   - Enhanced functionality without breaking changes

## Architecture Diagram

```
┌─────────────────┐     ┌───────────────────┐     ┌────────────────────┐
│ ATM Application │────▶│ Transaction Layer │────▶│ DAO Interface      │
└─────────────────┘     └───────────────────┘     └────────────────────┘
         │                        │                         │
         │                        │                         │
         ▼                        ▼                         ▼
┌─────────────────┐     ┌───────────────────┐     ┌────────────────────┐
│ Legacy Logging  │     │ CBS Logger        │◀────│ DAO Audit Logger   │
└─────────────────┘     └───────────────────┘     └────────────────────┘
         │                        │                         │
         │                        │                         │
         ▼                        ▼                         ▼
┌─────────────────┐     ┌───────────────────┐     ┌────────────────────┐
│ Legacy Log      │     │ Application Log   │     │ Transaction Log    │
│ Files           │     │ Security Log      │     │ Audit Log          │
└─────────────────┘     │ Error Log         │     │ Database Log       │
                        └───────────────────┘     └────────────────────┘
```

## Log Categories

| Category    | Purpose                                                | Retention |
|-------------|--------------------------------------------------------|-----------|
| Application | General application events, startup, shutdown          | 7 days    |
| Transaction | Financial transactions (deposits, withdrawals)         | 7 years   |
| Security    | Security events, login attempts, authorization         | 1 year    |
| Database    | Database operations, schema changes                    | 30 days   |
| Audit       | Compliance audit trail                                 | 7 years   |
| System      | System-level events                                    | 30 days   |

## Log Levels

| Level     | Description                                    | Example Use Case                           |
|-----------|------------------------------------------------|-------------------------------------------|
| DEBUG     | Detailed debug information                     | Tracing program execution for development  |
| INFO      | Informational messages                         | Normal operation events                    |
| WARNING   | Warning conditions                             | Issues that don't cause errors             |
| ERROR     | Error conditions                               | Application errors, recoverable            |
| CRITICAL  | Critical conditions                            | System errors, not easily recoverable      |
| SECURITY  | Security-related logs                          | Authentication, access control events      |
| ALERT     | Action must be taken immediately               | Critical security alerts                   |
| EMERGENCY | System is unusable                             | Complete system failure                    |

## Data Sensitivity Levels

| Level     | Description                      | Handling                         |
|-----------|----------------------------------|----------------------------------|
| PUBLIC    | Public information               | No special handling              |
| INTERNAL  | Internal information             | Minimal masking                  |
| PRIVATE   | Private user information         | Requires PII masking             |
| CRITICAL  | Critical security data           | Extensive masking or encryption  |

## Key Components

### CBS Logger

The core logging component that provides:
- Multi-level logging interface
- Automatic log file rotation
- Log retention policy management
- PII data masking
- Structured log formats

### DAO Audit Logger

Integration layer between DAO operations and the CBS logger:
- Records transaction operations with before/after state
- Maintains audit trails for regulatory compliance
- Handles card operations logging
- Records database operations
- Provides authentication event logging

### Log Structures

Several specialized log structures:
- `TransactionLogEntry` - For financial transactions
- `AuditLogEntry` - For audit trail records
- `SecurityLogEntry` - For security events

## Integration with Existing Code

The CBS logging system is designed for minimal disruption to existing code:
- Wrapper functions for legacy logging calls
- Enhanced transaction logging with both systems in parallel
- Direct CBS logger calls for new components

## Developer Guidelines

1. **Transaction Logging**
   - Always log before and after states for financial transactions
   - Use `record_transaction_audit()` for complete transaction logs
   - Include user ID, card number, and transaction amount

2. **Security Events**
   - Use `cbs_writeSecurityLog()` for all security-related events
   - Always specify severity level appropriately
   - Include source information (IP address) when available

3. **PII Handling**
   - Never log complete card numbers - use masking
   - Avoid logging sensitive user information
   - Use provided PII masking functions

4. **Error Handling**
   - Always log errors with appropriate context
   - Include recovery steps in error logs when possible
   - Use correct log levels based on severity

5. **Log Categories**
   - Use appropriate log categories for organizational purposes
   - Consider retention requirements when choosing categories
   - Add operation IDs for related log entry correlation

## Testing

A comprehensive test suite is provided in `test_cbs_logger.c` that demonstrates:
- Basic logging functionality
- Transaction logging with state changes
- Security event logging
- Card operation logging
- Database operation logging
- Log rotation and retention testing

Run the test program using:
```bash
cd tests
make -f Makefile.cbs_logger run
```

## Conclusion

The CBS Logger implementation provides a robust, industry-standard logging system that:
- Meets regulatory requirements for banking applications
- Provides comprehensive audit trails
- Protects sensitive data
- Integrates smoothly with existing code
- Supports troubleshooting and debugging
- Maintains evidence for dispute resolution
