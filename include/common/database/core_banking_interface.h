#ifndef CORE_BANKING_INTERFACE_H
#define CORE_BANKING_INTERFACE_H

#include <stdbool.h>
#include <stddef.h> // For size_t

// Transaction types from the core banking system
typedef enum {
    CBS_TRANSACTION_WITHDRAWAL,
    CBS_TRANSACTION_DEPOSIT, 
    CBS_TRANSACTION_TRANSFER,
    CBS_TRANSACTION_PAYMENT,
    CBS_TRANSACTION_BALANCE_INQUIRY,
    CBS_TRANSACTION_MINI_STATEMENT,
    CBS_TRANSACTION_PIN_CHANGE,
    CBS_TRANSACTION_INTEREST_CREDIT,
    CBS_TRANSACTION_FEE_DEBIT,
    CBS_TRANSACTION_REVERSAL
} TRANSACTION_TYPE;

// Transaction record structure
typedef struct {
    char transaction_id[37];
    char transaction_type[30];
    double amount;
    double balance;
    char date[20];
    char status[20];
} TransactionRecord;

/**
 * @brief Process a transaction in the core banking system
 * @param account_number Account number for the transaction
 * @param type Type of transaction (withdrawal, deposit, etc.)
 * @param channel Transaction channel (ATM, POS, etc.)
 * @param amount Transaction amount
 * @param transaction_id_out Output parameter for the generated transaction ID
 * @return true if transaction was successful, false otherwise
 */
bool cbs_process_transaction(const char* account_number, 
                            TRANSACTION_TYPE type, 
                            const char* channel, 
                            double amount, 
                            char* transaction_id_out);

/**
 * @brief Get account balance
 * @param account_number Account number to query
 * @param balance_out Output parameter for the balance
 * @return true if query was successful, false otherwise
 */
bool cbs_get_account_balance(const char* account_number, double* balance_out);

/**
 * @brief Transfer funds between accounts
 * @param source_account Source account number
 * @param destination_account Destination account number
 * @param amount Transfer amount
 * @param transfer_type Type of transfer (INTERNAL, NEFT, RTGS, etc.)
 * @param transaction_id_out Output parameter for the transaction ID
 * @return true if transfer was successful, false otherwise
 */
bool cbs_transfer_funds(const char* source_account, 
                       const char* destination_account, 
                       double amount, 
                       const char* transfer_type,
                       char* transaction_id_out);

/**
 * @brief Get mini statement with transaction history
 * @param account_number Account number
 * @param records Array to store transaction records
 * @param count Output parameter for number of records found
 * @param max_records Maximum number of records to retrieve
 * @return true if mini statement was retrieved successfully, false otherwise
 */
bool cbs_get_mini_statement(const char* account_number, 
                           TransactionRecord* records, 
                           int* count, 
                           int max_records);

/**
 * @brief Check if a withdrawal is within daily limits
 * @param card_number Card number
 * @param amount Withdrawal amount
 * @param channel Channel (ATM, POS, ONLINE)
 * @param remaining_limit Output parameter for remaining limit
 * @return true if withdrawal is allowed, false otherwise
 */
bool cbs_check_withdrawal_limit(const char* card_number, 
                               double amount, 
                               const char* channel, 
                               double* remaining_limit);

/**
 * @brief Create a new fixed deposit
 * @param account_number Source account for FD amount
 * @param customer_id Customer ID
 * @param principal_amount Principal amount for the FD
 * @param interest_rate Interest rate
 * @param tenure_months Tenure in months
 * @param payout_frequency Interest payout frequency
 * @param fd_id_out Output parameter for generated FD ID
 * @return true if FD creation was successful, false otherwise
 */
bool cbs_create_fixed_deposit(const char* account_number, 
                             const char* customer_id,
                             double principal_amount, 
                             double interest_rate,
                             int tenure_months,
                             const char* payout_frequency,
                             char* fd_id_out);

/**
 * @brief Process loan EMI payment
 * @param loan_id Loan ID
 * @param emi_amount Total EMI amount
 * @param principal_component Principal component of EMI
 * @param interest_component Interest component of EMI
 * @param payment_mode Payment mode (CASH, AUTO_DEBIT, etc.)
 * @return true if EMI payment was successful, false otherwise
 */
bool cbs_process_loan_emi(const char* loan_id,
                         double emi_amount,
                         double principal_component,
                         double interest_component,
                         const char* payment_mode);

/**
 * @brief Update card limits
 * @param card_number Card number
 * @param atm_limit Daily ATM withdrawal limit
 * @param pos_limit Daily POS transaction limit
 * @param online_limit Daily online transaction limit
 * @return true if update was successful, false otherwise
 */
bool cbs_update_card_limits(const char* card_number, 
                           double atm_limit, 
                           double pos_limit, 
                           double online_limit);

/**
 * @brief Add a new beneficiary
 * @param customer_id Customer ID
 * @param beneficiary_name Beneficiary name
 * @param account_number Beneficiary account number
 * @param ifsc_code Beneficiary bank IFSC code
 * @param bank_name Beneficiary bank name
 * @param account_type Beneficiary account type
 * @return true if beneficiary was added successfully, false otherwise
 */
bool cbs_add_beneficiary(const char* customer_id,
                        const char* beneficiary_name,
                        const char* account_number,
                        const char* ifsc_code,
                        const char* bank_name,
                        const char* account_type);

/**
 * @brief Get transaction history for an account
 * @param account_number Account number
 * @param records Array to store transaction records
 * @param count Output parameter for number of records found
 * @param max_records Maximum number of records to retrieve
 * @return true if successful, false otherwise
 */
bool cbs_get_transaction_history(const char* account_number, 
                                TransactionRecord* records, 
                                int* count, 
                                int max_records);

/**
 * @brief Get account number from card number
 * @param card_number Card number
 * @param account_number_out Output parameter for account number
 * @param account_number_size Size of account_number_out buffer
 * @return true if successful, false otherwise
 */
bool cbs_get_account_by_card(const char* card_number, 
                           char* account_number_out, 
                           size_t account_number_size);

/**
 * @brief Get balance by card number
 * @param card_number Card number
 * @param balance_out Output parameter for balance
 * @return true if successful, false otherwise
 */
bool cbs_get_balance_by_card(const char* card_number, double* balance_out);

/**
 * @brief Get detailed information about a card
 * @param card_number Card number to query
 * @param holder_name_out Output buffer for cardholder name
 * @param holder_name_size Size of holder_name_out buffer
 * @param account_number_out Output buffer for account number
 * @param account_number_size Size of account_number_out buffer
 * @param expiry_date_out Output buffer for expiry date
 * @param expiry_date_size Size of expiry_date_out buffer
 * @param is_active_out Output parameter for card active status
 * @param daily_limit_out Output parameter for daily transaction limit
 * @param card_type_out Output buffer for card type
 * @param card_type_size Size of card_type_out buffer
 * @return true if successful, false otherwise
 */
bool cbs_get_card_details(int card_number,
                         char* holder_name_out, size_t holder_name_size,
                         char* account_number_out, size_t account_number_size,
                         char* expiry_date_out, size_t expiry_date_size,
                         int* is_active_out,
                         double* daily_limit_out,
                         char* card_type_out, size_t card_type_size);

#endif /* CORE_BANKING_INTERFACE_H */
