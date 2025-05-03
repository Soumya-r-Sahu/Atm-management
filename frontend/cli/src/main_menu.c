/**
 * @file main_menu.c
 * @brief Main menu for the ATM Management System CLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <mysql/mysql.h>  /* For MYSQL type */
#include <common/utils/logger.h>
#include <common/database/database.h>
#include "menu_utils.h"
#include "customer_menu.h"
#include "admin_menu.h"
#include "transaction_menu.h"

// We'll skip including atm_main.h to avoid conflicts 

// Define logger levels
#define LOG_LEVEL_INFO 1

// Define needed logger macros if not defined
#ifndef LOG_INFO
#define LOG_INFO(format, ...) writeInfoLog(format, ##__VA_ARGS__)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(format, ...) writeErrorLog(format, ##__VA_ARGS__)
#endif

#ifndef LOG_WARNING
#define LOG_WARNING(format, ...) writeWarningLog(format, ##__VA_ARGS__)
#endif

// Stub function implementations
// Logger stubs
int logger_init(const char* filename, int level, int log_type) {
    // Stub implementation - returns success
    (void)filename; // Suppress unused parameter warning
    (void)level;    // Suppress unused parameter warning
    (void)log_type; // Suppress unused parameter warning
    return 0;
}

void logger_close(void) {
    // Stub implementation - does nothing
}

// Database stubs
MYSQL* db_connect(void) {
    // Stub implementation - returns a dummy pointer
    static int dummy;
    return (MYSQL*)&dummy;
}

void db_disconnect(MYSQL* conn) {
    // Stub implementation - does nothing
    (void)conn; // Suppress unused parameter warning
}

// Menu function stubs
void runCustomerMenu(const char *username, int userType) {
    // Stub implementation
    printf("Customer menu for user %s (type %d) would appear here\n", username, userType);
    printf("Press Enter to continue...");
    getchar();
}

void runAdminMenu(const char *username, int userType) {
    // Stub implementation
    printf("Admin menu for user %s (type %d) would appear here\n", username, userType);
    printf("Press Enter to continue...");
    getchar();
}

void runTransactionMenu(void) {
    // Stub implementation
    printf("Transaction menu would appear here\n");
    printf("Press Enter to continue...");
    getchar();
}

// Menu utilities
void clearScreen(void) {
    // Cross-platform clear screen
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

void printHeader(const char* title) {
    int len = strlen(title);
    printf("\n");
    for (int i = 0; i < len + 4; i++) printf("=");
    printf("\n  %s  \n", title);
    for (int i = 0; i < len + 4; i++) printf("=");
    printf("\n");
}

void printCentered(const char* text) {
    int padding = (80 - strlen(text)) / 2;
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s\n", text);
}

void getPassword(char* password, size_t size) {
    // Simple password input (not masked in this stub)
    fgets(password, size, stdin);
    password[strcspn(password, "\n")] = 0; // Remove newline
}

// Function prototypes for the remaining functions
void displayMainMenu(void);
void handleMainMenuChoice(int choice);
bool authenticateUser(char *username, char *password, int *userType);

/**
 * @brief Main entry point for the CLI
 * @return int Exit code
 */
int main(int argc, char *argv[]) {
    // Initialize logger
    if (logger_init("logs/cli.log", 1, LOG_LEVEL_INFO) != 0) {
        fprintf(stderr, "Failed to initialize logger\n");
        return 1;
    }
    
    LOG_INFO("ATM Management System CLI starting...");
    
    // Initialize database connection
    MYSQL *conn = db_connect();
    if (conn == NULL) {
        LOG_ERROR("Failed to connect to database");
        logger_close();
        return 1;
    }
    
    LOG_INFO("Database connection established");
    
    // Display welcome message
    clearScreen();
    printHeader("ATM MANAGEMENT SYSTEM");
    printf("\n\n");
    printCentered("Welcome to the ATM Management System");
    printf("\n\n");
    printCentered("Press Enter to continue...");
    getchar();
    
    // Main menu loop
    bool running = true;
    while (running) {
        clearScreen();
        displayMainMenu();
        
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
        
        if (choice == 4) {
            running = false;
        } else {
            handleMainMenuChoice(choice);
        }
    }
    
    // Clean up
    db_disconnect(conn);
    logger_close();
    
    // Display exit message
    clearScreen();
    printHeader("ATM MANAGEMENT SYSTEM");
    printf("\n\n");
    printCentered("Thank you for using the ATM Management System");
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
    printf("4. Exit\n");
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
                runAdminMenu(username, userType);
            } else {
                printf("\nInvalid username or password. Please try again.\n");
                sleep(2);
            }
            break;
            
        case 3: // ATM Card Operations
            runTransactionMenu();
            break;
            
        default:
            printf("Invalid choice. Please try again.\n");
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
