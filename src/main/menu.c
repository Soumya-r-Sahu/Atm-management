#include "menu.h"
#include "../validation/pin_validation.h"
#include "../database/database.h"
#include "../transaction/transaction_manager.h"
#include "../utils/logger.h"
#include "../utils/hash_utils.h"
#include "../utils/language_support.h"
#include "../config/config_manager.h"  // Added for getConfigValueInt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Include time.h for session management

#define EXIT_OPTION 7  // Added EXIT_OPTION constant

// Function to display the main menu and handle user selections
void displayMainMenu(int cardNumber) {
    // Get session timeout from config (in seconds)
    int sessionTimeout = getConfigValueInt(CONFIG_SESSION_TIMEOUT_SECONDS);
    if (sessionTimeout < 0) sessionTimeout = 180; // Default 3 minutes if config not found
    
    // Initialize timer
    time_t sessionStart = time(NULL);
    time_t currentTime;
    
    int choice;
    char holderName[50] = "Customer";
    char phoneNumber[15] = "0000000000";
    
    // Get card holder's name and phone number
    getCardHolderName(cardNumber, holderName, sizeof(holderName));
    getCardHolderPhone(cardNumber, phoneNumber, sizeof(phoneNumber));
    
    do {
        // Check for session timeout
        currentTime = time(NULL);
        if (difftime(currentTime, sessionStart) > sessionTimeout) {
            printf("Session timed out due to inactivity.\n");
            printf("Please insert your card again.\n");
            return;
        }
        
        printf("\n");
        printf(" ___________________________________________________\n");
        printf("|                                                   |\n");
        printf("|                   MAIN MENU                       |\n");
        printf("|___________________________________________________|\n");
        printf("| 1. Check Balance                                  |\n");
        printf("| 2. Deposit                                        |\n");
        printf("| 3. Withdraw                                       |\n");
        printf("| 4. Money Transfer                                 |\n");
        printf("| 5. Mini Statement                                 |\n");
        printf("| 6. Change PIN                                     |\n");
        printf("| 7. Exit                                           |\n");
        printf("|___________________________________________________|\n\n");
        
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: // Check Balance
                handleBalanceCheck(cardNumber, holderName, phoneNumber);
                break;
            case 2: // Deposit
                handleDeposit(cardNumber, holderName, phoneNumber);
                break;
            case 3: // Withdraw
                handleWithdrawal(cardNumber, holderName, phoneNumber);
                break;
            case 4: // Money Transfer
                handleMoneyTransfer(cardNumber, holderName, phoneNumber);
                break;
            case 5: // Mini Statement
                handleMiniStatement(cardNumber, holderName, phoneNumber);
                break;
            case 6: // Change PIN
                handlePinChange(cardNumber, holderName);
                break;
            case EXIT_OPTION: // Exit
                printf("\nThank you for using our ATM service!\n");
                return;
            default:
                printf("\nInvalid option selected. Please try again.\n");
        }
        
        // Reset session timer on user activity
        sessionStart = time(NULL);
        
    } while (1); // Infinite loop, will be exited on valid user action or session timeout
}

// Handle balance check operation
void handleBalanceCheck(int cardNumber, const char *username, const char *phoneNumber) {
    printf("\nChecking your account balance...\n");
    
    TransactionResult result = checkAccountBalance(cardNumber, username);
    
    if (result.success) {
        printf("\n===== BALANCE INFORMATION =====\n");
        printf("Account Holder: %s\n", username);
        printf("Current Balance: $%.2f\n", result.newBalance);
        printf("===============================\n");
        
        // Generate receipt
        generateReceipt(cardNumber, TRANSACTION_BALANCE_CHECK, 0.0f, result.newBalance, phoneNumber);
    } else {
        printf("\nError: %s\n", result.message);
    }
}

// Handle deposit operation
void handleDeposit(int cardNumber, const char *username, const char *phoneNumber) {
    float amount;
    
    printf("\n===== DEPOSIT =====\n");
    printf("Enter amount to deposit: $");
    scanf("%f", &amount);
    
    if (amount <= 0) {
        printf("\nError: Invalid amount. Please enter a positive value.\n");
        return;
    }
    
    printf("\nProcessing deposit of $%.2f...\n", amount);
    
    TransactionResult result = performDeposit(cardNumber, amount, username);
    
    if (result.success) {
        printf("\nDeposit successful!\n");
        printf("Previous Balance: $%.2f\n", result.oldBalance);
        printf("New Balance: $%.2f\n", result.newBalance);
        
        // Generate receipt
        generateReceipt(cardNumber, TRANSACTION_DEPOSIT, amount, result.newBalance, phoneNumber);
    } else {
        printf("\nError: %s\n", result.message);
    }
}

// Handle withdrawal operation
void handleWithdrawal(int cardNumber, const char *username, const char *phoneNumber) {
    float amount;
    
    printf("\n===== WITHDRAWAL =====\n");
    printf("Enter amount to withdraw: $");
    scanf("%f", &amount);
    
    if (amount <= 0) {
        printf("\nError: Invalid amount. Please enter a positive value.\n");
        return;
    }
    
    printf("\nProcessing withdrawal of $%.2f...\n", amount);
    
    TransactionResult result = performWithdrawal(cardNumber, amount, username);
    
    if (result.success) {
        printf("\nWithdrawal successful!\n");
        printf("Previous Balance: $%.2f\n", result.oldBalance);
        printf("New Balance: $%.2f\n", result.newBalance);
        
        // Generate receipt
        generateReceipt(cardNumber, TRANSACTION_WITHDRAWAL, amount, result.newBalance, phoneNumber);
    } else {
        printf("\nError: %s\n", result.message);
    }
}

// Handle money transfer operation
void handleMoneyTransfer(int cardNumber, const char *username, const char *phoneNumber) {
    int targetCardNumber;
    float amount;
    
    printf("\n===== MONEY TRANSFER =====\n");
    printf("Enter recipient's card number: ");
    scanf("%d", &targetCardNumber);
    
    if (targetCardNumber == cardNumber) {
        printf("\nError: Cannot transfer money to your own account.\n");
        return;
    }
    
    if (!doesCardExist(targetCardNumber)) {
        printf("\nError: Recipient card number is invalid.\n");
        return;
    }
    
    printf("Enter amount to transfer: $");
    scanf("%f", &amount);
    
    if (amount <= 0) {
        printf("\nError: Invalid amount. Please enter a positive value.\n");
        return;
    }
    
    printf("\nProcessing transfer of $%.2f to card %d...\n", amount, targetCardNumber);
    
    TransactionResult result = performFundTransfer(cardNumber, targetCardNumber, amount, username);
    
    if (result.success) {
        printf("\nTransfer successful!\n");
        printf("Previous Balance: $%.2f\n", result.oldBalance);
        printf("New Balance: $%.2f\n", result.newBalance);
        
        // Generate receipt
        generateReceipt(cardNumber, TRANSACTION_MONEY_TRANSFER, amount, result.newBalance, phoneNumber);
    } else {
        printf("\nError: %s\n", result.message);
    }
}

// Handle mini statement request
void handleMiniStatement(int cardNumber, const char *username, const char *phoneNumber) {
    printf("\nFetching your mini statement...\n");
    
    TransactionResult result = getMiniStatement(cardNumber, username);
    
    if (result.success) {
        printf("\n===== MINI STATEMENT =====\n");
        printf("Account Holder: %s\n", username);
        printf("Current Balance: $%.2f\n", result.newBalance);
        printf("\nRecent Transactions:\n");
        printf("(Detailed transaction history will be displayed here)\n");
        printf("===========================\n");
        
        // Generate receipt
        generateReceipt(cardNumber, TRANSACTION_MINI_STATEMENT, 0.0f, result.newBalance, phoneNumber);
    } else {
        printf("\nError: %s\n", result.message);
    }
}

// Handle PIN change operation
void handlePinChange(int cardNumber, const char *username) {
    char currentPinStr[10], newPinStr[10], confirmPinStr[10];
    
    printf("\n===== PIN CHANGE =====\n");
    printf("Enter current PIN: ");
    scanf("%s", currentPinStr);
    
    // Convert PIN string to number (for legacy validation)
    int currentPin = atoi(currentPinStr);
    
    // First try hash-based authentication
    bool authenticated = false;
    char* currentPinHash = hashPIN(currentPinStr);
    
    if (currentPinHash != NULL) {
        authenticated = validateCardWithHash(cardNumber, currentPinHash);
        free(currentPinHash);
    }
    
    // If hash validation fails, try legacy PIN validation
    if (!authenticated && !validateCard(cardNumber, currentPin)) {
        printf("\nError: Current PIN is incorrect.\n");
        recordFailedPINAttempt(cardNumber);
        return;
    }
    
    printf("Enter new PIN: ");
    scanf("%s", newPinStr);
    
    printf("Confirm new PIN: ");
    scanf("%s", confirmPinStr);
    
    if (strcmp(newPinStr, confirmPinStr) != 0) {
        printf("\nError: New PINs do not match.\n");
        return;
    }
    
    // Convert new PIN string to number
    int newPin = atoi(newPinStr);
    
    if (!isValidPINFormat(newPin)) {
        printf("\nError: Invalid PIN format. Please enter a 4-digit PIN.\n");
        return;
    }
    
    // Update PIN with hash for enhanced security
    char* newPinHash = hashPIN(newPinStr);
    bool success = false;
    
    if (newPinHash != NULL) {
        success = updatePINHash(cardNumber, newPinHash);
        free(newPinHash);
    } 
    
    // Fall back to legacy PIN update if hash update fails
    if (!success) {
        success = updatePIN(cardNumber, newPin);
    }
    
    if (success) {
        printf("\nPIN changed successfully!\n");
        
        // Log the PIN change
        char logMsg[100];
        sprintf(logMsg, "PIN changed for card %d", cardNumber);
        writeAuditLog("SECURITY", logMsg);
        
        // Reset PIN attempts counter
        char cardNumberStr[20];
        sprintf(cardNumberStr, "%d", cardNumber);
        resetPINAttempts(cardNumberStr, 0); // 0 for production mode
    } else {
        printf("\nError: Failed to change PIN.\n");
    }
}