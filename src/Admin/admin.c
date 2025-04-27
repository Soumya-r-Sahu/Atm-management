#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "admin_operations.h"
#include "admin_menu.h"
#include "admin_db.h"
#include "../utils/logger.h"
#include "../database/database.h"
#include "../config/config_manager.h" // New include for config manager

// Constants
#define ADMIN_LOCKOUT_DURATION 60 // seconds
#define MAX_ADMIN_ATTEMPTS 3

int main() {
    char adminId[50], adminPass[50];
    int choice, isLoggedIn = 0;
    int adminAttempts = MAX_ADMIN_ATTEMPTS;
    time_t adminLockoutTime = 0;
    
    // Main admin authentication loop
    while (!isLoggedIn) {
        clearScreen();
        printf("\n===== ATM Administration Panel =====\n\n");
        
        // Check if admin is locked out
        if (adminAttempts <= 0) {
            handleAdminLockout(&adminAttempts, &adminLockoutTime, ADMIN_LOCKOUT_DURATION);
            if (adminAttempts <= 0) {
                printf("\nPress Enter to continue...");
                getchar();
                continue;
            }
        }
        
        // Get admin credentials
        printf("Enter Admin ID: ");
        scanf("%49s", adminId);
        clearInputBuffer();
        
        printf("Enter Admin Password: ");
        scanf("%49s", adminPass);
        clearInputBuffer();
        
        // Validate credentials
        if (authenticateAdmin(adminId, adminPass)) {
            isLoggedIn = 1;
            writeAuditLog("ADMIN", "Admin login successful");
        } else {
            adminAttempts--;
            printf("\nInvalid admin credentials. %d attempts remaining.\n", adminAttempts);
            writeAuditLog("ADMIN", "Failed admin login attempt");
            
            printf("\nPress Enter to continue...");
            getchar();
        }
    }

    // Load system configurations
    if (!initializeConfigs()) {
        printf("Warning: Failed to load system configurations. Using defaults.\n");
    }
    
    // Admin menu loop
    while (isLoggedIn) {
        clearScreen();
        displayAdminMenu();
        
        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        clearInputBuffer();
        
        switch (choice) {
            case 1: // Dashboard
                displayDashboard();
                break;
                
            case 2: // Cash Management
                manageCash();
                break;
                
            case 3: // Transaction Management
                manageTransactions();
                break;
                
            case 4: // User Management
                manageUsers();
                break;
                
            case 5: // ATM Maintenance
                atmMaintenance();
                break;
                
            case 6: // Security Management
                securityManagement();
                break;
                
            case 7: // Settings
                manageSettings();
                break;
                
            case 8: // Notifications and Alerts
                manageNotifications();
                break;
                
            case 9: // Audit Logs
                viewAuditLogs();
                break;
                
            case 10: // Exit
                isLoggedIn = 0;
                writeAuditLog("ADMIN", "Admin logout");
                printf("\nLogging out of admin panel...\n");
                break;

            case 11: // System Configuration
                manageSystemConfigurations();
                break;
                
            default:
                printf("\nInvalid choice. Please try again.\n");
        }
        
        if (isLoggedIn && choice != 2 && choice != 3 && choice != 4 && choice != 5 
            && choice != 6 && choice != 7 && choice != 8 && choice != 9) {
            printf("\nPress Enter to continue...");
            getchar();
        }
    }

    // Free allocated resources for configs before exiting
    freeConfigs();
    
    return 0;
}