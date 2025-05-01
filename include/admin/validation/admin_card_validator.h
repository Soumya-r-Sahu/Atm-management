#ifndef ADMIN_CARD_VALIDATOR_H
#define ADMIN_CARD_VALIDATOR_H

#include <stdbool.h>

// Admin-specific card validation status codes
typedef enum {
    ADMIN_CARD_VALID,
    ADMIN_CARD_NOT_FOUND,
    ADMIN_CARD_EXPIRED,
    ADMIN_CARD_INACTIVE,
    ADMIN_CARD_FLAGGED,
    ADMIN_CARD_INVALID_PIN
} AdminCardValidationStatus;

// Detailed card information structure for admin interface
typedef struct {
    int card_number;
    char card_id[20];
    char account_id[20];
    char card_type[20];
    char expiry_date[20];
    char status[20];
    char holder_name[50];
    char phone_number[20];
    int total_transactions;
    char last_transaction_date[30];
    float last_transaction_amount;
    char last_login_date[30];
    int failed_login_attempts;
} CardDetails;

// Admin card validation result
typedef struct {
    AdminCardValidationStatus status;
    const char* message;
    float account_balance;
    CardDetails details;
} AdminCardValidationResult;

/**
 * Admin-specific card validation with enhanced security
 * @param card_number The card number to validate
 * @param pin The PIN to validate (or -1 to skip PIN validation)
 * @param security_level Security level for validation (0=basic, 1=enhanced)
 * @return Validation result with status, message, and additional card details
 */
AdminCardValidationResult validate_card_admin(int card_number, int pin, int security_level);

/**
 * Check if a card has been flagged for suspicious activity
 * @param card_number The card number to check
 * @return true if flagged, false otherwise
 */
bool is_card_flagged(int card_number);

/**
 * Get detailed card information for admin interface
 * @param card_number The card number
 * @param details Pointer to CardDetails structure to fill
 */
void get_card_details(int card_number, CardDetails* details);

/**
 * Get transaction count for a card
 * @param card_number The card number
 * @return Number of transactions
 */
int get_transaction_count(int card_number);

/**
 * Get the last transaction details for a card
 * @param card_number The card number
 * @param date Buffer to store date string
 * @param date_size Size of date buffer
 * @param amount Pointer to store amount
 */
void get_last_transaction(int card_number, char* date, size_t date_size, float* amount);

/**
 * Get the last login date for a card
 * @param card_number The card number
 * @param date Buffer to store date string
 * @param date_size Size of date buffer
 */
void get_last_login_date(int card_number, char* date, size_t date_size);

/**
 * Get the number of failed login attempts for a card
 * @param card_number The card number
 * @return Number of failed login attempts
 */
int get_failed_login_attempts(int card_number);

#endif /* ADMIN_CARD_VALIDATOR_H */