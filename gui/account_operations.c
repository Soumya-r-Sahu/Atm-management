/**
 * @file account_operations.c
 * @brief Implementation of account operations for the ATM Management System CLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include "../../include/common/utils/logger.h"
#include "../../include/common/database/database.h"
#include "../../include/frontend/cli/menu_utils.h"
#include "../../include/frontend/menus/menu_system.h"
#include "../../include/frontend/menus/account_menu.h"

/**
 * @brief View account details
 * @param username Customer username
 */
void viewAccountDetails(const char *username) {
    clearScreen();
    printHeader("ACCOUNT DETAILS");
    
    // In a real application, this would fetch data from the database
    // For demo purposes, we'll use mock data
    
    printf("\nCustomer Name: John Doe\n");
    printf("Customer ID: CUST123456\n");
    printf("Email: john.doe@example.com\n");
    printf("Phone: +91-9876543210\n");
    printf("\n");
    
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
    
    printf("\nLast Login: ");
    printDateTime("2023-05-03 10:15:30");
    printf("\n");
    
    pauseExecution();
}

/**
 * @brief View transaction history
 * @param username Customer username
 */
void viewTransactionHistory(const char *username) {
    clearScreen();
    printHeader("TRANSACTION HISTORY");
    
    // In a real application, this would fetch data from the database
    // For demo purposes, we'll use mock data
    
    printf("\nAccount: 1234567890 (Savings)\n\n");
    
    printLine('-', SCREEN_WIDTH);
    printf("%-20s %-30s %-15s %-15s\n", "Date", "Description", "Amount", "Balance");
    printLine('-', SCREEN_WIDTH);
    
    printf("%-20s %-30s ", "2023-05-01", "Cash Deposit");
    printCurrency(10000.00, "₹");
    printf("%15s ", "");
    printCurrency(25000.75, "₹");
    printf("\n");
    
    printf("%-20s %-30s ", "2023-04-25", "ATM Withdrawal");
    printCurrency(-5000.00, "₹");
    printf("%14s ", "");
    printCurrency(15000.75, "₹");
    printf("\n");
    
    printf("%-20s %-30s ", "2023-04-20", "Water Bill Payment");
    printCurrency(-1500.00, "₹");
    printf("%14s ", "");
    printCurrency(20000.75, "₹");
    printf("\n");
    
    printf("%-20s %-30s ", "2023-04-15", "Fund Transfer from Amit Kumar");
    printCurrency(8000.00, "₹");
    printf("%15s ", "");
    printCurrency(21500.75, "₹");
    printf("\n");
    
    printf("%-20s %-30s ", "2023-04-10", "Interest Credit");
    printCurrency(500.75, "₹");
    printf("%15s ", "");
    printCurrency(13500.75, "₹");
    printf("\n");
    
    printLine('-', SCREEN_WIDTH);
    
    pauseExecution();
}

/**
 * @brief Change PIN
 * @param username Customer username
 */
void changePin(const char *username) {
    clearScreen();
    printHeader("CHANGE PIN");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    printf("\nSelect Card:\n");
    printf("1. Debit Card (xxxx xxxx xxxx 1234)\n");
    printf("2. Credit Card (xxxx xxxx xxxx 5678)\n");
    
    int cardChoice = getInteger("Select Card", 1, 2);
    
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
    
    if (strlen(newPin) != 4 || !isdigit(newPin[0]) || !isdigit(newPin[1]) || 
        !isdigit(newPin[2]) || !isdigit(newPin[3])) {
        printError("PIN must be 4 digits!");
        pauseExecution();
        return;
    }
    
    if (strcmp(currentPin, "1234") != 0) {
        printError("Incorrect current PIN!");
        pauseExecution();
        return;
    }
    
    if (getConfirmation("\nConfirm PIN Change")) {
        // In a real application, this would update the database
        printSuccess("PIN changed successfully!");
        
        LOG_INFO("PIN changed for %s", username);
    } else {
        printInfo("PIN change cancelled.");
    }
    
    pauseExecution();
}
