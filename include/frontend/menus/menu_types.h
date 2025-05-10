/**
 * @file menu_types.h
 * @brief Defines menu types and common constants for the Core Banking System
 */

#ifndef MENU_TYPES_H
#define MENU_TYPES_H

// User types
typedef enum {
    USER_CUSTOMER_REGULAR = 1,
    USER_ADMIN = 2,
    USER_CUSTOMER_PREMIUM = 3,
    USER_ADMIN_SUPER = 4,
    USER_ATM_OPERATOR = 5
} UserType;

// Menu types
typedef enum {
    MENU_MAIN,
    MENU_CUSTOMER,
    MENU_ATM_OPERATIONS,
    MENU_ADMIN,
    MENU_ACCOUNT,
    MENU_TRANSACTION,
    MENU_BENEFICIARY,
    MENU_BILL_PAYMENT,
    MENU_SECURITY,
    MENU_REPORTS
} MenuType;

#endif /* MENU_TYPES_H */
