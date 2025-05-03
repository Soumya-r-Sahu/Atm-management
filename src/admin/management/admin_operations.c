#include "../../../include/admin/admin_operations.h"
#include "../../../include/admin/admin_auth.h"
#include "../../../include/common/utils/logger.h"
#include "../../../include/common/database/database.h"
#include "../../../include/common/database/card_account_management.h"
#include "../../../include/common/paths.h"
#include "../../../include/common/utils/path_manager.h" // For dynamic paths
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mysql/mysql.h>

#undef ATM_DATA_FILE
#define ATM_DATA_FILE getATMDataFilePath() // Use dynamic path

#undef TEMP_ATM_DATA_FILE
#define TEMP_ATM_DATA_FILE getTempATMDataFilePath() // Use dynamic path

// Forward declarations for helper functions
int createCustomerAccount(const char *accountHolderName, int *cardNumber, int *pin);
int generateUniqueCardNumber(void);
int generateRandomPin(void);

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
    
    // Create the account using database function
    // Using default values for optional parameters
    char pinStr[10];
    sprintf(pinStr, "%04d", *pin);
    char cardStr[20];
    sprintf(cardStr, "%d", *cardNumber);
    
    // Call the database function with required parameters
    int result = createNewAccount(
        accountHolderName,         // name
        "Address not provided",    // default address
        "Phone not provided",      // default phone
        "Email not provided",      // default email
        cardStr,                   // card number as string
        1000.0,                    // initial balance
        cardNumber,                // card number pointer for output
        pin                        // pin pointer for output
    );
    
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
    MYSQL* conn = initMySQLConnection();
    if (conn == NULL) {
        return 0;
    }

    char query[256];
    snprintf(query, sizeof(query), "UPDATE ATMs SET status = '%s' WHERE atm_id = '%s'", new_status, atm_id);

    if (mysql_query(conn, query)) {
        write_error_log(mysql_error(conn));
        mysql_close(conn);
        return 0;
    }

    mysql_close(conn);
    return 1;
}