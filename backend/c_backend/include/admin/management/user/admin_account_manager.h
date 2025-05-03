#ifndef ADMIN_ACCOUNT_MANAGER_H
#define ADMIN_ACCOUNT_MANAGER_H

#include "../../admin_auth.h"
#include <stdbool.h>

/**
 * Creates a new customer account
 * 
 * @param admin Pointer to the admin user who is creating the account
 * @return true if account created successfully, false otherwise
 */
bool create_account(AdminUser* admin);

/**
 * Regenerates a card PIN for an existing customer
 * 
 * @param admin Pointer to the admin user who is regenerating the PIN
 * @return true if PIN regenerated successfully, false otherwise
 */
bool regenerate_card_pin(AdminUser* admin);

/**
 * Toggles a card's status between active and blocked
 * 
 * @param admin Pointer to the admin user who is toggling the card status
 * @return true if status changed successfully, false otherwise
 */
bool toggle_card_status(AdminUser* admin);

#endif // ADMIN_ACCOUNT_MANAGER_H