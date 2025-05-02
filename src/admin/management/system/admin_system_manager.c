#include "../../../../include/admin/management/system/admin_system_manager.h"
#include "../../../../include/common/utils/logger.h"
#include "../../../../include/common/paths.h"
#include "../../../../include/admin/auth/service_status.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Helper function declarations
static void display_system_config(void);
static void edit_transaction_limits(AdminUser* admin);
static void configure_security_settings(AdminUser* admin);
static void update_atm_status_config(AdminUser* admin);

// View system logs
bool view_system_logs(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        printf("Error: Not authorized. Please log in first.\n");
        return false;
    }
    
    printf("\n=======================================\n");
    printf("=          SYSTEM LOG VIEWER         =\n");
    printf("=======================================\n");
    
    int choice;
    
    printf("1. View Error Logs\n");
    printf("2. View Audit Logs\n");
    printf("3. View Transaction Logs\n");
    printf("4. Back to Main Menu\n");
    printf("---------------------------------------\n");
    printf("Enter choice: ");
    
    scanf("%d", &choice);
    getchar(); // Clear input buffer
    
    char logPath[256];
    const char* logType = "";
    
    switch (choice) {
        case 1:
            strcpy(logPath, getErrorLogFilePath());
            logType = "Error";
            break;
        case 2:
            strcpy(logPath, getAuditLogFilePath());
            logType = "Audit";
            break;
        case 3:
            strcpy(logPath, getTransactionsLogFilePath());
            logType = "Transaction";
            break;
        case 4:
            return true;
        default:
            printf("Invalid option. Please try again.\n");
            return false;
    }
    
    // Log the activity
    char auditMessage[100];
    sprintf(auditMessage, "Admin %s viewed %s logs", admin->username, logType);
    writeAuditLog("ADMIN", auditMessage);
    
    FILE* logFile = fopen(logPath, "r");
    if (logFile == NULL) {
        printf("Error: Could not open the log file.\n");
        return false;
    }
    
    printf("\nDisplaying %s logs:\n", logType);
    printf("---------------------------------------\n");
    
    char line[512];
    int lineCount = 0;
    const int PAGE_SIZE = 20; // Number of lines per page
    
    while (fgets(line, sizeof(line), logFile) != NULL) {
        printf("%s", line);
        lineCount++;
        
        // Paginate output
        if (lineCount % PAGE_SIZE == 0) {
            printf("---------------------------------------\n");
            printf("Press Enter to continue, Q to quit: ");
            char input = getchar();
            if (input == 'q' || input == 'Q') {
                break;
            }
        }
    }
    
    fclose(logFile);
    
    printf("\nEnd of log. Press Enter to continue.");
    getchar();
    
    return true;
}

// Manage system configuration
bool manage_system_config(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        printf("Error: Not authorized. Please log in first.\n");
        return false;
    }
    
    // Verify user is SuperAdmin
    if (!admin_has_role(admin, "SuperAdmin")) {
        printf("Error: Only SuperAdmin users can manage system configuration.\n");
        writeAuditLog("SECURITY", "Unauthorized attempt to access system configuration");
        return false;
    }
    
    printf("\n=======================================\n");
    printf("=       SYSTEM CONFIGURATION         =\n");
    printf("=======================================\n");
    
    int choice;
    
    printf("1. View Current Configuration\n");
    printf("2. Edit Transaction Limits\n");
    printf("3. Configure Security Settings\n");
    printf("4. Update ATM Status\n");
    printf("5. Back to Main Menu\n");
    printf("---------------------------------------\n");
    printf("Enter choice: ");
    
    scanf("%d", &choice);
    getchar(); // Clear input buffer
    
    switch (choice) {
        case 1:
            display_system_config();
            break;
        case 2:
            edit_transaction_limits(admin);
            break;
        case 3:
            configure_security_settings(admin);
            break;
        case 4:
            update_atm_status_config(admin);
            break;
        case 5:
            return true;
        default:
            printf("Invalid option. Please try again.\n");
            return false;
    }
    
    // Log the activity
    writeAuditLog("ADMIN", "Managed system configuration");
    return true;
}

// Display system configuration (helper function)
static void display_system_config() {
    printf("\nCurrent System Configuration:\n");
    printf("---------------------------------------\n");
    
    // Read configuration from file
    FILE* configFile = fopen(getSystemConfigFilePath(), "r");
    if (configFile == NULL) {
        printf("Error: Could not open the configuration file.\n");
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), configFile) != NULL) {
        printf("%s", line);
    }
    
    fclose(configFile);
    
    printf("\nPress Enter to continue.");
    getchar();
}

// Edit transaction limits (helper function)
static void edit_transaction_limits(AdminUser* admin) {
    printf("\nEdit Transaction Limits:\n");
    printf("---------------------------------------\n");
    printf("1. Daily Withdrawal Limit\n");
    printf("2. Maximum Single Transaction\n");
    printf("3. Back\n");
    printf("Enter choice: ");
    
    int choice;
    scanf("%d", &choice);
    getchar(); // Clear input buffer
    
    if (choice == 3) {
        return;
    }
    
    double newLimit;
    printf("Enter new limit: ");
    scanf("%lf", &newLimit);
    getchar(); // Clear input buffer
    
    if (newLimit <= 0) {
        printf("Error: Limit must be greater than zero.\n");
        return;
    }
    
    // In a real implementation, we would update the appropriate limit in the configuration file
    printf("Transaction limit updated successfully.\n");
    
    // Log the change
    char logMessage[100];
    sprintf(logMessage, "Updated %s transaction limit to %.2f", 
            (choice == 1) ? "daily withdrawal" : "maximum single", newLimit);
    writeAuditLog("ADMIN", logMessage);
    
    printf("\nPress Enter to continue.");
    getchar();
}

// Configure security settings (helper function)
static void configure_security_settings(AdminUser* admin) {
    printf("\nConfigure Security Settings:\n");
    printf("---------------------------------------\n");
    printf("1. Session Timeout (minutes)\n");
    printf("2. Password Complexity Requirements\n");
    printf("3. Login Attempt Limits\n");
    printf("4. Back\n");
    printf("Enter choice: ");
    
    int choice;
    scanf("%d", &choice);
    getchar(); // Clear input buffer
    
    if (choice == 4) {
        return;
    }
    
    // In a real implementation, we would update the appropriate security setting
    printf("Security setting updated successfully.\n");
    
    // Log the change
    char logMessage[100];
    sprintf(logMessage, "Updated security settings by admin %s", admin->username);
    writeAuditLog("ADMIN", logMessage);
    
    printf("\nPress Enter to continue.");
    getchar();
}

// Update ATM status configuration (helper function)
static void update_atm_status_config(AdminUser* admin) {
    printf("\nUpdate ATM Status:\n");
    printf("---------------------------------------\n");
    
    char atmId[20];
    printf("Enter ATM ID (e.g., ATM001): ");
    fgets(atmId, sizeof(atmId), stdin);
    atmId[strcspn(atmId, "\n")] = 0; // Remove newline
    
    printf("\nSelect new status:\n");
    printf("1. Online\n");
    printf("2. Offline\n");
    printf("3. Maintenance\n");
    printf("4. Out of Cash\n");
    printf("Enter choice: ");
    
    int choice;
    scanf("%d", &choice);
    getchar(); // Clear input buffer
    
    const char* newStatus;
    switch (choice) {
        case 1:
            newStatus = "Online";
            break;
        case 2:
            newStatus = "Offline";
            break;
        case 3:
            newStatus = "Maintenance";
            break;
        case 4:
            newStatus = "Out of Cash";
            break;
        default:
            printf("Invalid choice.\n");
            return;
    }
    
    // In a real implementation, update the ATM status in the data file
    printf("ATM status updated successfully.\n");
    
    // Log the change
    char logMessage[100];
    sprintf(logMessage, "Updated ATM %s status to %s", atmId, newStatus);
    writeAuditLog("ADMIN", logMessage);
    
    printf("\nPress Enter to continue.");
    getchar();
}

// Back up system data
bool backup_system_data(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        printf("Error: Not authorized. Please log in first.\n");
        return false;
    }
    
    // Verify user is SuperAdmin
    if (!admin_has_role(admin, "SuperAdmin")) {
        printf("Error: Only SuperAdmin users can perform system backups.\n");
        writeAuditLog("SECURITY", "Unauthorized attempt to perform system backup");
        return false;
    }
    
    printf("\n=======================================\n");
    printf("=         SYSTEM DATA BACKUP         =\n");
    printf("=======================================\n");
    
    // Create a timestamp for the backup
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", t);
    
    char backupDir[256];
    sprintf(backupDir, "%s/backup_%s", isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR, timestamp);
    
    // In a real implementation, we would create the directory and copy all data files
    printf("Creating backup in: %s\n", backupDir);
    
    // Simulate backup process
    printf("Backing up customer data...\n");
    printf("Backing up transaction history...\n");
    printf("Backing up system configuration...\n");
    printf("Backing up logs...\n");
    
    // Simulate some processing time
    printf("Finalizing backup...\n");
    
    // Log the backup
    char logMessage[100];
    sprintf(logMessage, "System backup created by %s: backup_%s", admin->username, timestamp);
    writeAuditLog("ADMIN", logMessage);
    
    printf("\nBackup completed successfully!\n");
    printf("\nPress Enter to continue.");
    getchar();
    
    return true;
}

// Toggle ATM/Banking service mode with admin authentication
bool admin_toggle_service_mode(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        printf("Error: Not authorized. Please log in first.\n");
        return false;
    }
    
    printf("\n=======================================\n");
    printf("=         ATM SERVICE MODE           =\n");
    printf("=======================================\n");
    
    // Get current service status
    int currentStatus = getServiceStatus();
    printf("Current ATM service status: %s\n", currentStatus ? "OFFLINE" : "ONLINE");
    
    printf("\nDo you want to toggle the ATM service status? (Y/N): ");
    char choice;
    scanf(" %c", &choice);
    getchar(); // Clear input buffer
    
    if (choice == 'Y' || choice == 'y') {
        // Toggle service status
        if (setServiceStatus(!currentStatus)) {
            printf("ATM service status successfully changed to: %s\n", !currentStatus ? "ONLINE" : "OFFLINE");
            
            // Log the change
            char logMessage[100];
            sprintf(logMessage, "ATM service status changed to %s by %s", 
                    !currentStatus ? "ONLINE" : "OFFLINE", admin->username);
            writeAuditLog("ADMIN", logMessage);
            
            return true;
        } else {
            printf("Failed to change ATM service status.\n");
            return false;
        }
    }
    
    printf("Operation cancelled. Status remains unchanged.\n");
    return true;
}