#include "database.h"
#include "../utils/logger.h"
#include "../common/paths.h"
#include "../utils/hash_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Helper function to get current date as a string (YYYY-MM-DD)
static void getCurrentDate(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d", t);
}

// Helper function to get current timestamp as a string (YYYY-MM-DD HH:MM:SS)
static void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Check if a card number exists in the database
bool doesCardExist(int cardNumber) {
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card.txt file");
        return false;
    }
    
    char line[256];
    bool found = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], pinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

// Check if a card is active
bool isCardActive(int cardNumber) {
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card.txt file");
        return false;
    }
    
    char line[256];
    bool active = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], pinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                // Check if status is "Active"
                if (strstr(status, "Active") != NULL) {
                    active = true;
                }
                break;
            }
        }
    }
    
    fclose(file);
    return active;
}

// Validate card with PIN (legacy method, for backward compatibility)
bool validateCard(int cardNumber, int pin) {
    char pinStr[20];
    sprintf(pinStr, "%d", pin);
    char* pinHash = sha256_hash(pinStr);
    bool result = validateCardWithHash(cardNumber, pinHash);
    free(pinHash);
    return result;
}

// Validate card with PIN hash
bool validateCardWithHash(int cardNumber, const char* pinHash) {
    if (pinHash == NULL) {
        writeErrorLog("NULL PIN hash provided to validateCardWithHash");
        return false;
    }
    
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card.txt file");
        return false;
    }
    
    char line[256];
    bool valid = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], storedPinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, storedPinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                // Use secure hash comparison instead of strcmp
                if (secure_hash_compare(storedPinHash, pinHash)) {
                    valid = true;
                }
                break;
            }
        }
    }
    
    fclose(file);
    return valid;
}

// Update PIN for a card (legacy method)
bool updatePIN(int cardNumber, int newPin) {
    char pinStr[20];
    sprintf(pinStr, "%d", newPin);
    char* pinHash = sha256_hash(pinStr);
    bool result = updatePINHash(cardNumber, pinHash);
    free(pinHash);
    return result;
}

// Update PIN hash for a card
bool updatePINHash(int cardNumber, const char* pinHash) {
    if (pinHash == NULL) {
        writeErrorLog("NULL PIN hash provided to updatePINHash");
        return false;
    }
    
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card.txt file");
        return false;
    }
    
    char tempFileName[100];
    sprintf(tempFileName, "%s/temp/temp_card.txt", isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR);
    
    FILE* tempFile = fopen(tempFileName, "w");
    if (tempFile == NULL) {
        fclose(file);
        writeErrorLog("Failed to create temporary card file");
        return false;
    }
    
    char line[256];
    bool updated = false;
    
    // Copy header lines
    fgets(line, sizeof(line), file);
    fputs(line, tempFile);
    fgets(line, sizeof(line), file);
    fputs(line, tempFile);
    
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], oldPinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        char lineCopy[256];
        strcpy(lineCopy, line);
        
        if (sscanf(lineCopy, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, oldPinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                // Update the PIN with hash
                fprintf(tempFile, "%s | %s | %s | %s | %s | %s | %s\n", 
                        cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash);
                updated = true;
            } else {
                fputs(line, tempFile);
            }
        } else {
            fputs(line, tempFile);
        }
    }
    
    fclose(file);
    fclose(tempFile);
    
    if (updated) {
        // Replace original file with updated one
        if (remove(getCardFilePath()) == 0 && 
            rename(tempFileName, getCardFilePath()) == 0) {
            
            char logMsg[100];
            sprintf(logMsg, "PIN hash updated for card %d", cardNumber);
            writeAuditLog("SECURITY", logMsg);
            return true;
        }
    }
    
    // Remove temp file if update was not successful
    remove(tempFileName);
    return false;
}

// Get card holder's name by looking up customer info by card number
bool getCardHolderName(int cardNumber, char* name, size_t nameSize) {
    if (name == NULL || nameSize <= 0) {
        return false;
    }
    
    // First, find the account ID from the card number
    FILE* cardFile = fopen(getCardFilePath(), "r");
    if (cardFile == NULL) {
        writeErrorLog("Failed to open card.txt file");
        return false;
    }
    
    char line[256];
    char accountID[10] = "";
    
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
    
    if (strlen(accountID) == 0) {
        return false; // Card number not found
    }
    
    // Get customer name directly from customer.txt using the account ID
    FILE* customerFile = fopen(getCustomerFilePath(), "r");
    if (customerFile == NULL) {
        writeErrorLog("Failed to open customer.txt file");
        return false;
    }
    
    bool found = false;
    
    // Skip header lines
    fgets(line, sizeof(line), customerFile);
    fgets(line, sizeof(line), customerFile);
    
    // Format: Customer ID | Account ID | Account Holder Name | Type | Status | Balance
    char customerID[15], accIDFromFile[15], holderName[100], type[20], accountStatus[20], balance[20];
    
    while (fgets(line, sizeof(line), customerFile) != NULL) {
        if (sscanf(line, "%s | %s | %[^|] | %s | %s | %s", 
                   customerID, accIDFromFile, holderName, type, accountStatus, balance) >= 6) {
            if (strcmp(accIDFromFile, accountID) == 0) {
                // Trim any leading/trailing spaces from holder name
                int start = 0, end = strlen(holderName) - 1;
                while (holderName[start] == ' ') start++;
                while (end > start && holderName[end] == ' ') end--;
                holderName[end + 1] = '\0';
                
                strncpy(name, &holderName[start], nameSize - 1);
                name[nameSize - 1] = '\0';
                found = true;
                break;
            }
        }
    }
    
    fclose(customerFile);
    return found;
}

// Get card holder's phone number by looking up customer info
// Note: Since phone number is not included in customer.txt, 
// this function will provide a placeholder response
bool getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize) {
    if (phone == NULL || phoneSize <= 0) {
        return false;
    }
    
    // Confirm that the card number exists first
    if (doesCardExist(cardNumber)) {
        strncpy(phone, "9876543210", phoneSize - 1); // Provide a default phone number
        phone[phoneSize - 1] = '\0';
        return true;
    }
    
    return false;
}

// Fetch account balance for a card
float fetchBalance(int cardNumber) {
    if (cardNumber <= 0) {
        writeErrorLog("Invalid card number provided to fetchBalance");
        return -1.0f;
    }
    
    const char* cardFilePath = getCardFilePath();
    FILE* cardFile = fopen(cardFilePath, "r");
    if (cardFile == NULL) {
        char errorMsg[100];
        sprintf(errorMsg, "Failed to open card file at %s", cardFilePath);
        writeErrorLog(errorMsg);
        return -1.0f;
    }
    
    char line[256] = {0};
    char accountID[20] = {0};  // Increased size for safety
    bool found = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), cardFile) == NULL || fgets(line, sizeof(line), cardFile) == NULL) {
        writeErrorLog("Card file format error: missing header lines");
        fclose(cardFile);
        return -1.0f;
    }
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    char cardID[20] = {0}, accID[20] = {0}, cardNumberStr[30] = {0};
    char cardType[20] = {0}, expiryDate[30] = {0}, status[20] = {0}, pinHash[70] = {0};
    
    while (fgets(line, sizeof(line), cardFile) != NULL) {
        // Reset variables to avoid data leakage between iterations
        memset(cardID, 0, sizeof(cardID));
        memset(accID, 0, sizeof(accID));
        memset(cardNumberStr, 0, sizeof(cardNumberStr));
        memset(cardType, 0, sizeof(cardType));
        memset(expiryDate, 0, sizeof(expiryDate));
        memset(status, 0, sizeof(status));
        memset(pinHash, 0, sizeof(pinHash));
        
        if (sscanf(line, "%19s | %19s | %29s | %19s | %29s | %19s | %69s", 
                  cardID, accID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                strncpy(accountID, accID, sizeof(accountID) - 1);
                found = true;
                break;
            }
        }
    }
    
    fclose(cardFile);
    
    if (!found || strlen(accountID) == 0) {
        char errorMsg[100];
        sprintf(errorMsg, "Card number %d not found in database", cardNumber);
        writeErrorLog(errorMsg);
        return -1.0f; // Card number not found
    }
    
    // Now get the balance from customer.txt using the account ID
    const char* customerFilePath = getCustomerFilePath();
    FILE* customerFile = fopen(customerFilePath, "r");
    if (customerFile == NULL) {
        char errorMsg[100];
        sprintf(errorMsg, "Failed to open customer file at %s", customerFilePath);
        writeErrorLog(errorMsg);
        return -1.0f;
    }
    
    float balance = -1.0f;
    found = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), customerFile) == NULL || fgets(line, sizeof(line), customerFile) == NULL) {
        writeErrorLog("Customer file format error: missing header lines");
        fclose(customerFile);
        return -1.0f;
    }
    
    // Format: Customer ID | Account ID | Account Holder Name | Type | Status | Balance
    char customerID[20] = {0}, accIDFromFile[20] = {0}, holderName[100] = {0};
    char type[30] = {0}, accountStatus[30] = {0}, balanceStr[30] = {0};
    
    while (fgets(line, sizeof(line), customerFile) != NULL) {
        // Reset variables
        memset(customerID, 0, sizeof(customerID));
        memset(accIDFromFile, 0, sizeof(accIDFromFile));
        memset(holderName, 0, sizeof(holderName));
        memset(type, 0, sizeof(type));
        memset(accountStatus, 0, sizeof(accountStatus));
        memset(balanceStr, 0, sizeof(balanceStr));
        
        if (sscanf(line, "%19s | %19s | %99[^|] | %29s | %29s | %29s", 
                   customerID, accIDFromFile, holderName, type, accountStatus, balanceStr) >= 6) {
            if (strcmp(accIDFromFile, accountID) == 0) {
                balance = atof(balanceStr);
                found = true;
                break;
            }
        }
    }
    
    fclose(customerFile);
    
    if (!found) {
        char errorMsg[100];
        sprintf(errorMsg, "Account ID %s not found in customer database", accountID);
        writeErrorLog(errorMsg);
    }
    
    return balance;
}

// Update account balance for a card
bool updateBalance(int cardNumber, float newBalance) {
    if (cardNumber <= 0) {
        writeErrorLog("Invalid card number provided to updateBalance");
        return false;
    }
    
    if (newBalance < 0) {
        char errorMsg[100];
        sprintf(errorMsg, "Attempted to set negative balance (%.2f) for card %d", newBalance, cardNumber);
        writeErrorLog(errorMsg);
        return false;
    }
    
    // First, find the account ID from the card number
    const char* cardFilePath = getCardFilePath();
    FILE* cardFile = fopen(cardFilePath, "r");
    if (cardFile == NULL) {
        char errorMsg[100];
        sprintf(errorMsg, "Failed to open card file at %s", cardFilePath);
        writeErrorLog(errorMsg);
        return false;
    }
    
    char line[256] = {0};
    char accountID[20] = {0};  // Increased size for safety
    bool found = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), cardFile) == NULL || fgets(line, sizeof(line), cardFile) == NULL) {
        writeErrorLog("Card file format error: missing header lines");
        fclose(cardFile);
        return false;
    }
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    char cardID[20] = {0}, accID[20] = {0}, cardNumberStr[30] = {0};
    char cardType[20] = {0}, expiryDate[30] = {0}, status[20] = {0}, pinHash[70] = {0};
    
    while (fgets(line, sizeof(line), cardFile) != NULL) {
        // Reset variables to avoid data leakage between iterations
        memset(cardID, 0, sizeof(cardID));
        memset(accID, 0, sizeof(accID));
        memset(cardNumberStr, 0, sizeof(cardNumberStr));
        memset(cardType, 0, sizeof(cardType));
        memset(expiryDate, 0, sizeof(expiryDate));
        memset(status, 0, sizeof(status));
        memset(pinHash, 0, sizeof(pinHash));
        
        if (sscanf(line, "%19s | %19s | %29s | %19s | %29s | %19s | %69s", 
                  cardID, accID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                strncpy(accountID, accID, sizeof(accountID) - 1);
                found = true;
                break;
            }
        }
    }
    
    fclose(cardFile);
    
    if (!found || strlen(accountID) == 0) {
        char errorMsg[100];
        sprintf(errorMsg, "Card number %d not found in database", cardNumber);
        writeErrorLog(errorMsg);
        return false;
    }
    
    // Now update the balance in the customer.txt file
    const char* customerFilePath = getCustomerFilePath();
    FILE* customerFile = fopen(customerFilePath, "r");
    if (customerFile == NULL) {
        char errorMsg[100];
        sprintf(errorMsg, "Failed to open customer file at %s", customerFilePath);
        writeErrorLog(errorMsg);
        return false;
    }
    
    char tempFileName[256] = {0};
    sprintf(tempFileName, "%s/temp/temp_customer.txt", isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR);
    
    FILE* tempFile = fopen(tempFileName, "w");
    if (tempFile == NULL) {
        fclose(customerFile);
        char errorMsg[100];
        sprintf(errorMsg, "Failed to create temporary customer file at %s", tempFileName);
        writeErrorLog(errorMsg);
        return false;
    }
    
    bool updated = false;
    
    // Copy header lines
    if (fgets(line, sizeof(line), customerFile) == NULL || fgets(line, sizeof(line), customerFile) == NULL) {
        writeErrorLog("Customer file format error: missing header lines");
        fclose(customerFile);
        fclose(tempFile);
        remove(tempFileName);
        return false;
    }
    
    fputs(line, tempFile);
    
    // Reset the file pointer to read the second header line again
    fseek(customerFile, 0, SEEK_SET);
    fgets(line, sizeof(line), customerFile); // Skip first line
    fgets(line, sizeof(line), customerFile); // Read second line
    fputs(line, tempFile);
    
    char customerID[20] = {0}, accIDFromFile[20] = {0}, holderName[100] = {0};
    char type[30] = {0}, accountStatus[30] = {0}, balanceStr[30] = {0};
    
    while (fgets(line, sizeof(line), customerFile) != NULL) {
        // Reset variables
        memset(customerID, 0, sizeof(customerID));
        memset(accIDFromFile, 0, sizeof(accIDFromFile));
        memset(holderName, 0, sizeof(holderName));
        memset(type, 0, sizeof(type));
        memset(accountStatus, 0, sizeof(accountStatus));
        memset(balanceStr, 0, sizeof(balanceStr));
        
        if (sscanf(line, "%19s | %19s | %99[^|] | %29s | %29s | %29s", 
                   customerID, accIDFromFile, holderName, type, accountStatus, balanceStr) >= 6) {
            if (strcmp(accIDFromFile, accountID) == 0) {
                balance = atof(balanceStr);
                found = true;
                break;
            }
        }
    }
    
    fclose(customerFile);
    
    if (!found) {
        char errorMsg[100];
        sprintf(errorMsg, "Account ID %s not found in customer database", accountID);
        writeErrorLog(errorMsg);
    }
    
    return balance;
}

// Log a transaction to the transactions log
void logTransaction(int cardNumber, TransactionType type, float amount, bool success) {
    if (cardNumber <= 0) {
        writeErrorLog("Invalid card number provided to logTransaction");
        return;
    }
    
    // Get account ID from card number
    char accountID[20] = {0};  // Increased size for safety
    
    const char* cardFilePath = getCardFilePath();
    FILE* cardFile = fopen(cardFilePath, "r");
    if (cardFile == NULL) {
        char errorMsg[100];
        sprintf(errorMsg, "Failed to open card file at %s for transaction logging", cardFilePath);
        writeErrorLog(errorMsg);
        
        // Use fallback account ID based on card number
        sprintf(accountID, "C%d", cardNumber);
    } else {
        char line[256] = {0};
        bool found = false;
        
        // Skip header lines
        if (fgets(line, sizeof(line), cardFile) != NULL && fgets(line, sizeof(line), cardFile) != NULL) {
            char cardID[20] = {0}, accID[20] = {0}, cardNumberStr[30] = {0};
            char cardType[20] = {0}, expiryDate[30] = {0}, status[20] = {0}, pinHash[70] = {0};
            
            while (fgets(line, sizeof(line), cardFile) != NULL) {
                // Reset variables to avoid data leakage between iterations
                memset(cardID, 0, sizeof(cardID));
                memset(accID, 0, sizeof(accID));
                memset(cardNumberStr, 0, sizeof(cardNumberStr));
                memset(cardType, 0, sizeof(cardType));
                memset(expiryDate, 0, sizeof(expiryDate));
                memset(status, 0, sizeof(status));
                memset(pinHash, 0, sizeof(pinHash));
                
                if (sscanf(line, "%19s | %19s | %29s | %19s | %29s | %19s | %69s", 
                           cardID, accID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
                    int storedCardNumber = atoi(cardNumberStr);
                    if (storedCardNumber == cardNumber) {
                        strncpy(accountID, accID, sizeof(accountID) - 1);
                        found = true;
                        break;
                    }
                }
            }
        }
        
        fclose(cardFile);
        
        // If account ID not found, use card number as a fallback
        if (!found || strlen(accountID) == 0) {
            sprintf(accountID, "C%d", cardNumber);
        }
    }
    
    // Generate transaction ID with safety against overflow
    static int transactionCount = 0;
    if (transactionCount >= INT_MAX - 60000) {
        transactionCount = 0;  // Reset to avoid overflow
    }
    char transactionID[20] = {0};
    sprintf(transactionID, "T%d", 60000 + (++transactionCount));
    
    // Get current timestamp
    char timestamp[30] = {0};
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    
    // Get transaction type string
    char transactionTypeStr[30] = {0};
    switch (type) {
        case TRANSACTION_BALANCE_CHECK:
            strncpy(transactionTypeStr, "Balance Check", sizeof(transactionTypeStr) - 1);
            break;
        case TRANSACTION_DEPOSIT:
            strncpy(transactionTypeStr, "Deposit", sizeof(transactionTypeStr) - 1);
            break;
        case TRANSACTION_WITHDRAWAL:
            strncpy(transactionTypeStr, "Withdrawal", sizeof(transactionTypeStr) - 1);
            break;
        case TRANSACTION_PIN_CHANGE:
            strncpy(transactionTypeStr, "PIN Change", sizeof(transactionTypeStr) - 1);
            break;
        case TRANSACTION_MINI_STATEMENT:
            strncpy(transactionTypeStr, "Mini Statement", sizeof(transactionTypeStr) - 1);
            break;
        case TRANSACTION_MONEY_TRANSFER:
            strncpy(transactionTypeStr, "Transfer", sizeof(transactionTypeStr) - 1);
            break;
        case TRANSACTION_CARD_REQUEST:
            strncpy(transactionTypeStr, "Card Request", sizeof(transactionTypeStr) - 1);
            break;
        default:
            strncpy(transactionTypeStr, "Other", sizeof(transactionTypeStr) - 1);
    }
    
    // Format transaction remarks
    char remarks[50] = {0};
    switch (type) {
        case TRANSACTION_BALANCE_CHECK:
        case TRANSACTION_MINI_STATEMENT:
            strncpy(remarks, "Information Request", sizeof(remarks) - 1);
            break;
        case TRANSACTION_DEPOSIT:
            strncpy(remarks, "Cash Deposit", sizeof(remarks) - 1);
            break;
        case TRANSACTION_WITHDRAWAL:
            strncpy(remarks, "ATM Withdrawal", sizeof(remarks) - 1);
            break;
        case TRANSACTION_PIN_CHANGE:
            strncpy(remarks, "Security Update", sizeof(remarks) - 1);
            break;
        case TRANSACTION_MONEY_TRANSFER:
            strncpy(remarks, "Fund Transfer", sizeof(remarks) - 1);
            break;
        default:
            strncpy(remarks, "General Transaction", sizeof(remarks) - 1);
    }
    
    // Build consistent path for transactions log
    char transactionsLogPath[256] = {0};
    if (isTestingMode()) {
        sprintf(transactionsLogPath, "%s/test_transaction.txt", TEST_DATA_DIR);
    } else {
        // Ensure consistent path handling for production
        sprintf(transactionsLogPath, "%s/../logs/transactions.log", PROD_DATA_DIR);
    }
    
    FILE* file = fopen(transactionsLogPath, "a");
    if (file == NULL) {
        char errorMsg[100];
        sprintf(errorMsg, "Failed to open transactions log file at %s", transactionsLogPath);
        writeErrorLog(errorMsg);
        return;
    }
    
    // Log the transaction
    if (fprintf(file, "%-14s | %-10s | %-15s | %-8.2f | %-19s | %-17s | %s\n", 
            transactionID, accountID, transactionTypeStr, amount, timestamp, 
            success ? "Success" : "Failed", remarks) < 0) {
        writeErrorLog("Failed to write to transactions log file");
    }
    
    fclose(file);
    
    char logMsg[150];
    sprintf(logMsg, "Transaction logged: %s %s for card %d, amount: %.2f, status: %s", 
           transactionTypeStr, remarks, cardNumber, amount, success ? "Success" : "Failed");
    writeInfoLog(logMsg);
}

// Validate recipient account details (card number, account ID, branch code)
bool validateRecipientAccount(int cardNumber, const char* accountID, const char* branchCode) {
    if (cardNumber <= 0) {
        writeErrorLog("Invalid card number provided to validateRecipientAccount");
        return false;
    }
    
    if (accountID == NULL || strlen(accountID) == 0) {
        writeErrorLog("NULL or empty account ID provided to validateRecipientAccount");
        return false;
    }
    
    if (branchCode == NULL || strlen(branchCode) == 0) {
        writeErrorLog("NULL or empty branch code provided to validateRecipientAccount");
        return false;
    }
    
    // First, find the account ID associated with the card number
    const char* cardFilePath = getCardFilePath();
    FILE* cardFile = fopen(cardFilePath, "r");
    if (cardFile == NULL) {
        char errorMsg[100];
        sprintf(errorMsg, "Failed to open card file at %s for recipient validation", cardFilePath);
        writeErrorLog(errorMsg);
        return false;
    }
    
    char line[256] = {0};
    char cardAccountID[20] = {0};  // Increased size for safety
    bool found = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), cardFile) == NULL || fgets(line, sizeof(line), cardFile) == NULL) {
        writeErrorLog("Card file format error: missing header lines");
        fclose(cardFile);
        return false;
    }
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    char cardID[20] = {0}, storedAccountID[20] = {0}, cardNumberStr[30] = {0};
    char cardType[20] = {0}, expiryDate[30] = {0}, status[20] = {0}, pinHash[70] = {0};
    
    while (fgets(line, sizeof(line), cardFile) != NULL) {
        // Reset variables to avoid data leakage
        memset(cardID, 0, sizeof(cardID));
        memset(storedAccountID, 0, sizeof(storedAccountID));
        memset(cardNumberStr, 0, sizeof(cardNumberStr));
        memset(cardType, 0, sizeof(cardType));
        memset(expiryDate, 0, sizeof(expiryDate));
        memset(status, 0, sizeof(status));
        memset(pinHash, 0, sizeof(pinHash));
        
        int storedCardNumber;
        if (sscanf(line, "%19s | %19s | %d | %19s | %29s | %19s | %69s",
                  cardID, storedAccountID, &storedCardNumber, cardType, expiryDate, status, pinHash) >= 7) {
            if (storedCardNumber == cardNumber) {
                strncpy(cardAccountID, storedAccountID, sizeof(cardAccountID) - 1);
                found = true;
                break;
            }
        }
    }
    
    fclose(cardFile);
    
    if (!found || strlen(cardAccountID) == 0) {
        char errorMsg[100];
        sprintf(errorMsg, "Card number %d not found during recipient account validation", cardNumber);
        writeErrorLog(errorMsg);
        return false;
    }
    
    // Now check if the provided account ID matches the account ID associated with the card
    if (strcmp(cardAccountID, accountID) != 0) {
        char errorMsg[150];
        sprintf(errorMsg, "Account ID mismatch during recipient validation: provided %s, actual %s", accountID, cardAccountID);
        writeErrorLog(errorMsg);
        return false;
    }
    
    // Finally, check if the branch code matches
    const char* customerFilePath = getCustomerFilePath();
    FILE* customerFile = fopen(customerFilePath, "r");
    if (customerFile == NULL) {
        char errorMsg[100];
        sprintf(errorMsg, "Failed to open customer file at %s for branch validation", customerFilePath);
        writeErrorLog(errorMsg);
        return false;
    }
    
    bool branchMatches = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), customerFile) == NULL || fgets(line, sizeof(line), customerFile) == NULL) {
        writeErrorLog("Customer file format error: missing header lines");
        fclose(customerFile);
        return false;
    }
    
    // Format (for Customer file): Customer ID | Account ID | Account Holder Name | Type | Status | Balance | Branch Code
    char customerID[20] = {0}, accIDFromFile[20] = {0}, holderName[100] = {0};
    char type[20] = {0}, accountStatus[20] = {0}, balanceStr[30] = {0}, branchCodeFromFile[20] = {0};
    
    while (fgets(line, sizeof(line), customerFile) != NULL) {
        // Reset variables
        memset(customerID, 0, sizeof(customerID));
        memset(accIDFromFile, 0, sizeof(accIDFromFile));
        memset(holderName, 0, sizeof(holderName));
        memset(type, 0, sizeof(type));
        memset(accountStatus, 0, sizeof(accountStatus));
        memset(balanceStr, 0, sizeof(balanceStr));
        memset(branchCodeFromFile, 0, sizeof(branchCodeFromFile));
        
        // Try to parse the branch code if it's included in the file format
        if (sscanf(line, "%19s | %19s | %99[^|] | %19s | %19s | %29s | %19s",
                   customerID, accIDFromFile, holderName, type, accountStatus, balanceStr, branchCodeFromFile) >= 7) {
            if (strcmp(accIDFromFile, accountID) == 0) {
                if (strcmp(branchCodeFromFile, branchCode) == 0) {
                    branchMatches = true;
                } else {
                    char errorMsg[150];
                    sprintf(errorMsg, "Branch code mismatch: provided %s, actual %s for account %s", 
                           branchCode, branchCodeFromFile, accountID);
                    writeErrorLog(errorMsg);
                }
                break;
            }
        }
    }
    
    fclose(customerFile);
    
    // Log successful validation
    if (branchMatches) {
        char logMsg[150];
        sprintf(logMsg, "Successfully validated recipient: card %d, account %s, branch %s", 
               cardNumber, accountID, branchCode);
        writeInfoLog(logMsg);
    }
    
    return branchMatches;
}

