#include "atm_api.h"
#include "error_handler.h"
#include "paths.h"
#include "../utils/memory_utils.h"
#include "../utils/logger.h"
#include "../utils/encryption_utils.h"
#include "../validation/card_security.h"
#include "../validation/pin_validation.h"
#include "../transaction/transaction_manager.h"
#include "../utils/string_utils.h"
#include "../config/config_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// API Version
#define ATM_API_VERSION "1.0.0"

// Session management
typedef struct {
    char token[64];
    int card_number;
    time_t created;
    time_t expires;
    int is_admin;
} SessionInfo;

// Session storage
#define MAX_SESSIONS 100
static SessionInfo active_sessions[MAX_SESSIONS];
static int session_count = 0;

// Initialize API result with default values
static AtmApiResult create_api_result() {
    AtmApiResult result;
    result.success = 0;
    result.error_code = 0;
    result.message[0] = '\0';
    result.data = NULL;
    result.data_size = 0;
    return result;
}

// Set success result
static void set_success_result(AtmApiResult* result, const char* message) {
    result->success = 1;
    result->error_code = 0;
    strncpy(result->message, message, sizeof(result->message) - 1);
    result->message[sizeof(result->message) - 1] = '\0';
}

// Set error result
static void set_error_result(AtmApiResult* result, int error_code, const char* message) {
    result->success = 0;
    result->error_code = error_code;
    strncpy(result->message, message, sizeof(result->message) - 1);
    result->message[sizeof(result->message) - 1] = '\0';
    
    // Log the error
    char log_message[300];
    snprintf(log_message, sizeof(log_message), "API Error %d: %s", error_code, message);
    writeErrorLog(log_message);
}

// Create a session token
static char* create_session_token(int card_number, int is_admin) {
    char source[100];
    snprintf(source, sizeof(source), "%d-%ld-%d", card_number, time(NULL), rand());
    return generate_secure_token(32); // From encryption_utils.h
}

// Add a new session
static int add_session(int card_number, int is_admin, const char* token) {
    if (session_count >= MAX_SESSIONS) {
        // Remove oldest session if we're at capacity
        memmove(&active_sessions[0], &active_sessions[1], 
                (MAX_SESSIONS - 1) * sizeof(SessionInfo));
        session_count--;
    }
    
    int session_timeout = getConfigValueInt("session_timeout_minutes");
    if (session_timeout <= 0) session_timeout = 30; // Default 30 minutes
    
    // Add new session
    SessionInfo* session = &active_sessions[session_count];
    strncpy(session->token, token, sizeof(session->token) - 1);
    session->token[sizeof(session->token) - 1] = '\0';
    session->card_number = card_number;
    session->created = time(NULL);
    session->expires = session->created + (session_timeout * 60);
    session->is_admin = is_admin;
    
    session_count++;
    return 1;
}

// Find a session by token
static SessionInfo* find_session(const char* token) {
    for (int i = 0; i < session_count; i++) {
        if (strcmp(active_sessions[i].token, token) == 0) {
            return &active_sessions[i];
        }
    }
    return NULL;
}

// Remove a session by token
static int remove_session(const char* token) {
    for (int i = 0; i < session_count; i++) {
        if (strcmp(active_sessions[i].token, token) == 0) {
            // Move all sessions after this one forward
            if (i < session_count - 1) {
                memmove(&active_sessions[i], &active_sessions[i + 1], 
                        (session_count - i - 1) * sizeof(SessionInfo));
            }
            session_count--;
            return 1;
        }
    }
    return 0;
}

// Clean up expired sessions
static void cleanup_expired_sessions() {
    time_t now = time(NULL);
    
    for (int i = session_count - 1; i >= 0; i--) {
        if (active_sessions[i].expires <= now) {
            // Remove expired session
            if (i < session_count - 1) {
                memmove(&active_sessions[i], &active_sessions[i + 1], 
                        (session_count - i - 1) * sizeof(SessionInfo));
            }
            session_count--;
        }
    }
}

// Initialize the ATM API
AtmApiResult atm_api_init(int test_mode) {
    AtmApiResult result = create_api_result();
    
    // Set test mode
    setTestingMode(test_mode);
    
    // Initialize error handling
    error_init();
    
    // Initialize encryption
    if (!encryption_init(NULL)) {
        set_error_result(&result, ERR_SYSTEM, "Failed to initialize encryption system");
        return result;
    }
    
    // Initialize files and directories
    if (!initializeDataFiles()) {
        set_error_result(&result, ERR_FILE_ACCESS, "Failed to initialize data files");
        return result;
    }
    
    // Initialize card security
    if (!card_security_init()) {
        set_error_result(&result, ERR_SYSTEM, "Failed to initialize card security");
        return result;
    }
    
    // Load configuration
    if (!loadConfig(getSystemConfigFilePath())) {
        set_error_result(&result, ERR_CONFIG, "Failed to load configuration");
        return result;
    }
    
    // Initialize session management
    srand((unsigned int)time(NULL));
    session_count = 0;
    
    set_success_result(&result, "ATM API initialized successfully");
    return result;
}

// Authenticate a card with PIN
AtmApiResult atm_api_authenticate(int card_number, const char* pin) {
    AtmApiResult result = create_api_result();
    char card_str[20];
    
    snprintf(card_str, sizeof(card_str), "%d", card_number);
    
    // Check if card exists and is not locked
    if (card_security_is_card_locked(card_str, isTestingMode())) {
        set_error_result(&result, ERR_CARD_LOCKED, "Card is locked due to multiple failed attempts");
        return result;
    }
    
    // Validate PIN
    if (!validatePin(card_number, pin)) {
        // Record failed attempt and possibly lock the card
        int remaining = card_security_record_failed_attempt(card_str, isTestingMode());
        if (remaining > 0) {
            char msg[100];
            snprintf(msg, sizeof(msg), "Invalid PIN. %d attempts remaining before card is locked", remaining);
            set_error_result(&result, ERR_AUTHENTICATION, msg);
        } else {
            set_error_result(&result, ERR_CARD_LOCKED, "Card has been locked due to too many failed attempts");
        }
        return result;
    }
    
    // Reset failed attempts after successful authentication
    card_security_reset_attempts(card_str, isTestingMode());
    
    // Create session token
    char* token = create_session_token(card_number, 0);
    if (!token) {
        set_error_result(&result, ERR_SYSTEM, "Failed to create session token");
        return result;
    }
    
    // Add session
    add_session(card_number, 0, token);
    
    // Log successful authentication
    char log_msg[100];
    snprintf(log_msg, sizeof(log_msg), "Card %d authenticated successfully", card_number);
    writeAuditLog("AUTH", log_msg);
    
    // Set result
    set_success_result(&result, "Authentication successful");
    result.data = token; // This will need to be freed by the caller
    result.data_size = strlen(token) + 1;
    
    return result;
}

// Verify if a session is valid
AtmApiResult atm_api_verify_session(const char* auth_token) {
    AtmApiResult result = create_api_result();
    
    // Clean up expired sessions first
    cleanup_expired_sessions();
    
    // Find the session
    SessionInfo* session = find_session(auth_token);
    if (!session) {
        set_error_result(&result, ERR_AUTHENTICATION, "Invalid or expired session");
        return result;
    }
    
    // Check if session has expired
    time_t now = time(NULL);
    if (session->expires <= now) {
        remove_session(auth_token);
        set_error_result(&result, ERR_TIMEOUT, "Session has expired");
        return result;
    }
    
    // Session is valid
    set_success_result(&result, "Session is valid");
    return result;
}

// End a user session (logout)
AtmApiResult atm_api_end_session(const char* auth_token) {
    AtmApiResult result = create_api_result();
    
    // Try to remove the session
    if (remove_session(auth_token)) {
        set_success_result(&result, "Session ended successfully");
    } else {
        set_error_result(&result, ERR_AUTHENTICATION, "Invalid session token");
    }
    
    return result;
}

// Check account balance
AtmApiResult atm_api_check_balance(int card_number, const char* auth_token) {
    AtmApiResult result = create_api_result();
    
    // Verify session
    AtmApiResult session_result = atm_api_verify_session(auth_token);
    if (!session_result.success) {
        // Copy error from session verification
        result.success = session_result.success;
        result.error_code = session_result.error_code;
        strncpy(result.message, session_result.message, sizeof(result.message));
        return result;
    }
    
    // Verify card number matches session
    SessionInfo* session = find_session(auth_token);
    if (session->card_number != card_number && !session->is_admin) {
        set_error_result(&result, ERR_AUTHENTICATION, "Card number does not match authenticated session");
        return result;
    }
    
    // Check balance
    TransactionResult balance_result = checkAccountBalance(card_number, "API");
    
    if (balance_result.success) {
        set_success_result(&result, "Balance retrieved successfully");
        
        // Allocate memory for the balance data
        float* balance_ptr = (float*)MALLOC(sizeof(float), "Balance data");
        if (!balance_ptr) {
            set_error_result(&result, ERR_MEMORY_ALLOCATION, "Failed to allocate memory for balance data");
            return result;
        }
        
        *balance_ptr = balance_result.newBalance;
        result.data = balance_ptr;
        result.data_size = sizeof(float);
    } else {
        set_error_result(&result, ERR_TRANSACTION_FAILED, balance_result.message);
    }
    
    return result;
}

// Implement other API functions similarly...

// Cleanup
AtmApiResult atm_api_cleanup(void) {
    AtmApiResult result = create_api_result();
    
    // Clean up resources
    encryption_cleanup();
    
    // Clear sessions
    session_count = 0;
    
    set_success_result(&result, "ATM API cleaned up successfully");
    return result;
}

// Free result data
void atm_api_free_result(AtmApiResult* result) {
    if (result && result->data) {
        FREE(result->data);
        result->data = NULL;
        result->data_size = 0;
    }
}

// Get the current API version
AtmApiResult atm_api_get_version(void) {
    AtmApiResult result = create_api_result();
    
    char* version = (char*)MALLOC(strlen(ATM_API_VERSION) + 1, "API version string");
    if (!version) {
        set_error_result(&result, ERR_MEMORY_ALLOCATION, "Failed to allocate memory for version string");
        return result;
    }
    
    strcpy(version, ATM_API_VERSION);
    result.data = version;
    result.data_size = strlen(version) + 1;
    
    set_success_result(&result, "Version retrieved successfully");
    return result;
}