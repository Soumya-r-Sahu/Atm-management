#include "error_handler.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Static error context to store the last error
static ErrorContext last_error = {ERR_SUCCESS};

// Initialize the error handling system
void error_init(void) {
    // Clear the last error
    memset(&last_error, 0, sizeof(ErrorContext));
    last_error.code = ERR_SUCCESS;
    writeInfoLog("Error handling system initialized");
}

// Set the current error with context
void error_set(ErrorCode code, const char* message, const char* file, const char* function, int line) {
    // Store the error details
    last_error.code = code;
    strncpy(last_error.message, message, sizeof(last_error.message) - 1);
    last_error.message[sizeof(last_error.message) - 1] = '\0';
    
    strncpy(last_error.file, file, sizeof(last_error.file) - 1);
    last_error.file[sizeof(last_error.file) - 1] = '\0';
    
    strncpy(last_error.function, function, sizeof(last_error.function) - 1);
    last_error.function[sizeof(last_error.function) - 1] = '\0';
    
    last_error.line = line;
    last_error.custom_data = NULL;
    
    // Log the error
    error_log();
}

// Get the last error context
ErrorContext* error_get_last(void) {
    return &last_error;
}

// Clear the current error
void error_clear(void) {
    memset(&last_error, 0, sizeof(ErrorContext));
    last_error.code = ERR_SUCCESS;
}

// Get string representation of an error code
const char* error_code_to_string(ErrorCode code) {
    switch (code) {
        case ERR_SUCCESS: return "Success";
        case ERR_INVALID_INPUT: return "Invalid Input";
        case ERR_FILE_ACCESS: return "File Access Error";
        case ERR_MEMORY_ALLOCATION: return "Memory Allocation Failed";
        case ERR_AUTHENTICATION: return "Authentication Failed";
        case ERR_INSUFFICIENT_FUNDS: return "Insufficient Funds";
        case ERR_CARD_LOCKED: return "Card Locked";
        case ERR_MAINTENANCE_MODE: return "ATM in Maintenance Mode";
        case ERR_TRANSACTION_FAILED: return "Transaction Failed";
        case ERR_DATABASE: return "Database Error";
        case ERR_CONFIG: return "Configuration Error";
        case ERR_SYSTEM: return "System Error";
        case ERR_NETWORK: return "Network Error";
        case ERR_TIMEOUT: return "Timeout";
        case ERR_UNKNOWN:
        default: return "Unknown Error";
    }
}

// Log the current error
void error_log(void) {
    char error_msg[512];
    snprintf(error_msg, sizeof(error_msg), "[%s] %s (in %s at %s:%d)",
        error_code_to_string(last_error.code),
        last_error.message,
        last_error.function,
        last_error.file,
        last_error.line);
    
    writeErrorLog(error_msg);
}

// Handle error based on severity - may exit program for critical errors
void error_handle(ErrorCode code, const char* message) {
    // Set the error first
    SET_ERROR(code, message);
    
    // Handle based on severity
    switch (code) {
        case ERR_SUCCESS:
            // No action needed for success
            break;
            
        case ERR_INVALID_INPUT:
        case ERR_CARD_LOCKED:
        case ERR_INSUFFICIENT_FUNDS:
        case ERR_AUTHENTICATION:
        case ERR_MAINTENANCE_MODE:
            // User-facing errors, just log them
            break;
            
        case ERR_FILE_ACCESS:
        case ERR_TRANSACTION_FAILED:
        case ERR_DATABASE:
        case ERR_CONFIG:
        case ERR_NETWORK:
        case ERR_TIMEOUT:
            // System errors that can be recovered from
            fprintf(stderr, "System error: %s\n", message);
            break;
            
        case ERR_MEMORY_ALLOCATION:
        case ERR_SYSTEM:
        case ERR_UNKNOWN:
            // Critical errors that may require shutting down
            fprintf(stderr, "Critical error: %s\n", message);
            writeInfoLog("ATM system shutting down due to critical error");
            exit(code); // Exit with error code
            break;
    }
}