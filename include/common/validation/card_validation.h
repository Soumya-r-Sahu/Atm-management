#ifndef CARD_VALIDATION_H
#define CARD_VALIDATION_H

#include <stdbool.h>

/**
 * Check if a card number meets basic format requirements (6-digit number)
 * @param card_number The card number to validate
 * @return true if valid format, false otherwise
 */
bool is_valid_card_number(int card_number);

/**
 * Validate card number using Luhn algorithm (for real credit/debit cards)
 * @param cardNumber The card number to validate
 * @return true if valid, false otherwise
 */
bool validate_card_format(long long cardNumber);

/**
 * Check if a card exists in the database
 * @param cardNumber The card number to check
 * @return true if card exists, false otherwise
 */
bool does_card_exist(int cardNumber);

/**
 * Check if a card is active (not blocked or suspended)
 * @param cardNumber The card number to check
 * @return true if active, false otherwise
 */
bool is_card_active(int cardNumber);

/**
 * Validate a card number and PIN combination
 * @param cardNumber The card number to validate
 * @param pin The PIN to validate
 * @return true if valid card and PIN, false otherwise
 */
bool validate_card(int cardNumber, int pin);

/**
 * Validate a card number and PIN hash combination
 * @param cardNumber The card number to validate
 * @param pinHash The hashed PIN to validate
 * @return true if valid card and PIN hash, false otherwise
 */
bool validate_card_with_hash(int cardNumber, const char* pinHash);

/**
 * Check if a card is expired
 * @param cardNumber The card number to check
 * @return true if expired, false otherwise
 */
bool is_card_expired(int cardNumber);

/**
 * Get total withdrawals for a card for the current day
 * @param cardNumber The card number to check
 * @return Total withdrawal amount for today
 */
float get_daily_withdrawals(int cardNumber);

/**
 * Log a withdrawal transaction for daily limit tracking
 * @param cardNumber The card number
 * @param amount The withdrawal amount
 */
void log_withdrawal(int cardNumber, float amount);

/**
 * Track withdrawals for a specific date (for backdated processing)
 * @param cardNumber The card number
 * @param amount The withdrawal amount
 * @param date The date in YYYY-MM-DD format
 */
void log_withdrawal_for_date(int cardNumber, float amount, const char* date);

/**
 * Securely compare two hashes to prevent timing attacks
 * @param a First hash string
 * @param b Second hash string
 * @return true if equal, false otherwise
 */
bool secure_hash_compare(const char* a, const char* b);

#endif /* CARD_VALIDATION_H */