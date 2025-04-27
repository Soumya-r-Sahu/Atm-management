#include "admin_db.h"
#include "../utils/logger.h"
#include "../utils/hash_utils.h"
#include "../common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// File paths
#define ADMIN_CREDENTIALS_FILE "data/admin_credentials.txt"
#define TEMP_ADMIN_CRED_FILE "data/temp/temp_admin_credentials.txt"
#define STATUS_FILE "data/status.txt"

// ============================
// Admin Credentials Operations
// ============================

// Load admin credentials from file
int loadAdminCredentials(char *adminId, char *adminPass) {
    FILE *file = fopen(ADMIN_CREDENTIALS_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open admin_credentials.txt while loading admin credentials");
        return 0;
    }

    char line[256];
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
    char username[50], role[20], lastLogin[30], status[10];
    char passwordHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        // Skip separator lines
        if (line[0] == '+') continue;
        
        // Parse the table row format with pipe separators
        if (sscanf(line, "| %s | %s | %s | %s | %s | %s |", 
                  adminId, username, passwordHash, role, lastLogin, status) >= 6) {
            // Copy the password hash to adminPass
            strcpy(adminPass, passwordHash);
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
    // First, read the existing file to maintain the table format
    FILE *readFile = fopen(ADMIN_CREDENTIALS_FILE, "r");
    if (readFile == NULL) {
        writeErrorLog("Failed to open admin_credentials.txt for reading while updating credentials");
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
    char entries[10][256]; // Assuming no more than 10 admin entries
    int entryCount = 0;
    int targetEntryIndex = -1;
    char line[256];
    char adminId[50], username[50], passwordHash[65], role[20], lastLogin[30], status[10];
    
    while (fgets(line, sizeof(line), readFile) != NULL) {
        // Skip table separator lines
        if (line[0] == '+') {
            strcpy(entries[entryCount++], line);
            continue;
        }
        
        // Check if this is the target admin to update
        if (sscanf(line, "| %s | %s | %s | %s | %s | %s |", 
                  adminId, username, passwordHash, role, lastLogin, status) >= 6) {
            if (strcmp(adminId, newAdminId) == 0) {
                // Found the admin to update
                targetEntryIndex = entryCount;
            }
        }
        
        strcpy(entries[entryCount++], line);
    }
    
    fclose(readFile);
    
    // Now write back the file with the updated admin credentials
    FILE *writeFile = fopen(ADMIN_CREDENTIALS_FILE, "w");
    if (writeFile == NULL) {
        writeErrorLog("Failed to open admin_credentials.txt for writing while updating credentials");
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
            // Extract the values first
            sscanf(entries[i], "| %s | %s | %s | %s | %s | %s |", 
                  adminId, username, passwordHash, role, lastLogin, status);
                  

            // Write the updated line with new password hash
            fprintf(writeFile, "| %-14s | %-13s | %-32s | %-12s | %-19s | %-7s |\n", 
                   newAdminId, username, newAdminPass, role, lastLogin, status);
        } else {
            fprintf(writeFile, "%s", entries[i]);
        }
    }
    
    fclose(writeFile);
    
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
    
    fprintf(cardFile, "%s  | %s     | %-16d | Debit     | %s  | Active  | %s\n", 
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
    FILE *file = fopen(STATUS_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open status.txt while getting service status");
        return 0; // Default to Online if file can't be read
    }

    char status[20];
    if (fgets(status, sizeof(status), file) != NULL) {
        fclose(file);
        if (strstr(status, "Offline") != NULL) {
            return 1; // ATM is Offline
        }
    } else {
        fclose(file);
        writeErrorLog("Failed to read from status.txt");
        return 0; // Default to Online if read fails
    }

    return 0; // ATM is Online
}

// Set service status (0 = Online, 1 = Offline)
int setServiceStatus(int isOutOfService) {
    FILE *file = fopen(STATUS_FILE, "w");
    if (file == NULL) {
        writeErrorLog("Failed to open status.txt while setting service status");
        return 0;
    }

    if (isOutOfService) {
        fprintf(file, "Status: Offline");
        writeAuditLog("ADMIN", "ATM service set to Offline");
    } else {
        fprintf(file, "Status: Online");
        writeAuditLog("ADMIN", "ATM service set to Online");
    }

    fclose(file);
    return 1; // Status updated successfully
}