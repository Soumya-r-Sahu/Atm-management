#include "common/database/database_compat.h"
#include "common/database/database.h"
#include "common/validation/card_validation.h"
#include "common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
 * Compatibility layer to bridge snake_case and camelCase function naming
 * This file implements snake_case functions that call their camelCase equivalents
 */

// Card validation functions
bool does_card_exist(int card_number) {
    return doesCardExist(card_number);
}

bool is_card_active(int card_number) {
    return isCardActive(card_number);
}

bool validate_card_pin(int card_number, int pin) {
    return validateCard(card_number, pin);
}

bool validate_card_with_hash(int card_number, const char* pin_hash) {
    return validateCardWithHash(card_number, pin_hash);
}

bool validate_recipient_account(int card_number, const char* account_id, const char* branch_code) {
    return validateRecipientAccount(card_number, account_id, branch_code);
}

// Check if card has exceeded daily withdrawal limit
bool has_exceeded_daily_limit(int card_number, double amount) {
    double daily_total = get_daily_withdrawal_total(card_number);
    double daily_limit = 5000.0; // Default limit
    
    // Try to get limit from config
    const char* limit_str = get_config_value("daily_withdrawal_limit");
    if (limit_str) {
        double config_limit = atof(limit_str);
        if (config_limit > 0) {
            daily_limit = config_limit;
        }
    }
    
    return (daily_total + amount > daily_limit);
}

// Get daily withdrawal total for a card
double get_daily_withdrawal_total(int card_number) {
    return getDailyWithdrawals(card_number);
}

// PIN management functions
bool update_pin(int card_number, int new_pin) {
    return updatePIN(card_number, new_pin);
}

bool update_pin_hash(int card_number, const char* pin_hash) {
    return updatePINHash(card_number, pin_hash);
}

// Card holder information functions
bool get_card_holder_name(int card_number, char* name, size_t name_size) {
    return getCardHolderName(card_number, name, name_size);
}

bool get_card_holder_phone(int card_number, char* phone, size_t phone_size) {
    return getCardHolderPhone(card_number, phone, phone_size);
}

// Account balance functions
float fetch_balance(int card_number) {
    return fetchBalance(card_number);
}

bool update_balance(int card_number, float new_balance) {
    return updateBalance(card_number, new_balance);
}

// Withdrawal tracking functions
void log_withdrawal(int card_number, float amount) {
    logWithdrawal(card_number, amount);
}

void log_withdrawal_for_limit(int card_number, float amount, const char* date) {
    logWithdrawalForLimit(card_number, amount, date);
}

float get_daily_withdrawals(int card_number) {
    return getDailyWithdrawals(card_number);
}

// Card status management functions
bool block_card(int card_number) {
    return blockCard(card_number);
}

bool unblock_card(int card_number) {
    return unblockCard(card_number);
}

// Transaction logging function
void log_transaction(int card_number, TransactionType type, float amount, bool success) {
    logTransaction(card_number, type, amount, success);
}