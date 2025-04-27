#ifndef LOGGER_H
#define LOGGER_H

/**
 * Write a message to the error log
 * 
 * @param message The error message to log
 */
void writeErrorLog(const char *message);

/**
 * Write a message to the audit log
 * 
 * @param category The category of the audit entry (e.g., "AUTH", "TRANSACTION")
 * @param message The message to log
 */
void writeAuditLog(const char *category, const char *message);

/**
 * Write an informational message to the log
 * 
 * @param message The informational message to log
 */
void writeInfoLog(const char *message);

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
 * Log a withdrawal for daily limit tracking
 * 
 * @param cardNumber The card number associated with the withdrawal
 * @param amount The withdrawal amount
 * @param date The date of the withdrawal (YYYY-MM-DD)
 */
void logWithdrawalForLimit(int cardNumber, float amount, const char *date);

/**
 * Clear a log file (e.g., for resetting daily withdrawals)
 * 
 * @param logFile The path to the log file to clear
 */
void clearLogFile(const char *logFile);

#endif // LOGGER_H