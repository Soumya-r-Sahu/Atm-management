/**
 * @file menu_system.h
 * @brief Header file for the Core Banking System menu structure
 */

#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <stdbool.h>

// Include menu types first
#include "menu_types.h"

// Legacy user type definitions (kept for backward compatibility)
#define USER_ADMIN               0
#define USER_CUSTOMER_REGULAR    1
#define USER_ATM_OPERATOR        2
#define USER_CUSTOMER_PREMIUM    3

// Don't include other menu headers here to avoid circular dependencies
// Each menu implementation file should include its own header

// Core function prototypes 
// Note: These are provided for backward compatibility
// Prefer including the specific menu header files in your implementation

// Function prototypes for main menu handler
void runMainMenu(void);

// Function prototypes for customer menu
void runCustomerMenu(const char *username, int userType);
void runAccountMenu(const char *username);
void runTransactionMenu(const char *username);
void runBeneficiaryMenu(const char *username);
void runBillPaymentMenu(const char *username);

// Function prototypes for ATM operations menu
void runAtmOperationsMenu(const char *operatorId);
void runCashManagementMenu(const char *operatorId);
void runMaintenanceMenu(const char *operatorId);
void runDeviceStatusMenu(const char *operatorId);

// Function prototypes for admin menu
void runAdminMenu(const char *adminId);
void runUserManagementMenu(const char *adminId);
void runSystemConfigMenu(const char *adminId);
void runSecurityManagementMenu(const char *adminId);
void runAuditMenu(const char *adminId);

// NOTE: Common utility functions have been moved to menu_utils.h
// Include "../cli/menu_utils.h" in your implementation file if you need these functions

#endif /* MENU_SYSTEM_H */
