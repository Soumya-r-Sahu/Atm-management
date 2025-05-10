/**
 * @file account_menu.c
 * @brief Implementation of account management menu for the ATM Management System CLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include "../../include/common/utils/logger.h"
#include "../../include/common/database/database.h"
#include "../../include/frontend/cli/menu_utils.h"
#include "../../include/frontend/menus/menu_system.h"
#include "../../include/frontend/menus/account_menu.h"

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
    printf("4. Return to Main Menu\n");
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
            case 1: // View Account Details
                viewAccountDetails(username);
                break;
                
            case 2: // View Transaction History
                viewTransactionHistory(username);
                break;
                
            case 3: // Change PIN
                changePin(username);
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
