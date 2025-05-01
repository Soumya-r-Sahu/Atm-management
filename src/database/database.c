#include "../../include/common/database/database.h"
#include "../../include/common/utils/logger.h" // Updated path
#include "../../include/common/paths.h"       // Updated path
#include "../../include/common/security/hash_utils.h" // Updated path
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h> // Added for INT_MAX

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

// Initialize database
bool initialize_database(void) {
    // Ensure data directories exist
    if (!ensureDirectoryExists(PROD_DATA_DIR)) {
        writeErrorLog("Failed to create production data directory");
        return false;
    }
    
    if (!ensureDirectoryExists(TEST_DATA_DIR)) {
        writeErrorLog("Failed to create test data directory");
        return false;
    }
    
    if (!ensureDirectoryExists("data/temp")) {
        writeErrorLog("Failed to create temporary data directory");
        return false;
    }
    
    // Ensure all required database files exist
    const char* requiredFiles[] = {
        getCardFilePath(),
        getCustomerFilePath(),
        getAccountingFilePath(),
        getAdminCredentialsFilePath(),
        getSystemConfigFilePath()
    };
    
    for (int i = 0; i < sizeof(requiredFiles) / sizeof(requiredFiles[0]); i++) {
        FILE* file = fopen(requiredFiles[i], "a+");
        if (!file) {
            char errorMsg[100];
            sprintf(errorMsg, "Failed to initialize database file: %s", requiredFiles[i]);
            writeErrorLog(errorMsg);
            return false;
        }
        fclose(file);
    }
    
    writeInfoLog("Database initialized successfully");
    return true;
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

// Function to generate a card number in the format found in init_data_files.c
char* generateCardNumber() {
    char* cardNumber = (char*)malloc(20); // 16 digits + 3 dashes + null terminator
    if (!cardNumber) {
        writeErrorLog("Failed to allocate memory for card number");
        return NULL;
    }
    
    // Format: XXXX-XXXX-XXXX-XXXX
    sprintf(cardNumber, "%04d-%04d-%04d-%04d", 
            4000 + (rand() % 999),  // Start with 4 for "Visa-like" numbers
            1000 + (rand() % 9000),
            1000 + (rand() % 9000),
            1000 + (rand() % 9000));
    
    return cardNumber;
}

// Function to generate a 3-digit CVV
int generateCVV() {
    return 100 + (rand() % 900); // 3-digit number between 100 and 999
}

