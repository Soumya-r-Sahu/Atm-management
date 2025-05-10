/**
 * @file beneficiary_operations.c
 * @brief Implementation of beneficiary operations for the ATM Management System CLI
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
#include "../../include/frontend/menus/beneficiary_menu.h"

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
