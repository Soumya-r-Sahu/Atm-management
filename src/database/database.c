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
                if (strcmp(storedPinHash, pinHash) == 0) {
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
    // First, find the account ID from the card number
    FILE* cardFile = fopen(getCardFilePath(), "r");
    if (cardFile == NULL) {
        writeErrorLog("Failed to open card.txt file");
        return -1.0f;
    }
    
    char line[256];
    char accountID[10] = "";
    
    // Skip header lines
    fgets(line, sizeof(line), cardFile);
    fgets(line, sizeof(line), cardFile);
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
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
        return -1.0f; // Card number not found
    }
    
    // Now get the balance from customer.txt using the account ID
    FILE* customerFile = fopen(getCustomerFilePath(), "r");
    if (customerFile == NULL) {
        writeErrorLog("Failed to open customer.txt file");
        return -1.0f;
    }
    
    float balance = -1.0f;
    
    // Skip header lines
    fgets(line, sizeof(line), customerFile);
    fgets(line, sizeof(line), customerFile);
    
    // Format: Customer ID | Account ID | Account Holder Name | Type | Status | Balance
    char customerID[15], accIDFromFile[15], holderName[100], type[20], accountStatus[20], balanceStr[20];
    
    while (fgets(line, sizeof(line), customerFile) != NULL) {
        if (sscanf(line, "%s | %s | %[^|] | %s | %s | %s", 
                   customerID, accIDFromFile, holderName, type, accountStatus, balanceStr) >= 6) {
            if (strcmp(accountID, accIDFromFile) == 0) {
                balance = atof(balanceStr);
                break;
            }
        }
    }
    
    fclose(customerFile);
    return balance;
}

// Update account balance for a card
bool updateBalance(int cardNumber, float newBalance) {
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
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
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
    
    // Now update the balance in the customer.txt file
    FILE* customerFile = fopen(getCustomerFilePath(), "r");
    if (customerFile == NULL) {
        writeErrorLog("Failed to open customer.txt file");
        return false;
    }
    
    char tempFileName[100];
    sprintf(tempFileName, "%s/temp/temp_customer.txt", isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR);
    
    FILE* tempFile = fopen(tempFileName, "w");
    if (tempFile == NULL) {
        fclose(customerFile);
        writeErrorLog("Failed to create temporary customer file");
        return false;
    }
    
    bool updated = false;
    
    // Copy header lines
    fgets(line, sizeof(line), customerFile);
    fputs(line, tempFile);
    fgets(line, sizeof(line), customerFile);
    fputs(line, tempFile);
    
    char customerID[15], accIDFromFile[15], holderName[100], type[20], accountStatus[20], balanceStr[20];
    
    while (fgets(line, sizeof(line), customerFile) != NULL) {
        char lineCopy[256];
        strcpy(lineCopy, line);
        
        if (sscanf(lineCopy, "%s | %s | %[^|] | %s | %s | %s", 
                   customerID, accIDFromFile, holderName, type, accountStatus, balanceStr) >= 6) {
            if (strcmp(accountID, accIDFromFile) == 0) {
                // Update the balance
                fprintf(tempFile, "%s | %s | %s | %s | %s | %.2f\n", 
                        customerID, accIDFromFile, holderName, type, accountStatus, newBalance);
                updated = true;
            } else {
                fputs(line, tempFile);
            }
        } else {
            fputs(line, tempFile);
        }
    }
    
    fclose(customerFile);
    fclose(tempFile);
    
    if (updated) {
        // Replace original file with updated one
        if (remove(getCustomerFilePath()) == 0 && 
            rename(tempFileName, getCustomerFilePath()) == 0) {
            
            char logMsg[100];
            sprintf(logMsg, "Balance updated for account %s: %.2f", accountID, newBalance);
            writeAuditLog("ACCOUNTING", logMsg);
            return true;
        }
    }
    
    // Remove temp file if update was not successful
    remove(tempFileName);
    return false;
}

// Log a withdrawal for daily limit tracking
void logWithdrawal(int cardNumber, float amount) {
    char date[20];
    getCurrentDate(date, sizeof(date));
    
    // Withdrawals log file path
    char withdrawalsLogPath[200];
    sprintf(withdrawalsLogPath, "%s/withdrawals.log", 
            isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR "/../logs");
    
    FILE* file = fopen(withdrawalsLogPath, "a");
    if (file == NULL) {
        writeErrorLog("Failed to open withdrawals log file");
        return;
    }
    
    // Log the withdrawal in format: Card Number | Date | Amount
    fprintf(file, "%-11d | %-10s | %.2f\n", cardNumber, date, amount);
    
    fclose(file);
    
    // Also log to transaction log
    logTransaction(cardNumber, TRANSACTION_WITHDRAWAL, amount, true);
}

// Get total withdrawals for a card on the current day
float getDailyWithdrawals(int cardNumber) {
    char date[20];
    getCurrentDate(date, sizeof(date));
    
    // Withdrawals log file path
    char withdrawalsLogPath[200];
    sprintf(withdrawalsLogPath, "%s/withdrawals.log", 
            isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR "/../logs");
    
    FILE* file = fopen(withdrawalsLogPath, "r");
    if (file == NULL) {
        return 0.0f;  // No withdrawals or file doesn't exist yet
    }
    
    char line[100];
    float totalWithdrawals = 0.0f;
    
    while (fgets(line, sizeof(line), file) != NULL) {
        int storedCardNumber;
        char storedDate[20];
        float storedAmount;
        
        if (sscanf(line, "%d | %s | %f", &storedCardNumber, storedDate, &storedAmount) == 3) {
            if (storedCardNumber == cardNumber && strcmp(storedDate, date) == 0) {
                totalWithdrawals += storedAmount;
            }
        }
    }
    
    fclose(file);
    return totalWithdrawals;
}

// Block a card
bool blockCard(int cardNumber) {
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
    
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], pinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        char lineCopy[256];
        strcpy(lineCopy, line);
        
        if (sscanf(lineCopy, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                // Update the status to blocked
                fprintf(tempFile, "%s | %s | %s | %s | %s | %-7s | %s\n", 
                        cardID, accountID, cardNumberStr, cardType, expiryDate, "Blocked", pinHash);
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
            sprintf(logMsg, "Card %d has been blocked", cardNumber);
            writeAuditLog("SECURITY", logMsg);
            return true;
        }
    }
    
    // Remove temp file if update was not successful
    remove(tempFileName);
    return false;
}

// Unblock a card
bool unblockCard(int cardNumber) {
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
    
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], pinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        char lineCopy[256];
        strcpy(lineCopy, line);
        
        if (sscanf(lineCopy, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                // Update the status to active
                fprintf(tempFile, "%s | %s | %s | %s | %s | %-7s | %s\n", 
                        cardID, accountID, cardNumberStr, cardType, expiryDate, "Active", pinHash);
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
            sprintf(logMsg, "Card %d has been unblocked", cardNumber);
            writeAuditLog("SECURITY", logMsg);
            return true;
        }
    }
    
    // Remove temp file if update was not successful
    remove(tempFileName);
    return false;
}

// Log a transaction to the transactions log
void logTransaction(int cardNumber, TransactionType type, float amount, bool success) {
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
        sprintf(accountID, "C%d", cardNumber);
    }
    
    // Generate transaction ID
    static int transactionCount = 0;
    char transactionID[10];
    sprintf(transactionID, "T%d", 60000 + (++transactionCount));
    
    // Get current timestamp
    char timestamp[30];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    
    // Get transaction type string
    char transactionTypeStr[20];
    switch (type) {
        case TRANSACTION_BALANCE_CHECK:
            strcpy(transactionTypeStr, "Balance Check");
            break;
        case TRANSACTION_DEPOSIT:
            strcpy(transactionTypeStr, "Deposit");
            break;
        case TRANSACTION_WITHDRAWAL:
            strcpy(transactionTypeStr, "Withdrawal");
            break;
        case TRANSACTION_PIN_CHANGE:
            strcpy(transactionTypeStr, "PIN Change");
            break;
        case TRANSACTION_MINI_STATEMENT:
            strcpy(transactionTypeStr, "Mini Statement");
            break;
        case TRANSACTION_MONEY_TRANSFER:
            strcpy(transactionTypeStr, "Transfer");
            break;
        case TRANSACTION_CARD_REQUEST:
            strcpy(transactionTypeStr, "Card Request");
            break;
        default:
            strcpy(transactionTypeStr, "Other");
    }
    
    // Format transaction remarks
    char remarks[50];
    if (type == TRANSACTION_BALANCE_CHECK || type == TRANSACTION_MINI_STATEMENT) {
        strcpy(remarks, "Information Request");
    } else if (type == TRANSACTION_DEPOSIT) {
        strcpy(remarks, "Cash Deposit");
    } else if (type == TRANSACTION_WITHDRAWAL) {
        strcpy(remarks, "ATM Withdrawal");
    } else if (type == TRANSACTION_PIN_CHANGE) {
        strcpy(remarks, "Security Update");
    } else if (type == TRANSACTION_MONEY_TRANSFER) {
        strcpy(remarks, "Fund Transfer");
    } else {
        strcpy(remarks, "General Transaction");
    }
    
    // Open transactions log file
    char transactionsLogPath[200];
    sprintf(transactionsLogPath, "%s/transactions.log", 
            isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR "/../logs");
    
    FILE* file = fopen(transactionsLogPath, "a");
    if (file == NULL) {
        writeErrorLog("Failed to open transactions log file");
        return;
    }
    
    // Log the transaction
    fprintf(file, "%-14s | %-10s | %-15s | %-8.2f | %-19s | %-17s | %s\n", 
            transactionID, accountID, transactionTypeStr, amount, timestamp, 
            success ? "Success" : "Failed", remarks);
    
    fclose(file);
}

