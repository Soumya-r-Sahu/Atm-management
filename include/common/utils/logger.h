#ifndef LOGGER_H
#define LOGGER_H

/**
 * Initialize the logging system
 */
void initialize_logging(void);

/**
 * Close the logging system
 */
void close_logs(void);

/**
 * Write a message to the error log
 * 
 * @param message The error message to log
 */
void writeErrorLog(const char *message);

/**
 * Compatibility function for write_error_log
 * 
 * @param message The error message to log
 */
void write_error_log(const char* message);

/**
 * Write a message to the audit log
 * 
 * @param category The category of the audit entry (e.g., "AUTH", "TRANSACTION")
 * @param message The message to log
 */
void writeAuditLog(const char *category, const char *message);

/**
 * Compatibility function for write_audit_log
 * 
 * @param category The audit category
 * @param message The audit message to log
 */
void write_audit_log(const char* category, const char* message);

/**
 * Write an informational message to the log
 * 
 * @param message The informational message to log
 */
void writeInfoLog(const char *message);

/**
 * Compatibility function for write_info_log
 * 
 * @param message The info message to log
 */
void write_info_log(const char* message);

/**
 * Write a transaction to the transaction log
 * 
 * @param cardNumber The card number associated with the transaction
 * @param transactionType The type of transaction (e.g., "Deposit", "Withdrawal")
 * @param amount The transaction amount
 * @param success Whether the transaction was successful (1) or failed (0)
 */
void writeTransactionLog(int cardNumber, const char *transactionType, float amount, int success);

/**
 * Write a detailed transaction to the transaction log
 * 
 * @param transactionType The type of transaction
 * @param details Additional details about the transaction
 */
void writeDetailedTransactionLog(const char* transactionType, const char* details);

/**
 * Log a withdrawal for daily limit tracking
 * 
 * @param cardNumber The card number associated with the withdrawal
 * @param amount The withdrawal amount
 * @param date The date of the withdrawal (YYYY-MM-DD)
 */
void logWithdrawalForLimit(int cardNumber, float amount, const char *date);

/**
 * Write a withdrawal to the withdrawals log
 * 
 * @param cardNumber The card number making the withdrawal
 * @param amount The amount of the withdrawal
 */
void writeWithdrawalLog(int cardNumber, float amount);

/**
 * Clear a log file (e.g., for resetting daily withdrawals)
 * 
 * @param logFile The path to the log file to clear
 */
void clearLogFile(const char *logFile);

//=================================================================
// snake_case compatibility functions to bridge naming conventions
//=================================================================

// Snake_case versions for consistent code style
void write_error_log(const char *message);
void write_audit_log(const char *category, const char *message);
void write_info_log(const char *message);
void write_transaction_log(int cardNumber, const char *transactionType, float amount, int success);
void clear_log_file(const char *logFile);

#endif // LOGGER_H