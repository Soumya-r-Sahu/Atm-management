#include "transaction_manager.h"
#include "../database/database.h"
#include "../utils/logger.h"
#include "../utils/hash_utils.h"
#include "../common/paths.h"
#include "../database/customer_profile.h"
#include "../config/config_manager.h"  // Added config manager include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Define config constants if not defined elsewhere
#ifndef CONFIG_MAINTENANCE_MODE
#define CONFIG_MAINTENANCE_MODE "maintenance_mode"
#endif

#ifndef CONFIG_ATM_WITHDRAWAL_LIMIT
#define CONFIG_ATM_WITHDRAWAL_LIMIT "withdrawal_limit"
#endif

#ifndef CONFIG_DAILY_TRANSACTION_LIMIT
#define CONFIG_DAILY_TRANSACTION_LIMIT "daily_limit"
#endif

// Helper function to get current timestamp as a string
static void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Function to check account balance
TransactionResult checkAccountBalance(int cardNumber, const char* username) {
    TransactionResult result = {0};
    
    // Fetch balance from database
    float balance = fetchBalance(cardNumber);
    if (balance >= 0) {
        result.success = 1;
        result.newBalance = balance;
        result.oldBalance = balance;
        sprintf(result.message, "Current balance: $%.2f", balance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Checked balance: $%.2f", balance);
        writeTransactionDetails(username, "Balance Check", detailsLog);
        
        logTransaction(cardNumber, TRANSACTION_BALANCE_CHECK, 0.0f, 1);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch balance");
        
        logTransaction(cardNumber, TRANSACTION_BALANCE_CHECK, 0.0f, 0);
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
        writeTransactionDetails(username, "Deposit", detailsLog);
        
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
    
    // Check if ATM is in maintenance mode
    if (getConfigValueBool(CONFIG_MAINTENANCE_MODE)) {
        result.success = 0;
        strcpy(result.message, "Sorry, this ATM is currently in maintenance mode.");
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 0);
        return result;
    }

    // Get withdrawal limit from system configuration
    int withdrawalLimit = getConfigValueInt(CONFIG_ATM_WITHDRAWAL_LIMIT);
    if (withdrawalLimit < 0) withdrawalLimit = 25000; // Default if config not found

    // Get daily transaction limit
    int dailyLimit = getConfigValueInt(CONFIG_DAILY_TRANSACTION_LIMIT);
    if (dailyLimit < 0) dailyLimit = 50000; // Default if config not found
    
    // Check if amount exceeds the withdrawal limit
    if (amount > withdrawalLimit) {
        result.success = 0;
        sprintf(result.message, "Error: Amount exceeds withdrawal limit of $%d", withdrawalLimit);
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 0);
        return result;
    }
    
    // Check daily transaction total
    float dailyTotal = getDailyWithdrawals(cardNumber);
    if (dailyTotal + amount > dailyLimit) {
        result.success = 0;
        sprintf(result.message, "Error: Transaction would exceed your daily limit of $%d. Remaining limit: $%.2f", 
                dailyLimit, dailyLimit - dailyTotal);
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
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Withdrawal successful. New balance: $%.2f", newBalance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Withdrew $%.2f. Old balance: $%.2f, New balance: $%.2f", 
               amount, oldBalance, newBalance);
        writeTransactionDetails(username, "Withdrawal", detailsLog);
        
        // Track withdrawal for daily limit
        logWithdrawal(cardNumber, amount);
        
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 1);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to complete withdrawal");
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 0);
    }
    
    return result;
}

// Get mini statement (recent transactions)
TransactionResult getMiniStatement(int cardNumber, const char* username) {
    TransactionResult result = {0};
    
    // Set up transaction log file path
    char transactionsLogPath[200];
    sprintf(transactionsLogPath, "%s/transactions.log", 
            isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR "/../logs");
    
    FILE* file = fopen(transactionsLogPath, "r");
    if (file == NULL) {
        result.success = 0;
        strcpy(result.message, "No transaction history available.");
        logTransaction(cardNumber, TRANSACTION_MINI_STATEMENT, 0.0f, 0);
        return result;
    }
    
    // Get account ID from card number
    char accountID[10] = "";
    FILE* cardFile = fopen(CREDENTIALS_FILE, "r");
    if (cardFile != NULL) {
        char line[256];
        
        // Skip header lines
        fgets(line, sizeof(line), cardFile);
        fgets(line, sizeof(line), cardFile);
        
        char cardID[10], accID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], pinHash[65];
        
        while (fgets(line, sizeof(line), cardFile) != NULL) {
            if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                       cardID, accID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
                int storedCardNumber = atoi(cardNumberStr);
                if (storedCardNumber == cardNumber) {
                    strcpy(accountID, accID);
                    break;
                }
            }
        }
        
        fclose(cardFile);
    }
    
    // If account ID not found, use card number as a fallback
    if (strlen(accountID) == 0) {
        sprintf(accountID, "C%d", cardNumber);
    }
    
    // Count how many transactions are for this account
    int transactionCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Check if this line is for the current account ID
        if (strstr(line, accountID) != NULL) {
            transactionCount++;
        }
    }
    
    if (transactionCount == 0) {
        fclose(file);
        result.success = 0;
        strcpy(result.message, "No transaction history available for this account.");
        logTransaction(cardNumber, TRANSACTION_MINI_STATEMENT, 0.0f, 0);
        return result;
    }
    
    // Reset to beginning of file
    rewind(file);
    
    // Get current balance
    float balance = fetchBalance(cardNumber);
    if (balance < 0) {
        fclose(file);
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch current balance.");
        logTransaction(cardNumber, TRANSACTION_MINI_STATEMENT, 0.0f, 0);
        return result;
    }
    
    // Format mini statement with up to 5 most recent transactions
    char miniStatement[1024] = "Recent Transactions:\n";
    char transactions[5][256];
    int count = 0;
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, accountID) != NULL) {
            // Store this transaction
            strcpy(transactions[count % 5], line);
            count++;
        }
    }
    
    // Format the mini statement with the most recent transactions
    int startIdx = (count <= 5) ? 0 : count - 5;
    for (int i = startIdx; i < count; i++) {
        char transID[15], accID[10], transType[20], amountStr[15], timestamp[30], status[10], remarks[50];
        char formattedTrans[256];
        
        if (sscanf(transactions[i % 5], "%s | %s | %s | %s | %s | %s | %[^\n]", 
                   transID, accID, transType, amountStr, timestamp, status, remarks) >= 7) {
            sprintf(formattedTrans, "%-10s | %-8s | %8s | %s\n", 
                   transID, transType, amountStr, timestamp);
            strcat(miniStatement, formattedTrans);
        }
    }
    
    sprintf(miniStatement + strlen(miniStatement), 
           "\nCurrent Balance: $%.2f\n", balance);
    
    result.success = 1;
    result.newBalance = balance;
    result.oldBalance = balance;
    strcpy(result.message, miniStatement);
    
    fclose(file);
    logTransaction(cardNumber, TRANSACTION_MINI_STATEMENT, 0.0f, 1);
    
    // Log that mini-statement was requested
    char detailsLog[100];
    sprintf(detailsLog, "Mini statement requested. Current balance: $%.2f", balance);
    writeTransactionDetails(username, "Mini Statement", detailsLog);
    
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
    if (!doesCardExist(receiverCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Recipient card number is invalid");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Check if receiver card is active
    if (!isCardActive(receiverCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Recipient card is not active");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Get sender's balance
    float senderBalance = fetchBalance(senderCardNumber);
    if (senderBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch sender's account balance");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Check if sufficient funds
    if (senderBalance < amount) {
        result.success = 0;
        sprintf(result.message, "Error: Insufficient funds. Current balance: $%.2f", senderBalance);
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Perform the money transfer
    float receiverBalance = fetchBalance(receiverCardNumber);
    if (receiverBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch recipient's account balance");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Update balances of both sender and receiver
    if (updateBalance(senderCardNumber, senderBalance - amount) && 
        updateBalance(receiverCardNumber, receiverBalance + amount)) {
        result.success = 1;
        result.oldBalance = senderBalance;
        result.newBalance = senderBalance - amount;
        sprintf(result.message, "Transfer successful. Your new balance: $%.2f", result.newBalance);
        
        // Log the transaction for both accounts
        char detailsLog[100];
        sprintf(detailsLog, "Transferred $%.2f to card %d", amount, receiverCardNumber);
        writeTransactionDetails(username, "Money Transfer", detailsLog);
        
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 1);
        logTransaction(receiverCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 1);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to complete the transfer");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
    }
    
    return result;
}

// Function to write transaction log - renamed to avoid conflict with logger.h
void writeTransactionDetails(const char* username, const char* transactionType, const char* details) {
    // Get current timestamp
    char timestamp[30];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    
    // Format the log entry
    char logEntry[512];
    sprintf(logEntry, "[%s] User: %s | Type: %s | %s", 
           timestamp, username, transactionType, details);
    
    // Write to transaction log file
    char transactionsLogPath[200];
    sprintf(transactionsLogPath, "%s/transactions.log", 
            isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR "/../logs");
    
    FILE* file = fopen(transactionsLogPath, "a");
    if (file != NULL) {
        fprintf(file, "%s\n", logEntry);
        fclose(file);
    } else {
        // If transaction log file can't be opened, write to error log
        writeErrorLog("Failed to open transaction log file");
    }
}

// Function to generate and send a receipt
void generateReceipt(int cardNumber, TransactionType type, float amount, float balance, const char* phoneNumber) {
    char typeStr[30];
    
    // Convert transaction type enum to string
    switch (type) {
        case TRANSACTION_BALANCE_CHECK: 
            strcpy(typeStr, "Balance Check"); 
            break;
        case TRANSACTION_DEPOSIT: 
            strcpy(typeStr, "Deposit"); 
            break;
        case TRANSACTION_WITHDRAWAL: 
            strcpy(typeStr, "Withdrawal"); 
            break;
        case TRANSACTION_MONEY_TRANSFER: 
            strcpy(typeStr, "Money Transfer"); 
            break;
        case TRANSACTION_MINI_STATEMENT: 
            strcpy(typeStr, "Mini Statement"); 
            break;
        default: 
            strcpy(typeStr, "Transaction"); 
            break;
    }
    
    // Get current timestamp
    time_t now = time(NULL);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    // Get card holder's name
    char holderName[100];
    if (!getCardHolderName(cardNumber, holderName, sizeof(holderName))) {
        strcpy(holderName, "Customer");
    }
    
    // Format receipt
    char receipt[1024];
    sprintf(receipt, 
            "============= TRANSACTION RECEIPT =============\n"
            "Date/Time: %s\n"
            "Card Number: **** **** **** %04d\n"
            "Account Holder: %s\n"
            "Transaction Type: %s\n",
            timestamp,
            cardNumber % 10000,  // Last 4 digits of the card number
            holderName,
            typeStr);
    
    if (type != TRANSACTION_BALANCE_CHECK && type != TRANSACTION_MINI_STATEMENT) {
        sprintf(receipt + strlen(receipt), 
                "Amount: $%.2f\n", amount);
    }
    
    sprintf(receipt + strlen(receipt), 
            "Current Balance: $%.2f\n"
            "============================================\n"
            "Thank you for banking with us!\n",
            balance);
    
    // In a real implementation, we would send this receipt as an SMS to the phone number
    // For this simulation, we'll just write it to a log file
    printf("Receipt generated and ready to send to phone number: %s\n", phoneNumber);
    
    // Save receipt to file for testing purposes
    char receiptFileName[100];
    sprintf(receiptFileName, "%s/receipt_%d_%ld.txt", 
           isTestingMode() ? TEST_DATA_DIR "/temp" : PROD_DATA_DIR "/temp", 
           cardNumber, (long)time(NULL));
    
    FILE* file = fopen(receiptFileName, "w");
    if (file != NULL) {
        fprintf(file, "%s", receipt);
        fclose(file);
    } else {
        writeErrorLog("Failed to create receipt file");
    }
    
    // Log that receipt was generated
    char logMsg[100];
    sprintf(logMsg, "Receipt generated for card %d - %s", cardNumber, typeStr);
    writeAuditLog("RECEIPT", logMsg);
}

// Function to implement fund transfer (making it explicit)
TransactionResult performFundTransfer(int cardNumber, int targetCardNumber, float amount, const char* username) {
    // Delegate to the money transfer function
    return performMoneyTransfer(cardNumber, targetCardNumber, amount, username);
}