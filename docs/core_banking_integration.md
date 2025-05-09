# Core Banking System Integration

## Overview
This integration connects the ATM application with the Core Banking System (CBS) using a two-layer approach:

1. **Core Banking Interface** - Direct interface to the CBS database, implementing optimized banking functions
2. **Core Banking Adapter** - Adaptation layer that maps legacy ATM functions to the new CBS functions

## Architecture

```
┌───────────────────┐     ┌───────────────────┐     ┌───────────────────┐
│  ATM Application  │────▶│  Banking Adapter  │────▶│  Banking Interface│────▶ Database
└───────────────────┘     └───────────────────┘     └───────────────────┘
```

## Key Features

### Core Banking Functions
- Transaction processing with atomicity
- Balance inquiries
- Fund transfers between accounts
- Mini statements (transaction history)
- Withdrawal limit checks
- Fixed deposit creation
- Loan EMI processing
- Card limits management
- Beneficiary management

### Performance Optimizations
- Database transactions ensuring consistency
- Indexed queries
- Proper error handling and logging
- Optimized SQL statements with JOIN operations
- Helper functions for common operations
- Constant-time operations where possible

## Integration with ATM System
The integration is backward compatible with the existing ATM application. The `core_banking_adapter.c` file provides adapter functions that map legacy ATM function calls to the new core banking system functions.

## Testing

A test program (`test_core_banking.c`) is provided to verify the integration. It tests:
1. Balance inquiries
2. Deposits
3. Withdrawals
4. Fund transfers
5. Mini statements

### Running the Tests

```bash
cd tests
make
./test_core_banking <card_number>
```

## Future Enhancements
- Scheduled payment processing
- Interest calculation
- Account statement generation
- Loan application processing
- Cheque processing
- Additional security measures
- Configuration-based function behaviors

## Notes for Maintainers
- The core banking interface uses UUID format for transaction IDs
- All transactions are logged both in the new CBS and in the legacy system for backward compatibility
- The adapter layer handles mapping between card numbers and account numbers
