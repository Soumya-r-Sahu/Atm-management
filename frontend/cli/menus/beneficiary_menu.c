/**
 * @file beneficiary_menu.c
 * @brief Implementation of beneficiary menu for the Core Banking System
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
#include "../../../include/frontend/menus/beneficiary_menu.h"

/**
 * @brief Display the beneficiary menu
 * @param username Customer username
 */
static void displayBeneficiaryMenu(const char *username) {
    char title[100];
    sprintf(title, "BENEFICIARY MANAGEMENT - %s", username);
    printHeader(title);
    
    printf("\n");
    printf("1. View Beneficiaries\n");
    printf("2. Add Beneficiary\n");
    printf("3. Delete Beneficiary\n");
    printf("4. Return to Main Menu\n");
}

/**
 * @brief Run the beneficiary menu
 * @param username Customer username
 */
void runBeneficiaryMenu(const char *username) {
    bool running = true;
    
    while (running) {
        clearScreen();
        displayBeneficiaryMenu(username);
        
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
            case 1: // View Beneficiaries
                viewBeneficiaries(username);
                break;
                
            case 2: // Add Beneficiary
                addBeneficiary(username);
                break;
                
            case 3: // Delete Beneficiary
                deleteBeneficiary(username);
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

/**
 * @brief Delete beneficiary
 * @param username Customer username
 */
void deleteBeneficiary(const char *username) {
    clearScreen();
    printHeader("DELETE BENEFICIARY");
    
    // First show the list of beneficiaries
    printLine('-', SCREEN_WIDTH);
    printf("%-5s %-20s %-20s %-15s %-15s\n", "ID", "Name", "Account Number", "Bank", "IFSC Code");
    printLine('-', SCREEN_WIDTH);
    
    printf("%-5d %-20s %-20s %-15s %-15s\n", 1, "Rahul Sharma", "1122334455", "SBI", "SBIN0001234");
    printf("%-5d %-20s %-20s %-15s %-15s\n", 2, "Priya Patel", "5544332211", "HDFC", "HDFC0001234");
    printf("%-5d %-20s %-20s %-15s %-15s\n", 3, "Amit Kumar", "9988776655", "ICICI", "ICIC0001234");
    
    printLine('-', SCREEN_WIDTH);
    
    int beneficiaryId = getInteger("\nEnter Beneficiary ID to Delete", 1, 3);
    
    // Get the name based on the ID (in a real app, would fetch from database)
    const char *beneficiaryName = "";
    switch (beneficiaryId) {
        case 1: beneficiaryName = "Rahul Sharma"; break;
        case 2: beneficiaryName = "Priya Patel"; break;
        case 3: beneficiaryName = "Amit Kumar"; break;
    }
    
    printf("\nYou are about to delete beneficiary: %s\n", beneficiaryName);
    
    if (getConfirmation("\nConfirm Delete Beneficiary")) {
        // In a real application, this would update the database
        printSuccess("Beneficiary deleted successfully!");
        
        LOG_INFO("Beneficiary deleted by %s: %s (ID: %d)", username, beneficiaryName, beneficiaryId);
    } else {
        printInfo("Delete beneficiary cancelled.");
    }
    
    pauseExecution();
}
