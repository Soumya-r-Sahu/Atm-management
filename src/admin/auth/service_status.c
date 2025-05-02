#include "../../../include/admin/auth/service_status.h"
#include "../../../include/common/utils/logger.h"
#include "../../../include/common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Get current service status (0 = Online, 1 = Offline)
int getServiceStatus() {
    const char* filePath = getStatusFilePath();
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        // If file doesn't exist, create it with default status as "Online"
        FILE *newFile = fopen(filePath, "w");
        if (newFile != NULL) {
            fprintf(newFile, "Status: Online");
            fclose(newFile);
        }
        
        writeInfoLog("Created new status file with default status: Online");
        return 0; // Default to Online
    }

    char status[50] = {0}; // Increased buffer size for safety
    if (fgets(status, sizeof(status), file) != NULL) {
        fclose(file);
        // Case-insensitive check for "Offline" for robustness
        if (strstr(status, "Offline") != NULL || strstr(status, "offline") != NULL) {
            writeInfoLog("ATM service status checked: Offline");
            return 1; // ATM is Offline
        }
    } else {
        fclose(file);
        writeErrorLog("Failed to read from status file - file may be empty");
        return 0; // Default to Online if read fails
    }

    writeInfoLog("ATM service status checked: Online");
    return 0; // ATM is Online
}

// Set service status (0 = Online, 1 = Offline)
int setServiceStatus(int isOutOfService) {
    const char* filePath = getStatusFilePath();
    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        char errorMsg[256] = {0};
        sprintf(errorMsg, "Failed to open status file at %s for writing", filePath);
        writeErrorLog(errorMsg);
        return 0;
    }

    // First create backup of current status file
    char backupFilePath[256] = {0};
    sprintf(backupFilePath, "%s.bak", filePath);
    
    // Copy existing file to backup if it exists
    FILE* existingFile = fopen(filePath, "r");
    if (existingFile != NULL) {
        char buffer[256] = {0};
        FILE* backupFile = fopen(backupFilePath, "w");
        
        if (backupFile != NULL) {
            while (fgets(buffer, sizeof(buffer), existingFile) != NULL) {
                fputs(buffer, backupFile);
            }
            fclose(backupFile);
        }
        fclose(existingFile);
    }

    // Set the new status
    if (isOutOfService) {
        if (fprintf(file, "Status: Offline") < 0) {
            writeErrorLog("Failed to write to status file");
            fclose(file);
            return 0;
        }
        writeAuditLog("ADMIN", "ATM service set to Offline");
    } else {
        if (fprintf(file, "Status: Online") < 0) {
            writeErrorLog("Failed to write to status file");
            fclose(file);
            return 0;
        }
        writeAuditLog("ADMIN", "ATM service set to Online");
    }

    fclose(file);
    return 1; // Status updated successfully
}

// Toggle the ATM service mode between Online and Offline
int toggleServiceMode() {
    int currentStatus = getServiceStatus();
    return setServiceStatus(!currentStatus);
}