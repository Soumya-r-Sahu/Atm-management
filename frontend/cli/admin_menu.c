/**
 * @file admin_menu.c
 * @brief Implementation of admin menu for the ATM Management System CLI
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
#include "../../include/frontend/menus/admin_menu.h"

/**
 * @brief Display the admin menu
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
static void displayAdminMenu(const char *username, int userType) {
    char title[100];
    sprintf(title, "ADMIN MENU - %s", username);
    printHeader(title);
    
    printf("\n");
    printf("1. User Management\n");
    printf("2. System Configuration\n");
    printf("3. Reports\n");
    printf("4. Logout\n");
    
    if (userType == USER_ADMIN_SUPER) { // Super Admin
        printf("\n");
        printInfo("Super Admin Features:");
        printf("- Full system access\n");
        printf("- Create/delete admin users\n");
        printf("- Database management\n");
    }
}

/**
 * @brief Run the admin menu
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void runAdminMenu(const char *username, int userType) {
    bool running = true;
    
    while (running) {
        clearScreen();
        displayAdminMenu(username, userType);
        
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
            case 1: // User Management
                runUserManagementMenu(username, userType);
                break;
                
            case 2: // System Configuration
                runSystemConfigMenu(username, userType);
                break;
                
            case 3: // Reports
                runReportsMenu(username, userType);
                break;
                
            case 4: // Logout
                running = false;
                LOG_INFO("Admin %s logged out", username);
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}
