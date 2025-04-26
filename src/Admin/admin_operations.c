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

// Toggle ATM service mode (Online/Offline)
void toggleServiceMode() {
    int currentStatus = getServiceStatus();
    const char* currentStatusStr = currentStatus ? "Offline" : "Online";
    
    printf("\n===== Toggle Service Mode =====\n");
    printf("Current Status: %s\n", currentStatusStr);
    
    int confirm;
    printf("Do you want to change the status to %s? (1 for Yes, 0 for No): ", 
          currentStatus ? "Online" : "Offline");
    scanf("%d", &confirm);
    
    if (confirm) {
        if (setServiceStatus(!currentStatus)) {
            printf("\nService status successfully changed to %s.\n", 
                  currentStatus ? "Online" : "Offline");
            
            // Log the activity
            char logMsg[100];
            sprintf(logMsg, "Changed ATM service status to %s", 
                  currentStatus ? "Online" : "Offline");
            writeAuditLog("ADMIN", logMsg);
        } else {
            printf("\nError: Failed to change service status.\n");
            writeErrorLog("Failed to toggle service mode");
        }
    } else {
        printf("\nOperation cancelled.\n");
    }
}

// Get current ATM service status
int getServiceStatus() {
    // 0 = Online, 1 = Offline
    FILE *file = fopen("../data/status.txt", "r");
    
    if (file == NULL) {
        // Default to online if file doesn't exist
        return 0;
    }
    
    int status;
    fscanf(file, "%d", &status);
    fclose(file);
    
    return status;
}

// Set ATM service status
int setServiceStatus(int status) {
    FILE *file = fopen("../data/status.txt", "w");
    
    if (file == NULL) {
        return 0; // Failed to open/create file
    }
    
    fprintf(file, "%d", status);
    fclose(file);
    
    return 1; // Success
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