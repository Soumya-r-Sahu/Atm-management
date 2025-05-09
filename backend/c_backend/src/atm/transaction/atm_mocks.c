// Comprehensive mock implementations for ATM project
#include "../../../../../include/common/database/core_banking_interface.h"
#include "../../../../../include/common/database/account_management.h"
#include "../../../../../include/common/transaction/bill_payment.h"
#include "../../../../../include/common/utils/logger.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>

//===================================================
// Core Banking Interface mock functions
//===================================================

// Card verification
bool cbs_card_exists(int card_number) {
    printf("MOCK: Checking if card %d exists\n", card_number);
    return true;  // Always return true for testing
}

// Card activation check
bool cbs_is_card_active(int card_number) {
    printf("MOCK: Checking if card %d is active\n", card_number);
    return true;  // Always active for testing
}

// Transaction processing
bool cbs_process_transaction(const char* account_number, 
                            TRANSACTION_TYPE type, 
                            const char* channel, 
                            double amount, 
                            char* transaction_id_out) {
    printf("MOCK: Processing transaction type %d for account %s, amount: %.2f via %s\n", 
           type, account_number, amount, channel);
    
    // Generate a simple transaction ID based on timestamp
    sprintf(transaction_id_out, "MOCK-TRANS-%ld", (long)time(NULL));
    return true;  // Always succeed for testing
}

// Get account balance
bool cbs_get_account_balance(const char* account_number, double* balance_out) {
    printf("MOCK: Getting balance for account %s\n", account_number);
    *balance_out = 5000.00;  // Return a mock balance
    return true;
}

// Transfer funds function
bool cbs_transfer_funds(const char* source_account, 
                       const char* destination_account, 
                       double amount, 
                       const char* transfer_type,
                       char* transaction_id_out) {
    printf("MOCK: Transferring %.2f from %s to %s via %s\n", 
           amount, source_account, destination_account, transfer_type);
    
    sprintf(transaction_id_out, "MOCK-TRANS-%ld", (long)time(NULL));
    return true;
}

// Mini statement function
bool cbs_get_mini_statement(const char* account_number, 
                           TransactionRecord* records, 
                           int* count, 
                           int max_records) {
    printf("MOCK: Getting mini statement for account %s (max %d records)\n", 
           account_number, max_records);
    
    // Generate a few mock transactions
    *count = max_records > 3 ? 3 : max_records;
    
    for(int i = 0; i < *count; i++) {
        sprintf(records[i].transaction_id, "MOCK-TRANS-%d", i+1);
        sprintf(records[i].transaction_type, i % 2 == 0 ? "DEPOSIT" : "WITHDRAWAL");
        records[i].amount = (i+1) * 100.0;
        records[i].balance = 5000.0 - (i * 100.0);
        strcpy(records[i].date, "2025-05-09 10:00:00");
        strcpy(records[i].status, "SUCCESS");
    }
    
    return true;
}

// Get transaction history
bool cbs_get_transaction_history(const char* account_number, 
                                TransactionRecord* records, 
                                int* count, 
                                int max_records) {
    // Just reuse the mini statement function for simplicity
    return cbs_get_mini_statement(account_number, records, count, max_records);
}

// Daily withdrawal check
bool cbs_check_withdrawal_limit(const char* card_number, 
                               double amount, 
                               const char* channel, 
                               double* remaining_limit) {
    printf("MOCK: Checking withdrawal limit for card %s, amount: %.2f via %s\n", 
           card_number, amount, channel);
    
    *remaining_limit = 10000.0 - amount;  // Mock a daily limit of 10,000
    return amount <= 10000.0;  // Allow if within limit
}

// Daily withdrawal update (for our internal interface)
bool cbs_update_daily_withdrawal(const char* card_number, double amount) {
    printf("MOCK: Updating daily withdrawal for card %s, amount: %.2f\n", 
           card_number, amount);
    return true;  // Always succeed for testing
}

// Get card details
bool cbs_get_card_details(int card_number, 
                        char* holder_name_out, size_t holder_name_size,
                        char* account_number_out, size_t account_number_size,
                        char* expiry_date_out, size_t expiry_date_size,
                        int* is_active_out,
                        double* daily_limit_out,
                        char* card_type_out, size_t card_type_size) {
    printf("MOCK: Getting card details for %d\n", card_number);
    
    snprintf(holder_name_out, holder_name_size, "John Doe");
    snprintf(account_number_out, account_number_size, "ACC-%d", card_number);
    snprintf(expiry_date_out, expiry_date_size, "12/28");
    *is_active_out = 1;  // using int instead of bool
    *daily_limit_out = 10000.00;
    snprintf(card_type_out, card_type_size, "VISA GOLD");
    
    return true;
}

//===================================================
// Account Management mock functions
//===================================================

// Get account number from card
bool cbs_get_account_by_card(const char* card_number, 
                           char* account_number_out, 
                           size_t account_number_size) {
    printf("MOCK: Getting account for card %s\n", card_number);
    snprintf(account_number_out, account_number_size, "ACC-%s", card_number);
    return true;
}

// Get balance by card
bool cbs_get_balance_by_card(const char* card_number, double* balance_out) {
    printf("MOCK: Getting balance for card %s\n", card_number);
    *balance_out = 5000.00;
    return true;
}

// Update account balance
bool cbs_update_balance(const char* account_number, double new_balance, const char* transaction_type) {
    printf("MOCK: Updating balance for account %s to %.2f (type: %s)\n", 
           account_number, new_balance, transaction_type);
    return true;  // Always succeed for testing
}

// Get account holder name
bool cbs_get_account_holder_name(const char* account_number, char* name, size_t nameSize) {
    printf("MOCK: Getting account holder name for account %s\n", account_number);
    snprintf(name, nameSize, "John Doe");
    return true;
}

//===================================================
// Bill Payment mock functions
//===================================================

// Process bill payment
bool cbs_process_bill_payment(const char* card_number, 
                              const char* bill_type,
                              const char* bill_reference,
                              double amount, 
                              char* transaction_id_out) {
    printf("MOCK: Processing bill payment for card %s, type: %s, ref: %s, amount: %.2f\n", 
           card_number, bill_type, bill_reference, amount);
    
    // Generate a simple transaction ID
    sprintf(transaction_id_out, "BILL-%s-%ld", bill_type, (long)time(NULL));
    return true;  // Always succeed for testing
}

// Get bill payment history
bool cbs_get_bill_payment_history(const char* card_number,
                                TransactionRecord* records,
                                int* count,
                                int max_records) {
    printf("MOCK: Getting bill payment history for card %s (max %d records)\n", 
           card_number, max_records);
    
    // Generate a few mock bill payment records
    *count = max_records > 2 ? 2 : max_records;
    
    if (*count > 0) {
        strcpy(records[0].transaction_id, "BILL-ELEC-20250501");
        strcpy(records[0].transaction_type, "BILL_PAYMENT");
        records[0].amount = 120.50;
        records[0].balance = 5000.0 - 120.50;
        strcpy(records[0].date, "2025-05-01 15:30:00");
        strcpy(records[0].status, "SUCCESS");
    }
    
    if (*count > 1) {
        strcpy(records[1].transaction_id, "BILL-WATER-20250503");
        strcpy(records[1].transaction_type, "BILL_PAYMENT");
        records[1].amount = 45.75;
        records[1].balance = 5000.0 - 120.50 - 45.75;
        strcpy(records[1].date, "2025-05-03 10:15:00");
        strcpy(records[1].status, "SUCCESS");
    }
    
    return true;
}

// Verify bill details
bool cbs_verify_bill(const char* bill_type, 
                    const char* bill_reference, 
                    double* amount_out, 
                    char* due_date_out, 
                    char* payee_name_out,
                    size_t string_buffer_size) {
    printf("MOCK: Verifying bill type: %s, reference: %s\n", bill_type, bill_reference);
    
    // Return mock data
    *amount_out = 125.75;
    strncpy(due_date_out, "2025-05-15", string_buffer_size - 1);
    due_date_out[string_buffer_size - 1] = '\0';
    
    strncpy(payee_name_out, "City Utility Company", string_buffer_size - 1);
    payee_name_out[string_buffer_size - 1] = '\0';
    
    return true;
}

//===================================================
// Logger mock functions
//===================================================

// Initialize logger
bool initializeLogger(const char *log_dir) {
    printf("MOCK: Initializing logger with directory: %s\n", log_dir);
    return true;
}

// Clean up logger
void closeLogger(void) {
    printf("MOCK: Closing logger\n");
}

// Set global log level
void setLogLevel(LogLevel level) {
    printf("MOCK: Setting log level to %d\n", level);
}

// Get current log level
LogLevel getLogLevel(void) {
    return LOG_LEVEL_INFO;
}

// Write debug log
void writeDebugLog(const char *format, ...) {
    printf("MOCK DEBUG: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// Write info log
void writeInfoLog(const char *format, ...) {
    printf("MOCK INFO: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// Write warning log
void writeWarningLog(const char *format, ...) {
    printf("MOCK WARN: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// Write error log
void writeErrorLog(const char *format, ...) {
    printf("MOCK ERROR: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// Write critical log
void writeCriticalLog(const char *format, ...) {
    printf("MOCK CRITICAL: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// Write security log
void writeSecurityLog(const char *format, ...) {
    printf("MOCK SECURITY: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// Write transaction log
void writeTransactionLog(const char *transaction_type, 
                        const char *card_number, 
                        double amount,
                        const char *status,
                        const char *details) {
    printf("MOCK TRANSACTION: %s for card %s - $%.2f (%s) - %s\n", 
           transaction_type, card_number, amount, status, details);
}

// Enable console output
void enableConsoleOutput(bool enable) {
    printf("MOCK: %s console output\n", enable ? "Enabling" : "Disabling");
}

// Flush logs
void flushLogs(void) {
    printf("MOCK: Flushing logs\n");
}
