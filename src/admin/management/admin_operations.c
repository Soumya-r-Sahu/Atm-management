#include "../../include/admin/admin_operations.h"
#include "../../include/admin/admin_auth.h"
#include "../../utils/logger.h"
#include "../../database/database.h"
#include "../../database/card_account_management.h"  // Added include for card management functions
#include "../../common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Path to the ATM data file
#define ATM_DATA_FILE "data/atm_data.txt"
#define TEMP_ATM_DATA_FILE "data/temp/atm_data_temp.txt"

// Forward declarations for helper functions
int createCustomerAccount(const char *accountHolderName, int *cardNumber, int *pin);
int isCardNumberUnique(int cardNumber);
int generateUniqueCardNumber(void);
int generateRandomPin(void);
int createNewAccount(const char *accountHolderName, int cardNumber, int pin);

// Create a new account
int create_account(void) {
    char accountHolderName[100];
    int cardNumber = 0;
    int pin = 0;
    char choice;
    
    printf("\n===== Create New Account =====\n");
    
    // Clear input buffer first
    while (getchar() != '\n');
    
    printf("Enter account holder name: ");
    fgets(accountHolderName, sizeof(accountHolderName), stdin);
    
    // Remove newline character
    size_t len = strlen(accountHolderName);
    if (len > 0 && accountHolderName[len-1] == '\n') {
        accountHolderName[len-1] = '\0';
    }
    
    printf("Do you want to specify a card number? (y/n): ");
    scanf(" %c", &choice);
    
    if (choice == 'y' || choice == 'Y') {
        printf("Enter card number (6 digits): ");
        scanf("%d", &cardNumber);
        
        // Validate card number
        if (cardNumber < 100000 || cardNumber > 999999) {
            printf("Invalid card number. Must be 6 digits.\n");
            return 0;
        }
        
        // Check if card number is unique
        if (!isCardNumberUnique(cardNumber)) {
            printf("Error: Card number already exists.\n");
            return 0;
        }
    }
    
    printf("Do you want to specify a PIN? (y/n): ");
    scanf(" %c", &choice);
    
    if (choice == 'y' || choice == 'Y') {
        printf("Enter PIN (4 digits): ");
        scanf("%d", &pin);
        
        // Validate PIN
        if (pin < 1000 || pin > 9999) {
            printf("Invalid PIN. Must be 4 digits.\n");
            return 0;
        }
    }
    
    // Call the actual implementation function
    if (createCustomerAccount(accountHolderName, &cardNumber, &pin)) {
        printf("\nAccount created successfully!\n");
        printf("Card Number: %d\n", cardNumber);
        printf("PIN: %d\n", pin);
        printf("\nPlease instruct the customer to change their PIN on first use.\n");
        return 1;
    } else {
        printf("\nError: Failed to create account.\n");
        return 0;
    }
}

// Create a customer account with provided details (helper function)
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

// Toggle the ATM service mode
int toggle_service_mode(void) {
    int currentStatus = get_service_status();
    int newStatus = !currentStatus;
    
    if (set_service_status(newStatus)) {
        printf("\nATM service mode successfully %s.\n", newStatus ? "locked" : "unlocked");
        
        char logMsg[100];
        sprintf(logMsg, "ATM service mode %s by admin", newStatus ? "locked" : "unlocked");
        writeAuditLog("ADMIN", logMsg);
        
        return 1;
    } else {
        printf("\nFailed to change ATM service mode.\n");
        return 0;
    }
}

// Get the current service status
int get_service_status(void) {
    FILE* file = fopen("data/status.txt", "r");
    if (!file) {
        // If file doesn't exist, create it and set default status to unlocked (0)
        file = fopen("data/status.txt", "w");
        if (file) {
            fprintf(file, "0");
            fclose(file);
            return 0;
        }
        writeErrorLog("Failed to create status file");
        return 0;
    }
    
    int status;
    if (fscanf(file, "%d", &status) != 1) {
        fclose(file);
        return 0;
    }
    
    fclose(file);
    return status;
}

// Set the ATM service status
int set_service_status(int status) {
    FILE* file = fopen("data/status.txt", "w");
    if (!file) {
        writeErrorLog("Failed to open status file for writing");
        return 0;
    }
    
    fprintf(file, "%d", status ? 1 : 0);
    fclose(file);
    
    return 1;
}

// Regenerate a card PIN
void regenerate_card_pin(int card_number) {
    printf("\n===== Regenerate Card PIN =====\n");
    
    // Check if card exists
    if (!doesCardExist(card_number)) {
        printf("Card number %d does not exist.\n", card_number);
        return;
    }
    
    // Generate new PIN
    int newPin = rand() % 9000 + 1000; // 4-digit PIN between 1000-9999
    
    // Update PIN in database
    if (updatePIN(card_number, newPin)) {
        printf("PIN for card %d has been regenerated successfully.\n", card_number);
        printf("New PIN: %d\n", newPin);
        writeAuditLog("ADMIN", "Card PIN regenerated by admin");
    } else {
        printf("Failed to regenerate PIN for card %d.\n", card_number);
    }
}

// Toggle card status (block/unblock)
void toggle_card_status(int card_number) {
    printf("\n===== Toggle Card Status =====\n");
    
    // Check if card exists
    if (!doesCardExist(card_number)) {
        printf("Card number %d does not exist.\n", card_number);
        return;
    }
    
    // Get current status
    int isActive = isCardActive(card_number);
    
    // Toggle status
    if (isActive) {
        if (blockCard(card_number)) {
            printf("Card %d has been blocked successfully.\n", card_number);
            writeAuditLog("ADMIN", "Card blocked by admin");
        } else {
            printf("Failed to block card %d.\n", card_number);
        }
    } else {
        if (unblockCard(card_number)) {
            printf("Card %d has been unblocked successfully.\n", card_number);
            writeAuditLog("ADMIN", "Card unblocked by admin");
        } else {
            printf("Failed to unblock card %d.\n", card_number);
        }
    }
}

// Update ATM status
int update_atm_status(const char* atm_id, const char* new_status) {
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
        if (sscanf(line, "| %s |", atmIdFromLine) == 1 && strcmp(atmIdFromLine, atm_id) == 0) {
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
                       atm_id, location, new_status, totalCash, lastRefilled, transactionCount);
                
                // Log the activity
                char logMsg[200];
                sprintf(logMsg, "Updated ATM %s status from '%s' to '%s'", 
                        atm_id, status, new_status);
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