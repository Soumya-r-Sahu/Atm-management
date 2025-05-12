/**
 * @file dao_audit_logger.h
 * @brief Header file for the DAO audit logging system that integrates with the CBS logger
 * @author GitHub Copilot
 * @date May 10, 2025
 */

#ifndef DAO_AUDIT_LOGGER_H
#define DAO_AUDIT_LOGGER_H

#include <stdbool.h>
#include "common/utils/cbs_logger.h"

/**
 * @brief Record an audit log for a transaction operation
 * 
 * @param user_id The ID of the user performing the transaction
 * @param cardNumber The card number (will be masked in logs)
 * @param transaction_type The type of transaction (DEPOSIT, WITHDRAWAL, etc.)
 * @param amount The transaction amount
 * @param old_balance The balance before the transaction
 * @param new_balance The balance after the transaction
 * @param success Whether the transaction was successful
 */
void record_transaction_audit(
    const char* user_id,
    int cardNumber,
    const char* transaction_type,
    float amount,
    float old_balance,
    float new_balance,
    bool success);

/**
 * @brief Record an audit log for a card operation
 * 
 * @param user_id The ID of the user performing the operation
 * @param cardNumber The card number (will be masked in logs)
 * @param operation_type The type of operation (ACTIVATE, BLOCK, UNBLOCK, etc.)
 * @param before_status The card status before the operation
 * @param after_status The card status after the operation
 * @param success Whether the operation was successful
 */
void record_card_operation_audit(
    const char* user_id,
    int cardNumber,
    const char* operation_type,
    const char* before_status,
    const char* after_status,
    bool success);

/**
 * @brief Record an audit log for a database operation
 * 
 * @param operation_type The type of operation (INSERT, UPDATE, DELETE, SELECT)
 * @param table_name The name of the table
 * @param record_id The ID of the record (if applicable)
 * @param before_state The state of the record before the operation (JSON format)
 * @param after_state The state of the record after the operation (JSON format)
 * @param success Whether the operation was successful
 * @param error_message The error message if the operation failed
 */
void record_db_operation_audit(
    const char* operation_type,
    const char* table_name,
    const char* record_id,
    const char* before_state,
    const char* after_state,
    bool success,
    const char* error_message);

/**
 * @brief Record an audit log for a user authentication operation
 * 
 * @param user_id The ID of the user
 * @param auth_type The type of authentication (PIN, PASSWORD, OTP)
 * @param success Whether the authentication was successful
 * @param ip_address The IP address of the client
 * @param device_info The client device information
 */
void record_auth_audit(
    const char* user_id,
    const char* auth_type,
    bool success,
    const char* ip_address,
    const char* device_info);

/**
 * @brief Initialize the DAO audit logger system
 * 
 * @param log_directory The directory where log files will be stored
 * @param retention_days The number of days to retain log files
 * @return bool True if initialization was successful, false otherwise
 */
bool init_dao_audit_logger(const char* log_directory, int retention_days);

/**
 * @brief Close the DAO audit logger system
 */
void close_dao_audit_logger(void);

#endif /* DAO_AUDIT_LOGGER_H */
