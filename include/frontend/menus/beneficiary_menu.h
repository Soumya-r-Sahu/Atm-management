/**
 * @file beneficiary_menu.h
 * @brief Header file for beneficiary management menu in the Core Banking System
 */

#ifndef BENEFICIARY_MENU_H
#define BENEFICIARY_MENU_H

/**
 * @brief Run the beneficiary management menu
 * @param username Customer username
 */
void runBeneficiaryMenu(const char *username);

/**
 * @brief Add a new beneficiary
 * @param username Customer username
 */
void addBeneficiary(const char *username);

/**
 * @brief View beneficiary details
 * @param username Customer username
 */
void viewBeneficiaries(const char *username);

/**
 * @brief Delete a beneficiary
 * @param username Customer username
 */
void deleteBeneficiary(const char *username);

#endif /* BENEFICIARY_MENU_H */
