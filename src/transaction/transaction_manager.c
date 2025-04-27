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
    
    // Perform deposit by updating balance
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
    if (withdrawalLimit <= 0) {
        withdrawalLimit = 25000; // Default limit if not configured
    }

    // Get daily transaction limit from system configuration
    int dailyLimit = getConfigValueInt(CONFIG_DAILY_TRANSACTION_LIMIT);
    if (dailyLimit <= 0) {
        dailyLimit = 50000; // Default limit if not configured
    }
    
    // Validate withdrawal amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid withdrawal amount");
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 0);
        return result;
    }
    
    // Check if amount exceeds the ATM withdrawal limit
    if (amount > withdrawalLimit) {
        result.success = 0;
        sprintf(result.message, "Error: Amount exceeds withdrawal limit of $%d", withdrawalLimit);
        logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 0);
        return result;
    }
    
    // Check if amount exceeds daily withdrawal limit
    float todayWithdrawals = getDailyWithdrawals(cardNumber);
    if (todayWithdrawals + amount > dailyLimit) {
        result.success = 0;
        sprintf(result.message, "Error: Would exceed daily transaction limit of $%d", dailyLimit);
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
    FILE* cardFile = fopen(getCardFilePath(), "r");
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
        sprintf(accountID, "A%d", cardNumber);
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
    strcat(miniStatement, "\n");
    strcat(miniStatement, "Date       | Type        | Amount    | Status\n");
    strcat(miniStatement, "-------------------------------------\n");
    
    // Start from the most recent transaction (if more than 5, show only the last 5)
    int start = (count <= 5) ? 0 : count - 5;
    for (int i = start; i < count; i++) {
        char transactionID[10], accountIDFromLog[10], transactionType[20], statusStr[10], remarks[50];
        float amount;
        char timestamp[30];
        
        // Extract transaction details (simplified parsing)
        if (sscanf(transactions[i % 5], "%s | %s | %s | %f | %s | %s | %s",
                  transactionID, accountIDFromLog, transactionType, &amount,
                  timestamp, statusStr, remarks) >= 7) {
                  
            // Format date to show only the date part
            char dateStr[15];
            strncpy(dateStr, timestamp, 10);
            dateStr[10] = '\0';
            
            char formattedTransaction[100];
            sprintf(formattedTransaction, "%s | %-10s | $%-8.2f | %s\n",
                   dateStr, transactionType, amount, statusStr);
            strcat(miniStatement, formattedTransaction);
        }
    }
    
    strcat(miniStatement, "-------------------------------------\n");
    sprintf(line, "Current Balance: $%.2f", balance);
    strcat(miniStatement, line);
    
    fclose(file);
    
    result.success = 1;
    result.newBalance = balance;
    result.oldBalance = balance;
    strcpy(result.message, miniStatement);
    
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
    
    // Fetch sender balance
    float senderBalance = fetchBalance(senderCardNumber);
    if (senderBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch your account balance");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Check if sender has sufficient funds
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
        
        // Also log for recipient
        char recipientName[50] = "Unknown"; // Default if we can't find name
        getCardHolderName(receiverCardNumber, recipientName, sizeof(recipientName));
        
        char recipientLog[100];
        sprintf(recipientLog, "Received $%.2f from card %d (%s)", amount, senderCardNumber, username);
        writeTransactionDetails(recipientName, "Money Received", recipientLog);
        
        logTransaction(receiverCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 1);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Failed to complete transfer");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
    }
    
    return result;
}

// Alias for performMoneyTransfer to maintain backward compatibility
TransactionResult performFundTransfer(int cardNumber, int targetCardNumber, float amount, const char* username) {
    return performMoneyTransfer(cardNumber, targetCardNumber, amount, username);
}

// Function to log transaction details to the audit log
void writeTransactionDetails(const char* username, const char* type, const char* details) {
    char logEntry[300];
    sprintf(logEntry, "USER: %s | ACTION: %s | DETAILS: %s", 
            username, type, details);
    
    // Write to the audit log
    writeAuditLog("TRANSACTION", logEntry);
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
        case TRANSACTION_PIN_CHANGE: 
            strcpy(typeStr, "PIN Change"); 
            break;
        default: 
            strcpy(typeStr, "Unknown"); 
            break;
    }
    
    // Get date and time for receipt
    char timestamp[30];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    
    // Get card holder's name
    char holderName[50] = "Customer"; // Default if we can't get name
    getCardHolderName(cardNumber, holderName, sizeof(holderName));
    
    // Log that receipt was generated
    char logMsg[100];
    sprintf(logMsg, "Receipt generated for %s (Card: %d, Phone: %s)", 
           holderName, cardNumber, phoneNumber);
    writeAuditLog("RECEIPT", logMsg);
    
    // In a real system, we'd send an SMS receipt to phoneNumber
    // For our test implementation, we'll just print to console
    printf("\n=========== RECEIPT ==========\n");
    printf("Date: %s\n", timestamp);
    printf("Card: **** **** **** %04d\n", cardNumber % 10000);
    printf("Transaction: %s\n", typeStr);
    if (amount > 0) {
        printf("Amount: $%.2f\n", amount);
    }
    printf("Current Balance: $%.2f\n", balance);
    printf("============================\n");
    printf("Thank you for using our ATM!\n");
}