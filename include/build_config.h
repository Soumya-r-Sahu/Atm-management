/**
 * @file build_config.h
 * @brief Central build configuration for the ATM project
 */

#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

// Enable/disable MySQL integration
#ifdef NO_MYSQL
    #define USE_MYSQL_DB 0
    #define USE_MOCK_DB 1
#else
    #define USE_MYSQL_DB 1
    #define USE_MOCK_DB 0
#endif

// Enable/disable components
#define BUILD_CLI_FRONTEND 1
#define BUILD_ATM_SYSTEM 1
#define BUILD_ADMIN_SYSTEM 1
#define BUILD_TESTS 1

// Debug configuration
#define ENABLE_DEBUG_LOGS 1
#define ENABLE_SECURITY_LOGS 1

// Path configurations
#define DEFAULT_DATA_PATH "data"
#define DEFAULT_LOGS_PATH "logs"

// Build timestamp
#define BUILD_TIMESTAMP __DATE__ " " __TIME__

#endif // BUILD_CONFIG_H
