#ifndef ACCOUNT_MANAGEMENT_H
#define ACCOUNT_MANAGEMENT_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Core Banking System Account Management Functions
 * These functions provide access to CBS account operations with SQL backend
 */

/**
 * @brief Create a new account in the core banking system
 * @param name The name of the account holder
 * @param address The address of the account holder
 * @param phone The phone number of the account holder
 * @param email The email address of the account holder
 * @param accountType The type of account (savings, current, etc.)
 * @param initialDeposit The initial deposit amount
 * @param newAccountNumber Output parameter for the generated account number
 * @param newCardNumber Output parameter for the generated card number
 * @return true if the account was created successfully, false otherwise
 */
bool cbs_create_account(const char* name, const char* address, const char* phone, 
                       const char* email, const char* accountType, double initialDeposit,
                       char* newAccountNumber, char* newCardNumber);

/**
 * @brief Get the balance of an account
 * @param accountNumber The account number
 * @param balance Output parameter for the account balance
 * @return true if the balance was retrieved successfully, false otherwise
 */
bool cbs_get_balance(const char* accountNumber, double* balance);

/**
 * @brief Get the balance of an account using the card number
 * @param cardNumber The card number
 * @param balance Output parameter for the account balance
 * @return true if the balance was retrieved successfully, false otherwise
 */
bool cbs_get_balance_by_card(const char* cardNumber, double* balance);

/**
 * @brief Update the balance of an account
 * @param accountNumber The account number
 * @param newBalance The new balance
 * @param transactionType The type of transaction (WITHDRAWAL, DEPOSIT, etc.)
 * @return true if the balance was updated successfully, false otherwise
 */
bool cbs_update_balance(const char* accountNumber, double newBalance, const char* transactionType);

/**
 * @brief Get account number from card number
 * @param cardNumber Card number
 * @param accountNumber Output parameter for account number
 * @param accountNumberSize Size of accountNumber buffer
 * @return true if successful, false otherwise
 */
bool cbs_get_account_by_card(const char* cardNumber, char* accountNumber, size_t accountNumberSize);

/**
 * @brief Get the name of an account holder
 * @param accountNumber The account number
 * @param name Output parameter for the account holder's name
 * @param nameSize The size of the name buffer
 * @return true if the name was retrieved successfully, false otherwise
 */
bool cbs_get_account_holder_name(const char* accountNumber, char* name, size_t nameSize);

/**
 * @brief Get the name of a card holder
 * @param cardNumber The card number
 * @param name Output parameter for the card holder's name
 * @param nameSize The size of the name buffer
 * @return true if the name was retrieved successfully, false otherwise
 */
bool cbs_get_card_holder_name(const char* cardNumber, char* name, size_t nameSize);

/**
 * @brief Generate an account receipt
 * @param name The name of the account holder
 * @param accountNumber The account number
 * @param accountType The type of account
 * @param ifscCode The IFSC code
 * @param cardNumber The card number
 * @param expiryDate The card expiry date
 * @param cvv The card CVV
 * @param receipt Output parameter for the generated receipt
 * @param receiptSize The size of the receipt buffer
 * @return true if the receipt was generated successfully, false otherwise
 */
bool cbs_generate_account_receipt(const char* name, const char* accountNumber, const char* accountType,
                                 const char* ifscCode, const char* cardNumber, 
                                 const char* expiryDate, const char* cvv, 
                                 char* receipt, size_t receiptSize);

/**
 * @brief Get the account number associated with a card
 * @param cardNumber The card number
 * @param accountNumber Output parameter for the account number
 * @param accountNumberSize The size of the account number buffer
 * @return true if the account number was retrieved successfully, false otherwise
 */
bool cbs_get_account_by_card(const char* cardNumber, char* accountNumber, size_t accountNumberSize);

/* Legacy function names for backward compatibility */

bool getCardHolderName(int cardNumber, char* name, size_t nameSize);
bool fetchBalance(int cardNumber, float* balance);
bool updateBalance(int cardNumber, float newBalance);
bool createNewAccount(const char* name, const char* address, const char* phone, 
                     const char* email, const char* accountType, float initialDeposit,
                     int* newAccountNumber, int* newCardNumber);
bool generateAccountReceipt(const char* name, long accountNumber, const char* accountType,
                           const char* ifscCode, const char* cardNumber, 
                           const char* expiryDate, int cvv);

#endif /* ACCOUNT_MANAGEMENT_H */
