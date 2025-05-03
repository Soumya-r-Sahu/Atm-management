#ifndef CARD_VALIDATOR_H
#define CARD_VALIDATOR_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum allowed PIN attempts before card is locked
#define MAX_PIN_ATTEMPTS 3

// Card validation status codes
typedef enum {
    CARD_VALID = 0,
    CARD_INVALID_FORMAT,
    CARD_NOT_FOUND,
    CARD_EXPIRED,
    CARD_CVV_INVALID,
    CARD_BLOCKED,
    CARD_PIN_INVALID,
    CARD_ERROR
} CardValidationStatus;

// PIN validation status codes
typedef enum {
    PIN_VALID,
    PIN_INVALID,
    PIN_ATTEMPTS_EXCEEDED,
    PIN_CARD_BLOCKED
} PinValidationStatus;

// Card data structure
typedef struct {
    int card_id;                // Internal card ID
    char card_number[20];       // Card number as string
    int customer_id;            // Customer ID
    int pin;                    // PIN code (should be stored encrypted)
    int failed_attempts;        // Number of failed PIN attempts
    bool is_blocked;            // Flag to indicate if card is blocked
    float balance;              // Account balance
    char expiry_date[10];       // Expiry date in MM/YY format
    int cvv;                    // CVV code (3 digits)
    int is_virtual;             // Flag to indicate if card is virtual
} CardData;

/**
 * Validates card number format
 * @param cardNumber The card number string to validate
 * @return true if format is valid, false otherwise
 */
bool validate_card_format(const char* cardNumber);

/**
 * Validates a card based on card number
 * @param cardNumber The card number to validate
 * @return Card validation status
 */
CardValidationStatus validate_card(const char* cardNumber);

/**
 * Validates a virtual transaction with card number, CVV, and expiry date
 * @param cardNumber The card number
 * @param cvv The CVV code
 * @param expiryDate The expiry date in MM/YY format
 * @return Card validation status
 */
CardValidationStatus validate_virtual_transaction(const char* cardNumber, int cvv, const char* expiryDate);

/**
 * Gets card data from a card number
 * @param cardNumber The card number to look up
 * @return Pointer to allocated CardData structure (caller must free),
 *         or NULL if card not found
 */
CardData* get_card_data(const char* cardNumber);

/**
 * Check if virtual transactions have exceeded daily limits
 * @param cardNumber The card number
 * @param amount The current transaction amount
 * @return true if limit would be exceeded, false otherwise
 */
bool is_virtual_transaction_limit_exceeded(const char* cardNumber, float amount);

/**
 * Check if virtual ATM features are enabled in configuration
 * @return true if enabled, false otherwise
 */
bool is_virtual_atm_enabled();

#endif /* CARD_VALIDATOR_H */