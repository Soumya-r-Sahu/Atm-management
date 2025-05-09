#ifndef CORE_BANKING_ADAPTER_H
#define CORE_BANKING_ADAPTER_H

#include <stdbool.h>
#include "../../../include/atm/transaction/transaction_types.h"

/**
 * @brief Adapter for fetching account balance using core banking system
 * @param card_number Card number
 * @param balance Output parameter for the balance
 * @return true if successful, false otherwise
 */
bool cbs_adapter_fetch_balance(int card_number, float* balance);

/**
 * @brief Adapter for updating balance (DEPRECATED - use cbs_adapter_process_transaction)
 * @param card_number Card number
 * @param new_balance New balance
 * @return true if successful, false otherwise
 */
bool cbs_adapter_update_balance(int card_number, float new_balance);

/**
 * @brief Adapter for processing a transaction using core banking system
 * @param card_number Card number
 * @param type Transaction type from TransactionType enum
 * @param channel Transaction channel (e.g., "ATM", "POS")
 * @param amount Transaction amount
 * @param transaction_id_out Output parameter for transaction ID
 * @return true if successful, false otherwise
 */
bool cbs_adapter_process_transaction(int card_number, TransactionType type, 
                                    const char* channel, float amount, 
                                    char* transaction_id_out);

/**
 * @brief Adapter for transferring funds using core banking system
 * @param source_card Source card number
 * @param dest_card Destination card number
 * @param amount Transfer amount
 * @param transaction_id_out Output parameter for transaction ID
 * @return true if successful, false otherwise
 */
bool cbs_adapter_transfer_funds(int source_card, int dest_card, float amount, 
                              char* transaction_id_out);

/**
 * @brief Adapter for checking withdrawal limits using core banking system
 * @param card_number Card number
 * @param amount Withdrawal amount
 * @param remaining_limit Output parameter for remaining withdrawal limit
 * @return true if withdrawal is allowed, false otherwise
 */
bool cbs_adapter_check_withdrawal_limit(int card_number, float amount, double* remaining_limit);

/**
 * @brief Adapter for getting mini statement using core banking system
 * @param card_number Card number
 * @param atm_transactions Array to store transaction records
 * @param count Output parameter for number of records found
 * @param max_records Maximum number of records to retrieve
 * @return true if successful, false otherwise
 */
bool cbs_adapter_get_mini_statement(int card_number, Transaction* atm_transactions, 
                                 int* count, int max_records);

/**
 * @brief Adapter for blocking a card using core banking system
 * @param card_number Card number to block
 * @return true if successful, false otherwise
 */
bool cbs_adapter_block_card(int card_number);

/**
 * @brief Adapter for unblocking a card using core banking system
 * @param card_number Card number to unblock
 * @return true if successful, false otherwise
 */
bool cbs_adapter_unblock_card(int card_number);

#endif /* CORE_BANKING_ADAPTER_H */
