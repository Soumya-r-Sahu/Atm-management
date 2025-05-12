/**
 * @file main_menu.c
 * @brief Main menu for the ATM Management System CLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "../../include/common/utils/logger.h"
#include "../../include/common/database/database.h"
#include "../../include/atm/atm_main.h"
#include "../../include/common/database/card_account_management.h"
#include "../../include/frontend/cli/menu_utils.h"
#include "../../include/frontend/menus/menu_system.h"
#include "../../include/frontend/menus/customer_menu.h"
#include "../../include/frontend/menus/admin_menu.h"
#include "../../include/frontend/menus/transaction_menu.h"
#include "../../include/frontend/menus/cbs_admin_menu.h"
#include "../../include/frontend/menus/main_menu.h"

// Function prototypes from the menu system 
// These functions are now imported from menu_system.h
// No need to redefine them here

/**
 * @brief Main entry point for the CLI
 * @return int Exit code
 */
int main(int argc, char *argv[]) {
    // Initialize logger
    if (logger_init("logs/cli.log", 1, LOG_INFO) != 0) {
        fprintf(stderr, "Failed to initialize logger\n");
        return 1;
    }
    
    LOG_INFO("Core Banking System CLI starting...");
    
    // Initialize database connection
    MYSQL *conn = db_connect();
    if (conn == NULL) {
        LOG_ERROR("Failed to connect to database");
        logger_close();
        return 1;
    }
    
    LOG_INFO("Database connection established");
    
    // Initialize the menu system
    if (!initMenuSystem()) {
        LOG_ERROR("Failed to initialize menu system");
        db_disconnect(conn);
        logger_close();
        return 1;
    }
    
    // Display welcome message
    clearScreen();
    printHeader("CORE BANKING SYSTEM");
    printf("\n\n");
    printCentered("Welcome to the Core Banking System");
    printf("\n\n");
    printCentered("Press Enter to continue...");
    getchar();
    
    // Main menu loop using the new menu system
    bool running = true;
    while (running) {
        displayMainMenu();
        
        int choice = getInteger("\nEnter your choice (1-6): ", 1, 6);
        
        if (choice == 6) {
            running = false;
        } else {
            handleMainMenuChoice(choice);
        }
    }
      // Clean up
    closeMenuSystem();
    db_disconnect(conn);
    logger_close();
    
    // Display exit message
    clearScreen();
    printHeader("CORE BANKING SYSTEM");
    printf("\n\n");
    printCentered("Thank you for using the Core Banking System");
    printf("\n\n");
    
    return 0;
}

/**
 * @brief Display the main menu
 */
void displayMainMenu(void) {
    printHeader("MAIN MENU");
    printf("\n");
    printf("1. Customer Login\n");
    printf("2. Admin Login\n");
    printf("3. ATM Card Operations\n");
    printf("4. Core Banking Transactions\n");
    printf("5. Core Banking Admin\n");
    printf("6. Exit\n");
}

/**
 * @brief Handle main menu choice
 * @param choice User's choice
 */
void handleMainMenuChoice(int choice) {
    char username[50];
    char password[50];
    int userType = 0; // 1 = Customer, 2 = Admin
    bool authenticated = false;
    
    switch (choice) {
        case 1: // Customer Login
            clearScreen();
            printHeader("CUSTOMER LOGIN");
            printf("\n");
            printf("Enter Username: ");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = 0; // Remove newline
            
            printf("Enter Password: ");
            getPassword(password, sizeof(password));
            
            userType = 1;
            authenticated = authenticateUser(username, password, &userType);
            
            if (authenticated) {
                runCustomerMenu(username, userType);
            } else {
                printf("\nInvalid username or password. Please try again.\n");
                sleep(2);
            }
            break;
            
        case 2: // Admin Login
            clearScreen();
            printHeader("ADMIN LOGIN");
            printf("\n");
            printf("Enter Username: ");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = 0; // Remove newline
            
            printf("Enter Password: ");
            getPassword(password, sizeof(password));
            
            userType = 2;
            authenticated = authenticateUser(username, password, &userType);
            
            if (authenticated) {
                runAdminMenu(username);
            } else {
                printf("\nInvalid username or password. Please try again.\n");
                sleep(2);
            }
            break;
            
        case 3: // ATM Card Operations
            // TODO: Implement ATM card operations
            clearScreen();
            printHeader("ATM CARD OPERATIONS");
            printf("\nThis feature is coming soon.\n");
            printf("\nPress Enter to continue...");
            getchar();
            break;
            
        case 4: // Core Banking Transactions
            clearScreen();
            printHeader("CORE BANKING TRANSACTIONS");
            printf("\n");
            
            // Get card number
            int cardNumber;
            printf("Enter Card Number: ");
            if (scanf("%d", &cardNumber) != 1) {
                // Clear input buffer
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                printf("\nInvalid card number. Please try again.\n");
                sleep(2);
                break;
            }
            
            // Clear input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            
            // Validate card exists
            if (!cbs_card_exists(cardNumber)) {
                printf("\nCard not found. Please try again.\n");
                sleep(2);
                break;
            }
            
            // Check if card is active
            if (!cbs_is_card_active(cardNumber)) {
                printf("\nThis card is inactive or blocked.\n");
                sleep(2);
                break;
            }
            
            // Get PIN
            int pin;
            printf("Enter PIN: ");
            if (scanf("%d", &pin) != 1) {
                // Clear input buffer
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                printf("\nInvalid PIN. Please try again.\n");
                sleep(2);
                break;
            }
            
            // Clear input buffer
            c = 0;
            while ((c = getchar()) != '\n' && c != EOF);
            
            // Validate PIN
            if (!cbs_validate_card(cardNumber, pin)) {
                printf("\nInvalid PIN. Please try again.\n");
                sleep(2);
                break;
            }
              // Run transaction menu
            runTransactionMenu(cardNumber);
            break;
            
        case 5: // Core Banking Admin
            // Run CBS admin menu
            runCBSAdminMenu();
            break;
            
        default:
            printf("\nInvalid choice. Please try again.\n");
            sleep(2);
            break;
    }
}

/**
 * @brief Authenticate user
 * @param username Username
 * @param password Password
 * @param userType User type (1 = Customer, 2 = Admin)
 * @return bool True if authenticated, false otherwise
 */
bool authenticateUser(char *username, char *password, int *userType) {
    // For demo purposes, accept any login with demo credentials
    // In a real application, this would verify against the database
    
    if (*userType == 1) { // Customer
        if (strcmp(username, "customer") == 0 && strcmp(password, "password123") == 0) {
            LOG_INFO("Customer login successful: %s", username);
            return true;
        }
    } else if (*userType == 2) { // Admin
        if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
            LOG_INFO("Admin login successful: %s", username);
            return true;
        }
    }
    
    LOG_WARNING("Failed login attempt: %s (user type: %d)", username, *userType);
    return false;
}
