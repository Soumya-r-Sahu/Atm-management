/**
 * @file dao_factory.c
 * @brief Factory implementation for Database Access Objects
 * @version 1.0
 * @date May 10, 2025
 */

#include "../../include/common/database/dao_interface.h"
#include "../../include/common/utils/logger.h"
#include "../../include/common/database/db_unified_config.h"
#include <stdlib.h>

// External DAO creation functions
extern DatabaseAccessObject* createMySQLDAO(void);
extern DatabaseAccessObject* createFileBasedDAO(void);

// Singleton instance
static DatabaseAccessObject* instance = NULL;

/**
 * @brief Get the appropriate DAO based on configuration
 * This implements the Factory Pattern with Singleton optimization
 * 
 * @return DatabaseAccessObject* Active DAO instance
 */
DatabaseAccessObject* getDAO(void) {
    // Return existing instance if available
    if (instance) {
        return instance;
    }
    
    // Create new instance based on configuration
#if USE_MYSQL
    instance = createMySQLDAO();
    if (instance) {
        writeInfoLog("Created MySQL DAO implementation");
        return instance;
    } else {
        writeWarningLog("MySQL DAO creation failed, falling back to file-based");
    }
#endif
    
    // Fallback to file-based DAO
    instance = createFileBasedDAO();
    if (instance) {
        writeInfoLog("Created File-based DAO implementation");
    } else {
        writeErrorLog("Failed to create any DAO implementation");
    }
    
    return instance;
}

/**
 * @brief Release the DAO instance
 * Should be called during system shutdown
 */
void releaseDAO(void) {
    if (instance) {
        free(instance);
        instance = NULL;
        writeInfoLog("Released DAO instance");
    }
}
