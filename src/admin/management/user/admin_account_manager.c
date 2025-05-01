#include "../../../../include/admin/management/user/admin_account_manager.h"
#include "../../../../include/common/utils/logger.h"
#include "../../../../include/common/database/database.h"
#include "../../../../include/common/utils/path_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Create a new customer account
bool create_account(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        writeErrorLog("Unauthorized attempt to create account");
        return false;
    }
    
    char name[100];
    char address[200];
    char phone[20];
    char email[100];
    char accountType[20];
    float initialDeposit;
    
    printf("\n=======================================\n");
    printf("=        CREATE NEW ACCOUNT          =\n");
    printf("=======================================\n");
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    printf("Enter customer name: ");
    if (!fgets(name, sizeof(name), stdin)) {
        return false;
    }
    name[strcspn(name, "\n")] = 0; // Remove newline
    
    printf("Enter customer address: ");
    if (!fgets(address, sizeof(address), stdin)) {
        return false;
    }
    address[strcspn(address, "\n")] = 0;
    
    printf("Enter phone number: ");
    if (!fgets(phone, sizeof(phone), stdin)) {
        return false;
    }
    phone[strcspn(phone, "\n")] = 0;
    
    printf("Enter email address: ");
    if (!fgets(email, sizeof(email), stdin)) {
        return false;
    }
    email[strcspn(email, "\n")] = 0;
    
    printf("Account type (Savings/Current): ");
    if (!fgets(accountType, sizeof(accountType), stdin)) {
        return false;
    }
    accountType[strcspn(accountType, "\n")] = 0;
    
    printf("Initial deposit amount: ");
    char deposit_str[20];
    if (!fgets(deposit_str, sizeof(deposit_str), stdin)) {
        return false;
    }
    initialDeposit = atof(deposit_str);
    
    if (initialDeposit <= 0) {
        printf("Error: Initial deposit must be greater than zero.\n");
        return false;
    }
    
    int newAccountNumber = 0;
    int newCardNumber = 0;
    
    // Call database function to create account
    if (createNewAccount(name, address, phone, email, accountType, 
                       initialDeposit, &newAccountNumber, &newCardNumber)) {
        
        printf("\nAccount created successfully!\n");
        printf("Account Number: %d\n", newAccountNumber);
        printf("Card Number: %d\n", newCardNumber);
        
        // Generate receipt
        char cardNumberStr[20];
        sprintf(cardNumberStr, "%d", newCardNumber);
        
        char expiryDate[10];
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        sprintf(expiryDate, "%02d/%02d", (tm_info->tm_mon + 1), (tm_info->tm_year + 1900 + 5) % 100);
        
        int cvv = generateCVV();
        
        // Generate the account receipt
        generateAccountReceipt(name, newAccountNumber, accountType, "BANK0001", 
                             cardNumberStr, expiryDate, cvv);
        
        char logMsg[200];
        sprintf(logMsg, "Account created - Name: %s, Account: %d, Card: %d", 
                name, newAccountNumber, newCardNumber);
        writeAuditLog(admin->username, logMsg);
        
        return true;
    } else {
        printf("\nError: Failed to create account. Please try again.\n");
        writeErrorLog("Failed to create new account");
        return false;
    }
}

// Function to regenerate a card PIN for an existing account
bool regenerate_card_pin(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        writeErrorLog("Unauthorized attempt to regenerate PIN");
        return false;
    }
    
    printf("\n=======================================\n");
    printf("=          REGENERATE CARD PIN       =\n");
    printf("=======================================\n");
    
    char cardNumberStr[20];
    printf("Enter card number: ");
    if (!fgets(cardNumberStr, sizeof(cardNumberStr), stdin)) {
        return false;
    }
    cardNumberStr[strcspn(cardNumberStr, "\n")] = 0; // Remove newline
    
    int cardNumber = atoi(cardNumberStr);
    
    // Validate card exists
    if (!doesCardExist(cardNumber)) {
        printf("Error: Card number does not exist.\n");
        return false;
    }
    
    // Generate new PIN (4-digit number)
    int newPin = 1000 + (rand() % 9000);
    
    // Update PIN in database
    if (updatePIN(cardNumber, newPin)) {
        printf("\nPIN regenerated successfully!\n");
        printf("New PIN: %04d\n", newPin);
        printf("\nNOTE: Please inform the customer to change\n");
        printf("this PIN on first use for security purposes.\n");
        
        char cardHolderName[100];
        if (getCardHolderName(cardNumber, cardHolderName, sizeof(cardHolderName))) {
            char logMsg[200];
            sprintf(logMsg, "PIN regenerated for card %d (Customer: %s) by %s", 
                    cardNumber, cardHolderName, admin->username);
            writeAuditLog("SECURITY", logMsg);
        } else {
            char logMsg[200];
            sprintf(logMsg, "PIN regenerated for card %d by %s", 
                    cardNumber, admin->username);
            writeAuditLog("SECURITY", logMsg);
        }
        
        return true;
    } else {
        printf("\nError: Failed to regenerate PIN. Please try again.\n");
        writeErrorLog("Failed to regenerate PIN");
        return false;
    }
}

// Function to toggle card status (block/unblock)
bool toggle_card_status(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        writeErrorLog("Unauthorized attempt to toggle card status");
        return false;
    }
    
    printf("\n=======================================\n");
    printf("=         BLOCK/UNBLOCK CARD         =\n");
    printf("=======================================\n");
    
    char cardNumberStr[20];
    printf("Enter card number: ");
    if (!fgets(cardNumberStr, sizeof(cardNumberStr), stdin)) {
        return false;
    }
    cardNumberStr[strcspn(cardNumberStr, "\n")] = 0; // Remove newline
    
    int cardNumber = atoi(cardNumberStr);
    
    // Validate card exists
    if (!doesCardExist(cardNumber)) {
        printf("Error: Card number does not exist.\n");
        return false;
    }
    
    // Get current status
    bool isActive = isCardActive(cardNumber);
    
    printf("\nCurrent status: %s\n", isActive ? "ACTIVE" : "BLOCKED");
    printf("Do you want to %s this card? (y/n): ", isActive ? "BLOCK" : "UNBLOCK");
    
    char confirm[5];
    if (!fgets(confirm, sizeof(confirm), stdin)) {
        return false;
    }
    
    if (tolower(confirm[0]) == 'y') {
        // Toggle status (opposite of current)
        if (updateCardStatus(cardNumber, !isActive)) {
            printf("\nCard successfully %s!\n", isActive ? "BLOCKED" : "UNBLOCKED");
            
            char cardHolderName[100];
            if (getCardHolderName(cardNumber, cardHolderName, sizeof(cardHolderName))) {
                char logMsg[200];
                sprintf(logMsg, "Card %d (Customer: %s) %s by %s", 
                        cardNumber, cardHolderName, 
                        isActive ? "BLOCKED" : "UNBLOCKED", 
                        admin->username);
                writeAuditLog("SECURITY", logMsg);
            } else {
                char logMsg[200];
                sprintf(logMsg, "Card %d %s by %s", 
                        cardNumber, isActive ? "BLOCKED" : "UNBLOCKED", 
                        admin->username);
                writeAuditLog("SECURITY", logMsg);
            }
            
            return true;
        } else {
            printf("\nError: Failed to update card status. Please try again.\n");
            writeErrorLog("Failed to update card status");
            return false;
        }
    } else {
        printf("\nOperation cancelled.\n");
        return true;
    }
}