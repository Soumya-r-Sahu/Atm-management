#include "admin/ui/admin_menu.h"
#include "admin/admin_auth.h"
#include "admin/management/user/admin_account_manager.h"
#include "admin/management/system/admin_system_manager.h"
#include "common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Forward declarations for functions used in the menu
void show_create_admin_menu(AdminUser* admin);
void show_change_password_menu(AdminUser* admin);

// Initialize the admin menu
void admin_menu_initialize() {
    writeSecurityLog("ADMIN", "Admin menu initialized");
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
        
        // Simplified role system - just SuperAdmin or regular admin
        bool isSuperAdmin = admin_has_role(admin, "SuperAdmin");
        
        // User Management Options
        printf("1. Create New Customer Account\n");
        printf("4. Regenerate Card PIN\n");
        printf("5. Block/Unblock Card\n");
        
        // ATM Management Options
        printf("2. Toggle ATM/Banking Service Status\n");
        
        // Common options for all admin roles
        printf("3. View System Logs\n");
        
        // System Management - SuperAdmin only
        if (isSuperAdmin) {
            printf("6. Manage System Configuration\n");
            printf("7. Back Up System Data\n");
            printf("9. Create New Admin Account\n");
        }
        
        // Available to all admin users
        printf("8. Change Admin Password\n");
        printf("0. Exit\n");
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
        if (strlen(input) == 0 || !isdigit(input[0])) {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        choice = atoi(input);
        
        // Process choice
        switch (choice) {
            case 1:
                create_account(admin);
                break;
                
            case 2:
                toggle_service_mode(admin);
                break;
                
            case 3:
                // All roles can view logs
                view_system_logs(admin);
                break;
                
            case 4:
                regenerate_card_pin(admin);
                break;
                
            case 5:
                toggle_card_status(admin);
                break;
                
            case 6:
                if (isSuperAdmin) {
                    manage_system_config(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
                break;
                
            case 7:
                if (isSuperAdmin) {
                    backup_system_data(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
                break;
                
            case 8:
                // All roles can change their own password
                show_change_password_menu(admin);
                break;
                
            case 9:
                if (isSuperAdmin) {
                    show_create_admin_menu(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
                break;
                
            case 0:
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

// Show the create admin account menu for SuperAdmin users
void show_create_admin_menu(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        printf("Error: Not authorized. Please log in first.\n");
        return;
    }
    
    // Verify user is SuperAdmin
    if (!admin_has_role(admin, "SuperAdmin")) {
        printf("Error: Only SuperAdmin users can create admin accounts.\n");
        writeSecurityLog("SECURITY", "Unauthorized attempt to access admin creation menu");
        return;
    }
    
    printf("\n=======================================\n");
    printf("=        CREATE ADMIN ACCOUNT         =\n");
    printf("=======================================\n");
    
    // Get new admin username
    char new_username[50];
    printf("Enter username for new admin: ");
    fgets(new_username, sizeof(new_username), stdin);
    new_username[strcspn(new_username, "\n")] = 0; // Remove newline
    
    // Check if username already exists
    if (admin_username_exists(new_username)) {
        printf("Error: Username already exists. Please choose a different username.\n");
        return;
    }
    
    // Get and confirm password
    char new_password[100];
    char confirm_password[100];
    printf("Enter password for new admin: ");
    fgets(new_password, sizeof(new_password), stdin);
    new_password[strcspn(new_password, "\n")] = 0;
    
    printf("Confirm password: ");
    fgets(confirm_password, sizeof(confirm_password), stdin);
    confirm_password[strcspn(confirm_password, "\n")] = 0;
    
    if (strcmp(new_password, confirm_password) != 0) {
        printf("Error: Passwords don't match.\n");
        return;
    }
    
    // Display available roles
    printf("\nAvailable Admin Roles:\n");
    printf("1. SuperAdmin - Full system access\n");
    printf("2. ATMAdmin - ATM management access\n");
    printf("3. UserAdmin - Customer account management access\n");
    
    // Role selection
    const int MAX_AVAILABLE_ROLES = 3;
    const char* available_roles[3] = {
        "SuperAdmin",
        "ATMAdmin",
        "UserAdmin"
    };
    
    // Array to track selected roles
    bool selected_roles[3] = {false, false, false};
    int num_selected_roles = 0;
    
    // Get role selections
    printf("\nSelect roles for the new admin (enter role numbers one by one):\n");
    printf("(Enter 0 when finished selecting roles)\n");
    
    while (num_selected_roles < MAX_ADMIN_ROLES) {
        int role_choice;
        printf("Select role (0 to finish): ");
        scanf("%d", &role_choice);
        getchar(); // Clear input buffer
        
        if (role_choice == 0) {
            // Finished selecting roles
            break;
        } else if (role_choice >= 1 && role_choice <= MAX_AVAILABLE_ROLES) {
            // Valid role selection
            if (!selected_roles[role_choice-1]) {
                selected_roles[role_choice-1] = true;
                num_selected_roles++;
                printf("Added role: %s\n", available_roles[role_choice-1]);
            } else {
                printf("Role already selected.\n");
            }
        } else {
            printf("Invalid role selection. Please try again.\n");
        }
    }
    
    // Check if at least one role was selected
    if (num_selected_roles == 0) {
        printf("Error: At least one role must be selected.\n");
        return;
    }
    
    // Confirm creation
    printf("\nYou are about to create a new admin account with the following details:\n");
    printf("Username: %s\n", new_username);
    printf("Roles: ");
    
    // Prepare roles array for account creation
    const char* selected_role_names[MAX_ADMIN_ROLES];
    int role_index = 0;
    
    for (int i = 0; i < MAX_AVAILABLE_ROLES; i++) {
        if (selected_roles[i]) {
            printf("%s%s", (role_index > 0) ? ", " : "", available_roles[i]);
            selected_role_names[role_index++] = available_roles[i];
        }
    }
    printf("\n\n");
    
    // Final confirmation
    char confirm[10];
    printf("Confirm creation? (yes/no): ");
    fgets(confirm, sizeof(confirm), stdin);
    confirm[strcspn(confirm, "\n")] = 0;
    
    if (strcmp(confirm, "yes") == 0) {
        // Create the admin account
        if (create_admin_account(admin, new_username, new_password, selected_role_names, num_selected_roles)) {
            printf("\nAdmin account successfully created!\n");
            char log_message[200];
            sprintf(log_message, "Created new admin account '%s' with %d roles", new_username, num_selected_roles);
            writeSecurityLog("ADMIN", log_message);
        } else {
            printf("\nError: Failed to create admin account.\n");
        }
    } else {
        printf("\nAdmin account creation cancelled.\n");
    }
}