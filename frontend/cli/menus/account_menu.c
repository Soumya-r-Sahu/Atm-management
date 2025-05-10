/**
 * @file account_menu.c
 * @brief Implementation of account management menu for the Core Banking System
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include "../../../include/common/utils/logger.h"
#include "../../../include/common/database/database.h"
#include "../../../include/frontend/cli/menu_utils.h"
#include "../../../include/frontend/menus/menu_system.h"
#include "../../../include/frontend/menus/account_menu.h"

/**
 * @brief Display the account management menu
 * @param username Customer username
 */
static void displayAccountMenu(const char *username) {
    char title[100];
    sprintf(title, "ACCOUNT MANAGEMENT - %s", username);
    printHeader(title);
    
    printf("\n");
    printf("1. View Account Details\n");
    printf("2. View Transaction History\n");
    printf("3. Change PIN\n");
    printf("4. Request Statement\n");
    printf("5. Return to Main Menu\n");
}

/**
 * @brief Run the account management menu
 * @param username Customer username
 */
void runAccountMenu(const char *username) {
    bool running = true;
    
    while (running) {
        clearScreen();
        displayAccountMenu(username);
        
        int choice;
        printf("\nEnter your choice (1-5): ");
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
            case 1: // View Account Details
                viewAccountDetails(username);
                break;
                
            case 2: // View Transaction History
                viewTransactionHistory(username);
                break;
                
            case 3: // Change PIN
                changePin(username);
                break;
                
            case 4: // Request Statement
                requestStatement(username);
                break;
                
            case 5: // Return to Main Menu
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
 * @brief Request account statement
 * @param username Customer username
 */
void requestStatement(const char *username) {
    clearScreen();
    printHeader("REQUEST STATEMENT");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    printf("\nSelect Account:\n");
    printf("1. Savings (1234567890)\n");
    printf("2. Current (0987654321)\n");
    
    int accountChoice = getInteger("Select Account", 1, 2);
    
    printf("\nSelect Statement Period:\n");
    printf("1. Last Month\n");
    printf("2. Last Quarter\n");
    printf("3. Last Year\n");
    printf("4. Custom Period\n");
    
    int periodChoice = getInteger("Select Period", 1, 4);
    
    char startDate[20] = {0};
    char endDate[20] = {0};
    
    if (periodChoice == 4) {
        getString("Enter Start Date (YYYY-MM-DD): ", startDate, sizeof(startDate));
        getString("Enter End Date (YYYY-MM-DD): ", endDate, sizeof(endDate));
    }
    
    printf("\nDelivery Method:\n");
    printf("1. Email\n");
    printf("2. Download PDF\n");
    printf("3. Physical Mail (Charges Apply)\n");
    
    int deliveryChoice = getInteger("Select Delivery Method", 1, 3);
    
    printf("\nStatement Request Details:\n");
    printf("Account: %s\n", (accountChoice == 1) ? "Savings (1234567890)" : "Current (0987654321)");
    
    printf("Period: ");
    switch (periodChoice) {
        case 1:
            printf("Last Month\n");
            break;
        case 2:
            printf("Last Quarter\n");
            break;
        case 3:
            printf("Last Year\n");
            break;
        case 4:
            printf("Custom Period (%s to %s)\n", startDate, endDate);
            break;
    }
    
    printf("Delivery Method: ");
    switch (deliveryChoice) {
        case 1:
            printf("Email (john.doe@example.com)\n");
            break;
        case 2:
            printf("Download PDF\n");
            break;
        case 3:
            printf("Physical Mail (Charges: ₹50)\n");
            break;
    }
    
    if (getConfirmation("\nConfirm Statement Request")) {
        // In a real application, this would update the database
        printSuccess("Statement request submitted successfully!");
        printf("Reference Number: REQ%d\n", rand() % 1000000000);
        
        LOG_INFO("Statement requested by %s", username);
    } else {
        printInfo("Statement request cancelled.");
    }
    
    pauseExecution();
}
