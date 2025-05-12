/**
 * @file config.h
 * @brief Configuration structures and handlers
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "global.h"

/**
 * Configuration sections
 */
#define CONFIG_SECTION_DATABASE    "DATABASE"
#define CONFIG_SECTION_API         "API"
#define CONFIG_SECTION_LOGGING     "LOGGING"
#define CONFIG_SECTION_SECURITY    "SECURITY"
#define CONFIG_SECTION_SYSTEM      "SYSTEM"

/**
 * @brief Loads system configuration from file or database
 * 
 * @return int Status code (SUCCESS or error code)
 */
int load_config();

/**
 * @brief Validates the loaded configuration
 * 
 * @return int 1 if valid, 0 if invalid
 */
int validate_config();

/**
 * @brief Retrieves a specific configuration value as a string
 * 
 * @param key The configuration key to retrieve
 * @param default_value Default value to return if key not found
 * @return char* The configuration value
 */
char* get_config_value(const char* key, const char* default_value);

/**
 * @brief Retrieves a specific configuration value as an integer
 * 
 * @param key The configuration key to retrieve
 * @param default_value Default value to return if key not found
 * @return int The configuration value
 */
int get_config_int(const char* key, int default_value);

/**
 * @brief Retrieves a specific configuration value as a double
 * 
 * @param key The configuration key to retrieve
 * @param default_value Default value to return if key not found
 * @return double The configuration value
 */
double get_config_double(const char* key, double default_value);

/**
 * @brief Sets a configuration value
 * 
 * @param key The configuration key to set
 * @param value The value to set
 * @return int Status code (SUCCESS or error code)
 */
int set_config_value(const char* key, const char* value);

/**
 * @brief Saves the current configuration
 * 
 * @return int Status code (SUCCESS or error code)
 */
int save_config();

#endif /* CONFIG_H */
