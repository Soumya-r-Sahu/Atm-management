#include "admin_operations.h"
#include "admin_db.h"
#include "../utils/logger.h"
#include "../database/database.h"
#include "../common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Path to the ATM data file
#define ATM_DATA_FILE "data/atm_data.txt"
#define TEMP_ATM_DATA_FILE "data/temp/atm_data_temp.txt"

// Create new customer account with the given details
int createCustomerAccount(const char *accountHolderName, int *cardNumber, int *pin) {
    // Generate random card number and PIN if not provided
    if (*cardNumber <= 0) {
        *cardNumber = generateUniqueCardNumber();
    }
    
    if (*pin <= 0) {
        *pin = generateRandomPin();
    }
    
    // Log before trying to create the account
    char logMsg[100];
    sprintf(logMsg, "Attempting to create account for %s with card %d", 
            accountHolderName, *cardNumber);
    writeAuditLog("ADMIN", logMsg);
    
    // Create the account using the admin_db function
    int result = createNewAccount(accountHolderName, *cardNumber, *pin);
    
    if (result) {
        sprintf(logMsg, "Successfully created account for %s", accountHolderName);
    } else {
        sprintf(logMsg, "Failed to create account for %s", accountHolderName);
    }
    writeAuditLog("ADMIN", logMsg);
    
    return result;
}

// Block a customer's card
void blockCustomerCard(int cardNumber) {
    // First check if the card exists
    if (!doesCardExist(cardNumber)) {
        printf("\nError: Card number %d does not exist.\n", cardNumber);
        return;
    }
    
    // Check if card is already blocked
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        printf("\nError: Could not open card file for reading.\n");
        writeErrorLog("Failed to open card file while blocking card");
        return;
    }
    
    char line[256];
    int storedCardNumber;
    char status[10];
    int found = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], cardStatus[10], pinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, cardStatus, pinHash) >= 7) {
            storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                found = 1;
                strcpy(status, cardStatus);
                break;
            }
        }
    }
    
    fclose(file);
    
    if (!found) {
        printf("\nError: Card not found in the database.\n");
        return;
    }
    
    if (strstr(status, "Blocked") != NULL) {
        printf("\nCard is already blocked.\n");
        return;
    }
    
    // Block the card using database function
    if (blockCard(cardNumber)) {
        printf("\nCard %d has been blocked successfully.\n", cardNumber);
        
        // Log the action
        char logMsg[100];
        sprintf(logMsg, "Admin blocked card %d", cardNumber);
        writeAuditLog("ADMIN", logMsg);
    } else {
        printf("\nError: Failed to block card %d.\n", cardNumber);
        writeErrorLog("Failed to block card via admin interface");
    }
}

// Unblock a customer's card
void unblockCustomerCard(int cardNumber) {
    // First check if the card exists
    if (!doesCardExist(cardNumber)) {
        printf("\nError: Card number %d does not exist.\n", cardNumber);
        return;
    }
    
    // Check if card is already active
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        printf("\nError: Could not open card file for reading.\n");
        writeErrorLog("Failed to open card file while unblocking card");
        return;
    }
    
    char line[256];
    int storedCardNumber;
    char status[10];
    int found = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], cardStatus[10], pinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, cardStatus, pinHash) >= 7) {
            storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                found = 1;
                strcpy(status, cardStatus);
                break;
            }
        }
    }
    
    fclose(file);
    
    if (!found) {
        printf("\nError: Card not found in the database.\n");
        return;
    }
    
    if (strstr(status, "Active") != NULL) {
        printf("\nCard is already active.\n");
        return;
    }
    
    // Unblock the card using database function
    if (unblockCard(cardNumber)) {
        printf("\nCard %d has been unblocked successfully.\n", cardNumber);
        
        // Log the action
        char logMsg[100];
        sprintf(logMsg, "Admin unblocked card %d", cardNumber);
        writeAuditLog("ADMIN", logMsg);
    } else {
        printf("\nError: Failed to unblock card %d.\n", cardNumber);
        writeErrorLog("Failed to unblock card via admin interface");
    }
}

// Change customer card status (block/unblock)
void changeCardStatus(int cardNumber) {
    // First check if the card exists
    if (!doesCardExist(cardNumber)) {
        printf("\nError: Card number %d does not exist.\n", cardNumber);
        return;
    }
    
    // Check card's current status
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        printf("\nError: Could not open card file.\n");
        return;
    }
    
    char line[256];
    int storedCardNumber;
    char storedUsername[50], storedStatus[10];
    int found = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], pinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                found = 1;
                break;
            }
        }
    }
    
    fclose(file);
    
    if (!found) {
        printf("\nError: Card number %d does not exist.\n", cardNumber);
        return;
    }
    
    // Ask for new status
    printf("\nCurrent status: %s\n", status);
    printf("Enter new status (1 for Active, 2 for Blocked): ");
    int statusChoice;
    scanf("%d", &statusChoice);
    
    const char* newStatus;
    if (statusChoice == 1) {
        newStatus = "Active";
    } else if (statusChoice == 2) {
        newStatus = "Blocked";
    } else {
        printf("\nInvalid status choice.\n");
        return;
    }
    
    // Update the card details
    if (updateCardDetails(cardNumber, -1, newStatus)) {
        printf("\nCard status updated successfully to '%s'.\n", newStatus);
        
        // Log the activity
        char logMsg[100];
        sprintf(logMsg, "Changed card %d status to '%s'", cardNumber, newStatus);
        writeAuditLog("ADMIN", logMsg);
    } else {
        printf("\nError: Failed to update card status.\n");
        writeErrorLog("Failed to update card status via admin interface");
    }
}

// Update ATM status
int updateAtmStatus(const char* atmId, const char* newStatus) {
    FILE* file = fopen(ATM_DATA_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open ATM data file for reading");
        return 0; // Failed to open file
    }
    
    // Create a temporary file for writing the updated data
    FILE* tempFile = fopen(TEMP_ATM_DATA_FILE, "w");
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
    
    if (!found) {
        remove(TEMP_ATM_DATA_FILE);
        writeErrorLog("ATM ID not found in ATM data file");
        return 0;
    }
    
    // Replace the original file with the updated temp file
    if (remove(ATM_DATA_FILE) != 0) {
        writeErrorLog("Failed to delete original ATM data file");
        return 0;
    }
    
    if (rename(TEMP_ATM_DATA_FILE, ATM_DATA_FILE) != 0) {
        writeErrorLog("Failed to rename temp ATM data file");
        return 0;
    }
    
    return 1; // Update successful
}