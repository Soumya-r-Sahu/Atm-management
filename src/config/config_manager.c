#include "config_manager.h"
#include "../common/error_handler.h"
#include "../common/paths.h"
#include "../utils/memory_utils.h"
#include "../utils/logger.h"
#include "../utils/string_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Maximum number of configuration entries
#define MAX_CONFIG_ENTRIES 100

// Configuration change callback structure
typedef struct {
    char key[64];
    void (*callback)(const char* key);
} ConfigCallback;

// Configuration storage
static ConfigValue config_values[MAX_CONFIG_ENTRIES];
static int config_count = 0;

// Callback storage
#define MAX_CALLBACKS 50
static ConfigCallback callbacks[MAX_CALLBACKS];
static int callback_count = 0;

// Track changed keys for callbacks
static char changed_keys[MAX_CONFIG_ENTRIES][64];
static int changed_count = 0;

// Initialize the configuration system
int config_init(void) {
    config_count = 0;
    callback_count = 0;
    changed_count = 0;
    return 1;
}

// Find a configuration value by key
static ConfigValue* find_config_value(const char* key) {
    for (int i = 0; i < config_count; i++) {
        if (strcmp(config_values[i].key, key) == 0) {
            return &config_values[i];
        }
    }
    return NULL;
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

// Trim whitespace from a string
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

// Load configuration from a file
int loadConfig(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        SET_ERROR(ERR_FILE_ACCESS, "Could not open configuration file for reading");
        return 0;
    }
    
    // Clear existing configuration
    config_count = 0;
    
    char line[512];
    while (fgets(line, sizeof(line), file) && config_count < MAX_CONFIG_ENTRIES) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        
        // Find the equals sign
        char* equals = strchr(line, '=');
        if (!equals) {
            continue; // Not a valid config line
        }
        
        // Split key and value
        *equals = '\0';
        char* key = trim_whitespace(line);
        char* value = trim_whitespace(equals + 1);
        
        // Remove trailing newline if present
        size_t value_len = strlen(value);
        if (value_len > 0 && (value[value_len - 1] == '\n' || value[value_len - 1] == '\r')) {
            value[value_len - 1] = '\0';
        }
        
        // Determine value type and store
        ConfigValue config;
        strncpy(config.key, key, sizeof(config.key) - 1);
        config.key[sizeof(config.key) - 1] = '\0';
        
        strncpy(config.string_value, value, sizeof(config.string_value) - 1);
        config.string_value[sizeof(config.string_value) - 1] = '\0';
        
        // Check if value is an integer
        char* endptr;
        config.int_value = strtol(value, &endptr, 10);
        if (*endptr == '\0') {
            config.type = CONFIG_TYPE_INT;
        }
        // Check if value is a float
        else {
            config.float_value = strtof(value, &endptr);
            if (*endptr == '\0') {
                config.type = CONFIG_TYPE_FLOAT;
            }
            // Check if value is a boolean
            else if (strcasecmp(value, "true") == 0 || strcasecmp(value, "yes") == 0 || 
                     strcasecmp(value, "on") == 0 || strcmp(value, "1") == 0) {
                config.type = CONFIG_TYPE_BOOLEAN;
                config.bool_value = 1;
            } 
            else if (strcasecmp(value, "false") == 0 || strcasecmp(value, "no") == 0 || 
                     strcasecmp(value, "off") == 0 || strcmp(value, "0") == 0) {
                config.type = CONFIG_TYPE_BOOLEAN;
                config.bool_value = 0;
            }
            // Otherwise, it's a string
            else {
                config.type = CONFIG_TYPE_STRING;
            }
        }
        
        // Store the config value
        config_values[config_count] = config;
        config_count++;
    }
    
    fclose(file);
    
    char log_msg[100];
    snprintf(log_msg, sizeof(log_msg), "Loaded %d configuration entries from %s", 
             config_count, path);
    writeAuditLog("CONFIG", log_msg);
    
    return 1;
}

// Save current configuration to a file
int saveConfig(const char* path) {
    FILE* file = fopen(path, "w");
    if (!file) {
        SET_ERROR(ERR_FILE_ACCESS, "Could not open configuration file for writing");
        return 0;
    }
    
    // Write header
    fprintf(file, "# ATM System Configuration\n");
    fprintf(file, "# Auto-generated file - DO NOT EDIT MANUALLY\n\n");
    
    // Write all configuration values
    for (int i = 0; i < config_count; i++) {
        const ConfigValue* config = &config_values[i];
        
        // Write a comment about the type
        switch (config->type) {
            case CONFIG_TYPE_INT:
                fprintf(file, "# Type: Integer\n");
                break;
            case CONFIG_TYPE_FLOAT:
                fprintf(file, "# Type: Float\n");
                break;
            case CONFIG_TYPE_BOOLEAN:
                fprintf(file, "# Type: Boolean\n");
                break;
            case CONFIG_TYPE_STRING:
                fprintf(file, "# Type: String\n");
                break;
        }
        
        // Write the key-value pair
        fprintf(file, "%s = %s\n\n", config->key, config->string_value);
    }
    
    fclose(file);
    
    char log_msg[100];
    snprintf(log_msg, sizeof(log_msg), "Saved %d configuration entries to %s", 
             config_count, path);
    writeAuditLog("CONFIG", log_msg);
    
    return 1;
}

// Get string configuration value
const char* getConfigValue(const char* key, const char* defaultValue) {
    ConfigValue* config = find_config_value(key);
    if (config) {
        return config->string_value;
    }
    return defaultValue;
}

// Get integer configuration value
int getConfigValueInt(const char* key) {
    ConfigValue* config = find_config_value(key);
    if (config) {
        if (config->type == CONFIG_TYPE_INT || config->type == CONFIG_TYPE_BOOLEAN) {
            return config->int_value;
        }
        // Try to convert string to int
        return atoi(config->string_value);
    }
    return 0; // Default
}

// Get float configuration value
float getConfigValueFloat(const char* key) {
    ConfigValue* config = find_config_value(key);
    if (config) {
        if (config->type == CONFIG_TYPE_FLOAT) {
            return config->float_value;
        }
        // Try to convert string to float
        return atof(config->string_value);
    }
    return 0.0f; // Default
}

// Get boolean configuration value
int getConfigValueBool(const char* key) {
    ConfigValue* config = find_config_value(key);
    if (config) {
        if (config->type == CONFIG_TYPE_BOOLEAN) {
            return config->bool_value;
        }
        if (config->type == CONFIG_TYPE_INT) {
            return config->int_value != 0;
        }
        // Try to convert string to boolean
        if (strcasecmp(config->string_value, "true") == 0 ||
            strcasecmp(config->string_value, "yes") == 0 ||
            strcasecmp(config->string_value, "on") == 0 ||
            strcmp(config->string_value, "1") == 0) {
            return 1;
        }
    }
    return 0; // Default
}

// Set string configuration value
int setConfigValue(const char* key, const char* value) {
    if (!key || !value) {
        SET_ERROR(ERR_INVALID_INPUT, "NULL key or value in setConfigValue");
        return 0;
    }
    
    // Check if key exists
    ConfigValue* config = find_config_value(key);
    if (config) {
        // Update existing value
        strncpy(config->string_value, value, sizeof(config->string_value) - 1);
        config->string_value[sizeof(config->string_value) - 1] = '\0';
        config->type = CONFIG_TYPE_STRING;
        
        // Mark as changed for callbacks
        mark_key_changed(key);
    } else {
        // Add new value if there's space
        if (config_count < MAX_CONFIG_ENTRIES) {
            ConfigValue new_config;
            strncpy(new_config.key, key, sizeof(new_config.key) - 1);
            new_config.key[sizeof(new_config.key) - 1] = '\0';
            
            strncpy(new_config.string_value, value, sizeof(new_config.string_value) - 1);
            new_config.string_value[sizeof(new_config.string_value) - 1] = '\0';
            
            new_config.type = CONFIG_TYPE_STRING;
            
            config_values[config_count] = new_config;
            config_count++;
            
            // Mark as changed for callbacks
            mark_key_changed(key);
        } else {
            SET_ERROR(ERR_LIMIT_EXCEEDED, "Maximum configuration entries reached");
            return 0;
        }
    }
    
    return 1;
}

// Set integer configuration value
int setConfigValueInt(const char* key, int value) {
    if (!key) {
        SET_ERROR(ERR_INVALID_INPUT, "NULL key in setConfigValueInt");
        return 0;
    }
    
    // Convert to string for storage
    char value_str[32];
    snprintf(value_str, sizeof(value_str), "%d", value);
    
    // Check if key exists
    ConfigValue* config = find_config_value(key);
    if (config) {
        // Update existing value
        strncpy(config->string_value, value_str, sizeof(config->string_value) - 1);
        config->string_value[sizeof(config->string_value) - 1] = '\0';
        config->int_value = value;
        config->type = CONFIG_TYPE_INT;
        
        // Mark as changed for callbacks
        mark_key_changed(key);
    } else {
        // Add new value if there's space
        if (config_count < MAX_CONFIG_ENTRIES) {
            ConfigValue new_config;
            strncpy(new_config.key, key, sizeof(new_config.key) - 1);
            new_config.key[sizeof(new_config.key) - 1] = '\0';
            
            strncpy(new_config.string_value, value_str, sizeof(new_config.string_value) - 1);
            new_config.string_value[sizeof(new_config.string_value) - 1] = '\0';
            
            new_config.int_value = value;
            new_config.type = CONFIG_TYPE_INT;
            
            config_values[config_count] = new_config;
            config_count++;
            
            // Mark as changed for callbacks
            mark_key_changed(key);
        } else {
            SET_ERROR(ERR_LIMIT_EXCEEDED, "Maximum configuration entries reached");
            return 0;
        }
    }
    
    return 1;
}

// Set float configuration value
int setConfigValueFloat(const char* key, float value) {
    if (!key) {
        SET_ERROR(ERR_INVALID_INPUT, "NULL key in setConfigValueFloat");
        return 0;
    }
    
    // Convert to string for storage
    char value_str[32];
    snprintf(value_str, sizeof(value_str), "%f", value);
    
    // Check if key exists
    ConfigValue* config = find_config_value(key);
    if (config) {
        // Update existing value
        strncpy(config->string_value, value_str, sizeof(config->string_value) - 1);
        config->string_value[sizeof(config->string_value) - 1] = '\0';
        config->float_value = value;
        config->type = CONFIG_TYPE_FLOAT;
        
        // Mark as changed for callbacks
        mark_key_changed(key);
    } else {
        // Add new value if there's space
        if (config_count < MAX_CONFIG_ENTRIES) {
            ConfigValue new_config;
            strncpy(new_config.key, key, sizeof(new_config.key) - 1);
            new_config.key[sizeof(new_config.key) - 1] = '\0';
            
            strncpy(new_config.string_value, value_str, sizeof(new_config.string_value) - 1);
            new_config.string_value[sizeof(new_config.string_value) - 1] = '\0';
            
            new_config.float_value = value;
            new_config.type = CONFIG_TYPE_FLOAT;
            
            config_values[config_count] = new_config;
            config_count++;
            
            // Mark as changed for callbacks
            mark_key_changed(key);
        } else {
            SET_ERROR(ERR_LIMIT_EXCEEDED, "Maximum configuration entries reached");
            return 0;
        }
    }
    
    return 1;
}

// Set boolean configuration value
int setConfigValueBool(const char* key, int value) {
    if (!key) {
        SET_ERROR(ERR_INVALID_INPUT, "NULL key in setConfigValueBool");
        return 0;
    }
    
    // Convert to string for storage
    const char* value_str = value ? "true" : "false";
    
    // Check if key exists
    ConfigValue* config = find_config_value(key);
    if (config) {
        // Update existing value
        strncpy(config->string_value, value_str, sizeof(config->string_value) - 1);
        config->string_value[sizeof(config->string_value) - 1] = '\0';
        config->bool_value = value ? 1 : 0;
        config->type = CONFIG_TYPE_BOOLEAN;
        
        // Mark as changed for callbacks
        mark_key_changed(key);
    } else {
        // Add new value if there's space
        if (config_count < MAX_CONFIG_ENTRIES) {
            ConfigValue new_config;
            strncpy(new_config.key, key, sizeof(new_config.key) - 1);
            new_config.key[sizeof(new_config.key) - 1] = '\0';
            
            strncpy(new_config.string_value, value_str, sizeof(new_config.string_value) - 1);
            new_config.string_value[sizeof(new_config.string_value) - 1] = '\0';
            
            new_config.bool_value = value ? 1 : 0;
            new_config.type = CONFIG_TYPE_BOOLEAN;
            
            config_values[config_count] = new_config;
            config_count++;
            
            // Mark as changed for callbacks
            mark_key_changed(key);
        } else {
            SET_ERROR(ERR_LIMIT_EXCEEDED, "Maximum configuration entries reached");
            return 0;
        }
    }
    
    return 1;
}

// Check if configuration key exists
int hasConfigKey(const char* key) {
    return find_config_value(key) != NULL;
}

// Remove configuration key
int removeConfigKey(const char* key) {
    for (int i = 0; i < config_count; i++) {
        if (strcmp(config_values[i].key, key) == 0) {
            // Mark as changed for callbacks
            mark_key_changed(key);
            
            // Move all subsequent entries up one position
            if (i < config_count - 1) {
                memmove(&config_values[i], &config_values[i + 1], 
                       (config_count - i - 1) * sizeof(ConfigValue));
            }
            
            config_count--;
            return 1;
        }
    }
    
    return 0; // Key not found
}

// Reset configuration to defaults
int resetConfigToDefaults(void) {
    // Clear existing configuration
    for (int i = 0; i < config_count; i++) {
        mark_key_changed(config_values[i].key);
    }
    
    config_count = 0;
    
    // Set default values
    setConfigValueInt("max_failed_attempts", 3);
    setConfigValueInt("session_timeout_minutes", 30);
    setConfigValueFloat("min_withdrawal", 20.0f);
    setConfigValueFloat("max_withdrawal", 1000.0f);
    setConfigValueBool("enable_audit_logging", 1);
    setConfigValueBool("enable_encryption", 1);
    setConfigValue("log_level", "INFO");
    setConfigValue("currency_symbol", "$");
    setConfigValue("default_language", "en");
    setConfigValueInt("default_account_balance", 100);
    
    return 1;
}

// Get the type of a configuration value
ConfigValueType getConfigValueType(const char* key) {
    ConfigValue* config = find_config_value(key);
    if (config) {
        return config->type;
    }
    
    return CONFIG_TYPE_STRING; // Default
}

// Register a callback for configuration changes
int registerConfigChangeCallback(const char* key, void (*callback)(const char* key)) {
    if (!key || !callback) {
        SET_ERROR(ERR_INVALID_INPUT, "NULL key or callback in registerConfigChangeCallback");
        return 0;
    }
    
    if (callback_count < MAX_CALLBACKS) {
        strncpy(callbacks[callback_count].key, key, sizeof(callbacks[0].key) - 1);
        callbacks[callback_count].key[sizeof(callbacks[0].key) - 1] = '\0';
        callbacks[callback_count].callback = callback;
        
        callback_count++;
        return 1;
    }
    
    SET_ERROR(ERR_LIMIT_EXCEEDED, "Maximum callbacks reached");
    return 0;
}

// Apply configuration changes (calls callbacks for changed values)
void applyConfigChanges(void) {
    // Process all changed keys
    for (int i = 0; i < changed_count; i++) {
        const char* changed_key = changed_keys[i];
        
        // Find all callbacks that match this key
        for (int j = 0; j < callback_count; j++) {
            // Call the callback if the key matches exactly or the callback registered for "*" (all keys)
            if (strcmp(callbacks[j].key, changed_key) == 0 || strcmp(callbacks[j].key, "*") == 0) {
                callbacks[j].callback(changed_key);
            }
        }
    }
    
    // Clear changed keys list
    changed_count = 0;
}

// Initialize a new configuration file with default values
int createDefaultConfig(const char* path) {
    // Reset to defaults first
    resetConfigToDefaults();
    
    // Then save to the specified path
    return saveConfig(path);
}

// Get all configuration keys
int getAllConfigKeys(char** keys, int max_keys) {
    if (!keys || max_keys <= 0) {
        SET_ERROR(ERR_INVALID_INPUT, "Invalid parameters to getAllConfigKeys");
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < config_count && count < max_keys; i++) {
        strncpy(keys[count], config_values[i].key, 63);
        keys[count][63] = '\0';
        count++;
    }
    
    return count;
}

// Clean up configuration resources
void config_cleanup(void) {
    config_count = 0;
    callback_count = 0;
    changed_count = 0;
}