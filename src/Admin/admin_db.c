#include "admin_db.h"
#include "../utils/logger.h"
#include "../utils/hash_utils.h"
#include "../common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>

// ============================
// Admin Credentials Operations
// ============================

// Use the proper constants from paths.h
static const char* getLocalAdminCredentialsFilePath() {
    return isTestingMode() ? TEST_ADMIN_CREDENTIALS_FILE : PROD_ADMIN_CREDENTIALS_FILE;
}

// Use the implementation from paths.h instead of redefining
const char* getStatusFilePath() {
    return isTestingMode() ? TEST_DATA_DIR "/status.txt" : PROD_DATA_DIR "/status.txt";
}

// Load admin credentials from file
int loadAdminCredentials(char *adminId, char *adminPass) {
    if (adminId == NULL || adminPass == NULL) {
        writeErrorLog("NULL pointers provided to loadAdminCredentials");
        return 0;
    }
    
    const char* filePath = getLocalAdminCredentialsFilePath();
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        char errorMsg[256];
        sprintf(errorMsg, "Failed to open admin credentials file at %s", filePath);
        writeErrorLog(errorMsg);
        return 0;
    }

    char line[256] = {0};
    int found = 0;
    
    // Skip the header lines (first 3 lines including separator lines)
    for (int i = 0; i < 3; i++) {
        if (fgets(line, sizeof(line), file) == NULL) {
            fclose(file);
            writeErrorLog("Admin credentials file format is invalid - missing header lines");
            return 0;
        }
    }
    
    // Now read the actual admin data (first valid admin entry)
    char username[50] = {0}, role[20] = {0}, lastLogin[30] = {0}, status[10] = {0};
    char passwordHash[65] = {0};
    
    while (fgets(line, sizeof(line), file) != NULL) {
        // Skip separator lines
        if (line[0] == '+') continue;
        
        // Parse the table row format with pipe separators
        if (sscanf(line, "| %49s | %49s | %64s | %19s | %29s | %9s |", 
                  adminId, username, passwordHash, role, lastLogin, status) >= 6) {
            // Copy the password hash to adminPass
            strncpy(adminPass, passwordHash, 64);
            adminPass[64] = '\0';
            found = 1;
            break;
        }
    }
    
    fclose(file);
    
    if (!found) {
        writeErrorLog("No valid admin credentials found in file");
        return 0;
    }
    
    return 1; // Credentials loaded successfully
}

// Update admin credentials
int updateAdminCredentials(const char *newAdminId, const char *newAdminPass) {
    if (newAdminId == NULL || newAdminPass == NULL) {
        writeErrorLog("NULL parameters provided to updateAdminCredentials");
        return 0;
    }
    
    // First, read the existing file to maintain the table format
    const char* filePath = getLocalAdminCredentialsFilePath();
    FILE *readFile = fopen(filePath, "r");
    if (readFile == NULL) {
        char errorMsg[256];
        sprintf(errorMsg, "Failed to open admin credentials file at %s for reading", filePath);
        writeErrorLog(errorMsg);
        return 0;
    }
    
    // Get the header lines and table format
    char headerLines[3][256];
    for (int i = 0; i < 3; i++) {
        if (fgets(headerLines[i], sizeof(headerLines[i]), readFile) == NULL) {
            fclose(readFile);
            writeErrorLog("Admin credentials file format is invalid - missing header lines");
            return 0;
        }
    }
    
    // Store existing admin entries
    char entries[10][256] = {{0}}; // Assuming no more than 10 admin entries with zero initialization
    int entryCount = 0;
    int targetEntryIndex = -1;
    char line[256] = {0};
    char adminId[50] = {0}, username[50] = {0}, passwordHash[65] = {0}; 
    char role[20] = {0}, lastLogin[30] = {0}, status[10] = {0};
    
    while (fgets(line, sizeof(line), readFile) != NULL) {
        // Check array bounds to prevent overflow
        if (entryCount >= 10) {
            fclose(readFile);
            writeErrorLog("Too many admin entries in credentials file");
            return 0;
        }
        
        // Skip table separator lines
        if (line[0] == '+') {
            strncpy(entries[entryCount++], line, sizeof(entries[0]) - 1);
            continue;
        }
        
        // Reset variables before parsing
        memset(adminId, 0, sizeof(adminId));
        memset(username, 0, sizeof(username));
        memset(passwordHash, 0, sizeof(passwordHash));
        memset(role, 0, sizeof(role));
        memset(lastLogin, 0, sizeof(lastLogin));
        memset(status, 0, sizeof(status));
        
        // Check if this is the target admin to update
        if (sscanf(line, "| %49s | %49s | %64s | %19s | %29s | %9s |", 
                  adminId, username, passwordHash, role, lastLogin, status) >= 6) {
            if (strcmp(adminId, newAdminId) == 0) {
                // Found the admin to update
                targetEntryIndex = entryCount;
            }
        }
        
        strncpy(entries[entryCount++], line, sizeof(entries[0]) - 1);
    }
    
    fclose(readFile);
    
    // Create temp file path with proper directory
    char tempFileName[256] = {0};
    sprintf(tempFileName, "%s/temp/temp_admin_credentials.txt", 
            isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR);
    
    // Now write to the temp file with the updated admin credentials
    FILE *writeFile = fopen(tempFileName, "w");
    if (writeFile == NULL) {
        char errorMsg[256];
        sprintf(errorMsg, "Failed to create temporary file at %s", tempFileName);
        writeErrorLog(errorMsg);
        return 0;
    }
    
    // Write the header lines
    for (int i = 0; i < 3; i++) {
        fprintf(writeFile, "%s", headerLines[i]);
    }
    
    // Write the entries, updating the target admin if found
    for (int i = 0; i < entryCount; i++) {
        if (i == targetEntryIndex) {
            // This is the admin entry to update
            // Reset variables before parsing
            memset(adminId, 0, sizeof(adminId));
            memset(username, 0, sizeof(username));
            memset(passwordHash, 0, sizeof(passwordHash));
            memset(role, 0, sizeof(role));
            memset(lastLogin, 0, sizeof(lastLogin));
            memset(status, 0, sizeof(status));
            
            // Extract the values first
            sscanf(entries[i], "| %49s | %49s | %64s | %19s | %29s | %9s |", 
                  adminId, username, passwordHash, role, lastLogin, status);
            
            // Write the updated line with new password hash
            fprintf(writeFile, "| %-14s | %-13s | %-32s | %-12s | %-19s | %-7s |\n", 
                   newAdminId, username, newAdminPass, role, lastLogin, status);
        } else {
            fprintf(writeFile, "%s", entries[i]);
        }
    }
    
    fclose(writeFile);
    
    // If target admin entry wasn't found, log warning but don't consider it an error
    // as we might be adding a new admin entry
    if (targetEntryIndex == -1) {
        char warningMsg[150];
        sprintf(warningMsg, "Admin ID %s not found in credentials file during update", newAdminId);
        writeInfoLog(warningMsg);
    }
    
    // Replace original file with updated one
    if (remove(filePath) != 0) {
        char errorMsg[150];
        sprintf(errorMsg, "Failed to remove original credentials file at %s", filePath);
        writeErrorLog(errorMsg);
        remove(tempFileName);
        return 0;
    }
    
    if (rename(tempFileName, filePath) != 0) {
        char errorMsg[150];
        sprintf(errorMsg, "Failed to rename temp file from %s to %s", tempFileName, filePath);
        writeErrorLog(errorMsg);
        return 0;
    }
    
    // Log the update
    char logMsg[100];
    sprintf(logMsg, "Admin credentials updated successfully for %s", newAdminId);
    writeAuditLog("ADMIN", logMsg);
    
    return 1; // Credentials updated successfully
}

// ============================
// ATM Account Operations
// ============================

// Create a new ATM account with the given details
int createNewAccount(const char *accountHolderName, int cardNumber, int pin) {
    // Generate PIN hash
    char pinStr[10];
    sprintf(pinStr, "%d", pin);
    char *pinHash = sha256_hash(pinStr);
    if (pinHash == NULL) {
        writeErrorLog("Failed to generate PIN hash while creating new account");
        return 0;
    }
    
    // Get next available IDs
    int nextCustomerID = 10001; // Default start value
    int nextAccountID = 10001; // Default start value
    int nextCardID = 10001; // Default start value
    
    // Generate unique IDs
    char customerID[10], accountID[10], cardID[10];
    sprintf(customerID, "C%d", nextCustomerID);
    sprintf(accountID, "A%d", nextAccountID);
    sprintf(cardID, "D%d", nextCardID);
    
    // Generate expiry date (2 years from now)
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    tm_now->tm_year += 2;
    char expiryDate[11];
    strftime(expiryDate, sizeof(expiryDate), "%Y-%m-%d", tm_now);
    
    // Update customer.txt
    FILE *customerFile = fopen(getCustomerFilePath(), "a");
    if (customerFile == NULL) {
        writeErrorLog("Failed to open customer.txt while creating new account");
        free(pinHash);
        return 0;
    }
    
    fprintf(customerFile, "%s | %s | %-20s | Regular | Active | 0.00\n", 
            customerID, accountID, accountHolderName);
    fclose(customerFile);
    
    // Update card.txt
    FILE *cardFile = fopen(getCardFilePath(), "a");
    if (cardFile == NULL) {
        writeErrorLog("Failed to open card.txt while creating new account");
        free(pinHash);
        return 0;
    }
    
    fprintf(cardFile, "%s | %s | %-16d | Debit     | %s | Active  | %s\n", 
            cardID, accountID, cardNumber, expiryDate, pinHash);
    fclose(cardFile);
    
    // Clean up allocated memory
    free(pinHash);
    
    // Log the account creation
    char logMsg[100];
    sprintf(logMsg, "New account created for %s with card number %d", 
            accountHolderName, cardNumber);
    writeAuditLog("ADMIN", logMsg);
    
    return 1; // Account created successfully
}

// Generate a unique 6-digit card number
int generateUniqueCardNumber() {
    int cardNumber;
    srand(time(NULL)); // Seed the random number generator
    
    do {
        // Generate a random 6-digit number
        cardNumber = 100000 + (rand() % 900000);
    } while (!isCardNumberUnique(cardNumber));
    
    return cardNumber;
}

// Generate a random 4-digit PIN
int generateRandomPin() {
    srand(time(NULL)); // Seed the random number generator
    return 1000 + (rand() % 9000); // Generate a random 4-digit number
}

// Check if a card number is unique
int isCardNumberUnique(int cardNumber) {
    FILE *file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card.txt while checking card number uniqueness");
        return 1; // Assume it's unique if we can't check
    }
    
    char line[256];
    bool found = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], pinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL && !found) {
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
    return !found; // Return true if card number is unique (not found)
}

// Update card details (PIN and/or status)
int updateCardDetails(int cardNumber, int newPIN, const char *newStatus) {
    FILE *file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card.txt while updating card details");
        return 0;
    }

    char tempFileName[100];
    sprintf(tempFileName, "%s/temp/temp_card.txt", isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR);
    
    FILE *tempFile = fopen(tempFileName, "w");
    if (tempFile == NULL) {
        fclose(file);
        writeErrorLog("Failed to create temporary file while updating card details");
        return 0;
    }

    char line[256];
    bool found = false;

    // Copy header lines
    if (fgets(line, sizeof(line), file))
        fprintf(tempFile, "%s", line);
    if (fgets(line, sizeof(line), file))
        fprintf(tempFile, "%s", line);

    // Process each card line
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], storedPinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        char lineCopy[256];
        strcpy(lineCopy, line);
        
        if (sscanf(lineCopy, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, storedPinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                found = true;
                // Generate new PIN hash if needed
                char* finalPinHash = storedPinHash;
                char newPinHash[65];
                bool freePinHash = false;
                
                if (newPIN != -1) {
                    char pinStr[10];
                    sprintf(pinStr, "%d", newPIN);
                    char* tempPinHash = sha256_hash(pinStr);
                    if (tempPinHash != NULL) {
                        strcpy(newPinHash, tempPinHash);
                        finalPinHash = newPinHash;
                        free(tempPinHash);
                    }
                }
                
                // Update status if specified
                const char* finalStatus = (newStatus != NULL) ? newStatus : status;
                
                fprintf(tempFile, "%s | %s | %s | %s | %s | %-7s | %s\n", 
                        cardID, accountID, cardNumberStr, cardType, expiryDate, finalStatus, finalPinHash);
                
                // Log the update
                char logMsg[200];
                if (newPIN != -1 && newStatus != NULL) {
                    sprintf(logMsg, "Updated PIN and status to '%s' for card %d", 
                            finalStatus, cardNumber);
                } else if (newPIN != -1) {
                    sprintf(logMsg, "Updated PIN for card %d", cardNumber);
                } else {
                    sprintf(logMsg, "Updated status to '%s' for card %d", 
                            finalStatus, cardNumber);
                }
                writeAuditLog("ADMIN", logMsg);
            } else {
                fprintf(tempFile, "%s", line);
            }
        } else {
            fprintf(tempFile, "%s", line);
        }
    }

    fclose(file);
    fclose(tempFile);

    // Replace original file with updated file
    if (!found) {
        remove(tempFileName);
        writeErrorLog("Card not found while updating card details");
        return 0;
    }

    if (remove(getCardFilePath()) != 0 || 
        rename(tempFileName, getCardFilePath()) != 0) {
        writeErrorLog("Failed to replace card.txt with updated file");
        return 0;
    }

    return 1; // Card details updated successfully
}

// ============================
// ATM Service Status Operations
// ============================

// Toggle the ATM service mode between Online and Offline
int toggleServiceMode() {
    int currentStatus = getServiceStatus();
    return setServiceStatus(!currentStatus);
}

// Get current service status (0 = Online, 1 = Offline)
int getServiceStatus() {
    const char* filePath = getStatusFilePath();
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        char errorMsg[256] = {0};
        sprintf(errorMsg, "Failed to open status file at %s while getting service status", filePath);
        writeErrorLog(errorMsg);
        return 0; // Default to Online if file can't be read
    }

    char status[50] = {0}; // Increased buffer size for safety
    if (fgets(status, sizeof(status), file) != NULL) {
        fclose(file);
        // Case-insensitive check for "Offline" for robustness
        if (strstr(status, "Offline") != NULL || strstr(status, "offline") != NULL) {
            writeInfoLog("ATM service status checked: Offline");
            return 1; // ATM is Offline
        }
    } else {
        fclose(file);
        writeErrorLog("Failed to read from status file - file may be empty");
        return 0; // Default to Online if read fails
    }

    writeInfoLog("ATM service status checked: Online");
    return 0; // ATM is Online
}

// Set service status (0 = Online, 1 = Offline)
int setServiceStatus(int isOutOfService) {
    const char* filePath = getStatusFilePath();
    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        char errorMsg[256] = {0};
        sprintf(errorMsg, "Failed to open status file at %s while setting service status", filePath);
        writeErrorLog(errorMsg);
        return 0;
    }

    // First create backup of current status file
    char backupFilePath[256] = {0};
    sprintf(backupFilePath, "%s.bak", filePath);
    
    // Copy existing file to backup if it exists
    FILE* existingFile = fopen(filePath, "r");
    if (existingFile != NULL) {
        char buffer[256] = {0};
        FILE* backupFile = fopen(backupFilePath, "w");
        
        if (backupFile != NULL) {
            while (fgets(buffer, sizeof(buffer), existingFile) != NULL) {
                fputs(buffer, backupFile);
            }
            fclose(backupFile);
        }
        fclose(existingFile);
    }

    // Set the new status
    if (isOutOfService) {
        if (fprintf(file, "Status: Offline") < 0) {
            writeErrorLog("Failed to write to status file");
            fclose(file);
            return 0;
        }
        writeAuditLog("ADMIN", "ATM service set to Offline");
    } else {
        if (fprintf(file, "Status: Online") < 0) {
            writeErrorLog("Failed to write to status file");
            fclose(file);
            return 0;
        }
        writeAuditLog("ADMIN", "ATM service set to Online");
    }

    fclose(file);
    return 1; // Status updated successfully
}