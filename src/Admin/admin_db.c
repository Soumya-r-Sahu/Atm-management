#include "admin_db.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Use DATA_DIR if defined, otherwise fallback to relative paths
#ifdef DATA_DIR
    #define PATH_PREFIX DATA_DIR
#else
    #define PATH_PREFIX "../data"
#endif

// File paths with proper concatenation
#define ADMIN_CRED_FILE PATH_PREFIX "/admin_credentials.txt"
#define CREDENTIALS_FILE PATH_PREFIX "/credentials.txt"
#define ACCOUNTING_FILE PATH_PREFIX "/accounting.txt"
#define TEMP_CREDENTIALS_FILE PATH_PREFIX "/temp_credentials.txt"
#define STATUS_FILE PATH_PREFIX "/status.txt"

// ============================
// Admin Credentials Operations
// ============================

// Load admin credentials from admin_credentials.txt file
int loadAdminCredentials(char *adminId, char *adminPass) {
    FILE *file = fopen(ADMIN_CRED_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open admin_credentials.txt while loading admin credentials");
        return 0;
    }

    if (fscanf(file, "%s %s", adminId, adminPass) != 2) {
        fclose(file);
        writeErrorLog("Invalid format in admin_credentials.txt");
        return 0;
    }

    fclose(file);
    return 1; // Credentials loaded successfully
}

// Update admin credentials in admin_credentials.txt file
int updateAdminCredentials(const char *newAdminId, const char *newAdminPass) {
    FILE *file = fopen(ADMIN_CRED_FILE, "w");
    if (file == NULL) {
        writeErrorLog("Failed to open admin_credentials.txt for updating credentials");
        return 0;
    }

    fprintf(file, "%s %s", newAdminId, newAdminPass);
    fclose(file);
    
    char logMsg[100];
    sprintf(logMsg, "Admin credentials updated successfully");
    writeAuditLog("ADMIN", logMsg);
    
    return 1; // Credentials updated successfully
}

// ============================
// ATM Account Operations
// ============================

// Create a new ATM account with the given details
int createNewAccount(const char *accountHolderName, int cardNumber, int pin) {
    // Update credentials.txt
    FILE *credFile = fopen(CREDENTIALS_FILE, "a");
    if (credFile == NULL) {
        writeErrorLog("Failed to open credentials.txt while creating new account");
        return 0;
    }

    fprintf(credFile, "%-20s | %-11d | %-4d | Active\n", 
            accountHolderName, cardNumber, pin);
    fclose(credFile);

    // Update accounting.txt with initial balance of 0
    FILE *acctFile = fopen(ACCOUNTING_FILE, "a");
    if (acctFile == NULL) {
        writeErrorLog("Failed to open accounting.txt while creating new account");
        return 0;
    }

    fprintf(acctFile, "%-11d | %.2f\n", cardNumber, 0.0);
    fclose(acctFile);

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
        cardNumber = 100000 + rand() % 900000;
    } while (!isCardNumberUnique(cardNumber));
    
    return cardNumber;
}

// Generate a random 4-digit PIN
int generateRandomPin() {
    return 1000 + rand() % 9000; // 4-digit PIN between 1000 and 9999
}

// Check if a card number is unique
int isCardNumberUnique(int cardNumber) {
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open credentials.txt while checking card number uniqueness");
        return 0; // File not found, assume not unique to be safe
    }

    char line[256];
    // Skip the header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);

    int storedCardNumber;
    char storedUsername[50], storedStatus[10];
    int storedPIN;
    
    while (fscanf(file, "%49[^|] | %d | %d | %9s", 
                 storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            fclose(file);
            return 0; // Card number already exists
        }
    }

    fclose(file);
    return 1; // Card number is unique
}

// Update card details (PIN and/or status)
int updateCardDetails(int cardNumber, int newPIN, const char *newStatus) {
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open credentials.txt while updating card details");
        return 0;
    }

    FILE *tempFile = fopen(TEMP_CREDENTIALS_FILE, "w");
    if (tempFile == NULL) {
        fclose(file);
        writeErrorLog("Failed to create temporary file while updating card details");
        return 0;
    }

    char line[256];
    int storedCardNumber, storedPIN;
    char storedUsername[50], storedStatus[10];
    int found = 0;

    // Copy header lines
    if (fgets(line, sizeof(line), file))
        fprintf(tempFile, "%s", line);
    if (fgets(line, sizeof(line), file))
        fprintf(tempFile, "%s", line);

    // Process each account line
    while (fscanf(file, "%49[^|] | %d | %d | %9s", 
                 storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            found = 1;
            // Update PIN if specified (newPIN != -1)
            int finalPIN = (newPIN != -1) ? newPIN : storedPIN;
            // Update status if specified
            const char *finalStatus = (newStatus != NULL) ? newStatus : storedStatus;
            
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", 
                   storedUsername, storedCardNumber, finalPIN, finalStatus);
            
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
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", 
                   storedUsername, storedCardNumber, storedPIN, storedStatus);
        }
    }

    fclose(file);
    fclose(tempFile);

    // Replace original file with updated file
    if (!found) {
        remove(TEMP_CREDENTIALS_FILE);
        writeErrorLog("Card not found while updating card details");
        return 0;
    }

    if (remove(CREDENTIALS_FILE) != 0 || 
        rename(TEMP_CREDENTIALS_FILE, CREDENTIALS_FILE) != 0) {
        writeErrorLog("Failed to replace credentials.txt with updated file");
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