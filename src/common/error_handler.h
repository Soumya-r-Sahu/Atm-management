#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stddef.h>

// Error codes for ATM system
typedef enum {
    ERR_SUCCESS = 0,              // No error
    ERR_INVALID_INPUT = 1,        // Invalid user input
    ERR_FILE_ACCESS = 2,          // File access error
    ERR_MEMORY_ALLOCATION = 3,    // Memory allocation failure
    ERR_AUTHENTICATION = 4,       // Authentication failure
    ERR_INSUFFICIENT_FUNDS = 5,   // Insufficient funds for transaction
    ERR_CARD_LOCKED = 6,          // Card is locked
    ERR_MAINTENANCE_MODE = 7,     // ATM is in maintenance mode
    ERR_TRANSACTION_FAILED = 8,   // Transaction failed
    ERR_DATABASE = 9,             // Database error
    ERR_CONFIG = 10,              // Configuration error
    ERR_SYSTEM = 11,              // System error
    ERR_NETWORK = 12,             // Network error
    ERR_TIMEOUT = 13,             // Operation timeout
    ERR_UNKNOWN = 99              // Unknown/unhandled error
} ErrorCode;

// Error context structure to store additional information
typedef struct {
    ErrorCode code;           // Error code
    char message[256];        // Detailed error message
    char function[64];        // Function where error occurred
    char file[128];           // File where error occurred
    int line;                 // Line number where error occurred
    void* custom_data;        // Optional custom data related to the error
} ErrorContext;

// Initialize the error handling system
void error_init(void);

// Set the current error with context
void error_set(ErrorCode code, const char* message, const char* file, const char* function, int line);

// Get the last error context
ErrorContext* error_get_last(void);

// Clear the current error
void error_clear(void);

// Get string representation of an error code
const char* error_code_to_string(ErrorCode code);

// Log the current error
void error_log(void);

// Handle error based on severity - may exit program for critical errors
void error_handle(ErrorCode code, const char* message);

// Convenience macro to set error with current context
#define SET_ERROR(code, message) error_set(code, message, __FILE__, __func__, __LINE__)

// Convenience macro to handle error with current context
#define HANDLE_ERROR(code, message) error_handle(code, message)

#endif // ERROR_HANDLER_H