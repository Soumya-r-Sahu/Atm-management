/**
 * @file transaction_operations.c
 * @brief Implementation of transaction operations for the ATM Management System CLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "../../include/common/utils/logger.h"
#include "../../include/common/database/database.h"
#include "../../include/frontend/cli/menu_utils.h"
#include "../../include/frontend/menus/menu_system.h"
#include "../../include/frontend/menus/transaction_menu.h"

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
