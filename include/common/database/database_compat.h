#ifndef DATABASE_COMPAT_H
#define DATABASE_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include "atm/transaction/transaction_types.h"

// This header provides snake_case compatibility functions
// that map to the camelCase functions in database.h

// Card validation functions
bool does_card_exist(int card_number);
bool is_card_active(int card_number);
bool validate_card_pin(int card_number, int pin);
bool validate_card_with_hash(int card_number, const char* pin_hash);
bool validate_recipient_account(int card_number, const char* account_id, const char* branch_code);

// PIN management functions
bool update_pin(int card_number, int new_pin);
bool update_pin_hash(int card_number, const char* pin_hash);

// Card holder information functions
bool get_card_holder_name(int card_number, char* name, size_t name_size);
bool get_card_holder_phone(int card_number, char* phone, size_t phone_size);

// Account balance functions
float fetch_balance(int card_number);
bool update_balance(int card_number, float new_balance);

// Withdrawal tracking functions
void log_withdrawal(int card_number, float amount);
void log_withdrawal_for_limit(int card_number, float amount, const char* date);
float get_daily_withdrawals(int card_number);

// Card status management functions
bool block_card(int card_number);
bool unblock_card(int card_number);

// Transaction logging function
void log_transaction(int card_number, TransactionType type, float amount, bool success);

#endif // DATABASE_COMPAT_H