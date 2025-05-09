#ifndef CARD_ACCOUNT_MANAGEMENT_H
#define CARD_ACCOUNT_MANAGEMENT_H

#include <stdbool.h>  // Added for bool type
#include <stddef.h>  // Added for size_t

// Legacy Card operations
bool blockCard(int cardNumber);
bool unblockCard(int cardNumber);
float getDailyWithdrawals(int cardNumber);

// New CBS function names
bool cbs_block_card(int cardNumber);
bool cbs_unblock_card(int cardNumber);
float cbs_get_daily_withdrawals(int cardNumber);
bool cbs_validate_card(int cardNumber, int pin);
bool cbs_update_pin(int cardNumber, int newPin);
bool cbs_card_exists(int cardNumber);
bool cbs_is_card_active(int cardNumber);
bool cbs_get_card_holder_name(const char* card_number, char* name_out, size_t name_out_size);

#endif // CARD_ACCOUNT_MANAGEMENT_H