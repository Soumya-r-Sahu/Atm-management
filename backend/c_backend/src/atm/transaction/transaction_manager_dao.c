#include <atm/transaction/transaction_manager.h>
#include <common/database/dao_interface.h>   // Use DAO interface instead of direct database
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

// Write detailed transaction information to log
void writeTransactionDetails(const char* username, const char* transactionType, const char* details) {
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Create a log message with all details
    char logMessage[512];
    snprintf(logMessage, sizeof(logMessage), "[%s] %s performed %s: %s", 
            timestamp, username ? username : "Unknown user", transactionType, details);
    
    writeInfoLog(logMessage);
    
    // Also write to audit log for compliance purposes
    FILE* auditLog = fopen(getAuditLogFilePath(), "a");
    if (auditLog) {
        fprintf(auditLog, "%s\n", logMessage);
        fclose(auditLog);
    }
}

// Perform balance inquiry operation
TransactionResult performBalanceInquiry(int cardNumber, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Check if card exists
    if (!dao->doesCardExist(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        return result;
    }
    
    // Check if card is active
    if (!dao->isCardActive(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card is inactive or blocked");
        return result;
    }
    
    // Get current balance using DAO
    float balance = dao->fetchBalance(cardNumber);
    
    if (balance >= 0) {
        result.success = 1;
        result.newBalance = balance;
        result.oldBalance = balance;
        sprintf(result.message, "Current balance: $%.2f", balance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Checked balance: $%.2f", balance);
        writeTransactionDetails(username, "Balance Inquiry", detailsLog);
        
        // Log transaction using DAO
        dao->logTransaction(cardNumber, "Balance Inquiry", 0.0f, true);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch balance");
        
        // Log failed transaction
        dao->logTransaction(cardNumber, "Balance Inquiry", 0.0f, false);
    }
    
    return result;
}

// Perform virtual balance inquiry operation
TransactionResult performVirtualBalanceInquiry(const char* cardNumber, int cvv, const char* expiryDate, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Check if virtual ATM is enabled
    if (!is_virtual_atm_enabled()) {
        result.success = 0;
        strcpy(result.message, "Error: Virtual ATM functionality is currently disabled");
        return result;
    }
    
    // Convert string card number to int
    int cardNum = atoi(cardNumber);
    
    // Validate card format
    if (!validate_card_format(cardNum)) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid card number format");
        return result;
    }
    
    // Check if card exists
    if (!dao->doesCardExist(cardNum)) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found");
        return result;
    }
    
    // Validate CVV
    if (!dao->validateCardCVV(cardNum, cvv)) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid CVV");
        return result;
    }
    
    // Get current balance using DAO
    float balance = dao->fetchBalance(cardNum);
    
    if (balance >= 0) {
        result.success = 1;
        result.newBalance = balance;
        result.oldBalance = balance;
        sprintf(result.message, "Current balance: $%.2f", balance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Checked balance: $%.2f", balance);
        writeTransactionDetails(username, "Virtual Balance Check", detailsLog);
        
        // Log transaction using DAO
        dao->logTransaction(cardNum, "Virtual Balance Check", 0.0f, true);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch balance");
        
        // Log failed transaction
        dao->logTransaction(cardNum, "Virtual Balance Check", 0.0f, false);
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
        dao->logTransaction(cardNumber, "Deposit", amount, false);
        return result;
    }
    
    // Check if card exists and is active
    if (!dao->doesCardExist(cardNumber) || !dao->isCardActive(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Card not found or inactive");
        dao->logTransaction(cardNumber, "Deposit", amount, false);
        return result;
    }
    
    // Fetch current balance using DAO
    float oldBalance = dao->fetchBalance(cardNumber);
    if (oldBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch account balance");
        dao->logTransaction(cardNumber, "Deposit", amount, false);
        return result;
    }
    
    // Perform deposit by updating balance
    float newBalance = oldBalance + amount;
    if (dao->updateBalance(cardNumber, newBalance)) {
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Deposit successful. New balance: $%.2f", newBalance);
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Deposited $%.2f. Old balance: $%.2f, New balance: $%.2f", 
               amount, oldBalance, newBalance);
        writeTransactionDetails(username, "Deposit", detailsLog);
        
        dao->logTransaction(cardNumber, "Deposit", amount, true);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to update balance");
        dao->logTransaction(cardNumber, "Deposit", amount, false);
    }
    
    return result;
}

// Perform withdrawal operation
TransactionResult performWithdrawal(int cardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
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
    
    // Get ATM withdrawal limit
    float withdrawalLimit = get_config_float(CONFIG_ATM_WITHDRAWAL_LIMIT, 10000.0f);
    if (amount > withdrawalLimit) {
        result.success = 0;
        sprintf(result.message, "Error: Withdrawal amount exceeds ATM limit of $%.2f", withdrawalLimit);
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
        return result;
    }
    
    // Check daily withdrawal limit
    float dailyLimit = get_config_float(CONFIG_DAILY_TRANSACTION_LIMIT, 20000.0f);
    float dailyWithdrawals = dao->getDailyWithdrawals(cardNumber);
    
    if (dailyWithdrawals + amount > dailyLimit) {
        result.success = 0;
        sprintf(result.message, "Error: Daily withdrawal limit of $%.2f exceeded", dailyLimit);
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
    
    // Check if enough balance
    if (oldBalance < amount) {
        result.success = 0;
        strcpy(result.message, "Error: Insufficient funds");
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
        return result;
    }
    
    // Perform withdrawal by updating balance
    float newBalance = oldBalance - amount;
    if (dao->updateBalance(cardNumber, newBalance)) {
        result.success = 1;
        result.oldBalance = oldBalance;
        result.newBalance = newBalance;
        sprintf(result.message, "Withdrawal successful. New balance: $%.2f", newBalance);
        
        // Log the withdrawal for daily limit tracking
        dao->logWithdrawal(cardNumber, amount);
        
        // Log transaction details
        char detailsLog[100];
        sprintf(detailsLog, "Withdrawn $%.2f. Old balance: $%.2f, New balance: $%.2f", 
               amount, oldBalance, newBalance);
        writeTransactionDetails(username, "Withdrawal", detailsLog);
        
        // Log the transaction
        dao->logTransaction(cardNumber, "Withdrawal", amount, true);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to update balance");
        dao->logTransaction(cardNumber, "Withdrawal", amount, false);
    }
    
    return result;
}

// Perform mini statement operation
TransactionResult performMiniStatement(int cardNumber, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
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
    
    // Get mini statement using DAO (last 10 transactions)
    Transaction transactions[10];
    int transactionCount = 0;
    
    if (dao->getMiniStatement(cardNumber, transactions, 10, &transactionCount)) {
        result.success = 1;
        sprintf(result.message, "Retrieved %d recent transactions", transactionCount);
        
        // In a real implementation, the transactions would be attached to the result
        // But for this example, we just indicate success
        
        // Log the transaction
        char detailsLog[100];
        sprintf(detailsLog, "Retrieved mini statement with %d transactions", transactionCount);
        writeTransactionDetails(username, "Mini Statement", detailsLog);
        
        dao->logTransaction(cardNumber, "Mini Statement", 0.0f, true);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to retrieve transaction history");
        dao->logTransaction(cardNumber, "Mini Statement", 0.0f, false);
    }
    
    return result;
}

// Perform PIN change operation
TransactionResult performPINChange(int cardNumber, int oldPIN, int newPIN, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
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
    
    // Generate PIN hash (in a real implementation, use a secure hashing algorithm)
    char newPINHash[100];
    sprintf(newPINHash, "hash_%04d", newPIN);  // This is just a dummy implementation
    
    // Update PIN using DAO
    if (dao->updateCardPIN(cardNumber, newPINHash)) {
        result.success = 1;
        strcpy(result.message, "PIN changed successfully");
        
        // Log the transaction
        writeTransactionDetails(username, "PIN Change", "PIN changed successfully");
        dao->logTransaction(cardNumber, "PIN Change", 0.0f, true);
    } else {
        result.success = 0;
        strcpy(result.message, "Error: Unable to update PIN");
        dao->logTransaction(cardNumber, "PIN Change", 0.0f, false);
    }
    
    return result;
}

// Perform fund transfer operation
TransactionResult performFundTransfer(int sourceCardNumber, int destinationCardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    DatabaseAccessObject* dao = getDAO();  // Get DAO instance
    
    // Validate amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Error: Invalid transfer amount");
        dao->logTransaction(sourceCardNumber, "Fund Transfer", amount, false);
        return result;
    }
    
    // Check if source card exists and is active
    if (!dao->doesCardExist(sourceCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Source card not found");
        dao->logTransaction(sourceCardNumber, "Fund Transfer", amount, false);
        return result;
    }
    
    if (!dao->isCardActive(sourceCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Source card is inactive or blocked");
        dao->logTransaction(sourceCardNumber, "Fund Transfer", amount, false);
        return result;
    }
    
    // Check if destination card exists
    if (!dao->doesCardExist(destinationCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Destination card not found");
        dao->logTransaction(sourceCardNumber, "Fund Transfer", amount, false);
        return result;
    }
    
    // Check if destination card is active
    if (!dao->isCardActive(destinationCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Error: Destination card is inactive or blocked");
        dao->logTransaction(sourceCardNumber, "Fund Transfer", amount, false);
        return result;
    }
    
    // Fetch source balance
    float sourceBalance = dao->fetchBalance(sourceCardNumber);
    if (sourceBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch source account balance");
        dao->logTransaction(sourceCardNumber, "Fund Transfer", amount, false);
        return result;
    }
    
    // Check if enough balance in source account
    if (sourceBalance < amount) {
        result.success = 0;
        strcpy(result.message, "Error: Insufficient funds in source account");
        dao->logTransaction(sourceCardNumber, "Fund Transfer", amount, false);
        return result;
    }
    
    // Fetch destination balance
    float destinationBalance = dao->fetchBalance(destinationCardNumber);
    if (destinationBalance < 0) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to fetch destination account balance");
        dao->logTransaction(sourceCardNumber, "Fund Transfer", amount, false);
        return result;
    }
    
    // Update source balance
    float newSourceBalance = sourceBalance - amount;
    if (!dao->updateBalance(sourceCardNumber, newSourceBalance)) {
        result.success = 0;
        strcpy(result.message, "Error: Unable to update source account balance");
        dao->logTransaction(sourceCardNumber, "Fund Transfer", amount, false);
        return result;
    }
    
    // Update destination balance
    float newDestinationBalance = destinationBalance + amount;
    if (!dao->updateBalance(destinationCardNumber, newDestinationBalance)) {
        // Revert source account change if destination update fails
        dao->updateBalance(sourceCardNumber, sourceBalance);
        
        result.success = 0;
        strcpy(result.message, "Error: Unable to update destination account balance");
        dao->logTransaction(sourceCardNumber, "Fund Transfer", amount, false);
        return result;
    }
    
    // Transfer successful
    result.success = 1;
    result.oldBalance = sourceBalance;
    result.newBalance = newSourceBalance;
    sprintf(result.message, "Transfer successful. New balance: $%.2f", newSourceBalance);
    
    // Get destination account holder name
    char destinationName[100] = "recipient";
    dao->getCardHolderName(destinationCardNumber, destinationName, sizeof(destinationName));
    
    // Log the transaction
    char detailsLog[200];
    sprintf(detailsLog, "Transferred $%.2f to %s (Card: %d). Source balance: $%.2f → $%.2f", 
           amount, destinationName, destinationCardNumber, sourceBalance, newSourceBalance);
    writeTransactionDetails(username, "Fund Transfer", detailsLog);
    
    // Log transactions for both accounts
    dao->logTransaction(sourceCardNumber, "Fund Transfer Out", amount, true);
    dao->logTransaction(destinationCardNumber, "Fund Transfer In", amount, true);
    
    return result;
}

// Check if maintenance mode is active
bool isMaintenanceModeActive(void) {
    return get_config_bool(CONFIG_MAINTENANCE_MODE, false);
}

// Check if virtual ATM is enabled
bool is_virtual_atm_enabled(void) {
    return get_config_bool(CONFIG_VIRTUAL_ATM_ENABLED, false);
}

// Get the virtual ATM withdrawal limit
float get_virtual_withdrawal_limit(void) {
    return get_config_float(CONFIG_VIRTUAL_WITHDRAWAL_LIMIT, 5000.0f);
}
