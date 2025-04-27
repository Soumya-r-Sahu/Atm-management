#include "config_manager.h"
#include "../utils/logger.h"
#include "../common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global configuration array
SystemConfig* g_systemConfigs = NULL;
int g_configCount = 0;

#define CONFIG_FILE_PATH "../data/system_config.txt"
#define MAX_LINE_LENGTH 256
#define MAX_CONFIG_COUNT 50

// Initialize and load configurations
int initializeConfigs() {
    FILE* file = fopen(CONFIG_FILE_PATH, "r");
    if (!file) {
        writeErrorLog("Cannot open system configuration file");
        return 0;
    }

    // Skip header lines (first 2 lines)
    char buffer[MAX_LINE_LENGTH];
    fgets(buffer, sizeof(buffer), file);
    fgets(buffer, sizeof(buffer), file);

    // Allocate memory for configurations
    g_systemConfigs = (SystemConfig*)malloc(MAX_CONFIG_COUNT * sizeof(SystemConfig));
    if (!g_systemConfigs) {
        writeErrorLog("Failed to allocate memory for system configurations");
        fclose(file);
        return 0;
    }

    // Read configurations
    g_configCount = 0;
    while (fgets(buffer, sizeof(buffer), file) && g_configCount < MAX_CONFIG_COUNT) {
        // Parse line: ID | Name | Value | Updated At
        char* token = strtok(buffer, "|");
        if (!token) continue;
        
        // Trim spaces
        while (*token == ' ') token++;
        strncpy(g_systemConfigs[g_configCount].id, token, sizeof(g_systemConfigs[g_configCount].id) - 1);
        g_systemConfigs[g_configCount].id[sizeof(g_systemConfigs[g_configCount].id) - 1] = '\0';
        
        // Parse name
        token = strtok(NULL, "|");
        if (!token) continue;
        while (*token == ' ') token++;
        strncpy(g_systemConfigs[g_configCount].name, token, sizeof(g_systemConfigs[g_configCount].name) - 1);
        g_systemConfigs[g_configCount].name[sizeof(g_systemConfigs[g_configCount].name) - 1] = '\0';
        
        // Parse value
        token = strtok(NULL, "|");
        if (!token) continue;
        while (*token == ' ') token++;
        strncpy(g_systemConfigs[g_configCount].value, token, sizeof(g_systemConfigs[g_configCount].value) - 1);
        g_systemConfigs[g_configCount].value[sizeof(g_systemConfigs[g_configCount].value) - 1] = '\0';
        
        // Parse updated_at
        token = strtok(NULL, "\n");
        if (!token) continue;
        while (*token == ' ') token++;
        strncpy(g_systemConfigs[g_configCount].updated_at, token, sizeof(g_systemConfigs[g_configCount].updated_at) - 1);
        g_systemConfigs[g_configCount].updated_at[sizeof(g_systemConfigs[g_configCount].updated_at) - 1] = '\0';
        
        g_configCount++;
    }

    fclose(file);
    
    char logMessage[100];
    sprintf(logMessage, "Loaded %d system configurations", g_configCount);
    writeInfoLog(logMessage);
    
    return 1;
}

// Free allocated memory for configurations
void freeConfigs() {
    if (g_systemConfigs) {
        free(g_systemConfigs);
        g_systemConfigs = NULL;
        g_configCount = 0;
    }
}

// Get configuration value by name
const char* getConfigValue(const char* configName) {
    for (int i = 0; i < g_configCount; i++) {
        if (strcmp(g_systemConfigs[i].name, configName) == 0) {
            return g_systemConfigs[i].value;
        }
    }
    return NULL; // Configuration not found
}

// Get configuration value as integer
int getConfigValueInt(const char* configName) {
    const char* value = getConfigValue(configName);
    if (value) {
        return atoi(value);
    }
    return -1; // Configuration not found
}

// Get configuration value as double
double getConfigValueDouble(const char* configName) {
    const char* value = getConfigValue(configName);
    if (value) {
        return atof(value);
    }
    return -1.0; // Configuration not found
}

// Get configuration value as boolean
int getConfigValueBool(const char* configName) {
    const char* value = getConfigValue(configName);
    if (value) {
        if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
            return 1;
        }
    }
    return 0; // Default to false or configuration not found
}

// Update configuration (admin only)
int updateConfig(const char* configName, const char* newValue) {
    for (int i = 0; i < g_configCount; i++) {
        if (strcmp(g_systemConfigs[i].name, configName) == 0) {
            strncpy(g_systemConfigs[i].value, newValue, sizeof(g_systemConfigs[i].value) - 1);
            g_systemConfigs[i].value[sizeof(g_systemConfigs[i].value) - 1] = '\0';
            
            // Update timestamp
            time_t now = time(NULL);
            struct tm* tm_info = localtime(&now);
            strftime(g_systemConfigs[i].updated_at, sizeof(g_systemConfigs[i].updated_at), 
                    "%Y-%m-%d %H:%M:%S", tm_info);
            
            char logMessage[100];
            sprintf(logMessage, "Updated system config: %s = %s", configName, newValue);
            writeInfoLog(logMessage);
            
            return 1; // Success
        }
    }
    
    writeErrorLog("Attempted to update non-existent configuration");
    return 0; // Configuration not found
}

// Save configurations to file (admin only)
int saveConfigs() {
    FILE* file = fopen(CONFIG_FILE_PATH, "w");
    if (!file) {
        writeErrorLog("Cannot open system configuration file for writing");
        return 0;
    }
    
    // Write header
    fprintf(file, "Config ID | Config Name                | Config Value | Updated At\n");
    fprintf(file, "----------|----------------------------|-------------|---------------------\n");
    
    // Write each configuration
    for (int i = 0; i < g_configCount; i++) {
        fprintf(file, "%-8s | %-27s | %-11s | %-19s\n", 
                g_systemConfigs[i].id, 
                g_systemConfigs[i].name, 
                g_systemConfigs[i].value, 
                g_systemConfigs[i].updated_at);
    }
    
    fclose(file);
    writeInfoLog("System configurations saved to file");
    
    return 1; // Success
}