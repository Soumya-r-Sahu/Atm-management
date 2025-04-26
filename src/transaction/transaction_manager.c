#include "transaction_manager.h"
#include "../database/database.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TRANSACTION_LOG_FILE "../data/transaction_history.txt"
#define MAX_TRANSACTIONS_IN_MINI_STATEMENT 5

// Helper function to get current timestamp as a formatted string
static void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Check account balance
TransactionResult checkAccountBalance(int cardNumber, const char* username) {
    TransactionResult result = {0};
    
    float balance = fetchBalance(cardNumber);
    if (balance >= 0) {
        result.success = 1;
        result.newBalance = balance;
        sprintf(result.message, "Current balance: $%.2f", balance);
        
        logTransaction(cardNumber, TRANSACTION_BALANCE_CHECK, 0.0, 1);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch balance");
        
        logTransaction(cardNumber, TRANSACTION_BALANCE_CHECK, 0.0, 0);
    }
    
    return result;
}

// Perform deposit operation
TransactionResult performDeposit(int cardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid deposit amount");
        logTransaction(cardNumber, TRANSACTION_DEPOSIT, amount, 0);
        return result;
    }
    
    // Fetch current balance
    float oldBalance = fetchBalance(cardNumber);
    if (oldBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch account balance");
        logTransaction(cardNumber, TRANSACTION_DEPOSIT, amount, 0);
        return result;
    }
    
    // Perform deposit
    float newBalance = oldBalance + amount;
    if (updateBalance(cardNumber, newBalance)) {
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Deposit successful. New balance: $%.2f", newBalance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Deposited $%.2f. Old balance: $%.2f, New balance: $%.2f", 
               amount, oldBalance, newBalance);
        writeTransactionLog(username, "Deposit", detailsLog);
        
        logTransaction(cardNumber, TRANSACTION_DEPOSIT, amount, 1);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to update balance");
        logTransaction(cardNumber, TRANSACTION_DEPOSIT, amount, 0);
    }
    
    return result;
}

// Perform withdrawal operation
TransactionResult performWithdrawal(int cardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid withdrawal amount");
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 0);
        return result;
    }
    
    // Check for daily withdrawal limit
    float dailyWithdrawals = getDailyWithdrawals(cardNumber);
    if (dailyWithdrawals + amount > 1000) { // Example: $1000 daily limit
        result.success = 0;
        sprintf(result.message, "Error: Daily withdrawal limit exceeded. Remaining: $%.2f", 
               1000.0f - dailyWithdrawals);
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 0);
        return result;
    }
    
    // Fetch current balance
    float oldBalance = fetchBalance(cardNumber);
    if (oldBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch account balance");
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 0);
        return result;
    }
    
    // Check if sufficient funds
    if (oldBalance < amount) {
        result.success = 0;
        sprintf(result.message, "Error: Insufficient funds. Current balance: $%.2f", oldBalance);
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 0);
        return result;
    }
    
    // Perform withdrawal
    float newBalance = oldBalance - amount;
    if (updateBalance(cardNumber, newBalance)) {
        // Log the withdrawal for daily limit tracking
        logWithdrawal(cardNumber, amount);
        
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Withdrawal successful. New balance: $%.2f", newBalance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Withdrew $%.2f. Old balance: $%.2f, New balance: $%.2f", 
               amount, oldBalance, newBalance);
        writeTransactionLog(username, "Withdrawal", detailsLog);
        
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 1);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to update balance");
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 0);
    }
    
    return result;
}

// Perform money transfer between accounts
TransactionResult performMoneyTransfer(int senderCardNumber, int receiverCardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    
    // Validate transfer amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid transfer amount");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Check if receiver card exists
    if (!isCardValid(receiverCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Recipient card number is invalid");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Get sender's balance
    float senderBalance = fetchBalance(senderCardNumber);
    if (senderBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch sender's balance");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Check if sender has sufficient funds
    if (senderBalance < amount) {
        result.success = 0;
        strcpy(result.message, "Error: Insufficient funds for transfer");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Get receiver's balance
    float receiverBalance = fetchBalance(receiverCardNumber);
    if (receiverBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch recipient's balance");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Update sender's balance
    float newSenderBalance = senderBalance - amount;
    if (!updateBalance(senderCardNumber, newSenderBalance)) {
        result.success = 0;
        strcpy(result.message, "Error: Failed to update sender's balance");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Update receiver's balance
    float newReceiverBalance = receiverBalance + amount;
    if (!updateBalance(receiverCardNumber, newReceiverBalance)) {
        // Rollback sender's balance if receiver update fails
        updateBalance(senderCardNumber, senderBalance);
        result.success = 0;
        strcpy(result.message, "Error: Failed to update recipient's balance");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Set success result
    result.success = 1;
    result.oldBalance = senderBalance;
    result.newBalance = newSenderBalance;
    sprintf(result.message, "Successfully transferred $%.2f to card %d. Your new balance: $%.2f", 
            amount, receiverCardNumber, newSenderBalance);
            
    // Log successful transaction
    logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 1);
    
    // Write to transaction history file
    FILE *file = fopen(TRANSACTION_LOG_FILE, "a");
    if (file) {
        char timestamp[20];
        getCurrentTimestamp(timestamp, sizeof(timestamp));
        fprintf(file, "%d,%s,TRANSFER,$%.2f,%d,SUCCESS\n", 
                senderCardNumber, timestamp, amount, receiverCardNumber);
        fclose(file);
    }
    
    return result;
}

// Get mini statement (recent transactions)
TransactionResult getMiniStatement(int cardNumber, const char* username) {
    TransactionResult result = {0};
    char buffer[1000] = "";
    int count = 0;
    
    // Set up the initial values
    result.success = 1;
    result.newBalance = fetchBalance(cardNumber);
    
    // Open transaction history file
    FILE *file = fopen(TRANSACTION_LOG_FILE, "r");
    if (file == NULL) {
        result.success = 0;
        strcpy(result.message, "No transaction history available");
        return result;
    }
    
    // Get the most recent transactions for this card number
    char line[256];
    char recentTransactions[MAX_TRANSACTIONS_IN_MINI_STATEMENT][256] = {0};
    while (fgets(line, sizeof(line), file) != NULL) {
        char storedCardNumStr[20];
        sprintf(storedCardNumStr, "Card: %d", cardNumber);
        
        if (strstr(line, storedCardNumStr) != NULL) {
            // This is a transaction for our card number
            strcpy(recentTransactions[count % MAX_TRANSACTIONS_IN_MINI_STATEMENT], line);
            count++;
        }
    }
    
    fclose(file);
    
    // Format the mini statement
    if (count == 0) {
        strcpy(result.message, "No transaction history available for this card");
    } else {
        int start = (count <= MAX_TRANSACTIONS_IN_MINI_STATEMENT) ? 
                   0 : count % MAX_TRANSACTIONS_IN_MINI_STATEMENT;
        
        int displayed = 0;
        for (int i = 0; i < MAX_TRANSACTIONS_IN_MINI_STATEMENT && displayed < count; i++) {
            int idx = (start + i) % MAX_TRANSACTIONS_IN_MINI_STATEMENT;
            if (recentTransactions[idx][0] != '\0') {
                strcat(buffer, recentTransactions[idx]);
                displayed++;
            }
        }
        
        strcpy(result.message, buffer);
    }
    
    logTransaction(cardNumber, TRANSACTION_MINI_STATEMENT, 0.0, 1);
    return result;
}

// Log transaction details to the transaction history file
void logTransaction(int cardNumber, TransactionType type, float amount, int success) {
    FILE *file = fopen(TRANSACTION_LOG_FILE, "a");
    if (file == NULL) {
        writeErrorLog("Failed to open transaction history file");
        return;
    }
    
    // Get current timestamp
    char timestamp[30];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    
    // Determine transaction type string
    const char* typeStr;
    switch (type) {
        case TRANSACTION_BALANCE_CHECK:
            typeStr = "Balance Check";
            break;
        case TRANSACTION_DEPOSIT:
            typeStr = "Deposit";
            break;
        case TRANSACTION_WITHDRAWAL:
            typeStr = "Withdrawal";
            break;
        case TRANSACTION_PIN_CHANGE:
            typeStr = "PIN Change";
            break;
        case TRANSACTION_MINI_STATEMENT:
            typeStr = "Mini Statement";
            break;
        default:
            typeStr = "Unknown";
    }
    
    // Format the log entry
    fprintf(file, "[%s] Card: %d | Type: %s | Amount: $%.2f | Status: %s\n",
           timestamp, cardNumber, typeStr, amount, success ? "Success" : "Failed");
    
    fclose(file);
}

// Generate a transaction receipt
void generateReceipt(int cardNumber, TransactionType type, float amount, float balance, const char* phoneNumber) {
    printf("\n======= TRANSACTION RECEIPT =======\n");
    printf("Date/Time: ");
    
    // Print current timestamp
    char timestamp[30];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    printf("%s\n", timestamp);
    
    printf("Card Number: xxxx-xxxx-%d\n", cardNumber % 10000);
    
    // Print transaction details based on type
    switch (type) {
        case TRANSACTION_BALANCE_CHECK:
            printf("Transaction: Balance Inquiry\n");
            printf("Current Balance: $%.2f\n", balance);
            break;
            
        case TRANSACTION_DEPOSIT:
            printf("Transaction: Deposit\n");
            printf("Amount: $%.2f\n", amount);
            printf("Current Balance: $%.2f\n", balance);
            break;
            
        case TRANSACTION_WITHDRAWAL:
            printf("Transaction: Withdrawal\n");
            printf("Amount: $%.2f\n", amount);
            printf("Current Balance: $%.2f\n", balance);
            break;
            
        case TRANSACTION_PIN_CHANGE:
            printf("Transaction: PIN Change\n");
            printf("Status: Successful\n");
            printf("Current Balance: $%.2f\n", balance);
            break;
            
        case TRANSACTION_MINI_STATEMENT:
            printf("Transaction: Mini Statement\n");
            printf("Current Balance: $%.2f\n", balance);
            break;
    }
    
    // Print confirmation message for SMS
    if (phoneNumber != NULL && phoneNumber[0] != '\0') {
        printf("\nA confirmation SMS has been sent to ");
        printf("your registered mobile number ending with ");
        
        // Print last 4 digits of phone number
        size_t len = strlen(phoneNumber);
        if (len >= 4) {
            printf("%c%c%c%c\n", 
                  phoneNumber[len-4], phoneNumber[len-3], 
                  phoneNumber[len-2], phoneNumber[len-1]);
        } else {
            printf("%s\n", phoneNumber);
        }
    }
    
    printf("===================================\n");
}