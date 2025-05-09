/**
 * @file bill_payment.h
 * @brief Header file for bill payment functionality
 */

#ifndef BILL_PAYMENT_H
#define BILL_PAYMENT_H

#include <stdbool.h>
#include "../../../include/common/database/core_banking_interface.h"

/**
 * @brief Process bill payment through the Core Banking System
 * @param card_number Card number
 * @param bill_type Type of bill (ELECTRICITY, WATER, etc.)
 * @param bill_reference Reference number for the bill
 * @param amount Amount to pay
 * @param transaction_id_out Output parameter for transaction ID
 * @return true if payment was successful, false otherwise
 */
bool cbs_process_bill_payment(const char* card_number, 
                              const char* bill_type,
                              const char* bill_reference,
                              double amount, 
                              char* transaction_id_out);

/**
 * @brief Get bill payment history for a card
 * @param card_number Card number
 * @param records Array to store payment records
 * @param count Output parameter for number of records
 * @param max_records Maximum number of records to retrieve
 * @return true if retrieval was successful, false otherwise
 */
bool cbs_get_bill_payment_history(const char* card_number,
                                 TransactionRecord* records,
                                 int* count,
                                 int max_records);

#endif /* BILL_PAYMENT_H */
