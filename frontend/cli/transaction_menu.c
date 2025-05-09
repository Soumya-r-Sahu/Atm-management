/**
 * @file transaction_menu.c
 * @brief Implementation of transaction menu for the ATM Management System CLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "../../include/common/database/card_account_management.h"
#include "../../include/common/database/account_management.h"
#include "../../include/common/database/core_banking_interface.h"
#include "../../include/atm/transaction/transaction_processor.h"
#include "../../include/common/utils/logger.h"
#include "menu_utils.h"
#include "transaction_menu.h"

// Function prototypes
void displayTransactionMenu(const char *cardNumber);
void handleTransactionMenuChoice(int choice, int cardNumber);
void performBalanceInquiry(int cardNumber);
void performWithdrawal(int cardNumber);
void performDeposit(int cardNumber);
void performTransfer(int cardNumber);
void performPinChange(int cardNumber);
void viewMiniStatement(int cardNumber);
void performBillPayment(int cardNumber);

/**
 * @brief Run the transaction menu
 * @param cardNumber Customer card number
 */
void runTransactionMenu(int cardNumber) {
    bool running = true;
    char cardHolderName[100] = {0};
    char card_number_str[20];
    
    snprintf(card_number_str, sizeof(card_number_str), "%d", cardNumber);
    cbs_get_card_holder_name(card_number_str, cardHolderName, sizeof(cardHolderName));
    
    while (running) {
        clearScreen();
        printHeader("CORE BANKING SYSTEM - TRANSACTION MENU");
        printf("\nWelcome, %s!\n", cardHolderName);
        printf("Card Number: %d\n\n", cardNumber);
        
        displayTransactionMenu(card_number_str);
        
        int choice;
        printf("\nEnter your choice (1-8): ");
        if (scanf("%d", &choice) != 1) {
            // Clear input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            sleep(2);
            continue;
        }
        
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        if (choice == 8) {
            running = false;
        } else {
            handleTransactionMenuChoice(choice, cardNumber);
        }
    }
}

/**
 * @brief Display the transaction menu options
 * @param cardNumber Card number string
 */
void displayTransactionMenu(const char *cardNumber) {
    printf("Transaction Menu:\n");
    printf("1. Check Balance\n");
    printf("2. Withdraw Money\n");
    printf("3. Deposit Money\n");
    printf("4. Fund Transfer\n");
    printf("5. Change PIN\n");
    printf("6. Mini Statement\n");
    printf("7. Bill Payment\n");
    printf("8. Exit\n");
}

/**
 * @brief Handle transaction menu choices
 * @param choice Menu choice
 * @param cardNumber Card number
 */
void handleTransactionMenuChoice(int choice, int cardNumber) {
    switch (choice) {
        case 1:
            performBalanceInquiry(cardNumber);
            break;
        case 2:
            performWithdrawal(cardNumber);
            break;
        case 3:
            performDeposit(cardNumber);
            break;
        case 4:
            performTransfer(cardNumber);
            break;
        case 5:
            performPinChange(cardNumber);
            break;
        case 6:
            viewMiniStatement(cardNumber);
            break;
        case 7:
            performBillPayment(cardNumber);
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            sleep(2);
            break;
    }
}

/**
 * @brief Perform balance inquiry for the customer
 * @param cardNumber Card number
 */
void performBalanceInquiry(int cardNumber) {
    clearScreen();
    printHeader("BALANCE INQUIRY");
    
    TransactionResult result = process_balance_inquiry(cardNumber);
    
    if (result.status == TRANSACTION_SUCCESS) {
        printf("\nBalance Inquiry Successful\n");
        printf("Current Balance: $%.2f\n", result.balance_after);
    } else {
        printf("\nBalance Inquiry Failed\n");
        printf("Error: %s\n", result.message);
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Perform cash withdrawal
 * @param cardNumber Card number
 */
void performWithdrawal(int cardNumber) {
    clearScreen();
    printHeader("CASH WITHDRAWAL");
    
    // Check if card is active
    if (!cbs_is_card_active(cardNumber)) {
        printf("\nWithdrawal Failed: Card is inactive or blocked\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Get current balance
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", cardNumber);
    
    double balance = 0.0;
    if (!cbs_get_balance_by_card(card_number_str, &balance)) {
        printf("\nWithdrawal Failed: Could not retrieve account balance\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("\nCurrent Balance: $%.2f\n", balance);
    
    // Get withdrawal amount
    float amount;
    printf("\nEnter amount to withdraw: $");
    if (scanf("%f", &amount) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("\nInvalid amount\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    // Validate amount
    if (amount <= 0) {
        printf("\nWithdrawal Failed: Invalid amount\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    if (amount > balance) {
        printf("\nWithdrawal Failed: Insufficient funds\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Perform withdrawal
    TransactionResult result = process_withdrawal(cardNumber, amount);
    
    if (result.status == TRANSACTION_SUCCESS) {
        printf("\nWithdrawal Successful\n");
        printf("Amount Withdrawn: $%.2f\n", result.amount_processed);
        printf("Previous Balance: $%.2f\n", result.balance_before);
        printf("New Balance: $%.2f\n", result.balance_after);
        
        // Print receipt
        printf("\n======= WITHDRAWAL RECEIPT =======\n");
        printf("Card Number: %d\n", cardNumber);
        printf("Amount: $%.2f\n", amount);
        printf("Balance: $%.2f\n", result.balance_after);
        printf("Date: %s\n", __DATE__);
        printf("Time: %s\n", __TIME__);
        printf("Thank you for using Core Banking System\n");
        printf("=================================\n");
    } else {
        printf("\nWithdrawal Failed\n");
        printf("Error: %s\n", result.message);
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Perform cash deposit
 * @param cardNumber Card number
 */
void performDeposit(int cardNumber) {
    clearScreen();
    printHeader("CASH DEPOSIT");
    
    // Check if card is active
    if (!cbs_is_card_active(cardNumber)) {
        printf("\nDeposit Failed: Card is inactive or blocked\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Get current balance
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", cardNumber);
    
    double balance = 0.0;
    if (!cbs_get_balance_by_card(card_number_str, &balance)) {
        printf("\nDeposit Failed: Could not retrieve account balance\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("\nCurrent Balance: $%.2f\n", balance);
    
    // Get deposit amount
    float amount;
    printf("\nEnter amount to deposit: $");
    if (scanf("%f", &amount) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("\nInvalid amount\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    // Validate amount
    if (amount <= 0) {
        printf("\nDeposit Failed: Invalid amount\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Perform deposit
    TransactionResult result = process_deposit(cardNumber, amount);
    
    if (result.status == TRANSACTION_SUCCESS) {
        printf("\nDeposit Successful\n");
        printf("Amount Deposited: $%.2f\n", result.amount_processed);
        printf("Previous Balance: $%.2f\n", result.balance_before);
        printf("New Balance: $%.2f\n", result.balance_after);
        
        // Print receipt
        printf("\n======= DEPOSIT RECEIPT =======\n");
        printf("Card Number: %d\n", cardNumber);
        printf("Amount: $%.2f\n", amount);
        printf("Balance: $%.2f\n", result.balance_after);
        printf("Date: %s\n", __DATE__);
        printf("Time: %s\n", __TIME__);
        printf("Thank you for using Core Banking System\n");
        printf("=================================\n");
    } else {
        printf("\nDeposit Failed\n");
        printf("Error: %s\n", result.message);
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Perform fund transfer
 * @param cardNumber Source card number
 */
void performTransfer(int cardNumber) {
    clearScreen();
    printHeader("FUND TRANSFER");
    
    // Check if card is active
    if (!cbs_is_card_active(cardNumber)) {
        printf("\nTransfer Failed: Card is inactive or blocked\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Get current balance
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", cardNumber);
    
    double balance = 0.0;
    if (!cbs_get_balance_by_card(card_number_str, &balance)) {
        printf("\nTransfer Failed: Could not retrieve account balance\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("\nCurrent Balance: $%.2f\n", balance);
    
    // Get target card
    int targetCard;
    printf("\nEnter destination card number: ");
    if (scanf("%d", &targetCard) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("\nInvalid card number\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Validate target card
    if (!cbs_card_exists(targetCard)) {
        printf("\nTransfer Failed: Destination card does not exist\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    if (!cbs_is_card_active(targetCard)) {
        printf("\nTransfer Failed: Destination card is inactive or blocked\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    if (cardNumber == targetCard) {
        printf("\nTransfer Failed: Cannot transfer to the same card\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Get transfer amount
    float amount;
    printf("\nEnter amount to transfer: $");
    if (scanf("%f", &amount) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("\nInvalid amount\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    // Validate amount
    if (amount <= 0) {
        printf("\nTransfer Failed: Invalid amount\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    if (amount > balance) {
        printf("\nTransfer Failed: Insufficient funds\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Perform transfer
    TransactionResult result = process_transfer(cardNumber, targetCard, amount);
    
    if (result.status == TRANSACTION_SUCCESS) {
        printf("\nTransfer Successful\n");
        printf("Amount Transferred: $%.2f\n", result.amount_processed);
        printf("Previous Balance: $%.2f\n", result.balance_before);
        printf("New Balance: $%.2f\n", result.balance_after);
        
        // Print receipt
        printf("\n======= TRANSFER RECEIPT =======\n");
        printf("From Card: %d\n", cardNumber);
        printf("To Card: %d\n", targetCard);
        printf("Amount: $%.2f\n", amount);
        printf("Balance: $%.2f\n", result.balance_after);
        printf("Date: %s\n", __DATE__);
        printf("Time: %s\n", __TIME__);
        printf("Thank you for using Core Banking System\n");
        printf("=================================\n");
    } else {
        printf("\nTransfer Failed\n");
        printf("Error: %s\n", result.message);
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Perform PIN change
 * @param cardNumber Card number
 */
void performPinChange(int cardNumber) {
    clearScreen();
    printHeader("PIN CHANGE");
    
    // Check if card is active
    if (!cbs_is_card_active(cardNumber)) {
        printf("\nPIN Change Failed: Card is inactive or blocked\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Get current PIN
    int oldPin;
    printf("\nEnter your current PIN: ");
    if (scanf("%d", &oldPin) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("\nInvalid PIN\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Validate current PIN format
    if (oldPin < 1000 || oldPin > 9999) {
        printf("\nPIN Change Failed: PIN must be a 4-digit number\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Validate PIN
    if (!cbs_validate_card(cardNumber, oldPin)) {
        printf("\nPIN Change Failed: Incorrect current PIN\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Get new PIN
    int newPin;
    printf("\nEnter new PIN (4 digits): ");
    if (scanf("%d", &newPin) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("\nInvalid PIN\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    // Validate new PIN format
    if (newPin < 1000 || newPin > 9999) {
        printf("\nPIN Change Failed: PIN must be a 4-digit number\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Perform PIN change
    TransactionResult result = process_pin_change(cardNumber, oldPin, newPin);
    
    if (result.status == TRANSACTION_SUCCESS) {
        printf("\nPIN Change Successful\n");
        printf("Your PIN has been updated\n");
    } else {
        printf("\nPIN Change Failed\n");
        printf("Error: %s\n", result.message);
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief View mini statement
 * @param cardNumber Card number
 */
void viewMiniStatement(int cardNumber) {
    clearScreen();
    printHeader("MINI STATEMENT");
    
    // Check if card is active
    if (!cbs_is_card_active(cardNumber)) {
        printf("\nMini Statement Failed: Card is inactive or blocked\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Process mini statement request
    TransactionResult result = process_mini_statement(cardNumber);
    
    if (result.status == TRANSACTION_SUCCESS) {
        printf("\nCurrent Balance: $%.2f\n\n", result.balance_after);
        
        // Get recent transactions
        int transactionCount = 10; // Show last 10 transactions
        QueryResult transactions = get_recent_transactions(cardNumber, transactionCount);
        
        if (transactions.success && transactions.count > 0) {
            printf("%-20s %-15s %-10s %-15s %-10s\n", "Date", "Type", "Amount", "Balance", "Status");
            printf("-------------------------------------------------------------------\n");
            
            for (int i = 0; i < transactions.count; i++) {
                printf("%-20s %-15s $%-9.2f $%-14.2f %s\n", 
                       transactions.data[i].timestamp, 
                       transactions.data[i].transaction_type, 
                       transactions.data[i].amount, 
                       transactions.data[i].balance, 
                       transactions.data[i].status ? "Success" : "Failed");
            }
            
            // Free the transaction data
            if (transactions.data) {
                free(transactions.data);
            }
        } else {
            printf("No recent transactions found\n");
        }
    } else {
        printf("\nMini Statement Failed\n");
        printf("Error: %s\n", result.message);
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Perform bill payment
 * @param cardNumber Card number
 */
void performBillPayment(int cardNumber) {
    clearScreen();
    printHeader("BILL PAYMENT");
    
    // Check if card is active
    if (!cbs_is_card_active(cardNumber)) {
        printf("\nBill Payment Failed: Card is inactive or blocked\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Get current balance
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", cardNumber);
    
    double balance = 0.0;
    if (!cbs_get_balance_by_card(card_number_str, &balance)) {
        printf("\nBill Payment Failed: Could not retrieve account balance\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("\nCurrent Balance: $%.2f\n", balance);
    
    // Display bill payment options
    printf("\nBill Payment Options:\n");
    printf("1. Electricity Bill\n");
    printf("2. Water Bill\n");
    printf("3. Phone Bill\n");
    printf("4. Internet Bill\n");
    printf("5. Back to Transaction Menu\n");
    
    // Get bill type
    int billType;
    printf("\nSelect bill type (1-5): ");
    if (scanf("%d", &billType) != 1 || billType < 1 || billType > 5) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("\nInvalid selection\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    if (billType == 5) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return;
    }
    
    // Get bill reference number
    char billRef[20];
    printf("\nEnter bill reference number: ");
    scanf("%s", billRef);
    
    // Get payment amount
    float amount;
    printf("\nEnter amount to pay: $");
    if (scanf("%f", &amount) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("\nInvalid amount\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    // Validate amount
    if (amount <= 0) {
        printf("\nBill Payment Failed: Invalid amount\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    if (amount > balance) {
        printf("\nBill Payment Failed: Insufficient funds\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
      // Get bill type
    char *billTypes[] = {"", "ELECTRICITY", "WATER", "PHONE", "INTERNET"};
    
    // Process the bill payment via the transaction processor
    TransactionResult result = process_bill_payment(cardNumber, billTypes[billType], billRef, amount);
    
    if (result.status != TRANSACTION_SUCCESS) {
        printf("\nBill Payment Failed: %s\n", result.message);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
      // Success
    printf("\nBill Payment Successful\n");
    printf("Bill Type: %s\n", billTypes[billType]);
    printf("Reference Number: %s\n", billRef);
    printf("Amount Paid: $%.2f\n", amount);
    printf("Previous Balance: $%.2f\n", result.balance_before);
    printf("New Balance: $%.2f\n", result.balance_after);
    
    // Print receipt
    printf("\n======= BILL PAYMENT RECEIPT =======\n");
    printf("Card Number: %d\n", cardNumber);
    printf("Bill Type: %s\n", billTypes[billType]);
    printf("Reference: %s\n", billRef);
    printf("Amount: $%.2f\n", amount);
    printf("Balance: $%.2f\n", result.balance_after);
    printf("Date: %s\n", __DATE__);
    printf("Time: %s\n", __TIME__);
    printf("Thank you for using Core Banking System\n");
    printf("====================================\n");
    
    printf("\nPress Enter to continue...");
    getchar();
}
