/**
 * @file customer_menu.c
 * @brief Implementation of customer menu for the ATM Management System CLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <common/utils/logger.h>
#include <common/database/database.h>
#include "menu_utils.h"
#include "customer_menu.h"

// Function prototypes
void displayCustomerMenu(const char *username, int userType);
void handleCustomerMenuChoice(int choice, const char *username, int userType);
void viewAccountDetails(const char *username);
void viewTransactionHistory(const char *username);
void transferFunds(const char *username);
void payBill(const char *username);
void changePin(const char *username);
void viewBeneficiaries(const char *username);
void addBeneficiary(const char *username);

/**
 * @brief Run the customer menu
 * @param username Customer username
 * @param userType User type (1 = Regular Customer, 3 = Premium Customer)
 */
void runCustomerMenu(const char *username, int userType) {
    bool running = true;
    
    while (running) {
        clearScreen();
        displayCustomerMenu(username, userType);
        
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
            handleCustomerMenuChoice(choice, username, userType);
        }
    }
}

/**
 * @brief Display the customer menu
 * @param username Customer username
 * @param userType User type (1 = Regular Customer, 3 = Premium Customer)
 */
void displayCustomerMenu(const char *username, int userType) {
    char title[100];
    sprintf(title, "CUSTOMER MENU - %s", username);
    printHeader(title);
    
    printf("\n");
    printf("1. View Account Details\n");
    printf("2. View Transaction History\n");
    printf("3. Transfer Funds\n");
    printf("4. Pay Bill\n");
    printf("5. Change PIN\n");
    printf("6. View Beneficiaries\n");
    printf("7. Add Beneficiary\n");
    printf("8. Logout\n");
    
    if (userType == 3) { // Premium Customer
        printf("\n");
        printInfo("Premium Customer Features:");
        printf("- Higher transaction limits\n");
        printf("- Priority customer support\n");
        printf("- Reduced transaction fees\n");
    }
}

/**
 * @brief Handle customer menu choice
 * @param choice User's choice
 * @param username Customer username
 * @param userType User type (1 = Regular Customer, 3 = Premium Customer)
 */
void handleCustomerMenuChoice(int choice, const char *username, int userType) {
    switch (choice) {
        case 1: // View Account Details
            viewAccountDetails(username);
            break;
            
        case 2: // View Transaction History
            viewTransactionHistory(username);
            break;
            
        case 3: // Transfer Funds
            transferFunds(username);
            break;
            
        case 4: // Pay Bill
            payBill(username);
            break;
            
        case 5: // Change PIN
            changePin(username);
            break;
            
        case 6: // View Beneficiaries
            viewBeneficiaries(username);
            break;
            
        case 7: // Add Beneficiary
            addBeneficiary(username);
            break;
            
        default:
            printf("Invalid choice. Please try again.\n");
            sleep(2);
            break;
    }
}

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
 * @brief Pay bill
 * @param username Customer username
 */
void payBill(const char *username) {
    clearScreen();
    printHeader("BILL PAYMENT");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    printf("\nFrom Account: 1234567890 (Savings)\n");
    printf("Available Balance: ");
    printCurrency(25000.75, "₹");
    printf("\n\n");
    
    printf("Bill Type:\n");
    printf("1. Electricity\n");
    printf("2. Water\n");
    printf("3. Mobile\n");
    printf("4. DTH\n");
    printf("5. Gas\n");
    
    int billType = getInteger("Select Bill Type", 1, 5);
    
    char billerName[50];
    
    switch (billType) {
        case 1:
            strcpy(billerName, "Tata Power");
            break;
        case 2:
            strcpy(billerName, "Delhi Jal Board");
            break;
        case 3:
            strcpy(billerName, "Airtel");
            break;
        case 4:
            strcpy(billerName, "Tata Sky");
            break;
        case 5:
            strcpy(billerName, "Indraprastha Gas Limited");
            break;
    }
    
    char consumerNumber[20];
    getString("Enter Consumer Number: ", consumerNumber, sizeof(consumerNumber));
    
    char billNumber[20];
    getString("Enter Bill Number (optional): ", billNumber, sizeof(billNumber));
    
    double amount = getDouble("Enter Amount", 1.0, 25000.75);
    
    printf("\nBill Payment Details:\n");
    printf("From Account: 1234567890 (Savings)\n");
    printf("Biller: %s\n", billerName);
    printf("Consumer Number: %s\n", consumerNumber);
    if (strlen(billNumber) > 0) {
        printf("Bill Number: %s\n", billNumber);
    }
    printf("Amount: ");
    printCurrency(amount, "₹");
    printf("\n");
    
    if (getConfirmation("\nConfirm Payment")) {
        // In a real application, this would update the database
        printSuccess("Bill payment completed successfully!");
        printf("Transaction Reference: TXN%d\n", rand() % 1000000000);
        
        LOG_INFO("Bill payment completed: %s paid %.2f to %s (Consumer: %s)", 
                 username, amount, billerName, consumerNumber);
    } else {
        printInfo("Payment cancelled.");
    }
    
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

/**
 * @brief View beneficiaries
 * @param username Customer username
 */
void viewBeneficiaries(const char *username) {
    clearScreen();
    printHeader("BENEFICIARIES");
    
    // In a real application, this would fetch data from the database
    // For demo purposes, we'll use mock data
    
    printLine('-', SCREEN_WIDTH);
    printf("%-5s %-20s %-20s %-15s %-15s\n", "ID", "Name", "Account Number", "Bank", "IFSC Code");
    printLine('-', SCREEN_WIDTH);
    
    printf("%-5d %-20s %-20s %-15s %-15s\n", 1, "Rahul Sharma", "1122334455", "SBI", "SBIN0001234");
    printf("%-5d %-20s %-20s %-15s %-15s\n", 2, "Priya Patel", "5544332211", "HDFC", "HDFC0001234");
    printf("%-5d %-20s %-20s %-15s %-15s\n", 3, "Amit Kumar", "9988776655", "ICICI", "ICIC0001234");
    
    printLine('-', SCREEN_WIDTH);
    
    pauseExecution();
}

/**
 * @brief Add beneficiary
 * @param username Customer username
 */
void addBeneficiary(const char *username) {
    clearScreen();
    printHeader("ADD BENEFICIARY");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    char name[50];
    char accountNumber[20];
    char confirmAccountNumber[20];
    char bankName[50];
    char ifscCode[20];
    
    getString("Enter Beneficiary Name: ", name, sizeof(name));
    getString("Enter Account Number: ", accountNumber, sizeof(accountNumber));
    getString("Confirm Account Number: ", confirmAccountNumber, sizeof(confirmAccountNumber));
    
    if (strcmp(accountNumber, confirmAccountNumber) != 0) {
        printError("Account numbers do not match!");
        pauseExecution();
        return;
    }
    
    getString("Enter Bank Name: ", bankName, sizeof(bankName));
    getString("Enter IFSC Code: ", ifscCode, sizeof(ifscCode));
    
    printf("\nBeneficiary Details:\n");
    printf("Name: %s\n", name);
    printf("Account Number: %s\n", accountNumber);
    printf("Bank: %s\n", bankName);
    printf("IFSC Code: %s\n", ifscCode);
    
    if (getConfirmation("\nConfirm Add Beneficiary")) {
        // In a real application, this would update the database
        printSuccess("Beneficiary added successfully!");
        
        LOG_INFO("Beneficiary added by %s: %s (%s)", username, name, accountNumber);
    } else {
        printInfo("Add beneficiary cancelled.");
    }
    
    pauseExecution();
}
