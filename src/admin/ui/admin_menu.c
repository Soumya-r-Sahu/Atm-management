#include "../../../include/admin/ui/admin_menu.h"
#include "../../../include/admin/admin_auth.h"
#include "../../../include/admin/management/user/admin_account_manager.h"
#include "../../../include/admin/management/system/admin_system_manager.h"
#include "../../../include/common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Initialize the admin menu
void admin_menu_initialize() {
    writeAuditLog("ADMIN", "Admin menu initialized");
}

// Show the main admin menu
void show_admin_main_menu(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        printf("Error: Not authorized. Please log in first.\n");
        return;
    }
    
    bool running = true;
    
    while (running && admin->is_logged_in) {
        // Check if session is still valid
        if (!is_admin_session_valid(admin)) {
            printf("\nSession expired. Please log in again.\n");
            break;
        }
        
        // Update activity timestamp
        update_admin_activity(admin);
        
        // Display menu
        printf("\n=======================================\n");
        printf("=          ADMIN MAIN MENU           =\n");
        printf("=======================================\n");
        printf("Welcome, %s\n", admin->username);
        printf("---------------------------------------\n");
        printf("1. Create New Customer Account\n");
        printf("2. Toggle ATM/Banking Service Status\n");
        printf("3. View System Logs\n");
        printf("4. Regenerate Card PIN\n");
        printf("5. Block/Unblock Card\n");
        printf("6. Manage System Configuration\n");
        printf("7. Back Up System Data\n");
        printf("8. Change Admin Password\n");
        printf("9. Exit\n");
        printf("---------------------------------------\n");
        printf("Enter choice: ");
        
        // Get user choice
        int choice;
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error reading input.\n");
            continue;
        }
        
        // Clean the input
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) != 1 || !isdigit(input[0])) {
            printf("Invalid input. Please enter a number 1-9.\n");
            continue;
        }
        
        choice = input[0] - '0';
        
        // Process choice
        switch (choice) {
            case 1:
                create_account(admin);
                break;
                
            case 2:
                toggle_service_mode(admin);
                break;
                
            case 3:
                view_system_logs(admin);
                break;
                
            case 4:
                regenerate_card_pin(admin);
                break;
                
            case 5:
                toggle_card_status(admin);
                break;
                
            case 6:
                manage_system_config(admin);
                break;
                
            case 7:
                backup_system_data(admin);
                break;
                
            case 8:
                show_change_password_menu(admin);
                break;
                
            case 9:
                printf("\nLogging out and exiting...\n");
                running = false;
                break;
                
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}

// Show the change password menu
void show_change_password_menu(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        printf("Error: Not authorized. Please log in first.\n");
        return;
    }
    
    printf("\n=======================================\n");
    printf("=         CHANGE ADMIN PASSWORD       =\n");
    printf("=======================================\n");
    
    char current_password[100];
    char new_password[100];
    char confirm_password[100];
    
    printf("Enter current password: ");
    fgets(current_password, sizeof(current_password), stdin);
    current_password[strcspn(current_password, "\n")] = 0;
    
    printf("Enter new password: ");
    fgets(new_password, sizeof(new_password), stdin);
    new_password[strcspn(new_password, "\n")] = 0;
    
    printf("Confirm new password: ");
    fgets(confirm_password, sizeof(confirm_password), stdin);
    confirm_password[strcspn(confirm_password, "\n")] = 0;
    
    if (strcmp(new_password, confirm_password) != 0) {
        printf("Error: Passwords don't match.\n");
        return;
    }
    
    if (change_admin_password(admin, current_password, new_password)) {
        printf("Password changed successfully.\n");
    } else {
        printf("Failed to change password. Please verify your current password is correct.\n");
    }
}