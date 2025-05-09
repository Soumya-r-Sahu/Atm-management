#include <atm/transaction/transaction_manager.h>
#include <common/database/database.h>
#include <common/database/dao_interface.h>   // Include DAO interface
#include <common/utils/logger.h>
#include <common/utils/hash_utils.h>
#include <common/paths.h>
#include <common/database/customer_profile.h>
#include <common/config/config_manager.h>
#include <atm/validation/card_validator.h>
#include <common/utils/cbs_logger.h>      // CBS Logger
#include <common/utils/dao_audit_logger.h> // DAO Audit Logger
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// External function to get the DAO instance
extern DatabaseAccessObject* getDAO(void);

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

// Helper function to check if ATM is in maintenance mode
bool isMaintenanceModeActive(void) {
    return get_config_bool(CONFIG_MAINTENANCE_MODE, false);
}

// Check if virtual ATM is enabled
bool is_virtual_atm_enabled(void) {
    return get_config_bool(CONFIG_VIRTUAL_ATM_ENABLED, false);
}

// Write detailed transaction information to log using CBS logger
void writeTransactionDetails(const char* username, const char* transactionType, const char* details) {
    // Generate a transaction ID
    char transaction_id[37];
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    
    // Format: TXN-YYYYMMDD-HHMMSS-RANDOM
    snprintf(transaction_id, sizeof(transaction_id), 
             "TXN-%04d%02d%02d-%02d%02d%02d-%06d",
             tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
             tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec,
             rand() % 1000000);
    
    // Log using the CBS logger (more detailed and compliant)
    cbs_writeLog(LOG_CATEGORY_TRANSACTION, CBS_LOG_LEVEL_INFO,
                "User: %s, Type: %s, Details: %s", 
                username, transactionType, details);
                
    // Also write to traditional transaction log file for backward compatibility
    const char* transactionPath = isTestingMode() ? 
        TEST_TRANSACTIONS_LOG_FILE : PROD_TRANSACTIONS_LOG_FILE;
    
    // Create a log message with all details for backwards compatibility
    char logMessage[512];
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    snprintf(logMessage, sizeof(logMessage), "[%s] User: %s, Type: %s, Details: %s", 
             timestamp, username, transactionType, details);
    
    FILE* file = fopen(transactionPath, "a");
    if (file != NULL) {
        fprintf(file, "%s\n", logMessage);
        fclose(file);
    } else {
        // If transaction log cannot be opened, fall back to error log
        writeErrorLog("Failed to write transaction details");
        cbs_writeLog(LOG_CATEGORY_SYSTEM, CBS_LOG_LEVEL_ERROR, 
                    "Failed to write to transaction log file: %s", transactionPath);
    }
}

// Helper function to handle transaction logging with both traditional and CBS loggers
void enhanced_transaction_log(
    int cardNumber,
    const char* username,
    const char* transactionType, 
    float amount, 
    float old_balance, 
    float new_balance, 
    bool success) {
    
    // Use traditional logging mechanism
    DatabaseAccessObject* dao = getDAO();
    if (dao != NULL) {
        dao->logTransaction(cardNumber, transactionType, amount, success);
    }
    
    // Use new CBS compliant audit logging
    record_transaction_audit(
        username ? username : "UNKNOWN",
        cardNumber,
        transactionType,
        amount,
        old_balance,
        new_balance,
        success
    );
    
    // Log security information for failed transactions
    if (!success) {
        char card_str[20];
        snprintf(card_str, sizeof(card_str), "%d", cardNumber);
        
        cbs_writeSecurityLog(
            username ? username : "UNKNOWN",
            "TRANSACTION_FAILURE",
            "MEDIUM",
            "FAILED",
            card_str,
            "Failed transaction attempt",
            NULL  // No IP address available in this context
        );
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
        
        // Log the transaction with both traditional and CBS loggers
        char detailsLog[100];
        sprintf(detailsLog, "Checked balance: $%.2f", balance);
        writeTransactionDetails(username, "Balance Check", detailsLog);
        
        // Use enhanced logging that covers both systems
        enhanced_transaction_log(
            cardNumber,
            username,
            TRANSACTION_BALANCE_CHECK,
            0.0f,  // No amount for balance check
            balance,
            balance,
            true
        );
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch balance");
        
        // Log the failed transaction with both systems
        enhanced_transaction_log(
            cardNumber,
            username,
            TRANSACTION_BALANCE_CHECK,
            0.0f,
            0.0f,
            0.0f,
            false
        );
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
        
        // Log the validation failure
        cbs_writeSecurityLog(
            username ? username : "UNKNOWN",
            "CARD_VALIDATION_FAILURE",
            "MEDIUM",
            "REJECTED",
            "MASKED_CARD",
            "Invalid card number format",
            NULL
        );
        return result;
    }
    
    // Fetch card data and validate
    CardData* card = get_card_data(cardNumber);
    if (card == NULL) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        
        // Log the security event for card not found
        cbs_writeSecurityLog(
            username ? username : "UNKNOWN",
            "CARD_NOT_FOUND",
            "MEDIUM",
            "REJECTED",
            "MASKED_CARD",
            "Card not found in database",
            NULL
        );
        return result;
    }
    
    // Fetch balance from database using customer ID
    float balance = fetchBalanceByCustomerId(card->customer_id);
    int numericCardNumber = card->card_id;  // Get the numeric card ID
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
        
        // Use enhanced logging
        enhanced_transaction_log(
            numericCardNumber,
            username,
            "Virtual Balance Check",
            0.0f,
            balance,
            balance,
            true
        );
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch balance");
        
        // Log failure with both systems
        enhanced_transaction_log(
            numericCardNumber,
            username,
            "Virtual Balance Check",
            0.0f,
            0.0f,
            0.0f,
            false
        );
    }
    
    return result;
}

// Perform deposit operation
TransactionResult performDeposit(int cardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid deposit amount");
        
        // Log validation failure
        enhanced_transaction_log(
            cardNumber,
            username,
            "Deposit",
            amount,
            0.0f,
            0.0f,
            false
        );
        return result;
    }
    
    // Check if card exists and is active
    if (!dao->doesCardExist(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        
        // Log security event for card not found
        cbs_writeSecurityLog(
            username ? username : "UNKNOWN",
            "CARD_NOT_FOUND",
            "MEDIUM",
            "REJECTED",
            "TRANSACTION_ATTEMPT",
            "Deposit attempt with non-existent card",
            NULL
        );
        
        enhanced_transaction_log(
            cardNumber,
            username,
            "Deposit",
            amount,
            0.0f,
            0.0f,
            false
        );
        return result;
    }
    
    if (!dao->isCardActive(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is inactive or blocked");
        
        // Log security event for inactive card
        char card_str[20];
        snprintf(card_str, sizeof(card_str), "%d", cardNumber);
        
        cbs_writeSecurityLog(
            username ? username : "UNKNOWN",
            "INACTIVE_CARD_USAGE",
            "HIGH",
            "REJECTED",
            card_str,
            "Transaction attempt with inactive/blocked card",
            NULL
        );
        
        enhanced_transaction_log(
            cardNumber,
            username,
            "Deposit",
            amount,
            0.0f,
            0.0f,
            false
        );
        return result;
    }
    
    // Fetch current balance using DAO
    float oldBalance = dao->fetchBalance(cardNumber);
    if (oldBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch account balance");
        
        enhanced_transaction_log(
            cardNumber,
            username,
            "Deposit",
            amount,
            0.0f,
            0.0f,
            false
        );
        return result;
    }
    
    // Perform deposit by updating balance
    float newBalance = oldBalance + amount;
    if (dao->updateBalance(cardNumber, newBalance)) {
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Deposit successful. New balance: $%.2f", newBalance);
        
        // Log the transaction with both traditional and CBS loggers
        char detailsLog[100];
        sprintf(detailsLog, "Deposited $%.2f. Old balance: $%.2f, New balance: $%.2f", 
               amount, oldBalance, newBalance);
        writeTransactionDetails(username, "Deposit", detailsLog);
        
        // Use our enhanced logging function
        enhanced_transaction_log(
            cardNumber,
            username,
            "Deposit",
            amount,
            oldBalance,
            newBalance,
            true
        );
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to update balance");
        
        // Log failure
        enhanced_transaction_log(
            cardNumber,
            username,
            "Deposit",
            amount,
            oldBalance,
            oldBalance, // No change in balance
            false
        );
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
        
        // Log configuration restriction
        cbs_writeLog(LOG_CATEGORY_SECURITY, CBS_LOG_LEVEL_WARNING,
                    "Virtual deposit attempt when feature disabled: User %s, Amount: %.2f",
                    username ? username : "UNKNOWN", amount);
        return result;
    }
    
    // Validate the virtual transaction
    CardValidationStatus status = validate_virtual_transaction(cardNumber, cvv, expiryDate);
    if (status != CARD_VALID) {
        result.success = 0;
        
        // Prepare security log details
        const char *failure_reason = "Unknown validation failure";
        const char *severity = "MEDIUM";
        
        switch (status) {
            case CARD_INVALID_FORMAT:
                strcpy(result.message, "Error: Invalid card number format");
                failure_reason = "Invalid card number format";
                break;
            case CARD_NOT_FOUND:
                strcpy(result.message, "Error: Card not found");
                failure_reason = "Card not found";
                break;
            case CARD_EXPIRED:
                strcpy(result.message, "Error: Card is expired");
                failure_reason = "Expired card";
                severity = "HIGH";
                break;
            case CARD_CVV_INVALID:
                strcpy(result.message, "Error: Invalid CVV");
                failure_reason = "Invalid CVV";
                severity = "HIGH";
                break;
            case CARD_BLOCKED:
                strcpy(result.message, "Error: Card is blocked");
                failure_reason = "Blocked card";
                severity = "HIGH";
                break;
            default:
                strcpy(result.message, "Error: Card validation failed");
                failure_reason = "General validation failure";
        }
        
        // Log security event for virtual transaction validation failure
        cbs_writeSecurityLog(
            username ? username : "UNKNOWN",
            "VIRTUAL_TRANSACTION_VALIDATION_FAILURE",
            severity,
            "REJECTED",
            "MASKED_CARD",
            failure_reason,
            NULL
        );
        
        return result;
    }
    
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid deposit amount");
        
        // Log validation failure
        cbs_writeLog(LOG_CATEGORY_TRANSACTION, CBS_LOG_LEVEL_WARNING,
                    "Invalid deposit amount: %.2f, User: %s",
                    amount, username ? username : "UNKNOWN");
        return result;
    }
    
    // Get card data
    CardData* card = get_card_data(cardNumber);
    if (card == NULL) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to retrieve card data");
        
        cbs_writeLog(LOG_CATEGORY_SYSTEM, CBS_LOG_LEVEL_ERROR,
                    "Failed to retrieve card data for virtual deposit");
        return result;
    }
    
    // Fetch current balance using customer ID
    float oldBalance = fetchBalanceByCustomerId(card->customer_id);
    if (oldBalance < 0) {
        free(card);
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch account balance");
        
        cbs_writeLog(LOG_CATEGORY_DATABASE, CBS_LOG_LEVEL_ERROR,
                    "Failed to fetch account balance by customer ID");
        return result;
    }
    
    // Perform deposit by updating balance
    float newBalance = oldBalance + amount;
    if (updateBalanceByCustomerId(card->customer_id, newBalance)) {
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Virtual deposit successful. New balance: $%.2f", newBalance);
        
        // Log the transaction with both traditional and CBS loggers
        char detailsLog[100];
        sprintf(detailsLog, "Virtual deposited $%.2f. Old balance: $%.2f, New balance: $%.2f", 
               amount, oldBalance, newBalance);
        writeTransactionDetails(username, "Virtual Deposit", detailsLog);
        
        // Use enhanced logging with both systems
        enhanced_transaction_log(
            card->card_id,
            username,
            "Virtual Deposit",
            amount,
            oldBalance,
            newBalance,
            true
        );
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to update balance");
        
        // Log failure with both systems
        enhanced_transaction_log(
            card->card_id,
            username,
            "Virtual Deposit",
            amount,
            oldBalance,
            oldBalance,  // No change in balance
            false
        );
    }
    
    free(card);
    return result;
}

// Perform PIN change
TransactionResult performPINChange(int cardNumber, int oldPIN, int newPIN, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Check if ATM is in maintenance mode
    if (isMaintenanceModeActive()) {
        result.success = 0;
        strcpy(result.message, "Sorry, this ATM is currently in maintenance mode.");
        dao->logTransaction(cardNumber, "PIN Change", 0.0f, false);
        return result;
    }
    
    // Check if card exists and is active
    if (!dao->doesCardExist(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        return result;
    }
    
    if (!dao->isCardActive(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is inactive or blocked");
        return result;
    }
    
    // Validate old PIN
    if (!dao->validateCard(cardNumber, oldPIN)) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid PIN");
        dao->logTransaction(cardNumber, "PIN Change", 0.0f, false);
        return result;
    }
    
    // Validate new PIN format (4 digits)
    if (newPIN < 1000 || newPIN > 9999) {
        result.success = 0;
        strcpy(result.message, "Error: PIN must be a 4-digit number");
        dao->logTransaction(cardNumber, "PIN Change", 0.0f, false);
        return result;
    }
    
    // Convert PIN to hash (in a real implementation, use a secure hashing algorithm)
    char pinStr[20];
    sprintf(pinStr, "%d", newPIN);
    char* newPINHash = sha256_hash(pinStr);
    if (!newPINHash) {
        result.success = 0;
        strcpy(result.message, "Error: System error while processing PIN change");
        dao->logTransaction(cardNumber, "PIN Change", 0.0f, false);
        return result;
    }
    
    // Update PIN using DAO
    if (dao->updateCardPIN(cardNumber, newPINHash)) {
        result.success = 1;
        strcpy(result.message, "PIN changed successfully");
        
        // Log the transaction
        writeTransactionDetails(username, "PIN Change", "PIN changed successfully");
        dao->logTransaction(cardNumber, "PIN Change", 0.0f, true);
        
        // Free the allocated hash
        free(newPINHash);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to update PIN");
        dao->logTransaction(cardNumber, "PIN Change", 0.0f, false);
        
        // Free the allocated hash
        free(newPINHash);
    }
    
    return result;
}

// Perform withdrawal operation
TransactionResult performWithdrawal(int cardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Check if ATM is in maintenance mode
    if (isMaintenanceModeActive()) {
        result.success = 0;
        strcpy(result.message, "Sorry, this ATM is currently in maintenance mode.");
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
        return result;
    }

    // Get withdrawal limit from system configuration
    float withdrawalLimit = get_config_float(CONFIG_ATM_WITHDRAWAL_LIMIT, 25000.0f);
    
    // Get daily transaction limit from system configuration
    float dailyLimit = get_config_float(CONFIG_DAILY_TRANSACTION_LIMIT, 50000.0f);
    
    // Validate withdrawal amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid withdrawal amount");
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
        return result;
    }
    
    // Check if card exists and is active
    if (!dao->doesCardExist(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
        return result;
    }
    
    if (!dao->isCardActive(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is inactive or blocked");
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
        return result;
    }
    
    // Check if amount exceeds the ATM withdrawal limit
    if (amount > withdrawalLimit) {
        result.success = 0;
        sprintf(result.message, "Error: Amount exceeds withdrawal limit of $%.2f", withdrawalLimit);
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
        return result;
    }
    
    // Check if amount exceeds daily withdrawal limit
    float todayWithdrawals = dao->getDailyWithdrawals(cardNumber);
    if (todayWithdrawals + amount > dailyLimit) {
        result.success = 0;
        sprintf(result.message, "Error: Would exceed daily transaction limit of $%.2f", dailyLimit);
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
        return result;
    }
    
    // Fetch current balance
    float oldBalance = dao->fetchBalance(cardNumber);
    if (oldBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch account balance");
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
        return result;
    }
    
    // Check if sufficient funds
    if (oldBalance < amount) {
        result.success = 0;
        sprintf(result.message, "Error: Insufficient funds. Current balance: $%.2f", oldBalance);
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
        return result;
    }
    
    // Perform withdrawal
    float newBalance = oldBalance - amount;
    if (dao->updateBalance(cardNumber, newBalance)) {
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Withdrawal successful. New balance: $%.2f", newBalance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Withdrew $%.2f. Old balance: $%.2f, New balance: $%.2f", 
               amount, oldBalance, newBalance);
        writeTransactionDetails(username, "Withdrawal", detailsLog);
        
        // Track withdrawal for daily limit using DAO
        dao->logWithdrawal(cardNumber, amount);
        
        // Log transaction using DAO
        dao->logTransaction(cardNumber, "Withdrawal", amount, true);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to complete withdrawal");
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
    }
    
    return result;
}

// New function for virtual withdrawal operation
TransactionResult performVirtualWithdrawal(const char* cardNumber, int cvv, const char* expiryDate, float amount, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Check if virtual ATM is enabled
    if (!is_virtual_atm_enabled()) {
        result.success = 0;
        strcpy(result.message, "Error: Virtual ATM functionality is currently disabled");
        return result;
    }
    
    // Convert string card number to int for DAO operations
    int cardNum = atoi(cardNumber);
    
    // Check if card exists
    if (!dao->doesCardExist(cardNum)) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        return result;
    }
    
    // Check if card is active
    if (!dao->isCardActive(cardNum)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is inactive or blocked");
        return result;
    }
    
    // Validate CVV 
    if (!dao->validateCardCVV(cardNum, cvv)) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid CVV");
        return result;
    }
    
    // Check expiry date
    if (!dao->validateCardExpiry(cardNum, expiryDate)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is expired");
        return result;
    }
    
    // Check if ATM is in maintenance mode
    if (isMaintenanceModeActive()) {
        result.success = 0;
        strcpy(result.message, "Sorry, the banking system is currently in maintenance mode.");
        dao->logTransaction(cardNum, "Virtual Withdrawal", amount, false);
        return result;
    }
    
    // Get withdrawal limit from system configuration for virtual transactions
    float withdrawalLimit = get_config_float(CONFIG_VIRTUAL_WITHDRAWAL_LIMIT, 10000.0f);
    
    // Get daily transaction limit from system configuration
    float dailyLimit = get_config_float(CONFIG_DAILY_TRANSACTION_LIMIT, 50000.0f);
    
    // Validate withdrawal amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid withdrawal amount");
        dao->logTransaction(cardNum, "Virtual Withdrawal", amount, false);
        return result;
    }
    
    // Check if amount exceeds the virtual withdrawal limit
    if (amount > withdrawalLimit) {
        result.success = 0;
        sprintf(result.message, "Error: Amount exceeds virtual withdrawal limit of $%.2f", withdrawalLimit);
        dao->logTransaction(cardNum, "Virtual Withdrawal", amount, false);
        return result;
    }
    
    // Check if amount exceeds daily withdrawal limit
    float dailyWithdrawals = dao->getDailyWithdrawals(cardNum);
    if (dailyWithdrawals + amount > dailyLimit) {
        result.success = 0;
        sprintf(result.message, "Error: Would exceed daily transaction limit of $%.2f", dailyLimit);
        dao->logTransaction(cardNum, "Virtual Withdrawal", amount, false);
        return result;
    }
    
    // Fetch current balance using DAO
    float oldBalance = dao->fetchBalance(cardNum);
    if (oldBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch account balance");
        dao->logTransaction(cardNum, "Virtual Withdrawal", amount, false);
        return result;
    }
    
    // Check if sufficient funds
    if (oldBalance < amount) {
        result.success = 0;
        sprintf(result.message, "Error: Insufficient funds. Current balance: $%.2f", oldBalance);
        dao->logTransaction(cardNum, "Virtual Withdrawal", amount, false);
        return result;
    }
    
    // Perform withdrawal by updating balance
    float newBalance = oldBalance - amount;
    if (dao->updateBalance(cardNum, newBalance)) {
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Virtual withdrawal successful. New balance: $%.2f", newBalance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Virtual withdrew $%.2f. Old balance: $%.2f, New balance: $%.2f", 
                amount, oldBalance, newBalance);
        writeTransactionDetails(username, "Virtual Withdrawal", detailsLog);
        
        // Track withdrawal for daily limit using DAO
        dao->logWithdrawal(cardNum, amount);
        
        // Log transaction using DAO
        dao->logTransaction(cardNum, "Virtual Withdrawal", amount, true);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to complete withdrawal");
        dao->logTransaction(cardNum, "Virtual Withdrawal", amount, false);
    }
    
    return result;
}

// Perform bill payment operation
TransactionResult performBillPayment(int cardNumber, float amount, const char* billerName, const char* accountId, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Validate bill payment amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid payment amount");
        dao->logTransaction(cardNumber, "Bill Payment", amount, false);
        return result;
    }
    
    // Check if ATM is in maintenance mode
    if (isMaintenanceModeActive()) {
        result.success = 0;
        strcpy(result.message, "Sorry, this ATM is currently in maintenance mode.");
        dao->logTransaction(cardNumber, "Bill Payment", amount, false);
        return result;
    }
    
    // Check if card exists and is active
    if (!dao->doesCardExist(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        dao->logTransaction(cardNumber, "Bill Payment", amount, false);
        return result;
    }
    
    if (!dao->isCardActive(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is inactive or blocked");
        dao->logTransaction(cardNumber, "Bill Payment", amount, false);
        return result;
    }
    
    // Fetch current balance using DAO
    float oldBalance = dao->fetchBalance(cardNumber);
    if (oldBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch account balance");
        dao->logTransaction(cardNumber, "Bill Payment", amount, false);
        return result;
    }
    
    // Check if sufficient funds
    if (oldBalance < amount) {
        result.success = 0;
        sprintf(result.message, "Error: Insufficient funds. Current balance: $%.2f", oldBalance);
        dao->logTransaction(cardNumber, "Bill Payment", amount, false);
        return result;
    }
    
    // Perform bill payment by updating balance using DAO
    float newBalance = oldBalance - amount;
    if (dao->updateBalance(cardNumber, newBalance)) {
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Bill payment successful. New balance: $%.2f", newBalance);
        
        // Log the transaction
        char detailsLog[200];
        sprintf(detailsLog, "Paid $%.2f to %s (Acc: %s). Old balance: $%.2f, New balance: $%.2f", 
                amount, billerName, accountId, oldBalance, newBalance);
        writeTransactionDetails(username, "Bill Payment", detailsLog);
        
        // Track payment using DAO
        dao->logTransaction(cardNumber, "Bill Payment", amount, true);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to complete bill payment");
        dao->logTransaction(cardNumber, "Bill Payment", amount, false);
    }
    
    return result;
}

// Perform virtual bill payment operation
TransactionResult performVirtualBillPayment(const char* cardNumber, int cvv, const char* expiryDate, 
                                           float amount, const char* billerName, const char* accountId, 
                                           const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Check if virtual ATM is enabled
    if (!is_virtual_atm_enabled()) {
        result.success = 0;
        strcpy(result.message, "Error: Virtual ATM functionality is currently disabled");
        return result;
    }
    
    // Convert string card number to int for DAO operations
    int cardNum = atoi(cardNumber);
    
    // Check if card exists
    if (!dao->doesCardExist(cardNum)) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        return result;
    }
    
    // Check if card is active
    if (!dao->isCardActive(cardNum)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is inactive or blocked");
        return result;
    }
    
    // Validate CVV 
    if (!dao->validateCardCVV(cardNum, cvv)) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid CVV");
        return result;
    }
    
    // Check expiry date
    if (!dao->validateCardExpiry(cardNum, expiryDate)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is expired");
        return result;
    }
    
    // Validate bill payment amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid payment amount");
        dao->logTransaction(cardNum, "Virtual Bill Payment", amount, false);
        return result;
    }
    
    // Check if ATM is in maintenance mode
    if (isMaintenanceModeActive()) {
        result.success = 0;
        strcpy(result.message, "Sorry, the banking system is currently in maintenance mode.");
        dao->logTransaction(cardNum, "Virtual Bill Payment", amount, false);
        return result;
    }
    
    // Fetch current balance using DAO
    float oldBalance = dao->fetchBalance(cardNum);
    if (oldBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch account balance");
        dao->logTransaction(cardNum, "Virtual Bill Payment", amount, false);
        return result;
    }
    
    // Check if sufficient funds
    if (oldBalance < amount) {
        result.success = 0;
        sprintf(result.message, "Error: Insufficient funds. Current balance: $%.2f", oldBalance);
        dao->logTransaction(cardNum, "Virtual Bill Payment", amount, false);
        return result;
    }
    
    // Perform bill payment by updating balance
    float newBalance = oldBalance - amount;
    if (dao->updateBalance(cardNum, newBalance)) {
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Virtual bill payment successful. New balance: $%.2f", newBalance);
        
        // Log the transaction
        char detailsLog[200];
        sprintf(detailsLog, "Virtual payment of $%.2f to %s (Acc: %s). Old balance: $%.2f, New balance: $%.2f", 
                amount, billerName, accountId, oldBalance, newBalance);
        writeTransactionDetails(username, "Virtual Bill Payment", detailsLog);
        
        // Log transaction using DAO
        dao->logTransaction(cardNum, "Virtual Bill Payment", amount, true);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to complete bill payment");
        dao->logTransaction(cardNum, "Virtual Bill Payment", amount, false);
    }
    
    return result;
}

// Function to log bill payment for tracking and reporting
void logBillPayment(int cardNumber, float amount, const char* billerName, const char* accountId) {
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Create timestamp
    char timestamp[30];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    
    // Create log record in a more detailed format that includes biller information
    char detailsLog[300];
    sprintf(detailsLog, "Bill payment of $%.2f to %s (Account: %s)", amount, billerName, accountId);
    
    // Log using DAO
    dao->logTransaction(cardNumber, "Bill Payment", amount, true);
    
    // Create a more detailed audit trail
    const char* paymentLogPath = isTestingMode() ? 
        TEST_DATA_DIR "/bill_payments.log" : 
        PROD_DATA_DIR "/../logs/bill_payments.log";
    
    FILE* file = fopen(paymentLogPath, "a");
    if (file != NULL) {
        fprintf(file, "%s|%d|%.2f|%s|%s|%s\n", timestamp, cardNumber, amount, billerName, accountId, detailsLog);
        fclose(file);
    } else {
        // Log error to main error log
        writeErrorLog("Failed to open bill payment log file");
    }
    
    // Update daily transaction limits using DAO
    dao->logWithdrawal(cardNumber, amount); // Track as a withdrawal for limit purposes
}

// Get mini statement (recent transactions)
TransactionResult getMiniStatement(int cardNumber, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Check if card exists and is active
    if (!dao->doesCardExist(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        dao->logTransaction(cardNumber, "Mini Statement", 0.0f, false);
        return result;
    }
    
    if (!dao->isCardActive(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is inactive or blocked");
        dao->logTransaction(cardNumber, "Mini Statement", 0.0f, false);
        return result;
    }
    
    // Get mini statement using DAO (last 10 transactions)
    Transaction transactions[10];
    int transactionCount = 0;
    
    // Call the DAO to get recent transactions
    if (!dao->getMiniStatement(cardNumber, transactions, 10, &transactionCount)) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to retrieve transaction history");
        dao->logTransaction(cardNumber, "Mini Statement", 0.0f, false);
        return result;
    }
    
    // If no transactions found
    if (transactionCount == 0) {
        result.success = 0;
        strcpy(result.message, "No transaction history available for this account.");
        dao->logTransaction(cardNumber, "Mini Statement", 0.0f, false);
        return result;
    }
    
    // Get current balance using DAO
    float balance = dao->fetchBalance(cardNumber);
    if (balance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch current balance.");
        dao->logTransaction(cardNumber, "Mini Statement", 0.0f, false);
        return result;
    }
    
    // Format the mini statement
    char miniStatement[1024] = "Recent Transactions:\n\n";
    strcat(miniStatement, "Date       | Type        | Amount    | Status\n");
    strcat(miniStatement, "-------------------------------------\n");
    
    // Format the transactions into a mini statement
    for (int i = 0; i < transactionCount; i++) {
        // Format date to show only the date part
        char dateStr[15];
        strncpy(dateStr, transactions[i].timestamp, 10);
        dateStr[10] = '\0';
        
        // Format the transaction
        char formattedTransaction[100];
        sprintf(formattedTransaction, "%s | %-10s | $%-8.2f | %s\n",
               dateStr, transactions[i].type, transactions[i].amount, 
               transactions[i].success ? "Success" : "Failed");
        strcat(miniStatement, formattedTransaction);
    }
    
    // Add balance information
    char line[50];
    strcat(miniStatement, "-------------------------------------\n");
    sprintf(line, "Current Balance: $%.2f", balance);
    strcat(miniStatement, line);
    
    // Set success result
    result.success = 1;
    result.newBalance = balance;
    result.oldBalance = balance;
    strcpy(result.message, miniStatement);
    
    // Log the mini statement request
    dao->logTransaction(cardNumber, "Mini Statement", 0.0f, true);
    
    // Log that mini-statement was requested
    char detailsLog[100];
    sprintf(detailsLog, "Mini statement requested. Current balance: $%.2f", balance);
    writeTransactionDetails(username, "Mini Statement", detailsLog);
    
    return result;
}

// New function to get mini statement with card number string
TransactionResult getVirtualMiniStatement(const char* cardNumber, int cvv, const char* expiryDate, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Check if virtual ATM is enabled
    if (!is_virtual_atm_enabled()) {
        result.success = 0;
        strcpy(result.message, "Error: Virtual ATM functionality is currently disabled");
        return result;
    }
    
    // Convert string card number to int for DAO operations
    int cardNum = atoi(cardNumber); 
    
    // Check if card exists
    if (!dao->doesCardExist(cardNum)) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        return result;
    }
    
    // Check if card is active
    if (!dao->isCardActive(cardNum)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is inactive or blocked");
        return result;
    }
    
    // Validate CVV 
    if (!dao->validateCardCVV(cardNum, cvv)) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid CVV");
        return result;
    }
    
    // Check expiry
    if (!dao->validateCardExpiry(cardNum, expiryDate)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is expired");
        return result;
    }
    
    // Use the existing implementation now that we've validated the card
    result = getMiniStatement(cardNum, username);
    
    // Update the message to indicate this was a virtual transaction
    if (result.success) {
        char virtualHeader[50] = "VIRTUAL TRANSACTION\n\n";
        char originalMessage[1024];
        strcpy(originalMessage, result.message);
        
        // Prepend the virtual header to the message
        strcpy(result.message, virtualHeader);
        strcat(result.message, originalMessage);
        
        // Log this as a virtual transaction
        dao->logTransaction(cardNum, "Virtual Mini Statement", 0.0f, true);
    }
    
    return result;
}

// Perform money transfer between accounts with transaction atomicity
TransactionResult performMoneyTransfer(int senderCardNumber, int receiverCardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Check if ATM is in maintenance mode
    if (isMaintenanceModeActive()) {
        result.success = 0;
        strcpy(result.message, "Sorry, this ATM is currently in maintenance mode.");
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
        return result;
    }
    
    // Validate transfer amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid transfer amount");
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
        return result;
    }
    
    // Check if sender card exists and is active
    if (!dao->doesCardExist(senderCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Your card is not found");
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
        return result;
    }
    
    if (!dao->isCardActive(senderCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Your card is inactive or blocked");
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
        return result;
    }
    
    // Check if receiver card exists and is active
    if (!dao->doesCardExist(receiverCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Recipient card number is invalid");
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
        return result;
    }
    
    if (!dao->isCardActive(receiverCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Recipient card is not active");
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
        return result;
    }
    
    // Lock the transaction files to ensure atomicity (this will be handled by the DAO in an actual implementation)
    if (!lockTransactionFiles()) {
        result.success = 0;
        strcpy(result.message, "Error: System busy, please try again later");
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
        return result;
    }
    
    // Start transaction - create backup of account files (this will be handled by the DAO in an actual implementation)
    if (!backupAccountFiles()) {
        unlockTransactionFiles();
        result.success = 0;
        strcpy(result.message, "Error: Could not initiate transaction safely");
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
        return result;
    }
    
    // Fetch sender balance
    float senderBalance = dao->fetchBalance(senderCardNumber);
    if (senderBalance < 0) {
        // Rollback by restoring from backup
        restoreAccountFiles();
        unlockTransactionFiles();
        
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch your account balance");
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
        return result;
    }
    
    // Check if sender has sufficient funds
    if (senderBalance < amount) {
        // No need to rollback, just release locks
        unlockTransactionFiles();
        
        result.success = 0;
        sprintf(result.message, "Error: Insufficient funds. Current balance: $%.2f", senderBalance);
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
        return result;
    }
    
    // Fetch receiver balance
    float receiverBalance = dao->fetchBalance(receiverCardNumber);
    if (receiverBalance < 0) {
        // Rollback by restoring from backup
        restoreAccountFiles();
        unlockTransactionFiles();
        
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch recipient's account balance");
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
        return result;
    }
    
    // Update balances of both sender and receiver using DAO
    bool senderUpdateSuccess = dao->updateBalance(senderCardNumber, senderBalance - amount);
    bool receiverUpdateSuccess = dao->updateBalance(receiverCardNumber, receiverBalance + amount);
    
    if (!senderUpdateSuccess || !receiverUpdateSuccess) {
        // Rollback if either update fails
        restoreAccountFiles();
        unlockTransactionFiles();
        
        result.success = 0;
        strcpy(result.message, "Error: Failed to complete transfer");
        dao->logTransaction(senderCardNumber, "Money Transfer", amount, false);
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
    
    dao->logTransaction(senderCardNumber, "Money Transfer", amount, true);
    
    // Also log for recipient
    char recipientName[50] = "Unknown"; // Default if we can't find name
    dao->getCardHolderName(receiverCardNumber, recipientName, sizeof(recipientName));
    
    char recipientLog[100];
    sprintf(recipientLog, "Received $%.2f from card %d (%s)", amount, senderCardNumber, username);
    writeTransactionDetails(recipientName, "Money Received", recipientLog);
    
    dao->logTransaction(receiverCardNumber, "Money Received", amount, true);
    
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