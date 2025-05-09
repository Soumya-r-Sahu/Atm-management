#ifndef CARD_ACCOUNT_MANAGEMENT_H
#define CARD_ACCOUNT_MANAGEMENT_H

#include <stdbool.h>
#include <stddef.h> // For size_t

/**
 * @brief Core Banking System Card Management Functions
 * These functions provide access to CBS card operations with SQL backend
 */

// New CBS function names
bool cbs_block_card(int cardNumber);
bool cbs_unblock_card(int cardNumber);
float cbs_get_daily_withdrawals(int cardNumber);
bool cbs_validate_card(int cardNumber, int pin);
bool cbs_update_pin(int cardNumber, int newPin);
bool cbs_card_exists(int cardNumber);
bool cbs_is_card_active(int cardNumber);
bool cbs_get_card_holder_name(const char* card_number, char* name_out, size_t name_out_size);

// Legacy function names for backward compatibility
bool blockCard(int cardNumber);
bool unblockCard(int cardNumber); 
float getDailyWithdrawals(int cardNumber);

#endif // CARD_ACCOUNT_MANAGEMENT_H