#include "admin_operations.h"
#include "admin_db.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================
// Admin Authentication Functions
// ============================

// Authenticate admin credentials
int authenticateAdmin(const char* adminId, const char* adminPass) {
    char storedAdminId[50], storedAdminPass[50];
    
    // Load stored admin credentials
    if (!loadAdminCredentials(storedAdminId, storedAdminPass)) {
        printf("Error: Unable to load admin credentials.\n");
        return 0;
    }
    
    // Compare with entered credentials
    if (strcmp(adminId, storedAdminId) == 0 && strcmp(adminPass, storedAdminPass) == 0) {
        writeAuditLog("ADMIN", "Successful admin login");
        return 1; // Authentication successful
    }
    
    writeAuditLog("ADMIN", "Failed admin login attempt");
    return 0; // Authentication failed
}

// Handle admin lockout after multiple failed attempts
void handleAdminLockout(int* attempts, time_t* lockoutStartTime, int lockoutDuration) {
    if (*attempts <= 0) {
        time_t currentTime = time(NULL);
        if (*lockoutStartTime == 0) {
            // Start lockout
            *lockoutStartTime = currentTime;
            printf("Admin account is locked for %d seconds due to multiple failed attempts.\n", 
                  lockoutDuration);
            writeAuditLog("ADMIN", "Admin account locked due to multiple failed attempts");
        } else if (difftime(currentTime, *lockoutStartTime) >= lockoutDuration) {
            // Lockout period is over
            *attempts = 3; // Reset attempts
            *lockoutStartTime = 0; // Reset lockout
            printf("Lockout period has ended. You may try again.\n");
        } else {
            // Still in lockout period
            int remainingSeconds = lockoutDuration - (int)difftime(currentTime, *lockoutStartTime);
            printf("Admin account is locked. Please wait %d seconds.\n", remainingSeconds);
        }
    }
}

// ============================
// Core Admin Operations
// ============================

// Create a new ATM account
void createAccount() {
    char accountHolderName[50];
    int cardNumber, pin;
    float initialBalance;
    
    printf("\n===== Create New Account =====\n");
    
    // Get account holder name
    printf("Enter Account Holder Name: ");
    scanf(" %[^\n]s", accountHolderName);
    
    // Generate unique card number
    cardNumber = generateUniqueCardNumber();
    
    // Generate random PIN
    pin = generateRandomPin();
    
    // Get initial balance
    printf("Enter Initial Balance (optional): ");
    scanf("%f", &initialBalance);
    
    // Create the account
    if (createNewAccount(accountHolderName, cardNumber, pin)) {
        // Update the balance if initial balance was provided
        if (initialBalance > 0) {
            updateBalance(cardNumber, initialBalance);
        }
        
        // Show confirmation
        printf("\nAccount created successfully!\n");
        printf("Account Holder: %s\n", accountHolderName);
        printf("Card Number: %d\n", cardNumber);
        printf("PIN: %d\n", pin);
        printf("Initial Balance: $%.2f\n", initialBalance);
        printf("\nPlease share these details with the account holder.\n");
        
        // Log the activity
        char logMsg[200];
        sprintf(logMsg, "Created new account for %s with card number %d", 
                accountHolderName, cardNumber);
        writeAuditLog("ADMIN", logMsg);
    } else {
        printf("\nError: Failed to create account. Please try again.\n");
        writeErrorLog("Failed to create new account");
    }
}

// Regenerate PIN for a card
void regenerateCardPin(int cardNumber) {
    if (!isCardNumberUnique(cardNumber)) { // If not unique, it exists
        int newPIN = generateRandomPin();
        
        if (updateCardDetails(cardNumber, newPIN, NULL)) {
            printf("\nPIN successfully changed for card number %d.\n", cardNumber);
            printf("New PIN: %d\n", newPIN);
            printf("\nPlease share this new PIN with the account holder.\n");
            
            // Log the activity
            char logMsg[100];
            sprintf(logMsg, "Regenerated PIN for card number %d", cardNumber);
            writeAuditLog("ADMIN", logMsg);
        } else {
            printf("\nError: Failed to change PIN. Please try again.\n");
            writeErrorLog("Failed to regenerate PIN");
        }
    } else {
        printf("\nError: Card number %d does not exist.\n", cardNumber);
        writeErrorLog("Attempted to regenerate PIN for non-existent card");
    }
}

// Toggle card status (Active/Blocked)
void toggleCardStatus(int cardNumber) {
    if (!isCardNumberUnique(cardNumber)) { // If not unique, it exists
        // Determine current status
        FILE *file = fopen("../data/credentials.txt", "r");
        if (file == NULL) {
            printf("\nError: Unable to read card status.\n");
            writeErrorLog("Failed to read card status");
            return;
        }
        
        char line[256];
        int storedCardNumber;
        char storedUsername[50], storedStatus[10];
        int storedPIN;
        int found = 0;
        
        // Skip header lines
        fgets(line, sizeof(line), file);
        fgets(line, sizeof(line), file);
        
        while (fscanf(file, "%49[^|] | %d | %d | %9s", 
                     storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
            if (storedCardNumber == cardNumber) {
                found = 1;
                break;
            }
        }
        
        fclose(file);
        
        if (!found) {
            printf("\nError: Card number %d does not exist.\n", cardNumber);
            return;
        }
        
        // Ask for new status
        printf("\nCurrent status: %s\n", storedStatus);
        printf("Enter new status (1 for Active, 2 for Blocked): ");
        int statusChoice;
        scanf("%d", &statusChoice);
        
        const char* newStatus;
        if (statusChoice == 1) {
            newStatus = "Active";
        } else if (statusChoice == 2) {
            newStatus = "Blocked";
        } else {
            printf("\nError: Invalid status choice.\n");
            return;
        }
        
        // Update the status
        if (updateCardDetails(cardNumber, -1, newStatus)) {
            printf("\nCard status successfully changed to %s.\n", newStatus);
            
            // Log the activity
            char logMsg[100];
            sprintf(logMsg, "Changed status to %s for card number %d", 
                   newStatus, cardNumber);
            writeAuditLog("ADMIN", logMsg);
        } else {
            printf("\nError: Failed to change card status. Please try again.\n");
            writeErrorLog("Failed to update card status");
        }
    } else {
        printf("\nError: Card number %d does not exist.\n", cardNumber);
        writeErrorLog("Attempted to toggle status for non-existent card");
    }
}

// Update ATM status (Online, Offline, Under Maintenance)
int updateAtmStatus(const char* atmId, const char* newStatus) {
    FILE* file = fopen("data/atm_data.txt", "r");
    if (file == NULL) {
        writeErrorLog("Failed to open ATM data file for reading");
        return 0; // Failed to open file
    }
    
    // Create a temporary file for writing the updated data
    FILE* tempFile = fopen("data/temp/atm_data_temp.txt", "w");
    if (tempFile == NULL) {
        fclose(file);
        writeErrorLog("Failed to create temporary file for ATM data update");
        return 0; // Failed to create temporary file
    }
    
    char line[256];
    int lineCount = 0;
    int found = 0;
    
    // Read and copy the file line by line
    while (fgets(line, sizeof(line), file) != NULL) {
        lineCount++;
        
        // Copy header lines and separator lines as they are
        if (lineCount <= 3 || line[0] == '+') {
            fprintf(tempFile, "%s", line);
            continue;
        }
        
        // Check if this is the line with the specified ATM ID
        char atmIdFromLine[20];
        if (sscanf(line, "| %s |", atmIdFromLine) == 1 && strcmp(atmIdFromLine, atmId) == 0) {
            // Found the ATM to update
            found = 1;
            
            // Extract all fields
            char location[100];
            char status[30];
            double totalCash;
            char lastRefilled[30];
            int transactionCount;
            
            // Parse the current line
            // Format: | ATM001 | Main Branch, Downtown   | Online           | 250000.00  | 2025-04-25 08:00:00 | 123              |
            if (sscanf(line, "| %*s | %99[^|] | %29[^|] | %lf | %29[^|] | %d |",
                      location, status, &totalCash, lastRefilled, &transactionCount) == 5) {
                
                // Write the updated line to the temp file
                fprintf(tempFile, "| %s | %s | %s | %.2f | %s | %d |\n",
                       atmId, location, newStatus, totalCash, lastRefilled, transactionCount);
                
                // Log the activity
                char logMsg[200];
                sprintf(logMsg, "Updated ATM %s status from '%s' to '%s'", 
                        atmId, status, newStatus);
                writeAuditLog("ADMIN", logMsg);
            } else {
                // If there was an error parsing the line, copy it as-is
                fprintf(tempFile, "%s", line);
                writeErrorLog("Failed to parse ATM data line during status update");
            }
        } else {
            // Not the target ATM, copy line as-is
            fprintf(tempFile, "%s", line);
        }
    }
    
    // Close both files
    fclose(file);
    fclose(tempFile);
    
    // Check if ATM was found
    if (!found) {
        remove("data/temp/atm_data_temp.txt"); // Clean up temp file
        writeErrorLog("ATM ID not found during status update");
        return 0; // ATM ID not found
    }
    
    // Replace the original file with the updated file
    if (remove("data/atm_data.txt") != 0) {
        writeErrorLog("Failed to remove original ATM data file during update");
        return 0; // Failed to remove original file
    }
    
    if (rename("data/temp/atm_data_temp.txt", "data/atm_data.txt") != 0) {
        writeErrorLog("Failed to rename temporary ATM data file during update");
        return 0; // Failed to rename file
    }
    
    return 1; // Success
}