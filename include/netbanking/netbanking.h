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
    NB_SYSTEM_ERROR
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

#endif /* NETBANKING_H */