/**
 * @file beneficiary_menu.c
 * @brief Implementation of beneficiary management menu for the ATM Management System CLI
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
 * @brief Display the beneficiary management menu
 * @param username Customer username
 */
static void displayBeneficiaryMenu(const char *username) {
    char title[100];
    sprintf(title, "BENEFICIARY MANAGEMENT - %s", username);
    printHeader(title);
    
    printf("\n");
    printf("1. View Beneficiaries\n");
    printf("2. Add Beneficiary\n");
    printf("3. Return to Main Menu\n");
}

/**
 * @brief Run the beneficiary management menu
 * @param username Customer username
 */
void runBeneficiaryMenu(const char *username) {
    bool running = true;
    
    while (running) {
        clearScreen();
        displayBeneficiaryMenu(username);
        
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
            case 1: // View Beneficiaries
                viewBeneficiaries(username);
                break;
                
            case 2: // Add Beneficiary
                addBeneficiary(username);
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
