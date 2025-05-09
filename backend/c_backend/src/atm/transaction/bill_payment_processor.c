/**
 * @file bill_payment_processor.c
 * @brief Implementation of bill payment processor for ATM
 */

#include "../../../include/atm/transaction/transaction_processor.h"
#include "../../../include/common/database/card_account_management.h"
#include "../../../include/common/database/account_management.h"
#include "../../../include/common/transaction/bill_payment.h"
#include "../../../include/common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Process a bill payment
 * @param card_number Card number
 * @param bill_type Bill type (ELECTRICITY, WATER, etc.)
 * @param bill_reference Bill reference number
 * @param amount Payment amount
 * @return Transaction result
 */
TransactionResult process_bill_payment(int card_number, const char* bill_type, const char* bill_reference, float amount) {
    TransactionResult result = {0};
    
    // Check parameters
    if (!bill_type || !bill_reference || amount <= 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid bill payment parameters");
        return result;
    }
    
    // Check if card exists using CBS function
    if (!cbs_card_exists(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active using CBS function
    if (!cbs_is_card_active(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Get the account balance
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", card_number);
    
    double balance = 0.0;
    if (!cbs_get_balance_by_card(card_number_str, &balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Check if sufficient balance
    if (balance < amount) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Insufficient funds for bill payment");
        writeInfoLog("Bill payment failed due to insufficient funds: Card %d, Amount %.2f, Balance %.2f", 
                     card_number, amount, balance);
        return result;
    }
    
    // Store balance before transaction
    result.balance_before = balance;
    
    // Process the bill payment
    char transaction_id[37] = {0};
    if (!cbs_process_bill_payment(card_number_str, bill_type, bill_reference, amount, transaction_id)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Bill payment processing failed");
        writeErrorLog("Bill payment failed: Card %d, Bill Type %s, Ref %s, Amount %.2f", 
                     card_number, bill_type, bill_reference, amount);
        return result;
    }
    
    // Get updated balance
    if (!cbs_get_balance_by_card(card_number_str, &balance)) {
        // Payment succeeded but couldn't fetch new balance
        result.status = TRANSACTION_SUCCESS;
        result.amount_processed = amount;
        result.balance_after = result.balance_before - amount; // Estimate new balance
        sprintf(result.message, "Bill payment successful. Transaction ID: %s", transaction_id);
        writeInfoLog("Bill payment successful: Card %d, Bill Type %s, Ref %s, Amount %.2f, Transaction ID %s", 
                    card_number, bill_type, bill_reference, amount, transaction_id);
        return result;
    }
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = amount;
    result.balance_after = balance;
    sprintf(result.message, "Bill payment successful. Transaction ID: %s", transaction_id);
    
    writeInfoLog("Bill payment successful: Card %d, Bill Type %s, Ref %s, Amount %.2f, Transaction ID %s", 
                card_number, bill_type, bill_reference, amount, transaction_id);
                
    return result;
}
