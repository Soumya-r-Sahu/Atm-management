#ifndef CARD_SECURITY_H
#define CARD_SECURITY_H

/**
 * Initialize the card security service
 * @return 1 on success, 0 on failure
 */
int card_security_init(void);

/**
 * Record a failed PIN attempt and possibly lock the card
 * 
 * @param cardNumber Card number to record failure for
 * @param isTestMode Whether we're in test mode or not
 * @return Remaining attempts before lockout (0 if locked)
 */
int card_security_record_failed_attempt(const char* cardNumber, int isTestMode);

/**
 * Check if a card is currently locked out
 * 
 * @param cardNumber Card number to check
 * @param isTestMode Whether we're in test mode or not
 * @return 1 if card is locked, 0 if not locked
 */
int card_security_is_card_locked(const char* cardNumber, int isTestMode);

/**
 * Reset PIN attempts for a card (after successful auth)
 * 
 * @param cardNumber Card number to reset attempts for
 * @param isTestMode Whether we're in test mode or not
 * @return 1 on success, 0 on failure
 */
int card_security_reset_attempts(const char* cardNumber, int isTestMode);

/**
 * Get remaining PIN attempts before lockout
 * 
 * @param cardNumber Card number to check
 * @param isTestMode Whether we're in test mode or not  
 * @return Remaining attempts before lockout (0 if locked)
 */
int card_security_get_remaining_attempts(const char* cardNumber, int isTestMode);

/**
 * Lock a card manually (administrative action)
 * 
 * @param cardNumber Card number to lock
 * @param reason Reason for locking
 * @param isTestMode Whether we're in test mode or not
 * @return 1 on success, 0 on failure
 */
int card_security_lock_card(const char* cardNumber, const char* reason, int isTestMode);

/**
 * Unlock a card manually (administrative action)
 * 
 * @param cardNumber Card number to unlock  
 * @param adminId ID of admin performing the unlock
 * @param reason Reason for unlocking
 * @param isTestMode Whether we're in test mode or not
 * @return 1 on success, 0 on failure
 */
int card_security_unlock_card(const char* cardNumber, const char* adminId, const char* reason, int isTestMode);

/**
 * Get the time when a card will be automatically unlocked
 * 
 * @param cardNumber Card number to check
 * @param isTestMode Whether we're in test mode or not
 * @return Timestamp when card will be unlocked, 0 if not locked or permanently locked
 */
long card_security_get_unlock_time(const char* cardNumber, int isTestMode);

/**
 * Clean up expired card lockouts
 * 
 * @return Number of cards unlocked
 */
int card_security_cleanup_expired_locks(void);

#endif // CARD_SECURITY_H