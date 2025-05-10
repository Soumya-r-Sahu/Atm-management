/**
 * @file atm_operations_menu.c
 * @brief Implementation of ATM operations menu for the ATM Management System CLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

/* Include proper header files from include directory */
#include "../../include/common/utils/logger.h"
#include "../../include/common/database/database.h"
#include "../../include/common/database/card_account_management.h"
#include "../../include/atm/transaction/transaction_processor.h"
#include "../../include/atm/transaction/transaction_types.h"
#include "../../include/frontend/cli/menu_utils.h"
#include "../../include/frontend/menus/menu_system.h"
#include "../../include/frontend/menus/atm_operations_menu.h"

/**
 * @brief Display the ATM operations menu
 * @param cardNumber Card number
 */
static void displayATMOperationsMenu(int cardNumber) {
    char title[100];
    sprintf(title, "ATM OPERATIONS - Card %d", cardNumber);
    printHeader(title);
    
    printf("\n");
    printf("1. Check Balance\n");
    printf("2. Withdraw Cash\n");
    printf("3. Deposit Cash\n");
    printf("4. Change PIN\n");
    printf("5. Mini Statement\n");
    printf("6. Return to Main Menu\n");
}

/**
 * @brief Run the ATM operations menu
 * @param cardNumber Card number
 */
void runATMOperationsMenu(int cardNumber) {
    bool running = true;
    
    while (running) {
        clearScreen();
        displayATMOperationsMenu(cardNumber);
        
        int choice;
        printf("\nEnter your choice (1-6): ");
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
        
        switch (choice) {
            case 1: // Check Balance
                performBalanceInquiry(cardNumber);
                break;
                
            case 2: // Withdraw Cash
                performWithdrawal(cardNumber);
                break;
                
            case 3: // Deposit Cash
                performDeposit(cardNumber);
                break;
                
            case 4: // Change PIN
                performPinChange(cardNumber);
                break;
                
            case 5: // Mini Statement
                viewMiniStatement(cardNumber);
                break;
                
            case 6: // Return to Main Menu
                running = false;
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}

/**
 * @brief Perform balance inquiry
 * @param cardNumber Card number
 */
void performBalanceInquiry(int cardNumber) {
    clearScreen();
    printHeader("BALANCE INQUIRY");
    
    TransactionResult result = process_balance_inquiry(cardNumber);
    
    if (result.status == TRANSACTION_SUCCESS) {
        char cardHolderName[100];
        if (!getCardHolderName(cardNumber, cardHolderName, sizeof(cardHolderName))) {
            strcpy(cardHolderName, "Unknown");
        }
        
        printf("\nCard Number: %d\n", cardNumber);
        printf("Card Holder: %s\n", cardHolderName);
        printf("Available Balance: ");
        printCurrency(result.balance_after, "₹");
        printf("\n");
        
        LOG_INFO("Balance inquiry performed for card %d", cardNumber);
    } else {
        printf("\nError: %s\n", result.message);
        LOG_ERROR("Balance inquiry failed for card %d: %s", cardNumber, result.message);
    }
    
    pauseExecution();
}

/**
 * @brief Perform withdrawal
 * @param cardNumber Card number
 */
void performWithdrawal(int cardNumber) {
    clearScreen();
    printHeader("CASH WITHDRAWAL");
    
    // First retrieve the current balance to display
    TransactionResult balanceInfo = process_balance_inquiry(cardNumber);
    
    if (balanceInfo.status != TRANSACTION_SUCCESS) {
        printf("\nError: %s\n", balanceInfo.message);
        LOG_ERROR("Unable to fetch balance for card %d: %s", cardNumber, balanceInfo.message);
        pauseExecution();
        return;
    }
    
    float balance = (float)balanceInfo.balance_after;
    
    char cardHolderName[100];
    if (!getCardHolderName(cardNumber, cardHolderName, sizeof(cardHolderName))) {
        strcpy(cardHolderName, "Unknown");
    }
    
    printf("\nCard Number: %d\n", cardNumber);
    printf("Card Holder: %s\n", cardHolderName);
    printf("Available Balance: ");
    printCurrency(balance, "₹");
    printf("\n\n");
    
    printf("Withdrawal Amount Options:\n");
    printf("1. ₹1,000\n");
    printf("2. ₹2,000\n");
    printf("3. ₹5,000\n");
    printf("4. ₹10,000\n");
    printf("5. Other Amount\n");
    
    int choice = getInteger("Select Option", 1, 5);
    
    float amount;
    
    switch (choice) {
        case 1:
            amount = 1000.0;
            break;
        case 2:
            amount = 2000.0;
            break;
        case 3:
            amount = 5000.0;
            break;
        case 4:
            amount = 10000.0;
            break;
        case 5:
            amount = getDouble("Enter Amount", 100.0, balance);
            break;
    }
    
    printf("\nWithdrawal Details:\n");
    printf("Amount: ");
    printCurrency(amount, "₹");
    printf("\n");
    
    if (amount > balance) {
        printError("Insufficient balance!");
        pauseExecution();
        return;
    }
      // Check daily withdrawal limits
    float dailyWithdrawals = getDailyWithdrawals(cardNumber);
    float dailyLimit = 50000.0; // Default daily limit
    
    // In a production system, this would come from a configuration:
    // getConfigFloat("DAILY_WITHDRAWAL_LIMIT", &dailyLimit);
    
    if (dailyWithdrawals + amount > dailyLimit) {
        printError("Daily withdrawal limit exceeded!");
        printf("Remaining limit today: ");
        printCurrency(dailyLimit - dailyWithdrawals, "₹");
        printf("\n");
        pauseExecution();
        return;
    }
    
    if (getConfirmation("\nConfirm Withdrawal")) {
        // Process the withdrawal
        TransactionResult result = process_withdrawal(cardNumber, amount);
        
        if (result.status == TRANSACTION_SUCCESS) {
            printSuccess("Please collect your cash!");
            printf("New Balance: ");
            printCurrency(result.balance_after, "₹");
            printf("\n");
            printf("Transaction Reference: %s\n", result.message);
            
            LOG_INFO("Withdrawal completed: Card %d withdrew %.2f", cardNumber, amount);
        } else {
            printError("Withdrawal failed!");
            printf("Reason: %s\n", result.message);
            LOG_ERROR("Withdrawal failed for card %d: %s", cardNumber, result.message);
        }
    } else {
        printInfo("Withdrawal cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief Perform deposit
 * @param cardNumber Card number
 */
void performDeposit(int cardNumber) {
    clearScreen();
    printHeader("CASH DEPOSIT");
    
    // First retrieve the current balance to display
    TransactionResult balanceInfo = process_balance_inquiry(cardNumber);
    
    if (balanceInfo.status != TRANSACTION_SUCCESS) {
        printf("\nError: %s\n", balanceInfo.message);
        LOG_ERROR("Unable to fetch balance for card %d: %s", cardNumber, balanceInfo.message);
        pauseExecution();
        return;
    }
    
    float balance = (float)balanceInfo.balance_after;
    
    char cardHolderName[100];
    if (!getCardHolderName(cardNumber, cardHolderName, sizeof(cardHolderName))) {
        strcpy(cardHolderName, "Unknown");
    }
    
    printf("\nCard Number: %d\n", cardNumber);
    printf("Card Holder: %s\n", cardHolderName);
    printf("Current Balance: ");
    printCurrency(balance, "₹");
    printf("\n\n");
    
    float amount = getDouble("Enter Deposit Amount", 100.0, 100000.0);
    
    printf("\nDeposit Details:\n");
    printf("Amount: ");
    printCurrency(amount, "₹");
    printf("\n");
    
    if (getConfirmation("\nConfirm Deposit")) {
        // Process the deposit
        TransactionResult result = process_deposit(cardNumber, amount);
        
        if (result.status == TRANSACTION_SUCCESS) {
            printSuccess("Deposit completed successfully!");
            printf("New Balance: ");
            printCurrency(result.balance_after, "₹");
            printf("\n");
            printf("Transaction Reference: %s\n", result.message);
            
            LOG_INFO("Deposit completed: Card %d deposited %.2f", cardNumber, amount);
        } else {
            printError("Deposit failed!");
            printf("Reason: %s\n", result.message);
            LOG_ERROR("Deposit failed for card %d: %s", cardNumber, result.message);
        }
    } else {
        printInfo("Deposit cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief Perform PIN change
 * @param cardNumber Card number
 */
void performPinChange(int cardNumber) {
    clearScreen();
    printHeader("PIN CHANGE");
    
    printf("\nCard Number: %d\n\n", cardNumber);
    
    char currentPin[7];
    char newPin[7];
    char confirmPin[7];
    
    printf("Enter Current PIN: ");
    getPassword(currentPin, sizeof(currentPin));
    
    printf("Enter New PIN: ");
    getPassword(newPin, sizeof(newPin));
    
    printf("Confirm New PIN: ");
    getPassword(confirmPin, sizeof(confirmPin));
    
    if (strcmp(newPin, confirmPin) != 0) {
        printError("PINs do not match!");
        pauseExecution();
        return;
    }
    
    if (strlen(newPin) != 4) {
        printError("PIN must be 4 digits!");
        pauseExecution();
        return;
    }
    
    // Convert string PINs to integers for processing
    int currentPinNum = atoi(currentPin);
    int newPinNum = atoi(newPin);
    
    if (getConfirmation("\nConfirm PIN Change")) {
        // Process the PIN change through the transaction processor
        TransactionResult result = process_pin_change(cardNumber, currentPinNum, newPinNum);
        
        if (result.status == TRANSACTION_SUCCESS) {
            printSuccess("PIN changed successfully!");
            LOG_INFO("PIN changed for card %d", cardNumber);
        } else {
            printError("PIN change failed!");
            printf("Reason: %s\n", result.message);
            LOG_ERROR("PIN change failed for card %d: %s", cardNumber, result.message);
        }
    } else {
        printInfo("PIN change cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief View mini statement
 * @param cardNumber Card number
 */
void viewMiniStatement(int cardNumber) {
    clearScreen();
    printHeader("MINI STATEMENT");
    
    // Get cardholder name
    char cardHolderName[100];
    if (!getCardHolderName(cardNumber, cardHolderName, sizeof(cardHolderName))) {
        strcpy(cardHolderName, "Unknown");
    }
    
    // Get current balance
    TransactionResult balanceInfo = process_balance_inquiry(cardNumber);
    if (balanceInfo.status != TRANSACTION_SUCCESS) {
        printError("Failed to retrieve account information!");
        LOG_ERROR("Failed to retrieve balance for mini statement, card %d: %s", 
                 cardNumber, balanceInfo.message);
        pauseExecution();
        return;
    }
    
    // Get recent transactions
    QueryResult transactions = get_recent_transactions(cardNumber, 5); // Get last 5 transactions
    
    if (!transactions.success) {
        printError("Failed to retrieve transaction history!");
        LOG_ERROR("Failed to retrieve transaction history for card %d", cardNumber);
        pauseExecution();
        return;
    }
    
    printf("\nCard Number: %d\n", cardNumber);
    printf("Card Holder: %s\n\n", cardHolderName);
    
    printLine('-', SCREEN_WIDTH);
    printf("%-20s %-30s %-15s\n", "Date", "Description", "Amount");
    printLine('-', SCREEN_WIDTH);
    
    // Display the transactions
    for (int i = 0; i < transactions.count; i++) {
        Transaction tx = transactions.data[i];
        
        // Extract date from timestamp (assuming timestamp format YYYY-MM-DD HH:MM:SS)
        char date[11]; // YYYY-MM-DD + null terminator
        strncpy(date, tx.timestamp, 10);
        date[10] = '\0';
        
        printf("%-20s %-30s ", date, tx.transaction_type);
        printCurrency(tx.amount, "₹");
        printf("\n");
    }
    
    // If no transactions found
    if (transactions.count == 0) {
        printf("No recent transactions found.\n");
    }
    
    printLine('-', SCREEN_WIDTH);
    
    printf("\nAvailable Balance: ");
    printCurrency(balanceInfo.balance_after, "₹");
    printf("\n");
    
    LOG_INFO("Mini statement viewed for card %d", cardNumber);
    
    // Free the transaction data if needed
    if (transactions.data != NULL) {
        free(transactions.data);
    }
    
    pauseExecution();
}
