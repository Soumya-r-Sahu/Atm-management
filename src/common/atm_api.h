#ifndef ATM_API_H
#define ATM_API_H

#include "../transaction/transaction_types.h"

/**
 * ATM API Result Structure - Common return type for all API functions
 * Provides consistent error handling and return data
 */
typedef struct {
    int success;             // 1 if successful, 0 if failed
    int error_code;          // Error code if failed (0 if successful)
    char message[256];       // Success or error message
    void* data;              // Optional data pointer (must be freed by caller)
    size_t data_size;        // Size of data if applicable
} AtmApiResult;

/**
 * Transaction data structure for API operations
 */
typedef struct {
    int card_number;              // Card number for the transaction
    float amount;                 // Transaction amount
    TransactionType type;         // Type of transaction
    int target_card_number;       // Target card for transfers
    char description[100];        // Transaction description
    char auth_token[64];          // Authentication token for the session
} TransactionData;

/**
 * Card data structure for API operations
 */
typedef struct {
    int card_number;              // Card number
    char holder_name[50];         // Card holder name
    char phone_number[15];        // Card holder phone
    char email[100];              // Card holder email
    int is_active;                // Card active status
    float balance;                // Current balance (if retrieved)
} CardData;

/**
 * Authentication Functions
 */

/**
 * Initialize the ATM system API
 * Must be called before using any other API functions
 * 
 * @param test_mode Set to 1 for test mode, 0 for production
 * @return API result with success/failure info
 */
AtmApiResult atm_api_init(int test_mode);

/**
 * Authenticate a card with PIN
 * 
 * @param card_number Card number to authenticate
 * @param pin PIN code provided by user
 * @return API result with auth token in data field if successful
 */
AtmApiResult atm_api_authenticate(int card_number, const char* pin);

/**
 * Verify if a session is valid
 * 
 * @param auth_token Authentication token from successful login
 * @return API result with success/failure info
 */
AtmApiResult atm_api_verify_session(const char* auth_token);

/**
 * End a user session (logout)
 * 
 * @param auth_token Authentication token to invalidate
 * @return API result with success/failure info
 */
AtmApiResult atm_api_end_session(const char* auth_token);

/**
 * Transaction Functions
 */

/**
 * Check account balance
 * 
 * @param card_number Card number to check
 * @param auth_token Valid authentication token
 * @return API result with balance info in data field
 */
AtmApiResult atm_api_check_balance(int card_number, const char* auth_token);

/**
 * Perform a cash deposit
 * 
 * @param transaction Transaction data with amount
 * @return API result with new balance in data field
 */
AtmApiResult atm_api_deposit(const TransactionData* transaction);

/**
 * Perform a cash withdrawal
 * 
 * @param transaction Transaction data with amount
 * @return API result with new balance in data field
 */
AtmApiResult atm_api_withdraw(const TransactionData* transaction);

/**
 * Transfer money between accounts
 * 
 * @param transaction Transaction data with source, target and amount
 * @return API result with new balance in data field
 */
AtmApiResult atm_api_transfer(const TransactionData* transaction);

/**
 * Get mini statement (recent transactions)
 * 
 * @param card_number Card number to get statement for
 * @param auth_token Valid authentication token
 * @param count Number of transactions to retrieve (0 for default)
 * @return API result with statement data in data field
 */
AtmApiResult atm_api_get_mini_statement(int card_number, const char* auth_token, int count);

/**
 * Account Management Functions
 */

/**
 * Change PIN for a card
 * 
 * @param card_number Card number to change PIN for
 * @param auth_token Valid authentication token
 * @param current_pin Current PIN
 * @param new_pin New PIN
 * @return API result with success/failure info
 */
AtmApiResult atm_api_change_pin(int card_number, const char* auth_token, 
                               const char* current_pin, const char* new_pin);

/**
 * Get card details
 * 
 * @param card_number Card number to get details for
 * @param auth_token Valid authentication token
 * @return API result with card data in data field
 */
AtmApiResult atm_api_get_card_details(int card_number, const char* auth_token);

/**
 * Update card holder information
 * 
 * @param card_data Updated card data
 * @param auth_token Valid authentication token
 * @return API result with success/failure info
 */
AtmApiResult atm_api_update_card_info(const CardData* card_data, const char* auth_token);

/**
 * Admin Functions
 */

/**
 * Authenticate as admin
 * 
 * @param admin_id Admin ID
 * @param password Admin password
 * @return API result with admin auth token in data field if successful
 */
AtmApiResult atm_api_admin_login(const char* admin_id, const char* password);

/**
 * Create a new card
 * 
 * @param card_data New card data
 * @param initial_pin Initial PIN for the card
 * @param admin_token Valid admin authentication token
 * @return API result with new card number in data field
 */
AtmApiResult atm_api_create_card(const CardData* card_data, const char* initial_pin, 
                                const char* admin_token);

/**
 * Block a card
 * 
 * @param card_number Card number to block
 * @param reason Reason for blocking
 * @param admin_token Valid admin authentication token
 * @return API result with success/failure info
 */
AtmApiResult atm_api_block_card(int card_number, const char* reason, const char* admin_token);

/**
 * Unblock a card
 * 
 * @param card_number Card number to unblock
 * @param reason Reason for unblocking
 * @param admin_token Valid admin authentication token
 * @return API result with success/failure info
 */
AtmApiResult atm_api_unblock_card(int card_number, const char* reason, const char* admin_token);

/**
 * Get system status
 * 
 * @param admin_token Valid admin authentication token
 * @return API result with system status in data field
 */
AtmApiResult atm_api_get_system_status(const char* admin_token);

/**
 * Utility Functions
 */

/**
 * Clean up API resources
 * Should be called when shutting down the application
 * 
 * @return API result with success/failure info
 */
AtmApiResult atm_api_cleanup(void);

/**
 * Free data in an API result
 * Must be called for any API result that contains data
 * 
 * @param result Pointer to result structure containing data to free
 */
void atm_api_free_result(AtmApiResult* result);

/**
 * Set the language for API messages
 * 
 * @param language_code Language code (e.g., "en", "fr")
 * @return API result with success/failure info
 */
AtmApiResult atm_api_set_language(const char* language_code);

/**
 * Get the current API version
 * 
 * @return API result with version string in data field
 */
AtmApiResult atm_api_get_version(void);

#endif // ATM_API_H