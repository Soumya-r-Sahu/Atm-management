#ifndef PIN_VALIDATION_H
#define PIN_VALIDATION_H

#include <stddef.h> // Added for size_t type

/**
 * Validates a PIN against the stored hash
 * 
 * @param cardNumber The card number associated with the PIN
 * @param pin The PIN to validate
 * @param isTestMode Flag indicating if test mode is active
 * @return 1 if PIN is valid, 0 otherwise
 */
int validatePIN(const char* cardNumber, const char* pin, int isTestMode);

/**
 * Changes the PIN for a given card
 * 
 * @param cardNumber The card number to change PIN for
 * @param oldPin The current PIN
 * @param newPin The new PIN to set
 * @param isTestMode Flag indicating if test mode is active
 * @return 1 if PIN was changed successfully, 0 otherwise
 */
int changePIN(const char* cardNumber, const char* oldPin, const char* newPin, int isTestMode);

/**
 * Track failed PIN attempts and block card if necessary
 * 
 * @param cardNumber The card number to track attempts for
 * @param isTestMode Flag indicating if test mode is active
 * @return 0 if card is now blocked, 1 if attempts are still allowed
 */
int trackPINAttempt(const char* cardNumber, int isTestMode);

/**
 * Reset PIN attempts counter after successful validation
 * 
 * @param cardNumber The card number to reset attempts for
 * @param isTestMode Flag indicating if test mode is active
 */
void resetPINAttempts(const char* cardNumber, int isTestMode);

/**
 * Check if a card is locked out due to too many failed PIN attempts
 * 
 * @param cardNumber The card number to check
 * @param isTestMode Flag indicating if test mode is active
 * @return 1 if card is locked out, 0 otherwise
 */
int isCardLockedOut(const char* cardNumber, int isTestMode);

/**
 * Get the number of remaining PIN attempts before lockout
 * 
 * @param cardNumber The card number to check
 * @param isTestMode Flag indicating if test mode is active
 * @return Number of remaining attempts
 */
int getRemainingPINAttempts(const char* cardNumber, int isTestMode);

/**
 * Hash a PIN to store in the system
 * 
 * @param pin The PIN to hash
 * @return A newly allocated string containing the hash (must be freed by caller)
 */
char* hashPIN(const char* pin);

/**
 * Hash a PIN combined with a card number for additional security
 * 
 * @param pin The PIN to hash
 * @param cardNumber The card number to incorporate into the hash
 * @return A newly allocated string containing the hash (must be freed by caller)
 */
char* hashPINWithCard(const char* pin, const char* cardNumber);

/**
 * Record a failed PIN attempt for a card
 * 
 * @param cardNumber The card number (as integer)
 * @return 0 if card is now blocked, 1 if attempts are still allowed
 */
int recordFailedPINAttempt(int cardNumber);

/**
 * Check if a PIN has a valid format
 * 
 * @param pin The PIN to validate
 * @return 1 if format is valid, 0 otherwise
 */
int isValidPINFormat(int pin);

/**
 * Securely input a PIN without displaying it on screen
 * 
 * @param pin Buffer to store the entered PIN
 * @param size Size of the buffer
 * @return 1 if input was successful, 0 otherwise
 */
int secure_pin_entry(char* pin, size_t size);

#endif /* PIN_VALIDATION_H */