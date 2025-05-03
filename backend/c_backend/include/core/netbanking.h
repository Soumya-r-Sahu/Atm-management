/**
 * @file netbanking.h
 * @brief Core NetBanking functionality for the ATM Management System
 * @date May 1, 2025
 */

#ifndef NETBANKING_H
#define NETBANKING_H

#include <stdbool.h>
#include <common/database/database.h>
#include <common/security/encryption_utils.h>

/**
 * @brief Status codes for netbanking operations
 */
typedef enum {
    NB_SUCCESS,
    NB_INVALID_CREDENTIALS,
    NB_ACCOUNT_LOCKED,
    NB_TRANSACTION_FAILED,
    NB_INSUFFICIENT_BALANCE,
    NB_CONNECTION_ERROR,
    NB_INVALID_BENEFICIARY,
    NB_LIMIT_EXCEEDED,
    NB_SYSTEM_ERROR,
    NB_VIRTUAL_CARD_ERROR,
    NB_VIRTUAL_CARD_LIMIT_REACHED
} NetBankingStatus;

/**
 * @brief User session information for NetBanking
 */
typedef struct {
    int user_id;
    char username[50];
    char account_number[20];
    char session_token[64];
    time_t login_time;
    time_t last_activity;
    bool is_active;
} NetBankingSession;

/**
 * @brief NetBanking transaction record
 */
typedef struct {
    int transaction_id;
    char account_number[20];
    char beneficiary_account[20];
    char transaction_type[20];
    double amount;
    time_t timestamp;
    char status[20];
    char remarks[100];
    char reference_id[20];
} NetBankingTransaction;

/**
 * @brief Virtual Card information structure
 */
typedef struct {
    char card_id[20];
    char card_number[20];
    char cvv[5];
    char expiry_date[10];
    double spending_limit;
    bool is_active;
    time_t creation_date;
    char linked_account[20];
} VirtualCard;

/**
 * @brief Initialize the NetBanking system
 * @return true if initialization is successful, false otherwise
 */
bool netbanking_init(void);

/**
 * @brief Authenticate a user for NetBanking access
 * @param username The username provided by the user
 * @param password The password provided by the user
 * @param session Pointer to store the session information if authentication is successful
 * @return NetBankingStatus indicating the result of the authentication
 */
NetBankingStatus netbanking_authenticate(const char* username, const char* password, NetBankingSession* session);

/**
 * @brief Check account balance for an authenticated user
 * @param session The active user session
 * @param balance Pointer to store the account balance
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus netbanking_check_balance(const NetBankingSession* session, double* balance);

/**
 * @brief Transfer funds to another account
 * @param session The active user session
 * @param beneficiary_account The beneficiary account number
 * @param amount The amount to transfer
 * @param remarks Optional remarks for the transaction
 * @param transaction_id Pointer to store the transaction ID if successful
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus netbanking_fund_transfer(
    const NetBankingSession* session,
    const char* beneficiary_account,
    double amount,
    const char* remarks,
    int* transaction_id
);

/**
 * @brief Get transaction history for an account
 * @param session The active user session
 * @param transactions Array to store the transactions
 * @param max_count Maximum number of transactions to retrieve
 * @param actual_count Pointer to store the actual number of transactions retrieved
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus netbanking_get_transaction_history(
    const NetBankingSession* session,
    NetBankingTransaction* transactions,
    int max_count,
    int* actual_count
);

/**
 * @brief End a user's NetBanking session
 * @param session The session to end
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus netbanking_logout(NetBankingSession* session);

/**
 * @brief Change password for a NetBanking user
 * @param session The active user session
 * @param old_password The current password
 * @param new_password The new password
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus netbanking_change_password(
    const NetBankingSession* session,
    const char* old_password,
    const char* new_password
);

/**
 * @brief Add a new beneficiary for fund transfers
 * @param session The active user session
 * @param beneficiary_account The beneficiary account number
 * @param beneficiary_name The beneficiary name
 * @param bank_code The beneficiary's bank code
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus netbanking_add_beneficiary(
    const NetBankingSession* session,
    const char* beneficiary_account,
    const char* beneficiary_name,
    const char* bank_code
);

/**
 * @brief Generate a one-time password for secure transactions
 * @param session The active user session
 * @param otp Pointer to store the generated OTP
 * @param validity_mins Validity period of the OTP in minutes
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus netbanking_generate_otp(
    const NetBankingSession* session,
    char* otp,
    int* validity_mins
);

/**
 * @brief Create a new virtual card for online transactions
 * @param session The active user session
 * @param spending_limit The spending limit for the virtual card
 * @param card Output parameter to receive the created virtual card details
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus create_virtual_card(
    const NetBankingSession* session,
    double spending_limit,
    VirtualCard* card
);

/**
 * @brief Get list of virtual cards owned by the user
 * @param session The active user session
 * @param cards Array to store the virtual cards
 * @param max_count Maximum number of cards to retrieve
 * @param actual_count Output parameter for the actual number of cards retrieved
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus get_virtual_cards(
    const NetBankingSession* session,
    VirtualCard* cards,
    int max_count,
    int* actual_count
);

/**
 * @brief Block or unblock a virtual card
 * @param session The active user session
 * @param card_id The ID of the card to modify
 * @param block_status true to block, false to unblock
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus set_virtual_card_status(
    const NetBankingSession* session,
    const char* card_id,
    bool block_status
);

/**
 * @brief Update the spending limit of a virtual card
 * @param session The active user session
 * @param card_id The ID of the card to modify
 * @param new_limit The new spending limit for the card
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus update_virtual_card_limit(
    const NetBankingSession* session,
    const char* card_id,
    double new_limit
);

/**
 * @brief Delete a virtual card
 * @param session The active user session
 * @param card_id The ID of the card to delete
 * @return NetBankingStatus indicating the result of the operation
 */
NetBankingStatus delete_virtual_card(
    const NetBankingSession* session,
    const char* card_id
);

/**
 * @brief Show virtual card management menu
 * @param session The active user session
 * @return User's menu choice
 */
int show_virtual_card_menu(const NetBankingSession* session);

#endif /* NETBANKING_H */