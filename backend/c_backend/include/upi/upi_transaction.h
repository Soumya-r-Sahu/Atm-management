/**
 * @file upi_transaction.h
 * @brief Core UPI Transaction functionality for the ATM Management System
 * @date May 1, 2025
 */

#ifndef UPI_TRANSACTION_H
#define UPI_TRANSACTION_H

#include <stdbool.h>
#include <time.h>
#include <common/database/database.h>

/**
 * @brief Status codes for UPI operations
 */
typedef enum {
    UPI_SUCCESS,
    UPI_INVALID_VPA,
    UPI_AUTHENTICATION_FAILED,
    UPI_TRANSACTION_FAILED,
    UPI_INSUFFICIENT_BALANCE,
    UPI_CONNECTION_ERROR,
    UPI_LIMIT_EXCEEDED,
    UPI_SYSTEM_ERROR,
    UPI_INVALID_AMOUNT,
    UPI_DUPLICATE_TRANSACTION
} UpiStatus;

/**
 * @brief Structure to store UPI virtual payment address information
 */
typedef struct {
    char vpa_id[50];               // Virtual Payment Address (username@bankcode)
    char account_number[20];        // Linked bank account number
    char account_holder[50];        // Account holder name
    bool is_active;                 // Whether the VPA is active
    time_t creation_date;           // When the VPA was created
    char linked_mobile[15];         // Mobile number linked to VPA
} UpiVirtualAddress;

/**
 * @brief Structure to store UPI transaction details
 */
typedef struct {
    int transaction_id;             // Unique transaction ID
    char sender_vpa[50];            // Sender's virtual payment address
    char receiver_vpa[50];          // Receiver's virtual payment address
    char sender_account[20];        // Sender's account number
    char receiver_account[20];      // Receiver's account number
    double amount;                  // Transaction amount
    time_t timestamp;               // Transaction timestamp
    char status[20];                // Transaction status (SUCCESS/FAILED/PENDING)
    char remarks[100];              // Transaction remarks
    char reference_id[20];          // Reference ID for transaction
    char upi_pin_hash[100];         // Hash of the UPI PIN (only used during validation)
} UpiTransaction;

/**
 * @brief Initialize the UPI transaction system
 * @return true if initialization is successful, false otherwise
 */
bool upi_init(void);

/**
 * @brief Register a new UPI VPA for a customer
 * @param account_number Bank account number to link
 * @param vpa_prefix User-chosen VPA prefix (before @)
 * @param bank_code Bank code to use in VPA
 * @param mobile_number Mobile number to link with VPA
 * @param upi_pin UPI PIN for transactions
 * @param vpa_out Output parameter to return the created VPA
 * @return UpiStatus indicating the result of registration
 */
UpiStatus upi_register_vpa(
    const char* account_number,
    const char* vpa_prefix,
    const char* bank_code,
    const char* mobile_number,
    const char* upi_pin,
    char* vpa_out
);

/**
 * @brief Authenticate a UPI transaction with PIN
 * @param vpa The VPA attempting to make a transaction
 * @param upi_pin The UPI PIN for authentication
 * @return UpiStatus indicating authentication result
 */
UpiStatus upi_authenticate(const char* vpa, const char* upi_pin);

/**
 * @brief Execute a UPI money transfer
 * @param sender_vpa Sender's VPA
 * @param receiver_vpa Receiver's VPA
 * @param amount Amount to transfer
 * @param upi_pin Sender's UPI PIN for authentication
 * @param remarks Optional transaction remarks
 * @param transaction_id Output parameter to return the transaction ID
 * @return UpiStatus indicating the result of the transaction
 */
UpiStatus upi_transfer(
    const char* sender_vpa,
    const char* receiver_vpa,
    double amount,
    const char* upi_pin,
    const char* remarks,
    int* transaction_id
);

/**
 * @brief Check balance of account linked to a VPA
 * @param vpa The VPA to check balance for
 * @param upi_pin The UPI PIN for authentication
 * @param balance Output parameter to return the current balance
 * @return UpiStatus indicating the result of balance check
 */
UpiStatus upi_check_balance(const char* vpa, const char* upi_pin, double* balance);

/**
 * @brief Validate if a VPA exists in the system
 * @param vpa The VPA to validate
 * @param exists Output parameter (true if VPA exists)
 * @param account_holder Output parameter to return account holder's name if VPA exists
 * @return UpiStatus indicating the result of validation
 */
UpiStatus upi_validate_vpa(const char* vpa, bool* exists, char* account_holder);

/**
 * @brief Get transaction history for a VPA
 * @param vpa The VPA to get history for
 * @param upi_pin The UPI PIN for authentication
 * @param transactions Array to store transaction records
 * @param max_count Maximum number of transactions to retrieve
 * @param actual_count Output parameter to return actual number of transactions retrieved
 * @return UpiStatus indicating the result of the operation
 */
UpiStatus upi_get_transaction_history(
    const char* vpa,
    const char* upi_pin,
    UpiTransaction* transactions,
    int max_count,
    int* actual_count
);

/**
 * @brief Change UPI PIN
 * @param vpa The VPA to change PIN for
 * @param old_pin Current UPI PIN
 * @param new_pin New UPI PIN to set
 * @return UpiStatus indicating the result of PIN change
 */
UpiStatus upi_change_pin(const char* vpa, const char* old_pin, const char* new_pin);

/**
 * @brief Deactivate a UPI VPA
 * @param vpa The VPA to deactivate
 * @param upi_pin The UPI PIN for authentication
 * @return UpiStatus indicating the result of deactivation
 */
UpiStatus upi_deactivate_vpa(const char* vpa, const char* upi_pin);

#endif /* UPI_TRANSACTION_H */