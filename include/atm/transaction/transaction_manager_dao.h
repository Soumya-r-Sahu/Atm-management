/**
 * @file transaction_manager_dao.h
 * @brief Transaction Manager functions using DAO pattern
 * @version 1.0
 * @date May 10, 2025
 */

#ifndef TRANSACTION_MANAGER_DAO_H
#define TRANSACTION_MANAGER_DAO_H

#include "../include/atm/transaction/transaction_types.h"

/**
 * @brief Transaction result structure
 */
typedef struct {
    int success;           // 1 for success, 0 for failure
    float oldBalance;      // Balance before transaction
    float newBalance;      // Balance after transaction
    char message[200];     // Result message or error
} TransactionResult;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check account balance using the DAO pattern
 * @param cardNumber The card number
 * @param username The username performing the request
 * @return Transaction result
 */
TransactionResult checkAccountBalance_dao(int cardNumber, const char* username);

/**
 * @brief Perform deposit using the DAO pattern
 * @param cardNumber The card number
 * @param amount The amount to deposit
 * @param username The username performing the request
 * @return Transaction result
 */
TransactionResult performDeposit_dao(int cardNumber, float amount, const char* username);

/**
 * @brief Perform withdrawal using the DAO pattern
 * @param cardNumber The card number
 * @param amount The amount to withdraw
 * @param username The username performing the request
 * @return Transaction result
 */
TransactionResult performWithdrawal_dao(int cardNumber, float amount, const char* username);

/**
 * @brief Perform virtual withdrawal using the DAO pattern
 * @param cardNum The card number
 * @param amount The amount to withdraw
 * @param username The username performing the request
 * @return Transaction result
 */
TransactionResult performVirtualWithdrawal_dao(int cardNum, float amount, const char* username);

/**
 * @brief Perform PIN change using the DAO pattern
 * @param cardNumber The card number
 * @param oldPIN The old PIN
 * @param newPIN The new PIN
 * @param username The username performing the request
 * @return Transaction result
 */
TransactionResult performPINChange_dao(int cardNumber, const char* oldPIN, const char* newPIN, const char* username);

/**
 * @brief Get mini statement using the DAO pattern
 * @param cardNumber The card number
 * @param username The username performing the request
 * @return Transaction result with mini statement in message field
 */
TransactionResult getMiniStatement_dao(int cardNumber, const char* username);

/**
 * @brief Perform money transfer using the DAO pattern
 * @param senderCardNumber The sender's card number
 * @param receiverCardNumber The receiver's card number
 * @param amount The amount to transfer
 * @param username The username performing the request
 * @return Transaction result
 */
TransactionResult performMoneyTransfer_dao(int senderCardNumber, int receiverCardNumber, float amount, const char* username);

/**
 * @brief Perform bill payment using the DAO pattern
 * @param cardNumber The card number
 * @param billType The type of bill
 * @param billReference The bill reference number
 * @param amount The amount to pay
 * @param username The username performing the request
 * @return Transaction result
 */
TransactionResult performBillPayment_dao(int cardNumber, const char* billType, const char* billReference, float amount, const char* username);

/**
 * @brief Perform virtual bill payment using the DAO pattern
 * @param cardNum The card number
 * @param billType The type of bill
 * @param billReference The bill reference number
 * @param amount The amount to pay
 * @param username The username performing the request
 * @return Transaction result
 */
TransactionResult performVirtualBillPayment_dao(int cardNum, const char* billType, const char* billReference, float amount, const char* username);

/**
 * @brief Check if maintenance mode is active
 * @return true if maintenance mode is active, false otherwise
 */
bool isMaintenanceModeActive(void);

/**
 * @brief Check if virtual ATM is enabled
 * @return true if virtual ATM is enabled, false otherwise
 */
bool is_virtual_atm_enabled(void);

#ifdef __cplusplus
}
#endif

#endif /* TRANSACTION_MANAGER_DAO_H */
