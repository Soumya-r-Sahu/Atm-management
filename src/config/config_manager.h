#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

/**
 * @file config_manager.h
 * @brief Enhanced configuration management system for ATM
 * 
 * This module provides robust configuration management with support for
 * different data types, defaults, and change notification callbacks.
 */

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
    int int_value;
    float float_value;
    int bool_value;
    ConfigValueType type;
} ConfigValue;

/**
 * @brief Initialize the configuration system
 * @return 1 on success, 0 on failure
 */
int config_init(void);

/**
 * @brief Load configuration from a file
 * 
 * @param path Path to the configuration file
 * @return 1 on success, 0 on failure
 */
int loadConfig(const char* path);

/**
 * @brief Save current configuration to a file
 * 
 * @param path Path to save the configuration file
 * @return 1 on success, 0 on failure
 */
int saveConfig(const char* path);

/**
 * @brief Get string configuration value
 * 
 * @param key Configuration key
 * @param defaultValue Default value if key doesn't exist
 * @return Configuration value or default if not found
 */
const char* getConfigValue(const char* key, const char* defaultValue);

/**
 * @brief Get integer configuration value
 * 
 * @param key Configuration key
 * @return Integer value or 0 if not found
 */
int getConfigValueInt(const char* key);

/**
 * @brief Get float configuration value
 * 
 * @param key Configuration key
 * @return Float value or 0.0 if not found
 */
float getConfigValueFloat(const char* key);

/**
 * @brief Get boolean configuration value
 * 
 * @param key Configuration key
 * @return Boolean value (1 for true, 0 for false) or 0 if not found
 */
int getConfigValueBool(const char* key);

/**
 * @brief Set string configuration value
 * 
 * @param key Configuration key
 * @param value String value
 * @return 1 on success, 0 on failure
 */
int setConfigValue(const char* key, const char* value);

/**
 * @brief Set integer configuration value
 * 
 * @param key Configuration key
 * @param value Integer value
 * @return 1 on success, 0 on failure
 */
int setConfigValueInt(const char* key, int value);

/**
 * @brief Set float configuration value
 * 
 * @param key Configuration key
 * @param value Float value
 * @return 1 on success, 0 on failure
 */
int setConfigValueFloat(const char* key, float value);

/**
 * @brief Set boolean configuration value
 * 
 * @param key Configuration key
 * @param value Boolean value (0 for false, non-zero for true)
 * @return 1 on success, 0 on failure
 */
int setConfigValueBool(const char* key, int value);

/**
 * @brief Check if configuration key exists
 * 
 * @param key Configuration key
 * @return 1 if key exists, 0 otherwise
 */
int hasConfigKey(const char* key);

/**
 * @brief Remove configuration key
 * 
 * @param key Configuration key
 * @return 1 on success, 0 if key not found
 */
int removeConfigKey(const char* key);

/**
 * @brief Reset configuration to defaults
 * 
 * @return 1 on success, 0 on failure
 */
int resetConfigToDefaults(void);

/**
 * @brief Get the type of a configuration value
 * 
 * @param key Configuration key
 * @return ConfigValueType
 */
ConfigValueType getConfigValueType(const char* key);

/**
 * @brief Register a callback for configuration changes
 * 
 * The callback will be called when the specified key changes.
 * Register with key "*" to receive notifications for all changes.
 * 
 * @param key Configuration key to monitor
 * @param callback Function to call when key changes
 * @return 1 on success, 0 on failure
 */
int registerConfigChangeCallback(const char* key, void (*callback)(const char* key));

/**
 * @brief Apply configuration changes
 * 
 * Call this function after making changes to trigger callbacks
 */
void applyConfigChanges(void);

/**
 * @brief Initialize a new configuration file with default values
 * 
 * @param path Path to create the configuration file
 * @return 1 on success, 0 on failure
 */
int createDefaultConfig(const char* path);

/**
 * @brief Get all configuration keys
 * 
 * @param keys Array of strings to store keys (must be pre-allocated)
 * @param max_keys Maximum number of keys to store
 * @return Number of keys stored
 */
int getAllConfigKeys(char** keys, int max_keys);

/**
 * @brief Clean up configuration resources
 */
void config_cleanup(void);

#endif /* CONFIG_MANAGER_H */