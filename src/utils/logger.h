#ifndef LOGGER_H
#define LOGGER_H

    // Log user activities
void logActivity(const char *activity);

// Log error messages
void writeErrorLog(const char *errorMsg);

// Log transaction details
void writeTransactionLog(const char *accountHolderName, const char *operation, const char *details);

// Log audit information
void writeAuditLog(const char *action, const char *details);

// Legacy logging functions
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);

#endif // LOGGER_H