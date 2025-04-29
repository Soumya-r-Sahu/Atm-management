#include "customer_profile.h"
#include "../common/paths.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // Added for isspace()
#include <time.h>

// Helper function to parse a time string into a time_t value
static time_t parseTimeString(const char* timeString) {
    struct tm tm = {0};
    // Try to parse as full date time format first: YYYY-MM-DD HH:MM:SS
    if (sscanf(timeString, "%d-%d-%d %d:%d:%d", 
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday, 
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {
        tm.tm_year -= 1900;  // Years since 1900
        tm.tm_mon -= 1;      // Months are 0-11
        return mktime(&tm);
    }
    
    // Try to parse as date only format: YYYY-MM-DD
    if (sscanf(timeString, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) == 3) {
        tm.tm_year -= 1900;  // Years since 1900
        tm.tm_mon -= 1;      // Months are 0-11
        return mktime(&tm);
    }
    
    // If parsing fails, return current time
    return time(NULL);
}

// Helper function to format a time_t value into a date string
static void formatDateString(time_t timestamp, char* buffer, size_t bufferSize) {
    struct tm* tm = localtime(&timestamp);
    strftime(buffer, bufferSize, "%Y-%m-%d", tm);
}

// Helper function to format a time_t value into a date time string
static void formatDateTimeString(time_t timestamp, char* buffer, size_t bufferSize) {
    struct tm* tm = localtime(&timestamp);
    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", tm);
}

// Helper function to trim whitespace
static char* trim(const char* str) {
    static char buffer[256];
    if (str == NULL) return NULL;
    
    strncpy(buffer, str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    char* result = buffer;
    
    // Remove leading spaces
    while (*result != '\0' && isspace(*result)) {
        result++;
    }
    
    // Remove trailing spaces
    if (*result != '\0') {
        char* end = result + strlen(result) - 1;
        while (end > result && isspace(*end)) {
            *end = '\0';
            end--;
        }
    }
    
    return result;
}

// Helper function to parse customer status string
static CustomerStatus parseCustomerStatus(const char* statusStr) {
    char* trimmed = trim(strdup(statusStr));
    CustomerStatus status;
    
    if (strcasecmp(trimmed, "Active") == 0) {
        status = CUSTOMER_ACTIVE;
    } else if (strcasecmp(trimmed, "Inactive") == 0) {
        status = CUSTOMER_INACTIVE;
    } else {
        status = CUSTOMER_SUSPENDED;
    }
    
    free(trimmed);
    return status;
}

// Helper function to parse KYC status string
static KYCStatus parseKYCStatus(const char* statusStr) {
    char* trimmed = trim(strdup(statusStr));
    KYCStatus status;
    
    if (strcasecmp(trimmed, "Completed") == 0) {
        status = KYC_COMPLETED;
    } else {
        status = KYC_PENDING;
    }
    
    free(trimmed);
    return status;
}

// Helper function to parse card status string
static CardStatus parseCardStatus(const char* statusStr) {
    char* trimmed = trim(strdup(statusStr));
    CardStatus status;
    
    if (strcasecmp(trimmed, "Active") == 0) {
        status = CARD_ACTIVE;
    } else if (strcasecmp(trimmed, "Expired") == 0) {
        status = CARD_EXPIRED;
    } else {
        status = CARD_BLOCKED;
    }
    
    free(trimmed);
    return status;
}

// Helper function to parse account status string
static AccountStatus parseAccountStatus(const char* statusStr) {
    char* trimmed = trim(strdup(statusStr));
    AccountStatus status;
    
    if (strcasecmp(trimmed, "Active") == 0) {
        status = ACCOUNT_ACTIVE;
    } else if (strcasecmp(trimmed, "Closed") == 0) {
        status = ACCOUNT_CLOSED;
    } else {
        status = ACCOUNT_INACTIVE;
    }
    
    free(trimmed);
    return status;
}

// Helper function to parse account type string
static AccountType parseAccountType(const char* typeStr) {
    char* trimmed = trim(strdup(typeStr));
    AccountType type;
    
    if (strcasecmp(trimmed, "Current") == 0) {
        type = ACCOUNT_CURRENT;
    } else if (strcasecmp(trimmed, "FD") == 0) {
        type = ACCOUNT_FD;
    } else {
        type = ACCOUNT_SAVINGS;
    }
    
    free(trimmed);
    return type;
}

// Helper function to parse card type string
static CardType parseCardType(const char* typeStr) {
    char* trimmed = trim(strdup(typeStr));
    CardType type;
    
    if (strcasecmp(trimmed, "Credit") == 0) {
        type = CARD_CREDIT;
    } else {
        type = CARD_DEBIT;
    }
    
    free(trimmed);
    return type;
}

// Find a card by card number
static bool findCardByCardNumber(int cardNumber, Card* card) {
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card file");
        return false;
    }
    
    char line[512];
    bool found = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    while (fgets(line, sizeof(line), file) != NULL && !found) {
        char cardId[11], accountId[11], expiryDate[11], status[20], pinHash[65], cardType[20];
        int storedCardNumber;
        
        if (sscanf(line, "%10[^|] | %10[^|] | %d | %19[^|] | %10[^|] | %19[^|] | %64s",
                  cardId, accountId, &storedCardNumber, cardType, expiryDate, status, pinHash) == 7) {
            
            if (storedCardNumber == cardNumber) {
                // Found matching card
                strncpy(card->cardId, trim(cardId), sizeof(card->cardId) - 1);
                strncpy(card->accountId, trim(accountId), sizeof(card->accountId) - 1);
                card->cardNumber = storedCardNumber;
                card->cardType = parseCardType(cardType);
                strncpy(card->expiryDate, trim(expiryDate), sizeof(card->expiryDate) - 1);
                card->status = parseCardStatus(status);
                strncpy(card->pinHash, trim(pinHash), sizeof(card->pinHash) - 1);
                
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

// Find an account by account ID
static bool findAccountById(const char* accountId, Account* account) {
    FILE* file = fopen(getCustomerFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open account file");
        return false;
    }
    
    char line[512];
    bool found = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Account ID | Customer ID | Account Type | Balance | Branch Code | Account Status | Created At | Last Transaction
    while (fgets(line, sizeof(line), file) != NULL && !found) {
        char storedAccountId[11], customerId[11], accountType[20], branchCode[10], accountStatus[20];
        char createdAtStr[30], lastTransactionStr[30];
        float balance;
        
        if (sscanf(line, "%10[^|] | %10[^|] | %19[^|] | %f | %9[^|] | %19[^|] | %29[^|] | %29[^\n]",
                  storedAccountId, customerId, accountType, &balance, branchCode, 
                  accountStatus, createdAtStr, lastTransactionStr) == 8) {
            
            if (strcmp(trim(storedAccountId), trim(accountId)) == 0) {
                // Found matching account
                strncpy(account->accountId, trim(storedAccountId), sizeof(account->accountId) - 1);
                strncpy(account->customerId, trim(customerId), sizeof(account->customerId) - 1);
                account->accountType = parseAccountType(accountType);
                account->balance = balance;
                strncpy(account->branchCode, trim(branchCode), sizeof(account->branchCode) - 1);
                account->accountStatus = parseAccountStatus(accountStatus);
                account->createdAt = parseTimeString(trim(createdAtStr));
                account->lastTransaction = parseTimeString(trim(lastTransactionStr));
                
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

// Find a customer by customer ID
static bool findCustomerById(const char* customerId, CustomerProfile* profile) {
    FILE* file = fopen(getCustomerFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open customer file");
        return false;
    }
    
    char line[512];
    bool found = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Customer ID | Name | DOB | Address | Email | Mobile Number | KYC Status | Status | Created At | Last Login
    while (fgets(line, sizeof(line), file) != NULL && !found) {
        char storedCustomerId[11], name[100], dob[11], address[200], email[100], mobileNumber[15];
        char kycStatus[20], status[20], createdAtStr[30], lastLoginStr[30];
        
        if (sscanf(line, "%10[^|] | %99[^|] | %10[^|] | %199[^|] | %99[^|] | %14[^|] | %19[^|] | %19[^|] | %29[^|] | %29[^\n]",
                  storedCustomerId, name, dob, address, email, mobileNumber, 
                  kycStatus, status, createdAtStr, lastLoginStr) == 10) {
            
            if (strcmp(trim(storedCustomerId), trim(customerId)) == 0) {
                // Found matching customer
                strncpy(profile->customerId, trim(storedCustomerId), sizeof(profile->customerId) - 1);
                strncpy(profile->name, trim(name), sizeof(profile->name) - 1);
                strncpy(profile->dob, trim(dob), sizeof(profile->dob) - 1);
                strncpy(profile->address, trim(address), sizeof(profile->address) - 1);
                strncpy(profile->email, trim(email), sizeof(profile->email) - 1);
                strncpy(profile->mobileNumber, trim(mobileNumber), sizeof(profile->mobileNumber) - 1);
                profile->kycStatus = parseKYCStatus(kycStatus);
                profile->status = parseCustomerStatus(status);
                profile->createdAt = parseTimeString(trim(createdAtStr));
                profile->lastLogin = parseTimeString(trim(lastLoginStr));
                
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

// Load customer profile by card number
bool loadCustomerProfileByCardNumber(int cardNumber, CustomerProfile* profile) {
    if (profile == NULL) {
        writeErrorLog("NULL profile pointer provided to loadCustomerProfileByCardNumber");
        return false;
    }
    
    // First find the card
    Card card;
    if (!findCardByCardNumber(cardNumber, &card)) {
        char errorMsg[100];
        sprintf(errorMsg, "Card not found for card number %d", cardNumber);
        writeErrorLog(errorMsg);
        return false;
    }
    
    // Next find the account using the account ID from the card
    Account account;
    if (!findAccountById(card.accountId, &account)) {
        char errorMsg[100];
        sprintf(errorMsg, "Account not found for account ID %s", card.accountId);
        writeErrorLog(errorMsg);
        return false;
    }
    
    // Finally, find the customer using the customer ID from the account
    if (!findCustomerById(account.customerId, profile)) {
        char errorMsg[100];
        sprintf(errorMsg, "Customer not found for customer ID %s", account.customerId);
        writeErrorLog(errorMsg);
        return false;
    }
    
    return true;
}

// Load account by card number
bool loadAccountByCardNumber(int cardNumber, Account* account) {
    if (account == NULL) {
        writeErrorLog("NULL account pointer provided to loadAccountByCardNumber");
        return false;
    }
    
    // First find the card
    Card card;
    if (!findCardByCardNumber(cardNumber, &card)) {
        char errorMsg[100];
        sprintf(errorMsg, "Card not found for card number %d", cardNumber);
        writeErrorLog(errorMsg);
        return false;
    }
    
    // Next find the account using the account ID from the card
    if (!findAccountById(card.accountId, account)) {
        char errorMsg[100];
        sprintf(errorMsg, "Account not found for account ID %s", card.accountId);
        writeErrorLog(errorMsg);
        return false;
    }
    
    return true;
}

// Load card by card number
bool loadCardByCardNumber(int cardNumber, Card* card) {
    if (card == NULL) {
        writeErrorLog("NULL card pointer provided to loadCardByCardNumber");
        return false;
    }
    
    return findCardByCardNumber(cardNumber, card);
}

// Get recent transactions for an account
int getRecentTransactions(const char* accountId, Transaction* transactions, int maxTransactions) {
    const char* transactionFilePath = isTestingMode() ? 
        TEST_TRANSACTIONS_LOG_FILE : PROD_TRANSACTIONS_LOG_FILE;
    
    FILE* file = fopen(transactionFilePath, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open transaction file");
        return 0;
    }
    
    char line[512];
    int count = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Transaction ID | Account ID | Transaction Type | Amount | Transaction Time | Transaction Status | Transaction Remarks
    while (fgets(line, sizeof(line), file) != NULL && count < maxTransactions) {
        char transactionId[11], storedAccountId[11], transactionType[20], transactionTimeStr[30];
        char statusStr[20], remarks[200];
        float amount;
        
        if (sscanf(line, "%10[^|] | %10[^|] | %19[^|] | %f | %29[^|] | %19[^|] | %199[^\n]",
                  transactionId, storedAccountId, transactionType, &amount, 
                  transactionTimeStr, statusStr, remarks) == 7) {
            
            if (strcmp(trim(storedAccountId), trim(accountId)) == 0) {
                // This transaction is for the requested account
                strncpy(transactions[count].transactionId, trim(transactionId), sizeof(transactions[count].transactionId) - 1);
                strncpy(transactions[count].accountId, trim(storedAccountId), sizeof(transactions[count].accountId) - 1);
                strncpy(transactions[count].transactionType, trim(transactionType), sizeof(transactions[count].transactionType) - 1);
                transactions[count].amount = amount;
                transactions[count].transactionTime = parseTimeString(trim(transactionTimeStr));
                transactions[count].transactionStatus = (strcasecmp(trim(statusStr), "Success") == 0);
                strncpy(transactions[count].transactionRemarks, trim(remarks), sizeof(transactions[count].transactionRemarks) - 1);
                
                count++;
            }
        }
    }
    
    fclose(file);
    return count;
}

// Load virtual wallet for a user
bool loadVirtualWallet(const char* userId, VirtualWallet* wallet) {
    FILE* file = fopen(TEST_DATA_DIR "/test_virtual_wallet.txt", "r");
    if (file == NULL) {
        writeErrorLog("Failed to open virtual wallet file");
        return false;
    }
    
    char line[512];
    bool found = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Wallet ID | User ID | Balance | Last Refill Time | Refill Amount
    while (fgets(line, sizeof(line), file) != NULL && !found) {
        char walletId[11], storedUserId[11], lastRefillTimeStr[30];
        float balance, refillAmount;
        
        if (sscanf(line, "%10[^|] | %10[^|] | %f | %29[^|] | %f",
                  walletId, storedUserId, &balance, lastRefillTimeStr, &refillAmount) == 5) {
            
            if (strcmp(trim(storedUserId), trim(userId)) == 0) {
                // Found matching wallet
                strncpy(wallet->walletId, trim(walletId), sizeof(wallet->walletId) - 1);
                strncpy(wallet->userId, trim(storedUserId), sizeof(wallet->userId) - 1);
                wallet->balance = balance;
                wallet->lastRefillTime = parseTimeString(trim(lastRefillTimeStr));
                wallet->refillAmount = refillAmount;
                
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

// Save updated customer profile
bool saveCustomerProfile(const CustomerProfile* profile) {
    // In a real implementation, this would update the customer file
    // For the test version, we'll just log it
    char logMessage[256];
    sprintf(logMessage, "Saving customer profile for %s: %s (Status: %d)",
            profile->customerId, profile->name, profile->status);
    writeAuditLog("PROFILE", logMessage);
    
    return true; // Simulated success
}

// Save updated account
bool saveAccount(const Account* account) {
    // In a real implementation, this would update the account file
    // For the test version, we'll just log it
    char logMessage[256];
    sprintf(logMessage, "Saving account %s with balance %.2f (Status: %d)",
            account->accountId, account->balance, account->accountStatus);
    writeAuditLog("ACCOUNT", logMessage);
    
    return true; // Simulated success
}

// Save updated card
bool saveCard(const Card* card) {
    // In a real implementation, this would update the card file
    // For the test version, we'll just log it
    char logMessage[256];
    sprintf(logMessage, "Saving card %s with card number %d (Status: %d)",
            card->cardId, card->cardNumber, card->status);
    writeAuditLog("CARD", logMessage);
    
    return true; // Simulated success
}

// Record a new transaction
bool recordTransaction(const Transaction* transaction) {
    // In a real implementation, this would append to the transaction file
    // For the test version, we'll just log it
    char logMessage[256];
    sprintf(logMessage, "Recording %s transaction of %.2f for account %s: %s",
            transaction->transactionType, transaction->amount,
            transaction->accountId, transaction->transactionRemarks);
    writeAuditLog("TRANSACTION", logMessage);
    
    return true; // Simulated success
}

// Update virtual wallet balance
bool updateVirtualWallet(const VirtualWallet* wallet) {
    // In a real implementation, this would update the wallet file
    // For the test version, we'll just log it
    char logMessage[256];
    sprintf(logMessage, "Updating wallet %s for user %s with new balance %.2f",
            wallet->walletId, wallet->userId, wallet->balance);
    writeAuditLog("WALLET", logMessage);
    
    return true; // Simulated success
}