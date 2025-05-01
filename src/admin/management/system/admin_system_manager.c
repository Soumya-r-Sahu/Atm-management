#include "../../../../include/admin/management/system/admin_system_manager.h"
#include "../../../../include/common/utils/logger.h"
#include "../../../../include/common/database/database.h"
#include "../../../../include/common/utils/path_manager.h"
#include "../../../../include/common/config/config_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// Toggle ATM/Banking service mode (online/offline)
bool toggle_service_mode(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        writeErrorLog("Unauthorized attempt to toggle service mode");
        return false;
    }
    
    printf("\n=======================================\n");
    printf("=      TOGGLE ATM/BANKING STATUS     =\n");
    printf("=======================================\n");
    
    // Get current service status
    bool serviceActive = false;
    if (!getServiceStatus(&serviceActive)) {
        printf("Error: Failed to retrieve current service status.\n");
        return false;
    }
    
    printf("Current service status: %s\n\n", serviceActive ? "ONLINE" : "OFFLINE");
    printf("Do you want to switch to %s mode? (y/n): ", serviceActive ? "OFFLINE" : "ONLINE");
    
    char confirm[5];
    if (!fgets(confirm, sizeof(confirm), stdin)) {
        return false;
    }
    
    if (tolower(confirm[0]) == 'y') {
        bool newStatus = !serviceActive;
        
        if (setServiceStatus(newStatus)) {
            printf("\nService status successfully changed to: %s\n", 
                  newStatus ? "ONLINE" : "OFFLINE");
            
            char logMsg[100];
            sprintf(logMsg, "Service status changed to %s by admin %s", 
                   newStatus ? "ONLINE" : "OFFLINE", admin->username);
            writeAuditLog("SYSTEM", logMsg);
            
            if (!newStatus) {
                printf("\nWARNING: All ATM and banking services are now offline.\n");
                printf("Customers will not be able to perform any transactions.\n");
            } else {
                printf("\nAll ATM and banking services are now back online.\n");
                printf("Customers can resume normal transactions.\n");
            }
            return true;
        } else {
            printf("\nError: Failed to update service status. Please try again.\n");
            writeErrorLog("Failed to update service status");
            return false;
        }
    } else {
        printf("\nOperation cancelled. Service status remains %s.\n", 
              serviceActive ? "ONLINE" : "OFFLINE");
        return true;
    }
}

// View system log files
bool view_system_logs(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        writeErrorLog("Unauthorized attempt to view system logs");
        return false;
    }
    
    printf("\n=======================================\n");
    printf("=          VIEW SYSTEM LOGS          =\n");
    printf("=======================================\n");
    
    printf("Select log file to view:\n");
    printf("1. Error log\n");
    printf("2. Audit log\n");
    printf("3. Transaction log\n");
    printf("4. Withdrawals log\n");
    printf("5. Return to main menu\n");
    printf("Enter choice: ");
    
    char choice_str[5];
    if (!fgets(choice_str, sizeof(choice_str), stdin)) {
        return false;
    }
    
    int choice = atoi(choice_str);
    const char* logFilePath = NULL;
    const char* logType = NULL;
    
    switch (choice) {
        case 1:
            logFilePath = getErrorLogFilePath();
            logType = "Error";
            break;
        case 2:
            logFilePath = getAuditLogFilePath();
            logType = "Audit";
            break;
        case 3:
            logFilePath = getTransactionLogFilePath();
            logType = "Transaction";
            break;
        case 4:
            logFilePath = getWithdrawalLogFilePath();
            logType = "Withdrawals";
            break;
        case 5:
            return true;
        default:
            printf("Invalid option. Please try again.\n");
            return false;
    }
    
    if (!logFilePath) {
        printf("Error: Could not locate log file path.\n");
        return false;
    }
    
    // Open the log file
    FILE* file = fopen(logFilePath, "r");
    if (!file) {
        printf("Error: Could not open the %s log file.\n", logType);
        return false;
    }
    
    printf("\n============= %s LOG =============\n", logType);
    
    // Count total lines in file first to determine if we need to show most recent only
    int totalLines = 0;
    char countBuffer[512];
    while (fgets(countBuffer, sizeof(countBuffer), file) != NULL) {
        totalLines++;
    }
    
    // Reset file position to beginning
    rewind(file);
    
    const int MAX_LINES_TO_DISPLAY = 100;
    int linesToSkip = 0;
    
    if (totalLines > MAX_LINES_TO_DISPLAY) {
        linesToSkip = totalLines - MAX_LINES_TO_DISPLAY;
        printf("(Showing most recent %d entries out of %d total)\n\n", 
              MAX_LINES_TO_DISPLAY, totalLines);
        
        // Skip lines we don't want to display
        for (int i = 0; i < linesToSkip; i++) {
            if (fgets(countBuffer, sizeof(countBuffer), file) == NULL) {
                break;
            }
        }
    }
    
    // Display the log contents
    char line[512];
    int lineNum = linesToSkip + 1;
    while (fgets(line, sizeof(line), file) != NULL) {
        // Add a line number to each log entry for easier reference
        printf("%4d: %s", lineNum++, line);
    }
    
    printf("\n=========== END OF LOG ===========\n\n");
    
    fclose(file);
    
    // Wait for user to press enter before returning
    printf("Press Enter to continue...");
    getchar();
    
    return true;
}

// Manage system configuration settings
bool manage_system_config(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        writeErrorLog("Unauthorized attempt to manage system configuration");
        return false;
    }
    
    printf("\n=======================================\n");
    printf("=        SYSTEM CONFIGURATION        =\n");
    printf("=======================================\n");
    
    // Open the configuration file
    FILE* file = fopen(getSystemConfigFilePath(), "r");
    if (!file) {
        printf("Error: Could not open system configuration file.\n");
        return false;
    }
    
    // Read and display all configuration entries
    printf("Current Configuration Settings:\n");
    printf("----------------------------------------------------------------------------------\n");
    printf("| %-25s | %-20s | %-35s |\n", "Parameter", "Value", "Description");
    printf("----------------------------------------------------------------------------------\n");
    
    char line[512];
    int configCount = 0;
    char configNames[50][64]; // Store up to 50 configuration names
    
    // Skip header lines (first two lines)
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file) && configCount < 50) {
        char name[64] = {0}, value[64] = {0}, description[256] = {0};
        
        // Parse configuration line
        if (sscanf(line, "| %63[^|] | %63[^|] | %255[^|]", name, value, description) >= 3) {
            // Trim whitespace
            char* trimmed_name = name;
            while (*trimmed_name == ' ') trimmed_name++;
            char* end = trimmed_name + strlen(trimmed_name) - 1;
            while (end > trimmed_name && (*end == ' ' || *end == '\n')) *end-- = '\0';
            
            char* trimmed_value = value;
            while (*trimmed_value == ' ') trimmed_value++;
            end = trimmed_value + strlen(trimmed_value) - 1;
            while (end > trimmed_value && (*end == ' ' || *end == '\n')) *end-- = '\0';
            
            char* trimmed_desc = description;
            while (*trimmed_desc == ' ') trimmed_desc++;
            end = trimmed_desc + strlen(trimmed_desc) - 1;
            while (end > trimmed_desc && (*end == ' ' || *end == '\n')) *end-- = '\0';
            
            // Store the config name for later use
            strcpy(configNames[configCount], trimmed_name);
            
            // Display the configuration
            printf("| %-25s | %-20s | %-35s |\n", 
                  trimmed_name, trimmed_value, trimmed_desc);
            
            configCount++;
        }
    }
    
    printf("----------------------------------------------------------------------------------\n\n");
    fclose(file);
    
    // Offer options to edit or add configuration
    printf("Options:\n");
    printf("1. Edit an existing configuration\n");
    printf("2. Add a new configuration\n");
    printf("3. Return to main menu\n");
    printf("Enter choice: ");
    
    char choice_str[5];
    if (!fgets(choice_str, sizeof(choice_str), stdin)) {
        return false;
    }
    
    int choice = atoi(choice_str);
    
    switch (choice) {
        case 1: {
            // Edit existing configuration
            printf("\nEnter the parameter name to edit: ");
            char paramName[64];
            if (!fgets(paramName, sizeof(paramName), stdin)) {
                return false;
            }
            paramName[strcspn(paramName, "\n")] = 0; // Remove newline
            
            // Check if parameter exists
            bool found = false;
            for (int i = 0; i < configCount; i++) {
                if (strcmp(configNames[i], paramName) == 0) {
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                printf("Error: Parameter '%s' not found.\n", paramName);
                return false;
            }
            
            // Get current value
            const char* currentValue = get_config_value(paramName);
            if (currentValue) {
                printf("Current value: %s\n", currentValue);
            }
            
            printf("Enter new value: ");
            char newValue[64];
            if (!fgets(newValue, sizeof(newValue), stdin)) {
                return false;
            }
            newValue[strcspn(newValue, "\n")] = 0; // Remove newline
            
            // Update the configuration
            if (setConfigValue(paramName, newValue)) {
                printf("\nConfiguration updated successfully!\n");
                
                char logMsg[200];
                sprintf(logMsg, "Configuration '%s' updated to '%s' by %s", 
                       paramName, newValue, admin->username);
                writeAuditLog("CONFIG", logMsg);
                
                return true;
            } else {
                printf("\nError: Failed to update configuration. Please try again.\n");
                writeErrorLog("Failed to update configuration");
                return false;
            }
            break;
        }
        case 2: {
            // Add new configuration
            printf("\nEnter new parameter name: ");
            char paramName[64];
            if (!fgets(paramName, sizeof(paramName), stdin)) {
                return false;
            }
            paramName[strcspn(paramName, "\n")] = 0; // Remove newline
            
            printf("Enter parameter value: ");
            char paramValue[64];
            if (!fgets(paramValue, sizeof(paramValue), stdin)) {
                return false;
            }
            paramValue[strcspn(paramValue, "\n")] = 0; // Remove newline
            
            printf("Enter parameter description: ");
            char paramDesc[256];
            if (!fgets(paramDesc, sizeof(paramDesc), stdin)) {
                return false;
            }
            paramDesc[strcspn(paramDesc, "\n")] = 0; // Remove newline
            
            // Add the new configuration
            if (add_config(paramName, paramValue, paramDesc)) {
                printf("\nNew configuration added successfully!\n");
                
                char logMsg[200];
                sprintf(logMsg, "New configuration '%s' added with value '%s' by %s", 
                       paramName, paramValue, admin->username);
                writeAuditLog("CONFIG", logMsg);
                
                return true;
            } else {
                printf("\nError: Failed to add configuration. Please try again.\n");
                writeErrorLog("Failed to add configuration");
                return false;
            }
            break;
        }
        case 3:
            return true;
        default:
            printf("Invalid option. Please try again.\n");
            return false;
    }
    
    return true;
}

// Back up system data files
bool backup_system_data(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        writeErrorLog("Unauthorized attempt to backup system data");
        return false;
    }
    
    printf("\n=======================================\n");
    printf("=          BACKUP SYSTEM DATA        =\n");
    printf("=======================================\n");
    
    // Create a backup directory with timestamp
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char backupDir[256];
    char timestamp[64];
    
    strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", tm_now);
    snprintf(backupDir, sizeof(backupDir), "backup_%s", timestamp);
    
    // Create the backup directory
    if (mkdir(backupDir, 0755) != 0) {
        printf("Error: Failed to create backup directory.\n");
        writeErrorLog("Failed to create backup directory");
        return false;
    }
    
    printf("Creating backup in directory: %s\n\n", backupDir);
    
    // Get the data directory
    char dataDir[256];
    strcpy(dataDir, "data"); // Assuming data files are in the data directory
    
    // Backup log files
    char logDir[256];
    strcpy(logDir, "logs"); // Assuming log files are in the logs directory
    
    // Open the data directory
    DIR *data_dir = opendir(dataDir);
    if (!data_dir) {
        printf("Error: Could not open data directory.\n");
        writeErrorLog("Failed to open data directory for backup");
        return false;
    }
    
    // Create data directory inside backup
    char backupDataDir[512];
    snprintf(backupDataDir, sizeof(backupDataDir), "%s/data", backupDir);
    if (mkdir(backupDataDir, 0755) != 0) {
        printf("Error: Failed to create data backup directory.\n");
        writeErrorLog("Failed to create data backup directory");
        closedir(data_dir);
        return false;
    }
    
    printf("Backing up data files...\n");
    
    // Copy each file in the data directory
    struct dirent *entry;
    int files_copied = 0;
    int files_failed = 0;
    
    while ((entry = readdir(data_dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Build source and destination paths
        char srcPath[512];
        char dstPath[512];
        snprintf(srcPath, sizeof(srcPath), "%s/%s", dataDir, entry->d_name);
        snprintf(dstPath, sizeof(dstPath), "%s/%s", backupDataDir, entry->d_name);
        
        // Open source file
        FILE *srcFile = fopen(srcPath, "rb");
        if (!srcFile) {
            printf("Warning: Could not open source file %s\n", srcPath);
            files_failed++;
            continue;
        }
        
        // Open destination file
        FILE *dstFile = fopen(dstPath, "wb");
        if (!dstFile) {
            printf("Warning: Could not create destination file %s\n", dstPath);
            fclose(srcFile);
            files_failed++;
            continue;
        }
        
        // Copy file contents
        char buffer[4096];
        size_t bytesRead;
        
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
            if (fwrite(buffer, 1, bytesRead, dstFile) != bytesRead) {
                printf("Warning: Failed to write to %s\n", dstPath);
                files_failed++;
                fclose(srcFile);
                fclose(dstFile);
                continue;
            }
        }
        
        // Close files
        fclose(srcFile);
        fclose(dstFile);
        files_copied++;
        printf("  Backed up: %s\n", entry->d_name);
    }
    
    closedir(data_dir);
    
    // Now backup log files similarly
    DIR *log_dir = opendir(logDir);
    if (log_dir) {
        // Create log directory inside backup
        char backupLogDir[512];
        snprintf(backupLogDir, sizeof(backupLogDir), "%s/logs", backupDir);
        if (mkdir(backupLogDir, 0755) == 0) {
            printf("\nBacking up log files...\n");
            
            // Copy each file in the log directory
            while ((entry = readdir(log_dir)) != NULL) {
                // Skip . and ..
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }
                
                // Build source and destination paths
                char srcPath[512];
                char dstPath[512];
                snprintf(srcPath, sizeof(srcPath), "%s/%s", logDir, entry->d_name);
                snprintf(dstPath, sizeof(dstPath), "%s/%s", backupLogDir, entry->d_name);
                
                // Copy file
                FILE *srcFile = fopen(srcPath, "rb");
                if (!srcFile) {
                    printf("Warning: Could not open source file %s\n", srcPath);
                    files_failed++;
                    continue;
                }
                
                FILE *dstFile = fopen(dstPath, "wb");
                if (!dstFile) {
                    printf("Warning: Could not create destination file %s\n", dstPath);
                    fclose(srcFile);
                    files_failed++;
                    continue;
                }
                
                // Copy file contents
                char buffer[4096];
                size_t bytesRead;
                
                while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
                    if (fwrite(buffer, 1, bytesRead, dstFile) != bytesRead) {
                        printf("Warning: Failed to write to %s\n", dstPath);
                        files_failed++;
                        fclose(srcFile);
                        fclose(dstFile);
                        continue;
                    }
                }
                
                // Close files
                fclose(srcFile);
                fclose(dstFile);
                files_copied++;
                printf("  Backed up: %s\n", entry->d_name);
            }
        }
        closedir(log_dir);
    }
    
    printf("\nBackup completed!\n");
    printf("Files successfully backed up: %d\n", files_copied);
    
    if (files_failed > 0) {
        printf("Files failed: %d\n", files_failed);
    }
    
    // Create a backup info file
    char infoPath[512];
    snprintf(infoPath, sizeof(infoPath), "%s/backup_info.txt", backupDir);
    FILE *infoFile = fopen(infoPath, "w");
    
    if (infoFile) {
        fprintf(infoFile, "Backup created: %s\n", ctime(&now));
        fprintf(infoFile, "Created by: %s\n", admin->username);
        fprintf(infoFile, "Files backed up: %d\n", files_copied);
        fprintf(infoFile, "Files failed: %d\n", files_failed);
        fclose(infoFile);
    }
    
    char logMsg[200];
    sprintf(logMsg, "System backup created in '%s' by %s. Files: %d success, %d failed", 
           backupDir, admin->username, files_copied, files_failed);
    writeAuditLog("SYSTEM", logMsg);
    
    return true;
}