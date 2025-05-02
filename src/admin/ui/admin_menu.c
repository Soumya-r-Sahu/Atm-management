#include "../../../include/admin/ui/admin_menu.h"
#include "../../../include/admin/admin_auth.h"
#include "../../../include/admin/management/user/admin_account_manager.h"
#include "../../../include/admin/management/system/admin_system_manager.h"
#include "../../../include/common/utils/logger.h"
#include "../../../include/netbanking/netbanking.h"
#include "../../../include/upi_transaction/upi_transaction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Forward declarations for functions used in the menu
void manage_netbanking_services(AdminUser* admin);
void configure_upi_services(AdminUser* admin);
void view_digital_transaction_reports(AdminUser* admin);
void manage_virtual_cards(AdminUser* admin);
void show_create_admin_menu(AdminUser* admin);
void show_change_password_menu(AdminUser* admin);

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
        
        // Role-based menu display
        bool isSuperAdmin = admin_has_role(admin, "SuperAdmin");
        bool isATMAdmin = admin_has_role(admin, "ATMAdmin") || isSuperAdmin;
        bool isUserAdmin = admin_has_role(admin, "UserAdmin") || isSuperAdmin;
        bool isNetbankingAdmin = admin_has_role(admin, "NetbankingAdmin") || isSuperAdmin;
        
        // User Management Options - for UserAdmin and SuperAdmin roles
        if (isUserAdmin) {
            printf("1. Create New Customer Account\n");
            printf("4. Regenerate Card PIN\n");
            printf("5. Block/Unblock Card\n");
        }
        
        // ATM Management Options - for ATMAdmin and SuperAdmin roles
        if (isATMAdmin) {
            printf("2. Toggle ATM/Banking Service Status\n");
        }
        
        // Common options for all admin roles
        printf("3. View System Logs\n");
        
        // System Management - SuperAdmin only
        if (isSuperAdmin) {
            printf("6. Manage System Configuration\n");
            printf("7. Back Up System Data\n");
            printf("14. Create New Admin Account\n");
        }
        
        // Netbanking/Digital Services options - for NetbankingAdmin and SuperAdmin
        if (isNetbankingAdmin) {
            printf("10. Manage Netbanking Services\n");
            printf("11. Configure UPI Services\n");
            printf("12. View Digital Transaction Reports\n");
            printf("13. Manage Virtual Cards\n");
        }
        
        // Available to all admin users
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
        if (strlen(input) == 0 || !isdigit(input[0])) {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        choice = atoi(input);
        
        // Process choice
        switch (choice) {
            case 1:
                if (isUserAdmin) {
                    create_account(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
                break;
                
            case 2:
                if (isATMAdmin) {
                    toggle_service_mode(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
                break;
                
            case 3:
                // All roles can view logs
                view_system_logs(admin);
                break;
                
            case 4:
                if (isUserAdmin) {
                    regenerate_card_pin(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
                break;
                
            case 5:
                if (isUserAdmin) {
                    toggle_card_status(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
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
                printf("\nLogging out and exiting...\n");
                running = false;
                break;
                
            case 10:
                if (isNetbankingAdmin) {
                    manage_netbanking_services(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
                break;
                
            case 11:
                if (isNetbankingAdmin) {
                    configure_upi_services(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
                break;
                
            case 12:
                if (isNetbankingAdmin) {
                    view_digital_transaction_reports(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
                break;
                
            case 13:
                if (isNetbankingAdmin) {
                    manage_virtual_cards(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
                break;
                
            case 14:
                if (isSuperAdmin) {
                    show_create_admin_menu(admin);
                } else {
                    printf("Access denied: You don't have permission for this operation.\n");
                }
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

// Manage netbanking services
void manage_netbanking_services(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        printf("Error: Not authorized. Please log in first.\n");
        return;
    }
    
    printf("\n=======================================\n");
    printf("=      MANAGE NETBANKING SERVICES     =\n");
    printf("=======================================\n");
    
    int choice;
    
    printf("1. Enable/Disable Netbanking Services\n");
    printf("2. View Registered Netbanking Users\n");
    printf("3. Reset User Netbanking Password\n");
    printf("4. Set Transaction Limits\n");
    printf("5. Configure Two-Factor Authentication\n");
    printf("6. Back to Main Menu\n");
    printf("---------------------------------------\n");
    printf("Enter choice: ");
    
    scanf("%d", &choice);
    getchar(); // Clear input buffer
    
    switch (choice) {
        case 1:
            printf("Netbanking service status toggled.\n");
            writeAuditLog("ADMIN", "Netbanking service status changed");
            break;
        case 2:
            printf("Displaying registered netbanking users...\n");
            // Code to display netbanking users would go here
            break;
        case 3:
            printf("User netbanking password has been reset.\n");
            writeAuditLog("ADMIN", "Reset user netbanking password");
            break;
        case 4:
            printf("Transaction limits updated.\n");
            writeAuditLog("ADMIN", "Updated netbanking transaction limits");
            break;
        case 5:
            printf("Two-factor authentication settings updated.\n");
            writeAuditLog("ADMIN", "Updated 2FA settings");
            break;
        case 6:
            return;
        default:
            printf("Invalid option. Please try again.\n");
            break;
    }
}

// Configure UPI services
void configure_upi_services(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        printf("Error: Not authorized. Please log in first.\n");
        return;
    }
    
    printf("\n=======================================\n");
    printf("=         CONFIGURE UPI SERVICES      =\n");
    printf("=======================================\n");
    
    int choice;
    
    printf("1. Enable/Disable UPI Platform\n");
    printf("2. Set UPI Transaction Limits\n");
    printf("3. Manage UPI Merchants\n");
    printf("4. View UPI Statistics\n");
    printf("5. Back to Main Menu\n");
    printf("---------------------------------------\n");
    printf("Enter choice: ");
    
    scanf("%d", &choice);
    getchar(); // Clear input buffer
    
    switch (choice) {
        case 1:
            printf("UPI platform status toggled.\n");
            writeAuditLog("ADMIN", "UPI platform status changed");
            break;
        case 2:
            printf("UPI transaction limits updated.\n");
            writeAuditLog("ADMIN", "Updated UPI transaction limits");
            break;
        case 3:
            printf("UPI merchant settings updated.\n");
            writeAuditLog("ADMIN", "Updated UPI merchant settings");
            break;
        case 4:
            printf("Displaying UPI statistics...\n");
            // Code to display UPI statistics would go here
            break;
        case 5:
            return;
        default:
            printf("Invalid option. Please try again.\n");
            break;
    }
}

// View digital transaction reports
void view_digital_transaction_reports(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        printf("Error: Not authorized. Please log in first.\n");
        return;
    }
    
    printf("\n=======================================\n");
    printf("=    DIGITAL TRANSACTION REPORTS      =\n");
    printf("=======================================\n");
    
    int choice;
    
    printf("1. Daily Transactions Summary\n");
    printf("2. Monthly Transaction Report\n");
    printf("3. Failed Transaction Analysis\n");
    printf("4. High-Value Transaction Alert\n");
    printf("5. Back to Main Menu\n");
    printf("---------------------------------------\n");
    printf("Enter choice: ");
    
    scanf("%d", &choice);
    getchar(); // Clear input buffer
    
    switch (choice) {
        case 1:
            printf("Displaying daily transaction summary...\n");
            // Code to display daily transactions would go here
            break;
        case 2:
            printf("Displaying monthly transaction report...\n");
            // Code to display monthly report would go here
            break;
        case 3:
            printf("Analyzing failed transactions...\n");
            // Code to analyze failed transactions would go here
            break;
        case 4:
            printf("Displaying high-value transaction alerts...\n");
            // Code to display high-value alerts would go here
            break;
        case 5:
            return;
        default:
            printf("Invalid option. Please try again.\n");
            break;
    }
}

// Manage virtual cards
void manage_virtual_cards(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        printf("Error: Not authorized. Please log in first.\n");
        return;
    }
    
    printf("\n=======================================\n");
    printf("=         MANAGE VIRTUAL CARDS        =\n");
    printf("=======================================\n");
    
    int choice;
    
    printf("1. Issue New Virtual Card\n");
    printf("2. Block/Unblock Virtual Card\n");
    printf("3. Set Virtual Card Limits\n");
    printf("4. View Virtual Card Transactions\n");
    printf("5. Back to Main Menu\n");
    printf("---------------------------------------\n");
    printf("Enter choice: ");
    
    scanf("%d", &choice);
    getchar(); // Clear input buffer
    
    switch (choice) {
        case 1:
            printf("New virtual card issued successfully.\n");
            writeAuditLog("ADMIN", "Issued new virtual card");
            break;
        case 2:
            printf("Virtual card status updated.\n");
            writeAuditLog("ADMIN", "Updated virtual card status");
            break;
        case 3:
            printf("Virtual card limits updated.\n");
            writeAuditLog("ADMIN", "Updated virtual card limits");
            break;
        case 4:
            printf("Displaying virtual card transactions...\n");
            // Code to display virtual card transactions would go here
            break;
        case 5:
            return;
        default:
            printf("Invalid option. Please try again.\n");
            break;
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
        writeAuditLog("SECURITY", "Unauthorized attempt to access admin creation menu");
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
    printf("4. NetbankingAdmin - Digital banking service access\n");
    
    // Role selection
    const int MAX_AVAILABLE_ROLES = 4;
    const char* available_roles[4] = {
        "SuperAdmin",
        "ATMAdmin",
        "UserAdmin",
        "NetbankingAdmin"
    };
    
    // Array to track selected roles
    bool selected_roles[4] = {false, false, false, false};
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
            writeAuditLog("ADMIN", log_message);
        } else {
            printf("\nError: Failed to create admin account.\n");
        }
    } else {
        printf("\nAdmin account creation cancelled.\n");
    }
}