/**
 * @file admin_menu.c
 * @brief Implementation of admin menu for the Core Banking System
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
#include "../../../include/frontend/menus/admin_menu.h"

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
    printf("4. Security Management\n");
    printf("5. Audit Logs\n");
    printf("6. Logout\n");
    
    if (userType == USER_ADMIN_SUPER) { // Super Admin
        printf("\n");
        printInfo("Super Admin Features:");
        printf("- Database Administration\n");
        printf("- System-wide configuration\n");
        printf("- Security override capabilities\n");
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
                
            case 4: // Security Management
                runSecurityManagementMenu(username, userType);
                break;
                
            case 5: // Audit Logs
                runAuditMenu(username, userType);
                break;
                
            case 6: // Logout
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

/**
 * @brief Run the user management menu
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void runUserManagementMenu(const char *username, int userType) {
    bool running = true;
    
    while (running) {
        clearScreen();
        printHeader("USER MANAGEMENT");
        
        printf("\n");
        printf("1. View Users\n");
        printf("2. Add User\n");
        printf("3. Edit User\n");
        printf("4. Disable User\n");
        printf("5. Reset User Password\n");
        printf("6. Return to Admin Menu\n");
        
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
        
        switch (choice) {
            case 1: // View Users
                viewUsers(username, userType);
                break;
                
            case 2: // Add User
                addUser(username, userType);
                break;
                
            case 3: // Edit User
                editUser(username, userType);
                break;
                
            case 4: // Disable User
                disableUser(username, userType);
                break;
                
            case 5: // Reset User Password
                resetUserPassword(username, userType);
                break;
                
            case 6: // Return to Admin Menu
                running = false;
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}

/**
 * @brief View users
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void viewUsers(const char *username, int userType) {
    clearScreen();
    printHeader("VIEW USERS");
    
    // In a real application, this would fetch data from the database
    // For demo purposes, we'll use mock data
    
    printf("\nUser Type Filter:\n");
    printf("1. All Users\n");
    printf("2. Customers\n");
    printf("3. Admins\n");
    printf("4. ATM Operators\n");
    
    int filterChoice = getInteger("Select Filter", 1, 4);
    
    printLine('-', SCREEN_WIDTH);
    printf("%-5s %-20s %-15s %-15s %-15s\n", "ID", "Username", "User Type", "Status", "Last Login");
    printLine('-', SCREEN_WIDTH);
    
    if (filterChoice == 1 || filterChoice == 2) {
        printf("%-5d %-20s %-15s %-15s %-15s\n", 1001, "john_doe", "Customer (Regular)", "Active", "2023-05-09");
        printf("%-5d %-20s %-15s %-15s %-15s\n", 1002, "priya_sharma", "Customer (Premium)", "Active", "2023-05-10");
        printf("%-5d %-20s %-15s %-15s %-15s\n", 1003, "amit_patel", "Customer (Regular)", "Locked", "2023-05-05");
    }
    
    if (filterChoice == 1 || filterChoice == 3) {
        printf("%-5d %-20s %-15s %-15s %-15s\n", 2001, "admin_kumar", "Admin", "Active", "2023-05-10");
        printf("%-5d %-20s %-15s %-15s %-15s\n", 2002, "super_singh", "Admin (Super)", "Active", "2023-05-10");
    }
    
    if (filterChoice == 1 || filterChoice == 4) {
        printf("%-5d %-20s %-15s %-15s %-15s\n", 3001, "atm_op1", "ATM Operator", "Active", "2023-05-10");
        printf("%-5d %-20s %-15s %-15s %-15s\n", 3002, "atm_op2", "ATM Operator", "Inactive", "2023-05-01");
    }
    
    printLine('-', SCREEN_WIDTH);
    
    printf("\nOptions:\n");
    printf("1. View User Details\n");
    printf("2. Export User List\n");
    printf("3. Return to User Management\n");
    
    int optionChoice = getInteger("Select Option", 1, 3);
    
    if (optionChoice == 1) {
        int userId = getInteger("Enter User ID to view details", 1001, 3002);
        
        clearScreen();
        printHeader("USER DETAILS");
        
        // In a real application, this would fetch data from the database
        // For demo purposes, we'll use mock data
        printf("\nUser ID: %d\n", userId);
        printf("Username: john_doe\n");
        printf("Full Name: John Doe\n");
        printf("Email: john.doe@example.com\n");
        printf("Phone: +91-9876543210\n");
        printf("User Type: Customer (Regular)\n");
        printf("Status: Active\n");
        printf("Creation Date: 2023-01-15\n");
        printf("Last Login: 2023-05-09 14:30:45\n");
        printf("Account Locked: No\n");
        printf("Failed Login Attempts: 0\n");
        printf("\nLinked Accounts:\n");
        printf("- Savings Account: 1234567890\n");
        printf("- Current Account: 0987654321\n");
        
        LOG_INFO("Admin %s viewed details for user ID %d", username, userId);
    } else if (optionChoice == 2) {
        printf("\nExporting user list...\n");
        sleep(2);
        printSuccess("User list exported successfully!");
        printf("File saved as: user_list_export_%s.csv\n", "2023-05-10");
        
        LOG_INFO("Admin %s exported user list", username);
    }
    
    pauseExecution();
}

/**
 * @brief Add user
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void addUser(const char *username, int userType) {
    clearScreen();
    printHeader("ADD USER");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    printf("\nSelect User Type:\n");
    printf("1. Customer (Regular)\n");
    printf("2. Customer (Premium)\n");
    printf("3. ATM Operator\n");
    
    // Only super admin can add other admins
    if (userType == USER_ADMIN_SUPER) {
        printf("4. Admin\n");
        printf("5. Admin (Super)\n");
    }
    
    int maxChoice = (userType == USER_ADMIN_SUPER) ? 5 : 3;
    int userTypeChoice = getInteger("Select User Type", 1, maxChoice);
    
    char newUsername[50];
    char fullName[100];
    char email[100];
    char phone[20];
    
    getString("Enter Username: ", newUsername, sizeof(newUsername));
    getString("Enter Full Name: ", fullName, sizeof(fullName));
    getString("Enter Email: ", email, sizeof(email));
    getString("Enter Phone: ", phone, sizeof(phone));
    
    // Additional fields based on user type
    if (userTypeChoice == 1 || userTypeChoice == 2) { // Customer
        char address[200];
        char idType[50];
        char idNumber[50];
        
        getString("Enter Address: ", address, sizeof(address));
        
        printf("\nSelect ID Type:\n");
        printf("1. PAN Card\n");
        printf("2. Aadhaar Card\n");
        printf("3. Passport\n");
        printf("4. Driving License\n");
        
        int idTypeChoice = getInteger("Select ID Type", 1, 4);
        
        switch (idTypeChoice) {
            case 1: strcpy(idType, "PAN Card"); break;
            case 2: strcpy(idType, "Aadhaar Card"); break;
            case 3: strcpy(idType, "Passport"); break;
            case 4: strcpy(idType, "Driving License"); break;
        }
        
        getString("Enter ID Number: ", idNumber, sizeof(idNumber));
        
        printf("\nWould you like to create accounts for this customer?\n");
        printf("1. Savings Account\n");
        printf("2. Current Account\n");
        printf("3. Both\n");
        printf("4. None (Add later)\n");
        
        int accountChoice = getInteger("Select Option", 1, 4);
    }
    
    // Generate temporary password
    char tempPassword[10];
    for (int i = 0; i < 8; i++) {
        if (i % 3 == 0)
            tempPassword[i] = 'A' + (rand() % 26);
        else if (i % 3 == 1)
            tempPassword[i] = 'a' + (rand() % 26);
        else
            tempPassword[i] = '0' + (rand() % 10);
    }
    tempPassword[8] = '\0';
    
    printf("\nUser Details Summary:\n");
    printf("Username: %s\n", newUsername);
    printf("Full Name: %s\n", fullName);
    printf("Email: %s\n", email);
    printf("Phone: %s\n", phone);
    
    const char *userTypeString;
    switch (userTypeChoice) {
        case 1: userTypeString = "Customer (Regular)"; break;
        case 2: userTypeString = "Customer (Premium)"; break;
        case 3: userTypeString = "ATM Operator"; break;
        case 4: userTypeString = "Admin"; break;
        case 5: userTypeString = "Admin (Super)"; break;
        default: userTypeString = "Unknown"; break;
    }
    
    printf("User Type: %s\n", userTypeString);
    printf("Temporary Password: %s\n", tempPassword);
    
    if (getConfirmation("\nConfirm Add User")) {
        // In a real application, this would update the database
        printSuccess("User added successfully!");
        printf("User ID: %d\n", 1000 + rand() % 9000);
        
        LOG_INFO("Admin %s added new user %s (%s)", username, newUsername, userTypeString);
    } else {
        printInfo("Add user cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief Edit user
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void editUser(const char *username, int userType) {
    clearScreen();
    printHeader("EDIT USER");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    int userId = getInteger("Enter User ID to edit", 1000, 9999);
    
    // In a real application, this would fetch user details from the database
    // For demo purposes, we'll use mock data
    printf("\nCurrent User Details:\n");
    printf("User ID: %d\n", userId);
    printf("Username: john_doe\n");
    printf("Full Name: John Doe\n");
    printf("Email: john.doe@example.com\n");
    printf("Phone: +91-9876543210\n");
    printf("Status: Active\n");
    
    printf("\nSelect field to edit:\n");
    printf("1. Full Name\n");
    printf("2. Email\n");
    printf("3. Phone\n");
    printf("4. Status\n");
    
    int fieldChoice = getInteger("Select Field", 1, 4);
    
    char newValue[100];
    
    switch (fieldChoice) {
        case 1:
            getString("Enter new Full Name: ", newValue, sizeof(newValue));
            break;
            
        case 2:
            getString("Enter new Email: ", newValue, sizeof(newValue));
            break;
            
        case 3:
            getString("Enter new Phone: ", newValue, sizeof(newValue));
            break;
            
        case 4:
            printf("\nSelect new Status:\n");
            printf("1. Active\n");
            printf("2. Inactive\n");
            printf("3. Locked\n");
            
            int statusChoice = getInteger("Select Status", 1, 3);
            
            switch (statusChoice) {
                case 1: strcpy(newValue, "Active"); break;
                case 2: strcpy(newValue, "Inactive"); break;
                case 3: strcpy(newValue, "Locked"); break;
            }
            break;
    }
    
    const char *fieldName;
    switch (fieldChoice) {
        case 1: fieldName = "Full Name"; break;
        case 2: fieldName = "Email"; break;
        case 3: fieldName = "Phone"; break;
        case 4: fieldName = "Status"; break;
        default: fieldName = "Field"; break;
    }
    
    if (getConfirmation("\nConfirm Edit User")) {
        // In a real application, this would update the database
        printSuccess("User updated successfully!");
        
        LOG_INFO("Admin %s updated %s to '%s' for user ID %d", 
                 username, fieldName, newValue, userId);
    } else {
        printInfo("Edit user cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief Disable user
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void disableUser(const char *username, int userType) {
    clearScreen();
    printHeader("DISABLE USER");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    int userId = getInteger("Enter User ID to disable", 1000, 9999);
    
    // In a real application, this would fetch user details from the database
    // For demo purposes, we'll use mock data
    printf("\nUser Details:\n");
    printf("User ID: %d\n", userId);
    printf("Username: john_doe\n");
    printf("Full Name: John Doe\n");
    printf("Email: john.doe@example.com\n");
    printf("User Type: Customer (Regular)\n");
    printf("Status: Active\n");
    
    printf("\nReason for disabling:\n");
    printf("1. Account Dormant\n");
    printf("2. Security Concern\n");
    printf("3. User Request\n");
    printf("4. Other\n");
    
    int reasonChoice = getInteger("Select Reason", 1, 4);
    
    char reason[200];
    if (reasonChoice == 4) {
        getString("Enter specific reason: ", reason, sizeof(reason));
    } else {
        switch (reasonChoice) {
            case 1: strcpy(reason, "Account Dormant"); break;
            case 2: strcpy(reason, "Security Concern"); break;
            case 3: strcpy(reason, "User Request"); break;
            default: strcpy(reason, "Not specified"); break;
        }
    }
    
    if (getConfirmation("\nConfirm disable user")) {
        // In a real application, this would update the database
        printSuccess("User disabled successfully!");
        
        LOG_INFO("Admin %s disabled user ID %d. Reason: %s", 
                 username, userId, reason);
    } else {
        printInfo("Disable user cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief Reset user password
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void resetUserPassword(const char *username, int userType) {
    clearScreen();
    printHeader("RESET USER PASSWORD");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    int userId = getInteger("Enter User ID to reset password", 1000, 9999);
    
    // In a real application, this would fetch user details from the database
    // For demo purposes, we'll use mock data
    printf("\nUser Details:\n");
    printf("User ID: %d\n", userId);
    printf("Username: john_doe\n");
    printf("Full Name: John Doe\n");
    printf("Email: john.doe@example.com\n");
    printf("Phone: +91-9876543210\n");
    
    printf("\nReset Options:\n");
    printf("1. Generate Temporary Password\n");
    printf("2. Send OTP to Registered Email\n");
    printf("3. Send OTP to Registered Phone\n");
    
    int resetOption = getInteger("Select Reset Option", 1, 3);
    
    if (getConfirmation("\nConfirm password reset")) {
        // In a real application, this would update the database
        printSuccess("Password reset initiated successfully!");
        
        if (resetOption == 1) {
            // Generate temporary password
            char tempPassword[10];
            for (int i = 0; i < 8; i++) {
                if (i % 3 == 0)
                    tempPassword[i] = 'A' + (rand() % 26);
                else if (i % 3 == 1)
                    tempPassword[i] = 'a' + (rand() % 26);
                else
                    tempPassword[i] = '0' + (rand() % 10);
            }
            tempPassword[8] = '\0';
            
            printf("Temporary Password: %s\n", tempPassword);
            printf("The user will be prompted to change this password on next login.\n");
        } else if (resetOption == 2) {
            printf("OTP sent to registered email: j***@example.com\n");
            printf("The user will receive instructions to complete the reset process.\n");
        } else {
            printf("OTP sent to registered phone: +91-98***210\n");
            printf("The user will receive instructions to complete the reset process.\n");
        }
        
        LOG_INFO("Admin %s reset password for user ID %d", username, userId);
    } else {
        printInfo("Password reset cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief Run the system configuration menu
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void runSystemConfigMenu(const char *username, int userType) {
    bool running = true;
    
    while (running) {
        clearScreen();
        printHeader("SYSTEM CONFIGURATION");
        
        printf("\n");
        printf("1. General Settings\n");
        printf("2. Transaction Limits\n");
        printf("3. Fee Configuration\n");
        printf("4. ATM Configuration\n");
        printf("5. System Parameters\n");
        printf("6. Return to Admin Menu\n");
        
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
        
        // In a real implementation, these would be separate functions
        switch (choice) {
            case 1: // General Settings
                printInfo("General Settings feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 2: // Transaction Limits
                printInfo("Transaction Limits feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 3: // Fee Configuration
                printInfo("Fee Configuration feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 4: // ATM Configuration
                printInfo("ATM Configuration feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 5: // System Parameters
                printInfo("System Parameters feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 6: // Return to Admin Menu
                running = false;
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}

/**
 * @brief Run the reports menu
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void runReportsMenu(const char *username, int userType) {
    bool running = true;
    
    while (running) {
        clearScreen();
        printHeader("REPORTS");
        
        printf("\n");
        printf("1. Transaction Reports\n");
        printf("2. User Activity Reports\n");
        printf("3. ATM Performance Reports\n");
        printf("4. System Usage Reports\n");
        printf("5. Return to Admin Menu\n");
        
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
        
        // In a real implementation, these would be separate functions
        switch (choice) {
            case 1: // Transaction Reports
                printInfo("Transaction Reports feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 2: // User Activity Reports
                printInfo("User Activity Reports feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 3: // ATM Performance Reports
                printInfo("ATM Performance Reports feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 4: // System Usage Reports
                printInfo("System Usage Reports feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 5: // Return to Admin Menu
                running = false;
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}

/**
 * @brief Run the security management menu
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void runSecurityManagementMenu(const char *username, int userType) {
    bool running = true;
    
    while (running) {
        clearScreen();
        printHeader("SECURITY MANAGEMENT");
        
        printf("\n");
        printf("1. Access Control\n");
        printf("2. Security Policies\n");
        printf("3. System Logs\n");
        printf("4. Return to Admin Menu\n");
        
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
        
        // In a real implementation, these would be separate functions
        switch (choice) {
            case 1: // Access Control
                printInfo("Access Control feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 2: // Security Policies
                printInfo("Security Policies feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 3: // System Logs
                printInfo("System Logs feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 4: // Return to Admin Menu
                running = false;
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}

/**
 * @brief Run the audit menu
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void runAuditMenu(const char *username, int userType) {
    bool running = true;
    
    while (running) {
        clearScreen();
        printHeader("AUDIT LOGS");
        
        printf("\n");
        printf("1. User Audit Logs\n");
        printf("2. Transaction Audit Logs\n");
        printf("3. System Audit Logs\n");
        printf("4. Export Audit Logs\n");
        printf("5. Return to Admin Menu\n");
        
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
        
        // In a real implementation, these would be separate functions
        switch (choice) {
            case 1: // User Audit Logs
                printInfo("User Audit Logs feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 2: // Transaction Audit Logs
                printInfo("Transaction Audit Logs feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 3: // System Audit Logs
                printInfo("System Audit Logs feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 4: // Export Audit Logs
                printInfo("Export Audit Logs feature not implemented in this demo.");
                pauseExecution();
                break;
                
            case 5: // Return to Admin Menu
                running = false;
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}
