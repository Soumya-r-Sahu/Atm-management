/**
 * @file dao_audit_logger.c
 * @brief DAO layer audit logging with CBS compliance for ATM Management System
 * @version 1.0
 * @date May 10, 2025
 */

#include "common/utils/dao_audit_logger.h"
#include "common/utils/cbs_logger.h"
#include "common/database/dao_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Generate transaction ID based on timestamp and random number
static char* generate_transaction_id() {
    static char transaction_id[37];
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    
    // Format: TXN-YYYYMMDD-HHMMSS-RANDOM
    snprintf(transaction_id, sizeof(transaction_id), 
             "TXN-%04d%02d%02d-%02d%02d%02d-%06d",
             tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
             tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec,
             rand() % 1000000);
             
    return transaction_id;
}

// Convert card number to string format
static void card_number_to_string(int cardNumber, char* buffer, size_t size) {
    snprintf(buffer, size, "%d", cardNumber);
}

// Record a transaction with before and after account state
void record_transaction_audit(
    const char* user_id,
    int cardNumber,
    const char* transaction_type,
    float amount,
    float old_balance,
    float new_balance,
    bool success) {
    
    char card_str[20];
    card_number_to_string(cardNumber, card_str, sizeof(card_str));
    
    // Determine transaction status
    const char* status = success ? "SUCCESS" : "FAILED";
    
    // Generate transaction details
    char details[256];
    snprintf(details, sizeof(details), 
             "Transaction processed at ATM. User: %s", 
             user_id ? user_id : "SYSTEM");
    
    // Write to the CBS transaction log
    cbs_writeTransactionLog(
        generate_transaction_id(),     // Generate unique transaction ID
        user_id ? user_id : "SYSTEM",  // User ID or system
        card_str,                      // Card number (will be masked)
        NULL,                          // Account ID (not used here)
        transaction_type,              // Transaction type
        (double)amount,                // Amount
        (double)old_balance,           // Previous balance
        (double)new_balance,           // New balance
        status,                        // Status
        details                        // Additional details
    );
    
    // Create audit log with before/after state
    char before_state[100];
    char after_state[100];
    
    snprintf(before_state, sizeof(before_state), "{\"balance\":%.2f}", old_balance);
    snprintf(after_state, sizeof(after_state), "{\"balance\":%.2f}", new_balance);
    
    cbs_writeAuditLog(
        user_id ? user_id : "SYSTEM",
        transaction_type,
        "ACCOUNT",
        card_str,
        before_state,
        after_state,
        details
    );
}

// Record a database operation in the audit log
void record_database_audit(
    const char* user_id,
    const char* operation_type,
    const char* entity_type,
    const char* entity_id,
    const char* before_state,
    const char* after_state,
    bool success) {
    
    // Determine operation status
    const char* status = success ? "SUCCESS" : "FAILED";
    
    // Generate operation details
    char details[256];
    snprintf(details, sizeof(details), 
             "Database operation %s on %s, Status: %s", 
             operation_type, entity_type, status);
    
    // Write to the audit log
    cbs_writeAuditLog(
        user_id ? user_id : "SYSTEM",  // User ID or system
        operation_type,                // Operation type
        entity_type,                   // Entity type
        entity_id,                     // Entity ID
        before_state,                  // Before state
        after_state,                   // After state
        details                        // Additional details
    );
    
    // Also log to application log
    cbs_writeInfoLog("Database %s on %s:%s - %s", 
                    operation_type, entity_type, entity_id, status);
}

// Record card operation in the audit log
void record_card_operation_audit(
    const char* user_id,
    int cardNumber,
    const char* operation_type,
    const char* before_status,
    const char* after_status,
    bool success) {
    
    char card_str[20];
    card_number_to_string(cardNumber, card_str, sizeof(card_str));
    
    // Determine operation status
    const char* status = success ? "SUCCESS" : "FAILED";
    
    // Generate before and after state JSON
    char before_state[100] = "N/A";
    char after_state[100] = "N/A";
    
    if (before_status) {
        snprintf(before_state, sizeof(before_state), "{\"status\":\"%s\"}", before_status);
    }
    
    if (after_status) {
        snprintf(after_state, sizeof(after_state), "{\"status\":\"%s\"}", after_status);
    }
    
    // Generate operation details
    char details[256];
    snprintf(details, sizeof(details), 
             "Card operation %s on card %d, Status: %s", 
             operation_type, cardNumber, status);
    
    // Write security log for card operations
    cbs_writeSecurityLog("Card %s operation: %s -> %s, Status: %s",
                         card_str, before_status ? before_status : "N/A", 
                         after_status ? after_status : "N/A", status);
    
    // Write to the audit log
    cbs_writeAuditLog(
        user_id ? user_id : "SYSTEM",  // User ID or system
        operation_type,                // Operation type
        "CARD",                        // Entity type
        card_str,                      // Entity ID
        before_state,                  // Before state
        after_state,                   // After state
        details                        // Additional details
    );
}

// Log database errors
void log_database_error(
    const char* operation,
    const char* entity_type,
    const char* entity_id,
    const char* error_message) {
    
    // Write to error log
    cbs_writeErrorLog("Database error during %s on %s:%s - %s", 
                     operation, entity_type, entity_id, error_message);
    
    // Also record in the audit log
    char details[512];
    snprintf(details, sizeof(details), "Error: %s", error_message);
    
    cbs_writeAuditLog(
        "SYSTEM",                      // User ID
        operation,                     // Operation type
        entity_type,                   // Entity type
        entity_id,                     // Entity ID
        NULL,                          // Before state
        NULL,                          // After state
        details                        // Additional details
    );
}

// Compatibility wrapper for transaction logging
void log_dao_transaction(
    int cardNumber,
    const char* transactionType,
    float amount,
    bool success,
    const char* userName) {
    
    // For DAO transaction logs, we don't have before/after balances
    // So we'll just use 0.0 as placeholders
    record_transaction_audit(
        userName,
        cardNumber,
        transactionType,
        amount,
        0.0f,    // Old balance not available
        0.0f,    // New balance not available
        success
    );
}

// Log DAO operation with balance information
void log_dao_balance_update(
    int cardNumber,
    const char* transactionType,
    float amount,
    float oldBalance,
    float newBalance,
    bool success,
    const char* userName) {
    
    record_transaction_audit(
        userName,
        cardNumber,
        transactionType,
        amount,
        oldBalance,
        newBalance,
        success
    );
}

// Log card status change operation
void log_dao_card_status_change(
    int cardNumber,
    const char* operation,
    const char* oldStatus,
    const char* newStatus,
    bool success,
    const char* userName) {
    
    record_card_operation_audit(
        userName,
        cardNumber,
        operation,
        oldStatus,
        newStatus,
        success
    );
}
