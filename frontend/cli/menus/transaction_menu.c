/**
 * @file transaction_menu.c
 * @brief Implementation of transaction menu for the Core Banking System
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "../../../include/common/utils/logger.h"
#include "../../../include/common/database/database.h"
#include "../../../include/frontend/cli/menu_utils.h"
#include "../../../include/frontend/menus/menu_system.h"
#include "../../../include/frontend/menus/transaction_menu.h"

/**
 * @brief Display the transaction menu
 * @param username Customer username
 */
static void displayTransactionMenu(const char *username) {
    char title[100];
    sprintf(title, "TRANSACTION MANAGEMENT - %s", username);
    printHeader(title);
    
    printf("\n");
    printf("1. Transfer Funds\n");
    printf("2. Check Balance\n");
    printf("3. View Transaction History\n");
    printf("4. Return to Main Menu\n");
}

/**
 * @brief Run the transaction menu
 * @param username Customer username
 */
void runTransactionMenu(const char *username) {
    bool running = true;
    
    while (running) {
        clearScreen();
        displayTransactionMenu(username);
        
        int choice;
        printf("\nEnter your choice (1-4): ");
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
            case 1: // Transfer Funds
                transferFunds(username);
                break;
                
            case 2: // Check Balance
                checkBalanceEnquiry(username);
                break;
                
            case 3: // View Transaction History
                viewTransactionHistory(username);
                break;
                
            case 4: // Return to Main Menu
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
 * @brief Transfer funds
 * @param username Customer username
 */
void transferFunds(const char *username) {
    clearScreen();
    printHeader("TRANSFER FUNDS");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    printf("\nFrom Account: 1234567890 (Savings)\n");
    printf("Available Balance: ");
    printCurrency(25000.75, "₹");
    printf("\n\n");
    
    printf("Transfer Type:\n");
    printf("1. Own Account Transfer\n");
    printf("2. Beneficiary Transfer\n");
    printf("3. New Beneficiary\n");
    
    int transferType = getInteger("Select Transfer Type", 1, 3);
    
    char toAccount[20];
    char beneficiaryName[50];
    char bankName[50];
    char ifscCode[20];
    
    if (transferType == 1) {
        strcpy(toAccount, "0987654321");
        strcpy(beneficiaryName, "John Doe");
        strcpy(bankName, "Same Bank");
        strcpy(ifscCode, "SBIN0001234");
    } else if (transferType == 2) {
        strcpy(toAccount, "1122334455");
        strcpy(beneficiaryName, "Rahul Sharma");
        strcpy(bankName, "State Bank of India");
        strcpy(ifscCode, "SBIN0001234");
    } else {
        getString("Enter Beneficiary Name: ", beneficiaryName, sizeof(beneficiaryName));
        getString("Enter Account Number: ", toAccount, sizeof(toAccount));
        getString("Enter Bank Name: ", bankName, sizeof(bankName));
        getString("Enter IFSC Code: ", ifscCode, sizeof(ifscCode));
    }
    
    double amount = getDouble("Enter Amount", 1.0, 25000.75);
    
    char description[100];
    getString("Enter Description (optional): ", description, sizeof(description));
    if (strlen(description) == 0) {
        strcpy(description, "Fund Transfer");
    }
    
    printf("\nTransfer Details:\n");
    printf("From Account: 1234567890 (Savings)\n");
    printf("To Account: %s\n", toAccount);
    printf("Beneficiary: %s\n", beneficiaryName);
    printf("Bank: %s\n", bankName);
    printf("IFSC Code: %s\n", ifscCode);
    printf("Amount: ");
    printCurrency(amount, "₹");
    printf("\n");
    printf("Description: %s\n", description);
    
    if (getConfirmation("\nConfirm Transfer")) {
        // In a real application, this would update the database
        printSuccess("Transfer completed successfully!");
        printf("Transaction Reference: TXN%d\n", rand() % 1000000000);
        
        LOG_INFO("Fund transfer completed: %s transferred %.2f to %s (%s)", 
                 username, amount, beneficiaryName, toAccount);
    } else {
        printInfo("Transfer cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief Check balance enquiry
 * @param username Customer username
 */
void checkBalanceEnquiry(const char *username) {
    clearScreen();
    printHeader("BALANCE ENQUIRY");
    
    // In a real application, this would fetch data from the database
    // For demo purposes, we'll use mock data
    
    printLine('-', SCREEN_WIDTH);
    printf("%-20s %-20s %-15s %-15s\n", "Account Number", "Account Type", "Balance", "Status");
    printLine('-', SCREEN_WIDTH);
    
    printf("%-20s %-20s ", "1234567890", "Savings");
    printCurrency(25000.75, "₹");
    printf("%15s\n", "Active");
    
    printf("%-20s %-20s ", "0987654321", "Current");
    printCurrency(150000.50, "₹");
    printf("%15s\n", "Active");
    
    printLine('-', SCREEN_WIDTH);
    
    printf("\nLast Updated: ");
    printDateTime("2023-05-10 14:30:45");
    printf("\n");
    
    pauseExecution();
}
