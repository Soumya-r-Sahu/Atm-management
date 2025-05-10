/**
 * @file transaction_operations_menu.c
 * @brief Implementation of transaction operations menu for the Core Banking System CLI
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

/**
 * @brief Display the transaction operations menu
 * @param username Customer username
 */
static void displayTransactionOperationsMenu(const char *username) {
    char title[100];
    sprintf(title, "TRANSACTION MANAGEMENT - %s", username);
    printHeader(title);
    
    printf("\n");
    printf("1. Transfer Funds\n");
    printf("2. Pay Bill\n");
    printf("3. Return to Main Menu\n");
}

/**
 * @brief Run the transaction operations menu
 * @param username Customer username
 */
void runTransactionMenu(const char *username) {
    bool running = true;
    
    while (running) {
        clearScreen();
        displayTransactionOperationsMenu(username);
        
        int choice;
        printf("\nEnter your choice (1-3): ");
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
                
            case 2: // Pay Bill
                payBill(username);
                break;
                
            case 3: // Return to Main Menu
                running = false;
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}
