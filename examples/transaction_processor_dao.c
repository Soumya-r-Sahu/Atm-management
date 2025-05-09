/**
 * @file transaction_processor_dao.c
 * @brief Example file showing how to modify transaction processors to use DAO
 * @version 1.0
 * @date May 11, 2025
 */

#include "../../../../../include/atm/transaction/transaction_processor.h"
#include "../../../../../include/common/database/dao_interface.h"  // Include DAO interface
#include "../../../../../include/common/utils/logger.h"
#include "../../../../../include/common/transaction/bill_payment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Get DAO instance from factory
extern DatabaseAccessObject* getDAO(void);

// Process balance inquiry using DAO pattern
TransactionResult process_balance_inquiry(int card_number) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();
    
    // Check if card exists
    if (!dao->doesCardExist(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!dao->isCardActive(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is blocked or inactive");
        return result;
    }
    
    // Get account balance
    float balance = dao->fetchBalance(card_number);
    
    if (balance < 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to retrieve balance");
        return result;
    }
    
    // Get customer name for the receipt
    char customerName[100];
    if (dao->getCardHolderName(card_number, customerName, sizeof(customerName))) {
        // Set balance in result
        result.balance = balance;
        result.status = TRANSACTION_SUCCESS;
        sprintf(result.message, "Balance inquiry successful. Current balance: %.2f", balance);
        
        // Log the transaction
        dao->logTransaction(card_number, "Balance Inquiry", 0.0f, true);
    } else {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to retrieve account details");
    }
    
    return result;
}

// Process withdrawal using DAO pattern
TransactionResult process_withdrawal(int card_number, float amount) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();
    
    // Validate card number
    if (!dao->doesCardExist(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!dao->isCardActive(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is blocked or inactive");
        return result;
    }
    
    // Check for valid amount
    if (amount <= 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid withdrawal amount");
        return result;
    }
    
    // Get current balance
    float current_balance = dao->fetchBalance(card_number);
    
    if (current_balance < 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to retrieve balance");
        return result;
    }
    
    // Check if sufficient funds available
    if (current_balance < amount) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Insufficient funds");
        
        // Log the failed transaction
        dao->logTransaction(card_number, "Withdrawal", amount, false);
        return result;
    }
    
    // Check daily withdrawal limit
    float daily_withdrawals = dao->getDailyWithdrawals(card_number);
    if (daily_withdrawals + amount > 10000.0f) {  // 10,000 limit
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Daily withdrawal limit exceeded");
        
        // Log the failed transaction
        dao->logTransaction(card_number, "Withdrawal", amount, false);
        return result;
    }
    
    // Update account balance
    float new_balance = current_balance - amount;
    if (!dao->updateBalance(card_number, new_balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update account balance");
        
        // Log the failed transaction
        dao->logTransaction(card_number, "Withdrawal", amount, false);
        return result;
    }
    
    // Log the withdrawal
    dao->logWithdrawal(card_number, amount);
    
    // Set result values
    result.status = TRANSACTION_SUCCESS;
    result.amount = amount;
    result.balance = new_balance;
    sprintf(result.message, "Withdrawal successful. Amount: %.2f, New balance: %.2f", amount, new_balance);
    
    return result;
}

// Process deposit using DAO pattern
TransactionResult process_deposit(int card_number, float amount) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();
    
    // Validate card number
    if (!dao->doesCardExist(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!dao->isCardActive(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is blocked or inactive");
        return result;
    }
    
    // Check for valid amount
    if (amount <= 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid deposit amount");
        return result;
    }
    
    // Get current balance
    float current_balance = dao->fetchBalance(card_number);
    
    if (current_balance < 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to retrieve balance");
        return result;
    }
    
    // Update account balance
    float new_balance = current_balance + amount;
    if (!dao->updateBalance(card_number, new_balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update account balance");
        
        // Log the failed transaction
        dao->logTransaction(card_number, "Deposit", amount, false);
        return result;
    }
    
    // Log the successful transaction
    dao->logTransaction(card_number, "Deposit", amount, true);
    
    // Set result values
    result.status = TRANSACTION_SUCCESS;
    result.amount = amount;
    result.balance = new_balance;
    sprintf(result.message, "Deposit successful. Amount: %.2f, New balance: %.2f", amount, new_balance);
    
    return result;
}

// Process mini statement using DAO pattern
TransactionResult process_mini_statement(int card_number) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();
    
    // Validate card number
    if (!dao->doesCardExist(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!dao->isCardActive(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is blocked or inactive");
        return result;
    }
    
    // Get mini statement (last 5 transactions)
    Transaction transactions[5];
    int count = 0;
    
    bool success = dao->getMiniStatement(card_number, transactions, 5, &count);
    
    if (!success || count == 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "No transactions found or error retrieving mini statement");
        return result;
    }
    
    // Set result values
    result.status = TRANSACTION_SUCCESS;
    sprintf(result.message, "Mini statement generated successfully with %d transactions", count);
    
    // In a real implementation, the transactions would be attached to the result
    // or processed further here
    
    // Log this request as a transaction
    dao->logTransaction(card_number, "Mini Statement", 0.0f, true);
    
    return result;
}

// Process PIN change using DAO pattern
TransactionResult process_pin_change(int card_number, int old_pin, int new_pin) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();
    
    // Validate card number
    if (!dao->doesCardExist(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!dao->isCardActive(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is blocked or inactive");
        return result;
    }
    
    // Validate old PIN
    if (!dao->validateCard(card_number, old_pin)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid PIN");
        
        // Log failed PIN change attempt
        dao->logTransaction(card_number, "PIN Change", 0.0f, false);
        return result;
    }
    
    // Validate new PIN format
    if (new_pin < 1000 || new_pin > 9999) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "New PIN must be a 4-digit number");
        return result;
    }
    
    // In a real implementation, hash the PIN before storing
    char newPINHash[100];
    sprintf(newPINHash, "hash_%04d", new_pin);  // Dummy hash
    
    // Update PIN
    if (!dao->updateCardPIN(card_number, newPINHash)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update PIN");
        
        // Log failed PIN change
        dao->logTransaction(card_number, "PIN Change", 0.0f, false);
        return result;
    }
    
    // Log successful PIN change
    dao->logTransaction(card_number, "PIN Change", 0.0f, true);
    
    // Set result values
    result.status = TRANSACTION_SUCCESS;
    strcpy(result.message, "PIN changed successfully");
    
    return result;
}

// Process card blocking using DAO pattern
TransactionResult process_block_card(int card_number, int reason_code) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();
    
    // Validate card number
    if (!dao->doesCardExist(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Block the card
    if (!dao->blockCard(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to block card");
        return result;
    }
    
    // Log the card blocking
    char reason[50];
    switch (reason_code) {
        case 1:
            strcpy(reason, "Lost card");
            break;
        case 2:
            strcpy(reason, "Stolen card");
            break;
        case 3:
            strcpy(reason, "Suspicious activity");
            break;
        default:
            strcpy(reason, "Other reason");
            break;
    }
    
    // Log as transaction
    dao->logTransaction(card_number, "Card Block", 0.0f, true);
    
    // Set result values
    result.status = TRANSACTION_SUCCESS;
    sprintf(result.message, "Card blocked successfully. Reason: %s", reason);
    
    return result;
}
