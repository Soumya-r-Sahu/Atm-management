#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdbool.h>

// Config values
#define MAX_CONFIG_NAME_LENGTH 30
#define MAX_CONFIG_VALUE_LENGTH 50
#define MAX_CONFIGS 100
// Use the same definition as in other files for consistency
#define CONFIG_SESSION_TIMEOUT_SECONDS "session_timeout_seconds"

// Configuration value types
typedef enum {
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_FLOAT,
    CONFIG_TYPE_BOOLEAN
} ConfigValueType;

// Configuration value structure
typedef struct {
    char key[64];
    char string_value[256];
    union {
        int int_value;
        float float_value;
        int bool_value;
    };
    ConfigValueType type;
} ConfigValue;

// System Configuration structure (for admin interface)
typedef struct {
    char name[MAX_CONFIG_NAME_LENGTH];
    char value[MAX_CONFIG_VALUE_LENGTH];
    char description[100];
    int is_editable;
} Config;

// For backward compatibility - alias to initializeConfigs
int initialize_configs(void);

// Initialize configuration system
int initializeConfigs(void);

// Free resources used by system configurations
void freeConfigs(void);
void free_configs(void);

// Save system configurations to file
int saveConfigs(void);
int save_configs(void);  // Alias for backward compatibility

// Initialize the configuration system with modern interface
int config_init(void);

// Clean up configuration resources
void config_cleanup(void);

// Load configuration from a file
int loadConfig(const char* path);

// Save current configuration to a file
int saveConfig(const char* path);

// Get configuration values
const char* getConfigValue(const char* key, const char* defaultValue);
int getConfigValueInt(const char* key);
float getConfigValueFloat(const char* key);
int getConfigValueBool(const char* key);
ConfigValueType getConfigValueType(const char* key);

// Backward compatibility functions
const char* get_config_value(const char* config_name);
int get_config_value_int(const char* config_name);
float get_config_value_float(const char* config_name);
int get_config_value_bool(const char* config_name);

// Set configuration values
int setConfigValue(const char* key, const char* value);
int setConfigValueInt(const char* key, int value);
int setConfigValueFloat(const char* key, float value);
int setConfigValueBool(const char* key, int value);

// Find configuration
int find_config_index(const char* config_name);

// Update a configuration
int updateConfig(const char* name, const char* value);
int update_config(const char* config_name, const char* new_value);

// Add a new configuration
int addConfig(const char* config_name, const char* config_value, const char* description);

// Check if configuration key exists
int hasConfigKey(const char* key);

// Remove configuration key
int removeConfigKey(const char* key);

// Reset configuration to defaults
int resetConfigToDefaults(void);

// Register a callback for configuration changes
int registerConfigChangeCallback(const char* key, void (*callback)(const char* key));

// Apply configuration changes (calls callbacks for changed values)
void applyConfigChanges(void);

// Create a default configuration file
int createDefaultConfig(const char* path);

// Get all configuration keys
int getAllConfigKeys(char** keys, int max_keys);

#endif /* CONFIG_MANAGER_H */