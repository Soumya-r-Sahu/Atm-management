#include "atm/transaction/transaction_manager.h"
#include "atm/transaction/transaction_types.h"
#include "common/database/database.h"
#include "common/database/database_compat.h"
#include "common/utils/logger.h"
#include "common/config/config_manager.h"
#include "common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// File locks for transaction atomicity
static int transaction_lock_fd = -1;

// Helper function to get current timestamp in string format
static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_now);
}

// Helper function to get current date in string format
static void get_current_date(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d", tm_now);
}

// Lock transaction files for atomicity
int lockTransactionFiles() {
    const char* lock_file = "data/transaction.lock";
    
    // Try to create lock file exclusively
    transaction_lock_fd = open(lock_file, O_WRONLY | O_CREAT | O_EXCL, 0666);
    
    if (transaction_lock_fd == -1) {
        if (errno == EEXIST) {
            write_error_log("Another transaction is in progress");
        } else {
            write_error_log("Could not create transaction lock");
        }
        return 0;
    }
    
    // Write process ID to lock file
    char pid_str[20];
    sprintf(pid_str, "%d", getpid());
    write(transaction_lock_fd, pid_str, strlen(pid_str));
    
    return 1;
}

// Unlock transaction files
int unlockTransactionFiles() {
    if (transaction_lock_fd != -1) {
        close(transaction_lock_fd);
        transaction_lock_fd = -1;
        
        const char* lock_file = "data/transaction.lock";
        if (remove(lock_file) != 0) {
            write_error_log("Could not remove transaction lock file");
            return 0;
        }
    }
    
    return 1;
}

// Backup account files before transaction
int backupAccountFiles() {
    const char* account_file = getAccountingFilePath();
    char backup_file[256];
    snprintf(backup_file, sizeof(backup_file), "%s.bak", account_file);
    
    FILE* src = fopen(account_file, "r");
    FILE* dst = fopen(backup_file, "w");
    
    if (!src || !dst) {
        if (src) fclose(src);
        if (dst) fclose(dst);
        write_error_log("Failed to create backup files");
        return 0;
    }
    
    char buffer[4096];
    size_t bytes;
    
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }
    
    fclose(src);
    fclose(dst);
    
    return 1;
}

// Restore account files if transaction failed
int restoreAccountFiles() {
    const char* account_file = getAccountingFilePath();
    char backup_file[256];
    snprintf(backup_file, sizeof(backup_file), "%s.bak", account_file);
    
    // Remove the current file
    if (remove(account_file) != 0) {
        write_error_log("Failed to remove account file for restoration");
        return 0;
    }
    
    // Rename backup to original
    if (rename(backup_file, account_file) != 0) {
        write_error_log("Failed to restore account file from backup");
        return 0;
    }
    
    return 1;
}

// Write transaction details to log
void writeTransactionDetails(const char* username, const char* type, const char* details) {
    FILE* file = fopen(getTransactionsLogFilePath(), "a");
    if (!file) {
        write_error_log("Could not open transaction log file");
        return;
    }
    
    char timestamp[25];
    get_timestamp(timestamp, sizeof(timestamp));
    
    // Format: Timestamp | Username | Type | Details
    fprintf(file, "%s | %s | %s | %s\n", 
            timestamp, username, type, details);
    
    fclose(file);
}

// Generate a transaction receipt
void generateReceipt(int cardNumber, TransactionType type, float amount, float balance, const char* phoneNumber) {
    char receipt_file[256];
    char timestamp[25];
    char date[15];
    
    get_timestamp(timestamp, sizeof(timestamp));
    get_current_date(date, sizeof(date));
    
    sprintf(receipt_file, "data/receipts/receipt_%d_%s.txt", cardNumber, date);
    
    FILE* file = fopen(receipt_file, "w");
    if (!file) {
        // Try to create the directory if it doesn't exist
        mkdir("data/receipts", 0755);
        file = fopen(receipt_file, "w");
        if (!file) {
            write_error_log("Could not create receipt file");
            return;
        }
    }
    
    // Get card holder name
    char holder_name[50] = "Card Holder";
    get_card_holder_name(cardNumber, holder_name, sizeof(holder_name));
    
    // Receipt header
    fprintf(file, "===================================\n");
    fprintf(file, "           ATM RECEIPT            \n");
    fprintf(file, "===================================\n\n");
    fprintf(file, "Date/Time: %s\n", timestamp);
    fprintf(file, "Card Number: XXXX-XXXX-XXXX-%04d\n", cardNumber % 10000);
    fprintf(file, "Customer: %s\n\n", holder_name);
    
    // Transaction details
    fprintf(file, "Transaction Type: ");
    switch (type) {
        case TRANSACTION_WITHDRAWAL:
            fprintf(file, "WITHDRAWAL\n");
            break;
        case TRANSACTION_DEPOSIT:
            fprintf(file, "DEPOSIT\n");
            break;
        case TRANSACTION_TRANSFER:
            fprintf(file, "TRANSFER\n");
            break;
        case TRANSACTION_BALANCE:
            fprintf(file, "BALANCE INQUIRY\n");
            break;
        case TRANSACTION_PIN_CHANGE:
            fprintf(file, "PIN CHANGE\n");
            break;
        case TRANSACTION_MINI_STATEMENT:
            fprintf(file, "MINI STATEMENT\n");
            break;
        default:
            fprintf(file, "OTHER\n");
    }
    
    // Amount and balance
    if (type != TRANSACTION_BALANCE && type != TRANSACTION_PIN_CHANGE && type != TRANSACTION_MINI_STATEMENT) {
        fprintf(file, "Amount: $%.2f\n", amount);
    }
    
    fprintf(file, "Available Balance: $%.2f\n\n", balance);
    
    // Footer
    fprintf(file, "-----------------------------------\n");
    fprintf(file, "Thank you for using our ATM service\n");
    fprintf(file, "===================================\n");
    
    fclose(file);
    
    // SMS notification if phone number is provided
    if (phoneNumber && strlen(phoneNumber) > 0) {
        char sms_message[300];
        
        sprintf(sms_message, "ATM Transaction: ");
        
        switch (type) {
            case TRANSACTION_WITHDRAWAL:
                sprintf(sms_message + strlen(sms_message), 
                        "Withdrawal of $%.2f completed. New balance: $%.2f", amount, balance);
                break;
            case TRANSACTION_DEPOSIT:
                sprintf(sms_message + strlen(sms_message), 
                        "Deposit of $%.2f completed. New balance: $%.2f", amount, balance);
                break;
            case TRANSACTION_TRANSFER:
                sprintf(sms_message + strlen(sms_message), 
                        "Transfer of $%.2f completed. New balance: $%.2f", amount, balance);
                break;
            default:
                sprintf(sms_message + strlen(sms_message), 
                        "Transaction completed. Current balance: $%.2f", balance);
        }
        
        // In a real system, this would call an SMS API
        write_info_log(sms_message);
    }
}

// Check account balance
TransactionResult checkAccountBalance(int cardNumber, const char* username) {
    TransactionResult result = {0};
    
    // Verify the card exists and is active
    if (!doesCardExist(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Card not found");
        return result;
    }
    
    if (!isCardActive(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Card is blocked or inactive");
        return result;
    }
    
    // Get the balance
    float balance = fetchBalance(cardNumber);
    if (balance < 0) {
        result.success = 0;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Set the result
    result.success = 1;
    result.oldBalance = balance;
    result.newBalance = balance;
    sprintf(result.message, "Current balance: $%.2f", balance);
    
    // Log the transaction
    char log_details[100];
    sprintf(log_details, "Balance inquiry: $%.2f", balance);
    writeTransactionDetails(username, "BALANCE", log_details);
    
    return result;
}

// Perform deposit
TransactionResult performDeposit(int cardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    
    // Validate amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Invalid deposit amount");
        return result;
    }
    
    // Lock transactions for atomicity
    if (!lockTransactionFiles()) {
        result.success = 0;
        strcpy(result.message, "System busy, try again later");
        return result;
    }
    
    // Backup account files
    backupAccountFiles();
    
    // Get current balance
    float oldBalance = fetchBalance(cardNumber);
    if (oldBalance < 0) {
        unlockTransactionFiles();
        result.success = 0;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Calculate new balance
    float newBalance = oldBalance + amount;
    
    // Update balance
    if (!updateBalance(cardNumber, newBalance)) {
        // Restore from backup if update failed
        restoreAccountFiles();
        unlockTransactionFiles();
        
        result.success = 0;
        strcpy(result.message, "Failed to update account balance");
        return result;
    }
    
    // Get card holder phone number for receipt
    char phoneNumber[20] = "";
    get_card_holder_phone(cardNumber, phoneNumber, sizeof(phoneNumber));
    
    // Unlock files
    unlockTransactionFiles();
    
    // Set result
    result.success = 1;
    result.oldBalance = oldBalance;
    result.newBalance = newBalance;
    sprintf(result.message, "Successfully deposited $%.2f. New balance: $%.2f", amount, newBalance);
    
    // Log the transaction
    char log_details[100];
    sprintf(log_details, "Deposit: $%.2f, New balance: $%.2f", amount, newBalance);
    writeTransactionDetails(username, "DEPOSIT", log_details);
    
    // Generate receipt
    generateReceipt(cardNumber, TRANSACTION_DEPOSIT, amount, newBalance, phoneNumber);
    
    logTransaction(cardNumber, TRANSACTION_DEPOSIT, amount, 1);
    
    return result;
}

// Perform withdrawal
TransactionResult performWithdrawal(int cardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    
    // Validate amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Invalid withdrawal amount");
        return result;
    }
    
    // Lock transactions for atomicity
    if (!lockTransactionFiles()) {
        result.success = 0;
        strcpy(result.message, "System busy, try again later");
        return result;
    }
    
    // Get config for withdrawal limits
    float min_amount = 20.0f;  // Default minimum
    float max_amount = 1000.0f; // Default maximum
    
    const char* min_str = get_config_value("min_withdrawal_limit");
    if (min_str) {
        float config_min = atof(min_str);
        if (config_min > 0) min_amount = config_min;
    }
    
    const char* max_str = get_config_value("max_withdrawal_limit");
    if (max_str) {
        float config_max = atof(max_str);
        if (config_max > 0) max_amount = config_max;
    }
    
    // Check amount limits
    if (amount < min_amount) {
        unlockTransactionFiles();
        result.success = 0;
        sprintf(result.message, "Minimum withdrawal amount is $%.2f", min_amount);
        return result;
    }
    
    if (amount > max_amount) {
        unlockTransactionFiles();
        result.success = 0;
        sprintf(result.message, "Maximum withdrawal amount is $%.2f", max_amount);
        return result;
    }
    
    // Check daily withdrawal limit
    if (has_exceeded_daily_limit(cardNumber, amount)) {
        unlockTransactionFiles();
        result.success = 0;
        strcpy(result.message, "Daily withdrawal limit exceeded");
        return result;
    }
    
    // Backup account files
    backupAccountFiles();
    
    // Get current balance
    float oldBalance = fetchBalance(cardNumber);
    if (oldBalance < 0) {
        unlockTransactionFiles();
        result.success = 0;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Check if sufficient balance
    if (oldBalance < amount) {
        unlockTransactionFiles();
        result.success = 0;
        strcpy(result.message, "Insufficient balance for withdrawal");
        return result;
    }
    
    // Calculate new balance
    float newBalance = oldBalance - amount;
    
    // Update balance
    if (!updateBalance(cardNumber, newBalance)) {
        // Restore from backup if update failed
        restoreAccountFiles();
        unlockTransactionFiles();
        
        result.success = 0;
        strcpy(result.message, "Failed to update account balance");
        return result;
    }
    
    // Get card holder phone number for receipt
    char phoneNumber[20] = "";
    get_card_holder_phone(cardNumber, phoneNumber, sizeof(phoneNumber));
    
    // Log withdrawal for daily limit tracking
    char date[15];
    get_current_date(date, sizeof(date));
    log_withdrawal_for_limit(cardNumber, amount, date);
    
    // Unlock files
    unlockTransactionFiles();
    
    // Set result
    result.success = 1;
    result.oldBalance = oldBalance;
    result.newBalance = newBalance;
    sprintf(result.message, "Successfully withdrew $%.2f. New balance: $%.2f", amount, newBalance);
    
    // Log the transaction
    char log_details[100];
    sprintf(log_details, "Withdrawal: $%.2f, New balance: $%.2f", amount, newBalance);
    writeTransactionDetails(username, "WITHDRAWAL", log_details);
    
    // Generate receipt
    generateReceipt(cardNumber, TRANSACTION_WITHDRAWAL, amount, newBalance, phoneNumber);
    
    logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, 1);
    
    return result;
}

// Get mini statement for account
TransactionResult getMiniStatement(int cardNumber, const char* username) {
    TransactionResult result = {0};
    
    // Verify the card exists and is active
    if (!doesCardExist(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Card not found");
        return result;
    }
    
    if (!isCardActive(cardNumber)) {
        result.success = 0;
        strcpy(result.message, "Card is blocked or inactive");
        return result;
    }
    
    // Get current balance
    float balance = fetchBalance(cardNumber);
    if (balance < 0) {
        result.success = 0;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // In a real system, we would query transaction history from a database
    // Here we'll simulate by building a string with recent transactions
    
    const char* transactions_log = getTransactionsLogFilePath();
    FILE* file = fopen(transactions_log, "r");
    
    if (!file) {
        // No transaction log, still return success but with a message
        result.success = 1;
        result.oldBalance = balance;
        result.newBalance = balance;
        strcpy(result.message, "No recent transactions found. Current balance: $%.2f");
        return result;
    }
    
    // Build mini statement string
    char statement[500] = "";
    char line[256];
    int count = 0;
    
    // We'll show last 5 transactions in reverse order
    // In a real system, you would use a database query with ORDER BY
    
    // Read all lines first (inefficient but simple)
    char lines[10][256]; // Store up to 10 recent transactions
    int line_count = 0;
    
    while (fgets(line, sizeof(line), file) && line_count < 10) {
        // Look for this card number in the transaction log
        // Format depends on how transactions are logged
        if (strstr(line, username) != NULL) {
            strcpy(lines[line_count], line);
            line_count++;
        }
    }
    
    fclose(file);
    
    // Format the mini statement
    strcat(statement, "Recent Transactions:\n");
    
    // Show transactions in reverse order (most recent first)
    for (int i = line_count - 1; i >= 0 && count < 5; i--, count++) {
        char formatted_line[100];
        // Simplified formatting - in a real system you'd parse and format nicely
        sprintf(formatted_line, "%d. %s", count + 1, lines[i]);
        strcat(statement, formatted_line);
    }
    
    if (count == 0) {
        strcat(statement, "No recent transactions found.\n");
    }
    
    // Add current balance
    char balance_str[50];
    sprintf(balance_str, "\nCurrent Balance: $%.2f", balance);
    strcat(statement, balance_str);
    
    // Set result
    result.success = 1;
    result.oldBalance = balance;
    result.newBalance = balance;
    strncpy(result.message, statement, sizeof(result.message) - 1);
    result.message[sizeof(result.message) - 1] = '\0';
    
    // Log the transaction
    writeTransactionDetails(username, "MINI_STATEMENT", "Retrieved mini statement");
    
    // Generate receipt
    generateReceipt(cardNumber, TRANSACTION_MINI_STATEMENT, 0, balance, NULL);
    
    logTransaction(cardNumber, TRANSACTION_MINI_STATEMENT, 0.0f, 1);
    
    return result;
}

// Perform money transfer between accounts
TransactionResult performMoneyTransfer(int senderCardNumber, int receiverCardNumber, float amount, const char* username) {
    TransactionResult result = {0};
    
    // Validate amount
    if (amount <= 0) {
        result.success = 0;
        strcpy(result.message, "Invalid transfer amount");
        return result;
    }
    
    // Check sender card
    if (!doesCardExist(senderCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Sender card not found");
        return result;
    }
    
    if (!isCardActive(senderCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Sender card is blocked or inactive");
        return result;
    }
    
    // Check receiver card
    if (!doesCardExist(receiverCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Receiver card not found");
        return result;
    }
    
    if (!isCardActive(receiverCardNumber)) {
        result.success = 0;
        strcpy(result.message, "Receiver card is blocked or inactive");
        return result;
    }
    
    // Lock transactions for atomicity
    if (!lockTransactionFiles()) {
        result.success = 0;
        strcpy(result.message, "System busy, try again later");
        return result;
    }
    
    // Backup account files
    backupAccountFiles();
    
    // Get sender balance
    float senderBalance = fetchBalance(senderCardNumber);
    if (senderBalance < 0) {
        unlockTransactionFiles();
        result.success = 0;
        strcpy(result.message, "Could not retrieve sender account balance");
        return result;
    }
    
    // Check if sufficient balance
    if (senderBalance < amount) {
        unlockTransactionFiles();
        result.success = 0;
        strcpy(result.message, "Insufficient balance for transfer");
        return result;
    }
    
    // Get receiver balance
    float receiverBalance = fetchBalance(receiverCardNumber);
    if (receiverBalance < 0) {
        unlockTransactionFiles();
        result.success = 0;
        strcpy(result.message, "Could not retrieve receiver account balance");
        return result;
    }
    
    // Calculate new balances
    float newSenderBalance = senderBalance - amount;
    float newReceiverBalance = receiverBalance + amount;
    
    // Update sender balance
    if (!updateBalance(senderCardNumber, newSenderBalance)) {
        // Restore from backup if update failed
        restoreAccountFiles();
        unlockTransactionFiles();
        
        result.success = 0;
        strcpy(result.message, "Failed to update sender account balance");
        return result;
    }
    
    // Update receiver balance
    if (!updateBalance(receiverCardNumber, newReceiverBalance)) {
        // Restore from backup if update failed
        restoreAccountFiles();
        unlockTransactionFiles();
        
        result.success = 0;
        strcpy(result.message, "Failed to update receiver account balance");
        return result;
    }
    
    // Get card holder phone number for receipt
    char phoneNumber[20] = "";
    get_card_holder_phone(senderCardNumber, phoneNumber, sizeof(phoneNumber));
    
    // Unlock files
    unlockTransactionFiles();
    
    // Set result
    result.success = 1;
    result.oldBalance = senderBalance;
    result.newBalance = newSenderBalance;
    sprintf(result.message, "Successfully transferred $%.2f. New balance: $%.2f", 
            amount, newSenderBalance);
    
    // Log the transaction
    char log_details[150];
    sprintf(log_details, "Transfer: $%.2f from card %d to card %d. New sender balance: $%.2f", 
            amount, senderCardNumber, receiverCardNumber, newSenderBalance);
    writeTransactionDetails(username, "TRANSFER", log_details);
    
    // Generate receipt
    generateReceipt(senderCardNumber, TRANSACTION_TRANSFER, amount, newSenderBalance, phoneNumber);
    
    logTransaction(senderCardNumber, TRANSACTION_TRANSFER, amount, 1);
    
    return result;
}

// Alias for backward compatibility
TransactionResult performFundTransfer(int cardNumber, int targetCardNumber, float amount, const char* username) {
    return performMoneyTransfer(cardNumber, targetCardNumber, amount, username);
}