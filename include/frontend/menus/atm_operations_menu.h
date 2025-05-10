/**
 * @file atm_operations_menu.h
 * @brief Header file for ATM operations menu in the Core Banking System
 */

#ifndef ATM_OPERATIONS_MENU_H
#define ATM_OPERATIONS_MENU_H

/**
 * @brief Run the ATM operations menu
 * @param operatorId Operator ID for the ATM operator
 */
void runAtmOperationsMenu(const char *operatorId);

/**
 * @brief Run the cash management menu for ATM operators
 * @param operatorId Operator ID for the ATM operator
 */
void runCashManagementMenu(const char *operatorId);

/**
 * @brief Run the maintenance menu for ATM operators
 * @param operatorId Operator ID for the ATM operator
 */
void runMaintenanceMenu(const char *operatorId);

/**
 * @brief Run the device status menu for ATM operators
 * @param operatorId Operator ID for the ATM operator
 */
void runDeviceStatusMenu(const char *operatorId);

#endif /* ATM_OPERATIONS_MENU_H */
