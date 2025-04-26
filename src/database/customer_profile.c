#include "customer_profile.h"
#include "../utils/logger.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CREDENTIALS_FILE "../data/credentials.txt"
#define ACCOUNTING_FILE "../data/accounting.txt"
#define TEMP_CRED_FILE "../data/temp/temp_credentials.txt"
#define TEMP_ACC_FILE "../data/temp/temp_accounting.txt"

// Helper function to get current date as a string (YYYY-MM-DD)
static void getCurrentDate(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d", t);
}

// Function to retrieve a customer profile by card number
bool getCustomerProfile(int cardNumber, CustomerProfile *profile) {
    if (profile == NULL) {
        writeErrorLog("NULL profile pointer provided to getCustomerProfile");
        return false;
    }

    // Get the customer's name
    char name[100] = "";
    if (!getCardHolderName(cardNumber, name, sizeof(name))) {
        return false;
    }
    
    // Get the balance
    float balance = fetchBalance(cardNumber);
    if (balance < 0) {
        return false;
    }
    
    // Check if card is active
    char status[20] = "Unknown";
    if (isCardActive(cardNumber)) {
        strcpy(status, "Active");
    } else {
        strcpy(status, "Blocked");
    }
    
    // Fill in the profile
    profile->cardNumber = cardNumber;
    strncpy(profile->name, name, sizeof(profile->name) - 1);
    profile->name[sizeof(profile->name) - 1] = '\0';  // Ensure null termination
    
    profile->balance = balance;
    strncpy(profile->status, status, sizeof(profile->status) - 1);
    profile->status[sizeof(profile->status) - 1] = '\0';  // Ensure null termination
    
    // Get the last login date from credentials file or use current date if not found
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    bool foundLastLogin = false;
    
    if (file != NULL) {
        char line[256];
        char lastLogin[20] = "";
        
        // Skip header lines
        fgets(line, sizeof(line), file);
        fgets(line, sizeof(line), file);
        
        int storedCardNumber;
        int storedPIN;
        char storedUsername[100], storedStatus[20];
        
        while (fscanf(file, "%99[^|] | %d | %d | %19s", 
                     storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
            if (storedCardNumber == cardNumber) {
                profile->pin = storedPIN;
                foundLastLogin = true;
                break;
            }
        }
        
        fclose(file);
    }
    
    if (!foundLastLogin) {
        // If we didn't find the last login date, use the current date
        getCurrentDate(profile->lastLoginDate, sizeof(profile->lastLoginDate));
    }
    
    return true;
}

// Function to update a customer's profile
bool updateCustomerProfile(const CustomerProfile *profile) {
    if (profile == NULL) {
        writeErrorLog("NULL profile pointer provided to updateCustomerProfile");
        return false;
    }
    
    // Update the balance
    if (!updateBalance(profile->cardNumber, profile->balance)) {
        char logMsg[100];
        sprintf(logMsg, "Failed to update balance for card %d", profile->cardNumber);
        writeErrorLog(logMsg);
        return false;
    }
    
    // Update the status if needed
    bool isCurrentlyActive = isCardActive(profile->cardNumber);
    bool shouldBeActive = (strcmp(profile->status, "Active") == 0);
    
    if (isCurrentlyActive && !shouldBeActive) {
        if (!blockCard(profile->cardNumber)) {
            char logMsg[100];
            sprintf(logMsg, "Failed to block card %d", profile->cardNumber);
            writeErrorLog(logMsg);
            return false;
        }
    } else if (!isCurrentlyActive && shouldBeActive) {
        if (!unblockCard(profile->cardNumber)) {
            char logMsg[100];
            sprintf(logMsg, "Failed to unblock card %d", profile->cardNumber);
            writeErrorLog(logMsg);
            return false;
        }
    }
    
    // Update PIN if needed
    int currentPIN = 0;
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    if (file != NULL) {
        char line[256];
        // Skip headers
        fgets(line, sizeof(line), file);
        fgets(line, sizeof(line), file);
        
        int storedCardNumber;
        int storedPIN;
        char storedUsername[100], storedStatus[20];
        
        while (fscanf(file, "%99[^|] | %d | %d | %19s", 
                     storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
            if (storedCardNumber == profile->cardNumber) {
                currentPIN = storedPIN;
                break;
            }
        }
        fclose(file);
    }
    
    if (currentPIN != 0 && currentPIN != profile->pin) {
        if (!updatePIN(profile->cardNumber, profile->pin)) {
            char logMsg[100];
            sprintf(logMsg, "Failed to update PIN for card %d", profile->cardNumber);
            writeErrorLog(logMsg);
            return false;
        }
    }
    
    // Log the update
    char logMsg[100];
    sprintf(logMsg, "Updated profile for card %d", profile->cardNumber);
    writeAuditLog("ACCOUNT", logMsg);
    
    return true;
}

// Function to create a new customer profile
bool createCustomerProfile(const CustomerProfile *profile) {
    if (profile == NULL) {
        writeErrorLog("NULL profile pointer provided to createCustomerProfile");
        return false;
    }
    
    // Check if card already exists
    if (doesCardExist(profile->cardNumber)) {
        char logMsg[100];
        sprintf(logMsg, "Cannot create profile: Card %d already exists", profile->cardNumber);
        writeErrorLog(logMsg);
        return false;
    }
    
    // Add to credentials file
    FILE *credFile = fopen(CREDENTIALS_FILE, "a");
    if (credFile == NULL) {
        writeErrorLog("Failed to open credentials file for new profile");
        return false;
    }
    
    fprintf(credFile, "%-20s | %-11d | %-4d | %-9s\n", 
           profile->name, profile->cardNumber, profile->pin, profile->status);
    fclose(credFile);
    
    // Add to accounting file
    FILE *accFile = fopen(ACCOUNTING_FILE, "a");
    if (accFile == NULL) {
        writeErrorLog("Failed to open accounting file for new profile");
        // Try to revert the credentials change
        FILE *credTemp = fopen(TEMP_CRED_FILE, "w");
        if (credTemp != NULL) {
            FILE *origCred = fopen(CREDENTIALS_FILE, "r");
            if (origCred != NULL) {
                char line[256];
                while (fgets(line, sizeof(line), origCred) != NULL) {
                    int cardNum;
                    // Only copy lines that don't match the card number we tried to add
                    if (sscanf(line, "%*[^|] | %d", &cardNum) != 1 || cardNum != profile->cardNumber) {
                        fprintf(credTemp, "%s", line);
                    }
                }
                fclose(origCred);
            }
            fclose(credTemp);
            remove(CREDENTIALS_FILE);
            rename(TEMP_CRED_FILE, CREDENTIALS_FILE);
        }
        return false;
    }
    
    fprintf(accFile, "%-11d | %.2f\n", profile->cardNumber, profile->balance);
    fclose(accFile);
    
    // Log the creation
    char logMsg[100];
    sprintf(logMsg, "Created new profile for card %d", profile->cardNumber);
    writeAuditLog("ACCOUNT", logMsg);
    
    return true;
}

// Function to change a customer's PIN
bool changeCustomerPIN(int cardNumber, int newPIN) {
    if (newPIN < 1000 || newPIN > 9999) {
        char logMsg[100];
        sprintf(logMsg, "Invalid PIN format: %d for card %d", newPIN, cardNumber);
        writeErrorLog(logMsg);
        return false;
    }
    
    bool success = updatePIN(cardNumber, newPIN);
    if (success) {
        char logMsg[100];
        sprintf(logMsg, "PIN changed for card %d", cardNumber);
        writeAuditLog("ACCOUNT", logMsg);
    } else {
        char logMsg[100];
        sprintf(logMsg, "Failed to change PIN for card %d", cardNumber);
        writeErrorLog(logMsg);
    }
    
    return success;
}

// Function to block/unblock a customer's card
bool setCardStatus(int cardNumber, const char *status) {
    if (status == NULL) {
        writeErrorLog("NULL status pointer provided to setCardStatus");
        return false;
    }
    
    bool success = false;
    
    if (strcmp(status, "Active") == 0) {
        success = unblockCard(cardNumber);
    } else if (strcmp(status, "Blocked") == 0) {
        success = blockCard(cardNumber);
    } else {
        char logMsg[100];
        sprintf(logMsg, "Invalid status '%s' for card %d", status, cardNumber);
        writeErrorLog(logMsg);
        return false;
    }
    
    if (success) {
        char logMsg[100];
        sprintf(logMsg, "Card %d status changed to %s", cardNumber, status);
        writeAuditLog("ACCOUNT", logMsg);
    } else {
        char logMsg[100];
        sprintf(logMsg, "Failed to change status to %s for card %d", status, cardNumber);
        writeErrorLog(logMsg);
    }
    
    return success;
}

// Function to update customer's last login date
bool updateLastLogin(int cardNumber) {
    // NOTE: In a real system, we would store the last login date in the database
    // For this simple implementation, we'll just log the login
    char currentDate[20];
    getCurrentDate(currentDate, sizeof(currentDate));
    
    char logMsg[100];
    sprintf(logMsg, "Customer with card %d logged in on %s", cardNumber, currentDate);
    writeAuditLog("LOGIN", logMsg);
    
    return true;
}

// Function to print customer profile summary
void printCustomerSummary(const CustomerProfile *profile) {
    if (profile == NULL) {
        printf("Error: NULL profile pointer provided\n");
        return;
    }
    
    printf("\n==== Customer Profile Summary ====\n");
    printf("Name: %s\n", profile->name);
    
    // Mask the card number for display
    char maskedCard[25];
    char cardStr[20];
    sprintf(cardStr, "%d", profile->cardNumber);
    
    int len = strlen(cardStr);
    if (len <= 4) {
        strncpy(maskedCard, cardStr, sizeof(maskedCard) - 1);
    } else {
        int i, j;
        for (i = 0, j = 0; i < len - 4; i++) {
            if (i > 0 && i % 4 == 0) {
                maskedCard[j++] = ' ';
            }
            maskedCard[j++] = '*';
        }
        maskedCard[j++] = ' ';
        for (i = len - 4; i < len; i++) {
            maskedCard[j++] = cardStr[i];
        }
        maskedCard[j] = '\0';
    }
    
    printf("Card Number: %s\n", maskedCard);
    printf("Balance: $%.2f\n", profile->balance);
    printf("Status: %s\n", profile->status);
    printf("Last Login: %s\n", profile->lastLoginDate);
    printf("================================\n\n");
}

// Function to retrieve a customer's phone number by card number
char* getCustomerPhoneNumber(int cardNumber) {
    static char phoneNumber[20] = "";
    CustomerProfile profile;
    
    // Try to get the customer profile
    if (!getCustomerProfile(cardNumber, &profile)) {
        writeErrorLog("Failed to retrieve customer profile for phone number lookup");
        return NULL;
    }
    
    // Copy the phone number from the profile
    strncpy(phoneNumber, profile.phoneNumber, sizeof(phoneNumber) - 1);
    phoneNumber[sizeof(phoneNumber) - 1] = '\0';  // Ensure null termination
    
    return phoneNumber;
}