/**
 * @file main_menu.c
 * @brief Implementation of main menu for the Core Banking System
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
#include "../../../include/frontend/menus/main_menu.h"

/**
 * @brief Display the main menu
 */
void displayMainMenu(void) {
    clearScreen();
    printHeader("CORE BANKING SYSTEM");
    
    printf("\n");
    printf("1. Customer Login\n");
    printf("2. ATM Services\n");
    printf("3. Admin Login\n");
    printf("4. CBS Administration\n");
    printf("5. Help\n");
    printf("6. Exit\n");
    
    printf("\nWelcome to the Core Banking System. Please select an option.\n");
}

/**
 * @brief Handle the main menu choice
 * @param choice The user's menu choice
 */
void handleMainMenuChoice(int choice) {
    switch (choice) {
        case 1: { // Customer Login
            char username[50];
            char password[50];
            
            clearScreen();
            printHeader("CUSTOMER LOGIN");
            
            printf("\nPlease enter your credentials:\n");
            getString("Username: ", username, sizeof(username));
            getPassword("Password: ", password, sizeof(password));
            
            // In a real application, this would validate the credentials against the database            // Validate customer credentials using DAO
            DatabaseAccessObject* dao = getFrontendDAO();
            if (dao == NULL) {
                printError("System error: Database connection unavailable");
                sleep(2);
                return;
            }
            
            int userType = dao->validateUserCredentials(username, password);
            if (userType == USER_CUSTOMER || userType == USER_CUSTOMER_PREMIUM) {
                printSuccess("Login successful!");
                sleep(1);
                
                LOG_INFO("Customer %s logged in", username);
                runCustomerMenu(username, userType);
            } else {
                printError("Invalid username or password!");
                sleep(2);
                
                LOG_WARNING("Failed login attempt for username %s", username);
            }
            break;
        }
            
        case 2: { // ATM Services
            int cardNumber;
            int pin;
            
            clearScreen();
            printHeader("ATM SERVICES");
            
            printf("\nPlease enter your ATM card details:\n");
            cardNumber = getInteger("Card Number: ", 1000000000, 9999999999);
            pin = getInteger("PIN: ", 1000, 9999);
              // Validate card and PIN using DAO
            DatabaseAccessObject* dao = getFrontendDAO();
            if (dao == NULL) {
                printError("System error: Database connection unavailable");
                sleep(2);
                return;
            }
            
            if (dao->validateCard(cardNumber, pin)) {
                printSuccess("Card validated successfully!");
                sleep(1);
                
                LOG_INFO("ATM card %d validated", cardNumber);
                runATMOperationsMenu(cardNumber);
            } else {
                printError("Invalid card number or PIN!");
                sleep(2);
                
                LOG_WARNING("Failed ATM validation for card %d", cardNumber);
            }
            break;
        }
            
        case 3: { // Admin Login
            char adminId[50];
            char password[50];
            
            clearScreen();
            printHeader("ADMIN LOGIN");
            
            printf("\nPlease enter your admin credentials:\n");
            getString("Admin ID: ", adminId, sizeof(adminId));
            getPassword("Password: ", password, sizeof(password));
              // Validate admin credentials using DAO
            DatabaseAccessObject* dao = getFrontendDAO();
            if (dao == NULL) {
                printError("System error: Database connection unavailable");
                sleep(2);
                return;
            }
            
            int userType = dao->validateUserCredentials(adminId, password);
            if (userType == USER_ADMIN || userType == USER_ADMIN_SUPER) {
                if (userType == USER_ADMIN) {
                    printSuccess("Admin login successful!");
                } else {
                    printSuccess("Super admin login successful!");
                }
                sleep(1);
                
                LOG_INFO("Admin %s logged in with type %d", adminId, userType);
                runAdminMenu(adminId, userType);
            } else {
                printError("Invalid admin ID or password!");
                sleep(2);
                
                LOG_WARNING("Failed admin login attempt for ID %s", adminId);
            }
            break;
        }
            
        case 4: { // CBS Administration
            clearScreen();
            printHeader("CBS ADMINISTRATION");
            
            printf("\nAccess to CBS administration requires elevated privileges.\n");
            
            char adminId[50];
            char password[50];
            
            getString("CBS Admin ID: ", adminId, sizeof(adminId));
            getPassword("Password: ", password, sizeof(password));
              // Validate CBS admin credentials using DAO
            DatabaseAccessObject* dao = getFrontendDAO();
            if (dao == NULL) {
                printError("System error: Database connection unavailable");
                sleep(2);
                return;
            }
            
            // Check if the user has CBS admin privileges
            if (dao->validateUserCredentials(adminId, password) == USER_CBS_ADMIN) {
                printSuccess("CBS Admin access granted!");
                sleep(1);
                
                LOG_INFO("CBS Admin %s logged in", adminId);
                runCBSAdminMenu(adminId);
            } else {
                printError("Invalid CBS admin ID or password!");
                sleep(2);
                
                LOG_WARNING("Failed CBS admin login attempt for ID %s", adminId);
            }
            break;
        }
            
        case 5: { // Help
            clearScreen();
            printHeader("HELP & SUPPORT");
            
            printf("\nCore Banking System Help\n");
            printf("=======================\n\n");
            
            printf("For Customers:\n");
            printf("- Use option 1 to access your accounts, perform transactions, and manage your profile.\n");
            printf("- For ATM services, use option 2 with your card number and PIN.\n\n");
            
            printf("For Administrators:\n");
            printf("- Use option 3 to access the admin panel for user management and system settings.\n");
            printf("- CBS administration (option 4) is for system-level configuration and maintenance.\n\n");
            
            printf("Contact Information:\n");
            printf("- Customer Support: 1800-123-4567\n");
            printf("- Email: support@cbs.example.com\n");
            printf("- Hours: Monday-Saturday, 9 AM - 6 PM\n\n");
            
            printf("Technical Support:\n");
            printf("- For system issues, contact IT department at 1800-765-4321\n");
            printf("- Email: it-support@cbs.example.com\n\n");
            
            LOG_INFO("Help menu accessed");
            
            pauseExecution();
            break;
        }
            
        case 6: { // Exit
            clearScreen();
            printHeader("EXIT");
            
            printf("\nThank you for using the Core Banking System.\n");
            printf("Goodbye!\n\n");
            
            LOG_INFO("Application exit");
            
            exit(0);
            break;
        }
            
        default:
            printf("\nInvalid choice. Please try again.\n");
            sleep(2);
            break;
    }
}

/**
 * @brief Run the main menu
 */
void runMainMenu(void) {
    // Initialize DAO connection
    if (!initFrontendDAO()) {
        LOG_ERROR("Failed to initialize DAO - cannot continue");
        printf("\nFATAL ERROR: Database connection failed. Cannot start application.\n");
        printf("Please check database configuration and try again.\n");
        printf("Press Enter to exit...");
        getchar();
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        displayMainMenu();
        
        int choice;
        printf("\nEnter your choice (1-6): ");
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
        
        handleMainMenuChoice(choice);
    }
    
    // This will never be reached, but for completeness:
    closeFrontendDAO();
}
}
