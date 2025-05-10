/**
 * @file customer_menu.c
 * @brief Implementation of customer menu for the Core Banking System
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "../../../include/common/utils/logger.h"
#include "../../../include/common/database/database.h"
#include "../../../include/frontend/cli/dao_connector.h"
#include "../../../include/frontend/cli/menu_utils.h"
#include "../../../include/frontend/menus/customer_menu.h"
#include "../../../include/frontend/menus/menu_system.h"

/**
 * @brief Display the customer menu
 * @param username Customer username
 * @param userType User type (1 = Regular Customer, 3 = Premium Customer)
 */
static void displayCustomerMenu(const char *username, int userType) {
    char title[100];
    sprintf(title, "CUSTOMER MENU - %s", username);
    printHeader(title);
    
    printf("\n");
    printf("1. Account Management\n");
    printf("2. Transaction Management\n");
    printf("3. Beneficiary Management\n");
    printf("4. Bill Payments\n");
    printf("5. Logout\n");
    
    if (userType == USER_CUSTOMER_PREMIUM) { // Premium Customer
        printf("\n");
        printInfo("Premium Customer Features:");
        printf("- Higher transaction limits\n");
        printf("- Priority customer support\n");
        printf("- Reduced transaction fees\n");
    }
}

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
            case 1: // Account Management
                runAccountMenu(username);
                break;
                
            case 2: // Transaction Management
                runTransactionMenu(username);
                break;
                
            case 3: // Beneficiary Management
                runBeneficiaryMenu(username);
                break;
                
            case 4: // Bill Payments
                runBillPaymentMenu(username);
                break;
                
            case 5: // Logout
                running = false;
                LOG_INFO("User %s logged out", username);
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}
