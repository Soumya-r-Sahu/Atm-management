// Comprehensive mock implementations for ATM functions
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

// Include all required headers
#include "../../../../../include/common/database/card_account_management.h"
#include "../../../../../include/common/database/account_management.h"
#include "../../../../../include/common/database/core_banking_interface.h"
#include "../../../../../include/common/database/db_config.h"
#include "../../../../../include/common/utils/logger.h"
#include "../../../../../include/common/transaction/bill_payment.h"

// --------------------- Mock implementations for db_config.h ---------------------

bool db_init(void) { 
    printf("MOCK: db_init() called\n");
    return true; 
}

void db_cleanup(void) {
    printf("MOCK: db_cleanup() called\n");
}

MYSQL* db_get_connection(void) { 
    printf("MOCK: db_get_connection() called\n");
    return NULL; 
}

void db_release_connection(MYSQL* conn) {
    printf("MOCK: db_release_connection() called\n");
}

bool db_execute_query(const char* query) { 
    printf("MOCK: db_execute_query(%s) called\n", query);
    return true; 
}

bool db_execute_select(const char* query, void (*callback)(MYSQL_ROW row, void* user_data), void* user_data) { 
    printf("MOCK: db_execute_select(%s) called\n", query);
    return true; 
}

bool db_is_connected(void) { 
    return true; 
}

int db_get_active_connections(void) { 
    return 1; 
}

const char* db_get_error(void) { 
    return "Mock error"; 
}

bool db_prepared_query(const char* query, int bind_count, ...) { 
    printf("MOCK: db_prepared_query(%s) called\n", query);
    return true; 
}

bool db_begin_transaction(MYSQL* conn) { 
    printf("MOCK: db_begin_transaction() called\n");
    return true; 
}

bool db_commit_transaction(MYSQL* conn) { 
    printf("MOCK: db_commit_transaction() called\n");
    return true; 
}

bool db_rollback_transaction(MYSQL* conn) { 
    printf("MOCK: db_rollback_transaction() called\n");
    return true; 
}

bool db_escape_string(const char* input, char* output, size_t size) { 
    strncpy(output, input, size);
    output[size-1] = '\0'; // Ensure null termination
    return true; 
}

// --------------------- Mock implementations for card_account_management.h ---------------------

bool cbs_block_card(int cardNumber) {
    printf("MOCK: cbs_block_card(%d) called\n", cardNumber);
    return true;
}

bool cbs_unblock_card(int cardNumber) {
    printf("MOCK: cbs_unblock_card(%d) called\n", cardNumber);
    return true;
}

float cbs_get_daily_withdrawals(int cardNumber) {
    printf("MOCK: cbs_get_daily_withdrawals(%d) called\n", cardNumber);
    return 1000.0f;
}

bool cbs_validate_card(int cardNumber, int pin) {
    printf("MOCK: cbs_validate_card(%d, %d) called\n", cardNumber, pin);
    return true;
}

bool cbs_update_pin(int cardNumber, int newPin) {
    printf("MOCK: cbs_update_pin(%d, %d) called\n", cardNumber, newPin);
    return true;
}

bool cbs_card_exists(int cardNumber) {
    printf("MOCK: cbs_card_exists(%d) called\n", cardNumber);
    return true;
}

bool cbs_is_card_active(int cardNumber) {
    printf("MOCK: cbs_is_card_active(%d) called\n", cardNumber);
    return true;
}

bool cbs_get_card_holder_name(const char* card_number, char* name_out, size_t name_out_size) {
    printf("MOCK: cbs_get_card_holder_name(%s) called\n", card_number);
    strncpy(name_out, "John Doe", name_out_size);
    name_out[name_out_size-1] = '\0'; // Ensure null termination
    return true;
}

// Legacy function implementations
bool blockCard(int cardNumber) {
    return cbs_block_card(cardNumber);
}

bool unblockCard(int cardNumber) {
    return cbs_unblock_card(cardNumber);
}

float getDailyWithdrawals(int cardNumber) {
    return cbs_get_daily_withdrawals(cardNumber);
}

// --------------------- Mock implementations for account_management.h ---------------------

bool cbs_create_account(const char* name, const char* address, const char* phone, 
                      const char* email, const char* accountType, double initialDeposit,
                      char* newAccountNumber, char* newCardNumber) {
    printf("MOCK: cbs_create_account() called\n");
    strcpy(newAccountNumber, "ACC12345678");
    strcpy(newCardNumber, "5555123412341234");
    return true;
}

bool cbs_get_balance(const char* accountNumber, double* balance) {
    printf("MOCK: cbs_get_balance(%s) called\n", accountNumber);
    *balance = 5000.0;
    return true;
}

bool cbs_get_balance_by_card(const char* cardNumber, double* balance) {
    printf("MOCK: cbs_get_balance_by_card(%s) called\n", cardNumber);
    *balance = 5000.0;
    return true;
}

bool cbs_update_balance(const char* accountNumber, double newBalance, const char* transactionType) {
    printf("MOCK: cbs_update_balance(%s, %.2f, %s) called\n", accountNumber, newBalance, transactionType);
    return true;
}

bool cbs_get_account_by_card(const char* cardNumber, char* accountNumber, size_t accountNumberSize) {
    printf("MOCK: cbs_get_account_by_card(%s) called\n", cardNumber);
    snprintf(accountNumber, accountNumberSize, "ACC-%s", cardNumber);
    return true;
}

bool cbs_get_account_holder_name(const char* accountNumber, char* name, size_t nameSize) {
    printf("MOCK: cbs_get_account_holder_name(%s) called\n", accountNumber);
    snprintf(name, nameSize, "John Doe");
    return true;
}

// --------------------- Mock implementations for core_banking_interface.h ---------------------

bool cbs_process_transaction(const char* account_number, 
                            TRANSACTION_TYPE type, 
                            const char* channel, 
                            double amount, 
                            char* transaction_id_out) {
    printf("MOCK: cbs_process_transaction(%s, %d, %s, %.2f) called\n", 
           account_number, type, channel, amount);
    sprintf(transaction_id_out, "TXN%ld", (long)time(NULL));
    return true;
}

bool cbs_get_account_balance(const char* account_number, double* balance_out) {
    printf("MOCK: cbs_get_account_balance(%s) called\n", account_number);
    *balance_out = 5000.0;
    return true;
}

bool cbs_transfer_funds(const char* source_account, 
                       const char* destination_account, 
                       double amount, 
                       const char* transfer_type,
                       char* transaction_id_out) {
    printf("MOCK: cbs_transfer_funds(%s, %s, %.2f, %s) called\n", 
           source_account, destination_account, amount, transfer_type);
    sprintf(transaction_id_out, "TRF%ld", (long)time(NULL));
    return true;
}

bool cbs_get_mini_statement(const char* account_number, 
                           TransactionRecord* records, 
                           int* count, 
                           int max_records) {
    printf("MOCK: cbs_get_mini_statement(%s) called\n", account_number);
    
    // Generate mock records
    *count = max_records > 3 ? 3 : max_records;
    
    for (int i = 0; i < *count; i++) {
        sprintf(records[i].transaction_id, "TXN%d", i+1);
        sprintf(records[i].transaction_type, (i % 2 == 0) ? "DEPOSIT" : "WITHDRAWAL");
        records[i].amount = 100.0 * (i+1);
        records[i].balance = 5000.0 - (i * 100.0);
        strcpy(records[i].date, "2025-05-09 12:00:00");
        strcpy(records[i].status, "SUCCESS");
    }
    
    return true;
}

bool cbs_check_withdrawal_limit(const char* card_number, 
                               double amount, 
                               const char* channel, 
                               double* remaining_limit) {
    printf("MOCK: cbs_check_withdrawal_limit(%s, %.2f, %s) called\n", 
           card_number, amount, channel);
    *remaining_limit = 10000.0 - amount;
    return amount <= 10000.0;
}

bool cbs_get_transaction_history(const char* account_number, 
                                TransactionRecord* records, 
                                int* count, 
                                int max_records) {
    return cbs_get_mini_statement(account_number, records, count, max_records);
}

bool cbs_get_card_details(int card_number, 
                        char* holder_name_out, size_t holder_name_size,
                        char* account_number_out, size_t account_number_size,
                        char* expiry_date_out, size_t expiry_date_size,
                        int* is_active_out,
                        double* daily_limit_out,
                        char* card_type_out, size_t card_type_size) {
    printf("MOCK: cbs_get_card_details(%d) called\n", card_number);
    
    snprintf(holder_name_out, holder_name_size, "John Doe");
    snprintf(account_number_out, account_number_size, "ACC-%d", card_number);
    snprintf(expiry_date_out, expiry_date_size, "12/28");
    *is_active_out = 1;
    *daily_limit_out = 10000.00;
    snprintf(card_type_out, card_type_size, "VISA GOLD");
    
    return true;
}

// --------------------- Mock implementations for bill_payment.h ---------------------

bool cbs_process_bill_payment(const char* card_number, 
                             const char* bill_type,
                             const char* bill_reference,
                             double amount, 
                             char* transaction_id_out) {
    printf("MOCK: cbs_process_bill_payment(%s, %s, %s, %.2f) called\n", 
           card_number, bill_type, bill_reference, amount);
    
    sprintf(transaction_id_out, "BILL%ld", (long)time(NULL));
    return true;
}

bool cbs_get_bill_payment_history(const char* card_number,
                                TransactionRecord* records,
                                int* count,
                                int max_records) {
    printf("MOCK: cbs_get_bill_payment_history(%s) called\n", card_number);
    
    // Generate mock records
    *count = max_records > 2 ? 2 : max_records;
    
    if (*count > 0) {
        strcpy(records[0].transaction_id, "BILL001");
        strcpy(records[0].transaction_type, "BILL_PAYMENT");
        records[0].amount = 120.50;
        records[0].balance = 5000.0 - 120.50;
        strcpy(records[0].date, "2025-05-01 15:30:00");
        strcpy(records[0].status, "SUCCESS");
    }
    
    if (*count > 1) {
        strcpy(records[1].transaction_id, "BILL002");
        strcpy(records[1].transaction_type, "BILL_PAYMENT");
        records[1].amount = 45.75;
        records[1].balance = 5000.0 - 120.50 - 45.75;
        strcpy(records[1].date, "2025-05-03 10:15:00");
        strcpy(records[1].status, "SUCCESS");
    }
    
    return true;
}

bool cbs_verify_bill(const char* bill_type, 
                    const char* bill_reference, 
                    double* amount_out, 
                    char* due_date_out, 
                    char* payee_name_out,
                    size_t string_buffer_size) {
    printf("MOCK: cbs_verify_bill(%s, %s) called\n", bill_type, bill_reference);
    
    *amount_out = 125.75;
    strncpy(due_date_out, "2025-05-15", string_buffer_size - 1);
    due_date_out[string_buffer_size - 1] = '\0';
    
    strncpy(payee_name_out, "City Utility Company", string_buffer_size - 1);
    payee_name_out[string_buffer_size - 1] = '\0';
    
    return true;
}

// --------------------- Mock implementations for logger.h ---------------------

bool initializeLogger(const char* log_dir) {
    printf("MOCK: initializeLogger(%s) called\n", log_dir);
    return true;
}

void closeLogger(void) {
    printf("MOCK: closeLogger() called\n");
}

void setLogLevel(LogLevel level) {
    printf("MOCK: setLogLevel(%d) called\n", level);
}

LogLevel getLogLevel(void) {
    return LOG_LEVEL_INFO;
}

void writeDebugLog(const char* format, ...) {
    printf("DEBUG: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void writeInfoLog(const char* format, ...) {
    printf("INFO: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void writeWarningLog(const char* format, ...) {
    printf("WARNING: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void writeErrorLog(const char* format, ...) {
    printf("ERROR: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void writeCriticalLog(const char* format, ...) {
    printf("CRITICAL: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void writeSecurityLog(const char* format, ...) {
    printf("SECURITY: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void writeTransactionLog(const char* transaction_type, 
                         const char* card_number, 
                         double amount,
                         const char* status,
                         const char* details) {
    printf("TRANSACTION LOG: %s for card %s - $%.2f (%s) - %s\n", 
           transaction_type, card_number, amount, status, details);
}

void enableConsoleOutput(bool enable) {
    printf("MOCK: enableConsoleOutput(%s) called\n", enable ? "true" : "false");
}

void flushLogs(void) {
    printf("MOCK: flushLogs() called\n");
}

// --------------------- Additional functions ---------------------

// Special function for daily withdrawal update that was forward-declared
bool cbs_update_daily_withdrawal(const char* card_number, double amount) {
    printf("MOCK: cbs_update_daily_withdrawal(%s, %.2f) called\n", card_number, amount);
    return true;
}
