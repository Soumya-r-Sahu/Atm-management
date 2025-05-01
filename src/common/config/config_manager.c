#include "../../../include/common/config/config_manager.h"
#include "../../../include/common/utils/logger.h"
#include "../../../include/common/utils/path_manager.h"
#include "../../../include/common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Cross-platform strcasecmp implementation
#if defined(_WIN32) || defined(_WIN64)
int strcasecmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}
#endif

// Global system configurations
Config g_system_configs[MAX_CONFIGS];
int g_config_count = 0;

// Maximum number of configuration entries
#define MAX_CONFIG_ENTRIES 100

// Configuration change callback structure
typedef struct {
    char key[64];
    void (*callback)(const char* key);
} ConfigCallback;

// Modern configuration storage system
static ConfigValue config_values[MAX_CONFIG_ENTRIES];
static int config_count = 0;

// Callback storage
#define MAX_CALLBACKS 50
static ConfigCallback callbacks[MAX_CALLBACKS];
static int callback_count = 0;

// Track changed keys for callbacks
static char changed_keys[MAX_CONFIG_ENTRIES][64];
static int changed_count = 0;

// Maximum size for configuration values
#define MAX_CONFIG_VALUE_SIZE 256

// Internal buffer to return config values
static char config_value_buffer[MAX_CONFIG_VALUE_SIZE];

// Helper function to trim whitespace
static char* trim_whitespace(char* str) {
    char* end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return str; // All spaces
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end + 1) = 0;
    
    return str;
}

// Function to trim leading and trailing whitespace from a string
static void trim_whitespace(char* str) {
    if (!str) return;
    
    // Trim leading space
    char* start = str;
    while(isspace((unsigned char)*start)) start++;
    
    if(start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    // Trim trailing space
    char* end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end+1) = '\0';
}

// Add a key to the changed keys list
static void mark_key_changed(const char* key) {
    // Check if key is already in changed list
    for (int i = 0; i < changed_count; i++) {
        if (strcmp(changed_keys[i], key) == 0) {
            return; // Already marked as changed
        }
    }
    
    // Add to changed list if there's space
    if (changed_count < MAX_CONFIG_ENTRIES) {
        strncpy(changed_keys[changed_count], key, sizeof(changed_keys[0]) - 1);
        changed_keys[changed_count][sizeof(changed_keys[0]) - 1] = '\0';
        changed_count++;
    }
}

// Find a configuration value by key (for modern config system)
static ConfigValue* find_config_value(const char* key) {
    for (int i = 0; i < config_count; i++) {
        if (strcmp(config_values[i].key, key) == 0) {
            return &config_values[i];
        }
    }
    return NULL;
}

// Initialize the modern configuration system
int config_init(void) {
    config_count = 0;
    callback_count = 0;
    changed_count = 0;
    return 1;
}

// Clean up configuration resources
void config_cleanup(void) {
    config_count = 0;
    callback_count = 0;
    changed_count = 0;
}

// Initialize system configurations - Legacy function preserved for compatibility
int initialize_configs(void) {
    g_config_count = 0;

    // Default system configurations
    const char* config_file = SYSTEM_CONFIG_FILE;
    
    // Try to load from file first
    FILE* file = fopen(config_file, "r");
    if (file) {
        char line[512];
        
        // Skip header lines if present
        if (fgets(line, sizeof(line), file) != NULL && strstr(line, "Name") != NULL) {
            // Skip header line
        }
        
        // Read configurations
        while (fgets(line, sizeof(line), file) && g_config_count < MAX_CONFIGS) {
            char name[MAX_CONFIG_NAME_LENGTH], value[MAX_CONFIG_VALUE_LENGTH], description[100];
            int is_editable = 1;
            
            // Try to parse the line
            // Format: Name | Value | Description | Editable (optional)
            if (sscanf(line, "%29[^|] | %49[^|] | %99[^|] | %d", 
                      name, value, description, &is_editable) >= 3) {
                
                // Trim whitespace
                char* trimmed_name = trim_whitespace(name);
                char* trimmed_value = trim_whitespace(value);
                char* trimmed_desc = trim_whitespace(description);
                
                // Add to config array
                strncpy(g_system_configs[g_config_count].name, trimmed_name, MAX_CONFIG_NAME_LENGTH - 1);
                g_system_configs[g_config_count].name[MAX_CONFIG_NAME_LENGTH - 1] = '\0';
                
                strncpy(g_system_configs[g_config_count].value, trimmed_value, MAX_CONFIG_VALUE_LENGTH - 1);
                g_system_configs[g_config_count].value[MAX_CONFIG_VALUE_LENGTH - 1] = '\0';
                
                strncpy(g_system_configs[g_config_count].description, trimmed_desc, 99);
                g_system_configs[g_config_count].description[99] = '\0';
                
                g_system_configs[g_config_count].is_editable = is_editable;
                
                g_config_count++;
            }
        }
        
        fclose(file);
        
        if (g_config_count > 0) {
            char msg[100];
            snprintf(msg, sizeof(msg), "Loaded %d system configurations from file", g_config_count);
            write_info_log("Loaded system configurations");
            return 1;
        }
    }
    
    // If file doesn't exist or is empty, create default configurations
    
    // Create default configurations
    strncpy(g_system_configs[g_config_count].name, "max_withdrawal_limit", MAX_CONFIG_NAME_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].value, "10000", MAX_CONFIG_VALUE_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].description, "Maximum amount that can be withdrawn in a single transaction", 99);
    g_system_configs[g_config_count].is_editable = 1;
    g_config_count++;
    
    strncpy(g_system_configs[g_config_count].name, "min_withdrawal_limit", MAX_CONFIG_NAME_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].value, "100", MAX_CONFIG_VALUE_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].description, "Minimum amount that can be withdrawn in a single transaction", 99);
    g_system_configs[g_config_count].is_editable = 1;
    g_config_count++;
    
    strncpy(g_system_configs[g_config_count].name, "daily_withdrawal_limit", MAX_CONFIG_NAME_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].value, "25000", MAX_CONFIG_VALUE_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].description, "Maximum amount that can be withdrawn in a day", 99);
    g_system_configs[g_config_count].is_editable = 1;
    g_config_count++;
    
    strncpy(g_system_configs[g_config_count].name, "session_timeout", MAX_CONFIG_NAME_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].value, "120", MAX_CONFIG_VALUE_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].description, "Session timeout in seconds", 99);
    g_system_configs[g_config_count].is_editable = 1;
    g_config_count++;
    
    strncpy(g_system_configs[g_config_count].name, "max_pin_attempts", MAX_CONFIG_NAME_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].value, "3", MAX_CONFIG_VALUE_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].description, "Maximum number of incorrect PIN attempts before card is blocked", 99);
    g_system_configs[g_config_count].is_editable = 1;
    g_config_count++;

    strncpy(g_system_configs[g_config_count].name, "default_language", MAX_CONFIG_NAME_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].value, "English", MAX_CONFIG_VALUE_LENGTH - 1);
    strncpy(g_system_configs[g_config_count].description, "Default language for ATM interface", 99);
    g_system_configs[g_config_count].is_editable = 1;
    g_config_count++;
    
    // Save the default configurations to file
    save_configs();
    
    write_audit_log("CONFIG", "Created default system configurations");
    return 1;
}

// Alias for backward compatibility
int initializeConfigs(void) {
    return initialize_configs();
}

// Free resources used by system configurations
void free_configs(void) {
    // Nothing to free since we're using static arrays
    g_config_count = 0;
}

// Alias for backward compatibility
void freeConfigs(void) {
    free_configs();
}

// Save system configurations to file
int save_configs(void) {
    const char* config_file = SYSTEM_CONFIG_FILE;
    FILE* file = fopen(config_file, "w");
    if (!file) {
        write_error_log("Failed to open system config file for writing");
        return 0;
    }
    
    // Write header
    fprintf(file, "Name | Value | Description | Editable\n");
    
    // Write configurations
    for (int i = 0; i < g_config_count; i++) {
        fprintf(file, "%s | %s | %s | %d\n",
               g_system_configs[i].name,
               g_system_configs[i].value,
               g_system_configs[i].description,
               g_system_configs[i].is_editable);
    }
    
    fclose(file);
    write_audit_log("CONFIG", "Saved system configurations to file");
    return 1;
}

// Alias for backward compatibility
int saveConfigs(void) {
    return save_configs();
}

// Find a configuration by name
int find_config_index(const char* config_name) {
    for (int i = 0; i < g_config_count; i++) {
        if (strcmp(g_system_configs[i].name, config_name) == 0) {
            return i;
        }
    }
    return -1; // Not found
}

// Update a configuration value
int update_config(const char* config_name, const char* new_value) {
    int index = find_config_index(config_name);
    if (index >= 0) {
        if (g_system_configs[index].is_editable) {
            strncpy(g_system_configs[index].value, new_value, MAX_CONFIG_VALUE_LENGTH - 1);
            g_system_configs[index].value[MAX_CONFIG_VALUE_LENGTH - 1] = '\0';
            
            // Mark as changed for modern config system
            mark_key_changed(config_name);
            
            char msg[200];
            snprintf(msg, sizeof(msg), "Updated config '%s' to '%s'", config_name, new_value);
            write_audit_log("CONFIG", msg);
            return 1;
        } else {
            write_error_log("Attempted to update non-editable configuration");
            return 0;
        }
    }
    
    write_error_log("Attempted to update non-existent configuration");
    return 0;
}

// Alias for backward compatibility
int updateConfig(const char* name, const char* value) {
    return update_config(name, value);
}

// Add a new configuration
int add_config(const char* config_name, const char* config_value, const char* description) {
    if (g_config_count >= MAX_CONFIGS) {
        write_error_log("Cannot add new configuration: maximum limit reached");
        return 0;
    }
    
    // Check if it already exists
    if (find_config_index(config_name) >= 0) {
        write_error_log("Configuration already exists");
        return 0;
    }
    
    // Add new configuration
    strncpy(g_system_configs[g_config_count].name, config_name, MAX_CONFIG_NAME_LENGTH - 1);
    g_system_configs[g_config_count].name[MAX_CONFIG_NAME_LENGTH - 1] = '\0';
    
    strncpy(g_system_configs[g_config_count].value, config_value, MAX_CONFIG_VALUE_LENGTH - 1);
    g_system_configs[g_config_count].value[MAX_CONFIG_VALUE_LENGTH - 1] = '\0';
    
    strncpy(g_system_configs[g_config_count].description, description, 99);
    g_system_configs[g_config_count].description[99] = '\0';
    
    g_system_configs[g_config_count].is_editable = 1;
    g_config_count++;
    
    char msg[100];
    snprintf(msg, sizeof(msg), "Added new configuration '%s' with value '%s'", config_name, config_value);
    write_audit_log("CONFIG", msg);
    
    return 1;
}

// Alias for backward compatibility
int addConfig(const char* config_name, const char* config_value, const char* description) {
    return add_config(config_name, config_value, description);
}

// Get configuration value as string
const char* get_config_value(const char* key) {
    if (!key) {
        writeErrorLog("NULL key passed to get_config_value");
        return NULL;
    }
    
    FILE* file = fopen(getSystemConfigFilePath(), "r");
    if (!file) {
        writeErrorLog("Failed to open system config file");
        return NULL;
    }
    
    // Clear the buffer
    memset(config_value_buffer, 0, MAX_CONFIG_VALUE_SIZE);
    
    char line[512];
    bool found = false;
    
    // Skip header lines (first two lines)
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file)) {
        // Extract key and value from config line
        char config_key[64] = {0};
        char config_value[256] = {0};
        char description[256] = {0};
        
        // Format: | KEY | VALUE | DESCRIPTION |
        if (sscanf(line, "| %63[^|] | %255[^|] | %255[^|] |", config_key, config_value, description) >= 2) {
            trim_whitespace(config_key);
            trim_whitespace(config_value);
            
            if (strcmp(config_key, key) == 0) {
                strncpy(config_value_buffer, config_value, MAX_CONFIG_VALUE_SIZE - 1);
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found ? config_value_buffer : NULL;
}

// Get configuration value as boolean (true/false)
bool getConfigValueBool(const char* key) {
    const char* value = get_config_value(key);
    if (!value) {
        return false;
    }
    
    // Check for true values
    if (strcasecmp(value, "true") == 0 || 
        strcasecmp(value, "yes") == 0 || 
        strcasecmp(value, "1") == 0 ||
        strcasecmp(value, "enable") == 0 ||
        strcasecmp(value, "enabled") == 0) {
        return true;
    }
    
    return false;
}

// Get configuration value as integer
int getConfigValueInt(const char* key, int defaultValue) {
    const char* value = get_config_value(key);
    if (!value) {
        return defaultValue;
    }
    
    // Convert to integer
    char* end;
    int result = strtol(value, &end, 10);
    
    // Check if conversion was successful
    if (*end != '\0' && !isspace(*end)) {
        writeErrorLog("Failed to convert config value to integer");
        return defaultValue;
    }
    
    return result;
}

// Get configuration value as float
float getConfigValueFloat(const char* key, float defaultValue) {
    const char* value = get_config_value(key);
    if (!value) {
        return defaultValue;
    }
    
    // Convert to float
    char* end;
    float result = strtof(value, &end);
    
    // Check if conversion was successful
    if (*end != '\0' && !isspace(*end) && *end != '₹') {
        writeErrorLog("Failed to convert config value to float");
        return defaultValue;
    }
    
    return result;
}

// Set configuration value
bool setConfigValue(const char* key, const char* value) {
    if (!key || !value) {
        writeErrorLog("NULL parameters to setConfigValue");
        return false;
    }
    
    FILE* file = fopen(getSystemConfigFilePath(), "r");
    if (!file) {
        writeErrorLog("Failed to open system config file");
        return false;
    }
    
    char tempPath[256];
    snprintf(tempPath, sizeof(tempPath), "%s.tmp", getSystemConfigFilePath());
    
    FILE* tempFile = fopen(tempPath, "w");
    if (!tempFile) {
        fclose(file);
        writeErrorLog("Failed to create temp config file");
        return false;
    }
    
    char line[512];
    bool updated = false;
    
    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fputs(line, tempFile);
    }
    if (fgets(line, sizeof(line), file)) {
        fputs(line, tempFile);
    }
    
    // Process and update the needed line
    while (fgets(line, sizeof(line), file)) {
        char config_key[64] = {0};
        char config_value[256] = {0};
        char description[256] = {0};
        
        // Make a copy of the line for scanning
        char line_copy[512];
        strcpy(line_copy, line);
        
        // Format: | KEY | VALUE | DESCRIPTION |
        if (sscanf(line_copy, "| %63[^|] | %255[^|] | %255[^|] |", config_key, config_value, description) >= 2) {
            trim_whitespace(config_key);
            
            if (strcmp(config_key, key) == 0) {
                fprintf(tempFile, "| %-25s | %-19s | %-36s |\n", key, value, description);
                updated = true;
            } else {
                fputs(line, tempFile);
            }
        } else {
            fputs(line, tempFile);
        }
    }
    
    // If key not found, append it
    if (!updated) {
        fprintf(tempFile, "| %-25s | %-19s | %-36s |\n", key, value, "Added dynamically");
    }
    
    fclose(file);
    fclose(tempFile);
    
    // Replace original with temp file
    if (remove(getSystemConfigFilePath()) != 0) {
        writeErrorLog("Failed to remove original config file");
        remove(tempPath);
        return false;
    }
    
    if (rename(tempPath, getSystemConfigFilePath()) != 0) {
        writeErrorLog("Failed to rename temp config file");
        return false;
    }
    
    char logMsg[256];
    snprintf(logMsg, sizeof(logMsg), "Config updated: %s = %s", key, value);
    writeAuditLog("CONFIG", logMsg);
    
    return true;
}

// Add a new configuration setting
bool add_config(const char* key, const char* value, const char* description) {
    if (!key || !value) {
        writeErrorLog("NULL parameters to add_config");
        return false;
    }
    
    // Check if config already exists
    if (get_config_value(key) != NULL) {
        // Config exists, just update the value
        return setConfigValue(key, value);
    }
    
    // Config doesn't exist, append new entry
    FILE* file = fopen(getSystemConfigFilePath(), "a");
    if (!file) {
        writeErrorLog("Failed to open system config file for append");
        return false;
    }
    
    // Trim description if it's too long
    char trimmed_desc[37] = {0};
    if (description) {
        strncpy(trimmed_desc, description, 36);
    } else {
        strcpy(trimmed_desc, "Added dynamically");
    }
    
    fprintf(file, "| %-25s | %-19s | %-36s |\n", key, value, trimmed_desc);
    fclose(file);
    
    char logMsg[256];
    snprintf(logMsg, sizeof(logMsg), "New config added: %s = %s", key, value);
    writeAuditLog("CONFIG", logMsg);
    
    return true;
}

// Get multiple configuration values at once
bool get_multiple_config_values(const char** keys, char** values, int count, size_t value_size) {
    if (!keys || !values || count <= 0) {
        writeErrorLog("Invalid parameters to get_multiple_config_values");
        return false;
    }
    
    bool success = true;
    for (int i = 0; i < count; i++) {
        const char* value = get_config_value(keys[i]);
        if (value) {
            strncpy(values[i], value, value_size - 1);
            values[i][value_size - 1] = '\0';
        } else {
            values[i][0] = '\0';
            success = false;
        }
    }
    
    return success;
}

// Initialize the configuration system
bool initialize_config() {
    FILE* file = fopen(getSystemConfigFilePath(), "r");
    if (!file) {
        writeErrorLog("Failed to open system config file during initialization");
        return false;
    }
    
    fclose(file);
    writeInfoLog("Configuration system initialized");
    return true;
}

#undef CONFIG_SESSION_TIMEOUT_SECONDS
#define CONFIG_SESSION_TIMEOUT_SECONDS "session_timeout_seconds"