#ifndef CARD_ACCOUNT_MANAGEMENT_H
#define CARD_ACCOUNT_MANAGEMENT_H

#include <stdbool.h>  // Added for bool type

// Card operations
bool blockCard(int cardNumber);
bool unblockCard(int cardNumber);
float getDailyWithdrawals(int cardNumber);

#endif // CARD_ACCOUNT_MANAGEMENT_H