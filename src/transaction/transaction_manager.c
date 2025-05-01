#include <atm/transaction/transaction_manager.h>
#include <common/database/database.h>
#include <common/utils/logger.h>
#include <common/utils/hash_utils.h>
#include <common/paths.h>
#include <common/database/customer_profile.h>
#include <common/config/config_manager.h>
#include <atm/validation/card_validator.h>
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

// Define virtual ATM constants
#ifndef CONFIG_VIRTUAL_ATM_ENABLED
#define CONFIG_VIRTUAL_ATM_ENABLED "enable_virtual_atm"
#endif

#ifndef CONFIG_VIRTUAL_WITHDRAWAL_LIMIT
#define CONFIG_VIRTUAL_WITHDRAWAL_LIMIT "virtual_withdrawal_limit"
#endif

// Helper function to get current timestamp as a string
static void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Write detailed transaction information to log
void writeTransactionDetails(const char* username, const char* transactionType, const char* details) {
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Create a log message with all details
    char logMessage[512];
    snprintf(logMessage, sizeof(logMessage), "[%s] User: %s, Type: %s, Details: %s", 
             timestamp, username, transactionType, details);
    
    // Log to transaction file
    const char* transactionPath = isTestingMode() ? 
        TEST_TRANSACTIONS_LOG_FILE : PROD_TRANSACTIONS_LOG_FILE;
    
    FILE* file = fopen(transactionPath, "a");
    if (file != NULL) {
        fprintf(file, "%s\n", logMessage);
        fclose(file);
    } else {
        // If transaction log cannot be opened, fall back to error log
        writeErrorLog("Failed to write transaction details");
    }
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

// New function to check account balance with card number string
TransactionResult checkAccountBalanceByCardNumber(const char* cardNumber, const char* username) {
    TransactionResult result = {0};
    
    // Validate card format
    if (!validate_card_format(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid card number format");
        return result;
    }
    
    // Fetch card data and validate
    CardData* card = get_card_data(cardNumber);
    if (card == NULL) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        return result;
    }
    
    // Fetch balance from database using customer ID
    float balance = fetchBalanceByCustomerId(card->customer_id);
    free(card);  // Free card data after use
    
    if (balance >= 0) {
        result.success = 1;
        result.newBalance = balance;
        result.oldBalance = balance;
        sprintf(result.message, "Current balance: $%.2f", balance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Checked balance: $%.2f", balance);
        writeTransactionDetails(username, "Virtual Balance Check", detailsLog);
        
        logTransaction(0, TRANSACTION_BALANCE_CHECK, 0.0f, 1);  // Use 0 as placeholder for cardNumber
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch balance");
        
        logTransaction(0, TRANSACTION_BALANCE_CHECK, 0.0f, 0);
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

// New function for virtual deposit operation
TransactionResult performVirtualDeposit(const char* cardNumber, int cvv, const char* expiryDate, float amount, const char* username) {
    TransactionResult result = {0};
    
    // Check if virtual ATM is enabled
    if (!is_virtual_atm_enabled()) {
        result.success = 0;
        strcpy(result.message, "Error: Virtual ATM functionality is currently disabled");
        return result;
    }
    
    // Validate the virtual transaction
    CardValidationStatus status = validate_virtual_transaction(cardNumber, cvv, expiryDate);
    if (status != CARD_VALID) {
        result.success = 0;
        switch (status) {
            case CARD_INVALID_FORMAT:
                strcpy(result.message, "Error: Invalid card number format");
                break;
            case CARD_NOT_FOUND:
                strcpy(result.message, "Error: Card not found");
                break;
            case CARD_EXPIRED:
                strcpy(result.message, "Error: Card is expired");
                break;
            case CARD_CVV_INVALID:
                strcpy(result.message, "Error: Invalid CVV");
                break;
            case CARD_BLOCKED:
                strcpy(result.message, "Error: Card is blocked");
                break;
            default:
                strcpy(result.message, "Error: Card validation failed");
        }
        return result;
    }
    
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid deposit amount");
        return result;
    }
    
    // Get card data
    CardData* card = get_card_data(cardNumber);
    if (card == NULL) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to retrieve card data");
        return result;
    }
    
    // Fetch current balance using customer ID
    float oldBalance = fetchBalanceByCustomerId(card->customer_id);
    if (oldBalance < 0) {
        free(card);
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch account balance");
        return result;
    }
    
    // Perform deposit by updating balance
    float newBalance = oldBalance + amount;
    if (updateBalanceByCustomerId(card->customer_id, newBalance)) {
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Virtual deposit successful. New balance: $%.2f", newBalance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Virtual deposited $%.2f. Old balance: $%.2f, New balance: $%.2f", 
               amount, oldBalance, newBalance);
        writeTransactionDetails(username, "Virtual Deposit", detailsLog);
        
        logTransaction(card->card_id, TRANSACTION_DEPOSIT, amount, 1);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to update balance");
        logTransaction(card->card_id, TRANSACTION_DEPOSIT, amount, 0);
    }
    
    free(card);
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

// New function for virtual withdrawal operation
TransactionResult performVirtualWithdrawal(const char* cardNumber, int cvv, const char* expiryDate, float amount, const char* username) {
    TransactionResult result = {0};
    
    // Check if virtual ATM is enabled
    if (!is_virtual_atm_enabled()) {
        result.success = 0;
        strcpy(result.message, "Error: Virtual ATM functionality is currently disabled");
        return result;
    }
    
    // Validate the virtual transaction
    CardValidationStatus status = validate_virtual_transaction(cardNumber, cvv, expiryDate);
    if (status != CARD_VALID) {
        result.success = 0;
        switch (status) {
            case CARD_INVALID_FORMAT:
                strcpy(result.message, "Error: Invalid card number format");
                break;
            case CARD_NOT_FOUND:
                strcpy(result.message, "Error: Card not found");
                break;
            case CARD_EXPIRED:
                strcpy(result.message, "Error: Card is expired");
                break;
            case CARD_CVV_INVALID:
                strcpy(result.message, "Error: Invalid CVV");
                break;
            case CARD_BLOCKED:
                strcpy(result.message, "Error: Card is blocked");
                break;
            default:
                strcpy(result.message, "Error: Card validation failed");
        }
        return result;
    }
    
    // Check if ATM is in maintenance mode
    if (getConfigValueBool(CONFIG_MAINTENANCE_MODE)) {
        result.success = 0;
        strcpy(result.message, "Sorry, the banking system is currently in maintenance mode.");
        return result;
    }
    
    // Get card data
    CardData* card = get_card_data(cardNumber);
    if (card == NULL) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to retrieve card data");
        return result;
    }
    
    // Get withdrawal limit from system configuration for virtual transactions
    const char* virtualLimitStr = get_config_value(CONFIG_VIRTUAL_WITHDRAWAL_LIMIT);
    int withdrawalLimit = virtualLimitStr ? atoi(virtualLimitStr) : 10000; // Default virtual limit
    
    // Get daily transaction limit from system configuration
    const char* dailyLimitStr = get_config_value(CONFIG_DAILY_TRANSACTION_LIMIT);
    int dailyLimit = dailyLimitStr ? atoi(dailyLimitStr) : 50000; // Default daily limit
    
    // Validate withdrawal amount
    if (amount <= 0) {
        free(card);
        result.success = 0;
        strcpy(result.message, "Error: Invalid withdrawal amount");
        return result;
    }
    
    // Check if amount exceeds the virtual withdrawal limit
    if (amount > withdrawalLimit) {
        free(card);
        result.success = 0;
        sprintf(result.message, "Error: Amount exceeds virtual withdrawal limit of $%d", withdrawalLimit);
        return result;
    }
    
    // Check if amount exceeds daily withdrawal limit
    if (is_virtual_transaction_limit_exceeded(cardNumber, amount)) {
        free(card);
        result.success = 0;
        sprintf(result.message, "Error: Would exceed daily transaction limit of $%d", dailyLimit);
        return result;
    }
    
    // Fetch current balance using customer ID
    float oldBalance = fetchBalanceByCustomerId(card->customer_id);
    if (oldBalance < 0) {
        free(card);
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch account balance");
        return result;
    }
    
    // Check if sufficient funds
    if (oldBalance < amount) {
        free(card);
        result.success = 0;
        sprintf(result.message, "Error: Insufficient funds. Current balance: $%.2f", oldBalance);
        return result;
    }
    
    // Perform withdrawal by updating balance
    float newBalance = oldBalance - amount;
    if (updateBalanceByCustomerId(card->customer_id, newBalance)) {
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Virtual withdrawal successful. New balance: $%.2f", newBalance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Virtual withdrew $%.2f. Old balance: $%.2f, New balance: $%.2f", 
                amount, oldBalance, newBalance);
        writeTransactionDetails(username, "Virtual Withdrawal", detailsLog);
        
        // Track withdrawal for daily limit
        logWithdrawal(card->card_id, amount);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to complete withdrawal");
    }
    
    free(card);
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

// New function to get mini statement with card number string
TransactionResult getVirtualMiniStatement(const char* cardNumber, int cvv, const char* expiryDate, const char* username) {
    TransactionResult result = {0};
    
    // Check if virtual ATM is enabled
    if (!is_virtual_atm_enabled()) {
        result.success = 0;
        strcpy(result.message, "Error: Virtual ATM functionality is currently disabled");
        return result;
    }
    
    // Validate the virtual transaction
    CardValidationStatus status = validate_virtual_transaction(cardNumber, cvv, expiryDate);
    if (status != CARD_VALID) {
        result.success = 0;
        switch (status) {
            case CARD_INVALID_FORMAT:
                strcpy(result.message, "Error: Invalid card number format");
                break;
            case CARD_NOT_FOUND:
                strcpy(result.message, "Error: Card not found");
                break;
            case CARD_EXPIRED:
                strcpy(result.message, "Error: Card is expired");
                break;
            case CARD_CVV_INVALID:
                strcpy(result.message, "Error: Invalid CVV");
                break;
            case CARD_BLOCKED:
                strcpy(result.message, "Error: Card is blocked");
                break;
            default:
                strcpy(result.message, "Error: Card validation failed");
        }
        return result;
    }
    
    // Get card data
    CardData* card = get_card_data(cardNumber);
    if (card == NULL) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to retrieve card data");
        return result;
    }
    
    // Use the existing implementation but with card ID from the card data
    result = getMiniStatement(card->card_id, username);
    
    // Update the message to indicate this was a virtual transaction
    if (result.success) {
        char virtualHeader[50] = "VIRTUAL TRANSACTION\n\n";
        char originalMessage[1024];
        strcpy(originalMessage, result.message);
        
        // Prepend the virtual header to the message
        strcpy(result.message, virtualHeader);
        strcat(result.message, originalMessage);
    }
    
    free(card);
    return result;
}

// Perform money transfer between accounts with transaction atomicity
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
    
    // Lock the transaction files to ensure atomicity
    if (!lockTransactionFiles()) {
        result.success = 0;
        strcpy(result.message, "Error: System busy, please try again later");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Start transaction - create backup of account files
    if (!backupAccountFiles()) {
        unlockTransactionFiles();
        result.success = 0;
        strcpy(result.message, "Error: Could not initiate transaction safely");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Fetch sender balance
    float senderBalance = fetchBalance(senderCardNumber);
    if (senderBalance < 0) {
        // Rollback by restoring from backup
        restoreAccountFiles();
        unlockTransactionFiles();
        
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch your account balance");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Check if sender has sufficient funds
    if (senderBalance < amount) {
        // No need to rollback, just release locks
        unlockTransactionFiles();
        
        result.success = 0;
        sprintf(result.message, "Error: Insufficient funds. Current balance: $%.2f", senderBalance);
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Fetch receiver balance
    float receiverBalance = fetchBalance(receiverCardNumber);
    if (receiverBalance < 0) {
        // Rollback by restoring from backup
        restoreAccountFiles();
        unlockTransactionFiles();
        
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch recipient's account balance");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Update balances of both sender and receiver
    int senderUpdateSuccess = updateBalance(senderCardNumber, senderBalance - amount);
    int receiverUpdateSuccess = updateBalance(receiverCardNumber, receiverBalance + amount);
    
    if (!senderUpdateSuccess || !receiverUpdateSuccess) {
        // Rollback if either update fails
        restoreAccountFiles();
        unlockTransactionFiles();
        
        result.success = 0;
        strcpy(result.message, "Error: Failed to complete transfer");
        logTransaction(senderCardNumber, TRANSACTION_MONEY_TRANSFER, amount, 0);
        return result;
    }
    
    // Transaction successful, commit changes
    unlockTransactionFiles();
    
    // Update result with success info
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
    
    return result;
}

// New function for virtual money transfer
TransactionResult performVirtualMoneyTransfer(const char* senderCardNumber, int cvv, const char* expiryDate, 
                                            const char* receiverCardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    
    // Check if virtual ATM is enabled
    if (!is_virtual_atm_enabled()) {
        result.success = 0;
        strcpy(result.message, "Error: Virtual ATM functionality is currently disabled");
        return result;
    }
    
    // Validate the sender card virtual transaction
    CardValidationStatus status = validate_virtual_transaction(senderCardNumber, cvv, expiryDate);
    if (status != CARD_VALID) {
        result.success = 0;
        switch (status) {
            case CARD_INVALID_FORMAT:
                strcpy(result.message, "Error: Invalid sender card number format");
                break;
            case CARD_NOT_FOUND:
                strcpy(result.message, "Error: Sender card not found");
                break;
            case CARD_EXPIRED:
                strcpy(result.message, "Error: Sender card is expired");
                break;
            case CARD_CVV_INVALID:
                strcpy(result.message, "Error: Invalid sender CVV");
                break;
            case CARD_BLOCKED:
                strcpy(result.message, "Error: Sender card is blocked");
                break;
            default:
                strcpy(result.message, "Error: Sender card validation failed");
        }
        return result;
    }
    
    // Validate the receiver card format and existence
    CardValidationStatus receiverStatus = validate_card(receiverCardNumber);
    if (receiverStatus != CARD_VALID) {
        result.success = 0;
        switch (receiverStatus) {
            case CARD_INVALID_FORMAT:
                strcpy(result.message, "Error: Invalid receiver card number format");
                break;
            case CARD_NOT_FOUND:
                strcpy(result.message, "Error: Receiver card not found");
                break;
            case CARD_BLOCKED:
                strcpy(result.message, "Error: Receiver card is blocked");
                break;
            default:
                strcpy(result.message, "Error: Receiver card validation failed");
        }
        return result;
    }
    
    // Validate transfer amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid transfer amount");
        return result;
    }
    
    // Get sender and receiver card data
    CardData* senderCard = get_card_data(senderCardNumber);
    CardData* receiverCard = get_card_data(receiverCardNumber);
    
    if (!senderCard || !receiverCard) {
        if (senderCard) free(senderCard);
        if (receiverCard) free(receiverCard);
        result.success = 0;
        strcpy(result.message, "Error: Unable to retrieve card data");
        return result;
    }
    
    // Lock the transaction files to ensure atomicity
    if (!lockTransactionFiles()) {
        free(senderCard);
        free(receiverCard);
        result.success = 0;
        strcpy(result.message, "Error: System busy, please try again later");
        return result;
    }
    
    // Start transaction - create backup of account files
    if (!backupAccountFiles()) {
        unlockTransactionFiles();
        free(senderCard);
        free(receiverCard);
        result.success = 0;
        strcpy(result.message, "Error: Could not initiate transaction safely");
        return result;
    }
    
    // Fetch sender balance
    float senderBalance = fetchBalanceByCustomerId(senderCard->customer_id);
    if (senderBalance < 0) {
        // Rollback by restoring from backup
        restoreAccountFiles();
        unlockTransactionFiles();
        free(senderCard);
        free(receiverCard);
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch your account balance");
        return result;
    }
    
    // Check if sender has sufficient funds
    if (senderBalance < amount) {
        // No need to rollback, just release locks
        unlockTransactionFiles();
        free(senderCard);
        free(receiverCard);
        result.success = 0;
        sprintf(result.message, "Error: Insufficient funds. Current balance: $%.2f", senderBalance);
        return result;
    }
    
    // Fetch receiver balance
    float receiverBalance = fetchBalanceByCustomerId(receiverCard->customer_id);
    if (receiverBalance < 0) {
        // Rollback by restoring from backup
        restoreAccountFiles();
        unlockTransactionFiles();
        free(senderCard);
        free(receiverCard);
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch recipient's account balance");
        return result;
    }
    
    // Update balances of both sender and receiver
    int senderUpdateSuccess = updateBalanceByCustomerId(senderCard->customer_id, senderBalance - amount);
    int receiverUpdateSuccess = updateBalanceByCustomerId(receiverCard->customer_id, receiverBalance + amount);
    
    if (!senderUpdateSuccess || !receiverUpdateSuccess) {
        // Rollback if either update fails
        restoreAccountFiles();
        unlockTransactionFiles();
        free(senderCard);
        free(receiverCard);
        result.success = 0;
        strcpy(result.message, "Error: Failed to complete transfer");
        return result;
    }
    
    // Transaction successful, commit changes
    unlockTransactionFiles();
    
    // Update result with success info
    result.success = 1;
    result.oldBalance = senderBalance;
    result.newBalance = senderBalance - amount;
    sprintf(result.message, "Virtual transfer successful. Your new balance: $%.2f", result.newBalance);
    
    // Log the transaction for both accounts
    char detailsLog[100];
    sprintf(detailsLog, "Virtual transferred $%.2f to card %s", amount, receiverCardNumber);
    writeTransactionDetails(username, "Virtual Money Transfer", detailsLog);
    
    logTransaction(senderCard->card_id, TRANSACTION_MONEY_TRANSFER, amount, 1);
    
    // Also log for recipient
    char recipientName[50] = "Unknown"; // Default if we can't find name
    getCardHolderName(receiverCard->card_id, recipientName, sizeof(recipientName));
    
    char recipientLog[150];
    sprintf(recipientLog, "Received $%.2f from virtual transaction by %s (card: %s)", 
            amount, username, senderCardNumber);
    writeTransactionDetails(recipientName, "Money Received", recipientLog);
    
    logTransaction(receiverCard->card_id, TRANSACTION_MONEY_TRANSFER, amount, 1);
    
    free(senderCard);
    free(receiverCard);
    return result;
}

// Generate transaction receipt
void generateReceipt(int cardNumber, TransactionType type, float amount, float balance, const char* phoneNumber) {
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Get a unique receipt number
    int receiptNumber = (int)time(NULL) % 100000 + cardNumber % 1000;
    
    // Convert transaction type to string
    const char* typeStr;
    switch (type) {
        case TRANSACTION_BALANCE_CHECK: typeStr = "Balance Check"; break;
        case TRANSACTION_DEPOSIT: typeStr = "Deposit"; break;
        case TRANSACTION_WITHDRAWAL: typeStr = "Withdrawal"; break;
        case TRANSACTION_MONEY_TRANSFER: typeStr = "Money Transfer"; break;
        case TRANSACTION_MINI_STATEMENT: typeStr = "Mini Statement"; break;
        case TRANSACTION_PIN_CHANGE: typeStr = "PIN Change"; break;
        default: typeStr = "Unknown Transaction"; break;
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║             ATM RECEIPT                  ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ Receipt #: %d                       ║\n", receiptNumber);
    printf("║ Date: %s              ║\n", timestamp);
    printf("║ Card: %d                          ║\n", cardNumber);
    printf("║ Transaction: %-28s ║\n", typeStr);
    
    if (type != TRANSACTION_BALANCE_CHECK && type != TRANSACTION_MINI_STATEMENT && type != TRANSACTION_PIN_CHANGE) {
        printf("║ Amount: $%-30.2f ║\n", amount);
    }
    
    printf("║ Balance: $%-29.2f ║\n", balance);
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ Thank you for using our ATM services     ║\n");
    
    // Display customer service contact info
    if (strlen(phoneNumber) > 0 && strcmp(phoneNumber, "0000000000") != 0) {
        printf("║ For inquiries: %-24s ║\n", phoneNumber);
    } else {
        printf("║ For inquiries: 1-800-BANK-HELP        ║\n");
    }
    
    printf("╚══════════════════════════════════════════╝\n\n");
    
    // Log the receipt generation
    char logMsg[100];
    sprintf(logMsg, "Receipt generated for card %d, transaction: %s", 
            cardNumber, typeStr);
    writeAuditLog("RECEIPT", logMsg);
}

// Generate virtual transaction receipt with card data
void generateVirtualReceipt(const char* cardNumber, const char* customerName, TransactionType type, float amount, float balance) {
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Get a unique receipt number
    int receiptNumber = (int)time(NULL) % 100000;
    
    // Convert transaction type to string
    const char* typeStr;
    switch (type) {
        case TRANSACTION_BALANCE_CHECK: typeStr = "Balance Check"; break;
        case TRANSACTION_DEPOSIT: typeStr = "Virtual Deposit"; break;
        case TRANSACTION_WITHDRAWAL: typeStr = "Virtual Withdrawal"; break;
        case TRANSACTION_MONEY_TRANSFER: typeStr = "Virtual Money Transfer"; break;
        case TRANSACTION_MINI_STATEMENT: typeStr = "Virtual Mini Statement"; break;
        case TRANSACTION_PIN_CHANGE: typeStr = "PIN Change"; break;
        default: typeStr = "Virtual Transaction"; break;
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║           VIRTUAL ATM RECEIPT            ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ Receipt #: %d                       ║\n", receiptNumber);
    printf("║ Date: %s              ║\n", timestamp);
    printf("║ Card: %s      ║\n", cardNumber);
    printf("║ Name: %-32s ║\n", customerName);
    printf("║ Transaction: %-28s ║\n", typeStr);
    
    if (type != TRANSACTION_BALANCE_CHECK && type != TRANSACTION_MINI_STATEMENT && type != TRANSACTION_PIN_CHANGE) {
        printf("║ Amount: $%-30.2f ║\n", amount);
    }
    
    printf("║ Balance: $%-29.2f ║\n", balance);
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ Thank you for using our Virtual Banking   ║\n");
    printf("║ For inquiries: 1-800-VIRTUAL-BANK        ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    // Log the receipt generation
    char logMsg[150];
    sprintf(logMsg, "Virtual receipt generated for card %s, transaction: %s", 
            cardNumber, typeStr);
    writeAuditLog("RECEIPT", logMsg);
    
    // Also create a file-based receipt
    char receiptFilename[100];
    sprintf(receiptFilename, "logs/receipts/virtual_%d.txt", receiptNumber);
    
    FILE* receiptFile = fopen(receiptFilename, "w");
    if (receiptFile) {
        fprintf(receiptFile, "===========================================\n");
        fprintf(receiptFile, "           VIRTUAL ATM RECEIPT            \n");
        fprintf(receiptFile, "===========================================\n");
        fprintf(receiptFile, "Receipt #: %d\n", receiptNumber);
        fprintf(receiptFile, "Date: %s\n", timestamp);
        fprintf(receiptFile, "Card: %s\n", cardNumber);
        fprintf(receiptFile, "Name: %s\n", customerName);
        fprintf(receiptFile, "Transaction: %s\n", typeStr);
        
        if (type != TRANSACTION_BALANCE_CHECK && type != TRANSACTION_MINI_STATEMENT && type != TRANSACTION_PIN_CHANGE) {
            fprintf(receiptFile, "Amount: $%.2f\n", amount);
        }
        
        fprintf(receiptFile, "Balance: $%.2f\n", balance);
        fprintf(receiptFile, "===========================================\n");
        fprintf(receiptFile, "Thank you for using our Virtual Banking\n");
        fprintf(receiptFile, "For inquiries: 1-800-VIRTUAL-BANK\n");
        fprintf(receiptFile, "===========================================\n");
        
        fclose(receiptFile);
    }
}

// Helper functions for transaction atomicity
int lockTransactionFiles() {
    // Create a lock file to indicate that a transaction is in progress
    FILE* lockFile = fopen("data/temp/transaction.lock", "w");
    if (lockFile == NULL) {
        writeErrorLog("Failed to create transaction lock file");
        return 0;
    }
    
    // Write current timestamp and process ID to the lock file
    time_t now = time(NULL);
    fprintf(lockFile, "LOCKED:%ld:%d\n", (long)now, getpid());
    fclose(lockFile);
    
    return 1;
}

int unlockTransactionFiles() {
    // Remove the lock file
    if (remove("data/temp/transaction.lock") != 0) {
        writeErrorLog("Failed to remove transaction lock file");
        return 0;
    }
    return 1;
}

int backupAccountFiles() {
    // Backup accounting file
    const char* accountingFile = isTestingMode() ? TEST_ACCOUNTING_FILE : PROD_ACCOUNTING_FILE;
    const char* backupFile = "data/temp/accounting.bak";
    
    FILE* source = fopen(accountingFile, "r");
    if (source == NULL) {
        writeErrorLog("Failed to open accounting file for backup");
        return 0;
    }
    
    FILE* backup = fopen(backupFile, "w");
    if (backup == NULL) {
        writeErrorLog("Failed to create accounting backup file");
        fclose(source);
        return 0;
    }
    
    // Copy the file content
    char buffer[1024];
    size_t bytesRead;
    
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytesRead, backup);
    }
    
    fclose(source);
    fclose(backup);
    
    return 1;
}

int restoreAccountFiles() {
    // Restore accounting file from backup
    const char* accountingFile = isTestingMode() ? TEST_ACCOUNTING_FILE : PROD_ACCOUNTING_FILE;
    const char* backupFile = "data/temp/accounting.bak";
    
    FILE* backup = fopen(backupFile, "r");
    if (backup == NULL) {
        writeErrorLog("Failed to open accounting backup file for restore");
        return 0;
    }
    
    FILE* dest = fopen(accountingFile, "w");
    if (dest == NULL) {
        writeErrorLog("Failed to open accounting file for restore");
        fclose(backup);
        return 0;
    }
    
    // Copy the file content
    char buffer[1024];
    size_t bytesRead;
    
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), backup)) > 0) {
        fwrite(buffer, 1, bytesRead, dest);
    }
    
    fclose(backup);
    fclose(dest);
    
    return 1;
}

// Generate account creation receipt with card details
void generateAccountCreationReceipt(const char* customerName, const char* accountNumber, 
                                   const char* accountType, const char* ifscCode, 
                                   const char* cardNumber, const char* expiryDate, int cvv) {
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Create a unique receipt ID
    char receiptId[20];
    sprintf(receiptId, "ACC%ld", (long)time(NULL));
    
    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║         ACCOUNT CREATION RECEIPT         ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ Receipt ID: %-28s ║\n", receiptId);
    printf("║ Date: %-33s ║\n", timestamp);
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ ACCOUNT DETAILS:                         ║\n");
    printf("║ Customer: %-30s ║\n", customerName);
    printf("║ Account #: %-29s ║\n", accountNumber);
    printf("║ Account Type: %-26s ║\n", accountType);
    printf("║ IFSC Code: %-28s ║\n", ifscCode);
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ CARD DETAILS:                            ║\n");
    printf("║ Card #: %-31s ║\n", cardNumber);
    printf("║ Expiry: %-31s ║\n", expiryDate);
    printf("║ CVV: %-34d ║\n", cvv);
    printf("║                                          ║\n");
    printf("║ KEEP YOUR CARD DETAILS CONFIDENTIAL      ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ Your PIN has been sent to your           ║\n");
    printf("║ registered mobile number.                ║\n");
    printf("║                                          ║\n");
    printf("║ Please change your PIN at first use.     ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ Thank you for choosing our bank!         ║\n");
    printf("║ For support: 1-800-BANK-HELP             ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    // Also save receipt to a file
    char filename[100];
    sprintf(filename, "logs/receipts/account_%s.txt", accountNumber);
    
    // Create receipts directory if it doesn't exist
    system("mkdir -p logs/receipts");
    
    FILE* file = fopen(filename, "w");
    if (file) {
        fprintf(file, "=======================================\n");
        fprintf(file, "      ACCOUNT CREATION RECEIPT         \n");
        fprintf(file, "=======================================\n");
        fprintf(file, "Receipt ID: %s\n", receiptId);
        fprintf(file, "Date: %s\n", timestamp);
        fprintf(file, "---------------------------------------\n");
        fprintf(file, "ACCOUNT DETAILS:\n");
        fprintf(file, "Customer: %s\n", customerName);
        fprintf(file, "Account #: %s\n", accountNumber);
        fprintf(file, "Account Type: %s\n", accountType);
        fprintf(file, "IFSC Code: %s\n", ifscCode);
        fprintf(file, "---------------------------------------\n");
        fprintf(file, "CARD DETAILS:\n");
        fprintf(file, "Card #: %s\n", cardNumber);
        fprintf(file, "Expiry: %s\n", expiryDate);
        fprintf(file, "CVV: %03d\n", cvv);
        fprintf(file, "\n");
        fprintf(file, "KEEP YOUR CARD DETAILS CONFIDENTIAL\n");
        fprintf(file, "---------------------------------------\n");
        fprintf(file, "Your PIN has been sent to your\n");
        fprintf(file, "registered mobile number.\n\n");
        fprintf(file, "Please change your PIN at first use.\n");
        fprintf(file, "---------------------------------------\n");
        fprintf(file, "Thank you for choosing our bank!\n");
        fprintf(file, "For support: 1-800-BANK-HELP\n");
        fprintf(file, "=======================================\n");
        fclose(file);
    }
    
    // Log the receipt generation
    char logMsg[150];
    sprintf(logMsg, "Account creation receipt generated for account %s", accountNumber);
    writeAuditLog("RECEIPT", logMsg);
}

// Alias function for performMoneyTransfer to maintain backward compatibility
TransactionResult performFundTransfer(int senderCardNumber, int receiverCardNumber, float amount, const char* username) {
    // Simply call the existing implementation with the same parameters
    return performMoneyTransfer(senderCardNumber, receiverCardNumber, amount, username);
}