#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

// Configuration structure
typedef struct {
    char id[7];
    char name[30];
    char value[30];
    char updated_at[30];
} SystemConfig;

// Constants for config names (for easy reference)
#define CONFIG_ATM_WITHDRAWAL_LIMIT    "ATM_WITHDRAWAL_LIMIT"
#define CONFIG_MIN_BALANCE             "MIN_BALANCE"
#define CONFIG_MAX_WRONG_PIN_ATTEMPTS  "MAX_WRONG_PIN_ATTEMPTS"
#define CONFIG_SESSION_TIMEOUT_SECONDS "SESSION_TIMEOUT_SECONDS"
#define CONFIG_DAILY_TRANSACTION_LIMIT "DAILY_TRANSACTION_LIMIT"
#define CONFIG_DEFAULT_LANGUAGE        "DEFAULT_LANGUAGE"
#define CONFIG_MAINTENANCE_MODE        "MAINTENANCE_MODE"

// Array of all configurations
extern SystemConfig* g_systemConfigs;
extern int g_configCount;

// Initialize and load configurations
int initializeConfigs();

// Free allocated memory for configurations
void freeConfigs();

// Get configuration by name
const char* getConfigValue(const char* configName);
int getConfigValueInt(const char* configName);
double getConfigValueDouble(const char* configName);
int getConfigValueBool(const char* configName);

// Update configuration (admin only)
int updateConfig(const char* configName, const char* newValue);

// Save configurations to file (admin only)
int saveConfigs();

#endif // CONFIG_MANAGER_H