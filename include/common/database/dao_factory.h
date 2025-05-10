/**
 * @file dao_factory.h
 * @brief Factory pattern for Database Access Objects
 * @version 1.0
 * @date May 10, 2025
 */

#ifndef DAO_FACTORY_H
#define DAO_FACTORY_H

#include "dao_interface.h"

/**
 * @brief Get the appropriate DAO based on configuration
 * This implements the Factory Pattern with Singleton optimization
 * 
 * @return DatabaseAccessObject* Active DAO instance
 */
DatabaseAccessObject* getDAO(void);

/**
 * @brief Create a MySQL DAO implementation
 * @return DatabaseAccessObject* MySQL DAO implementation or NULL on failure
 */
DatabaseAccessObject* createMySQLDAO(void);

/**
 * @brief Create a file-based DAO implementation
 * @return DatabaseAccessObject* File-based DAO implementation
 */
DatabaseAccessObject* createFileBasedDAO(void);

/**
 * @brief Create a mock DAO implementation for testing
 * @return DatabaseAccessObject* Mock DAO implementation
 */
DatabaseAccessObject* createMockDAO(void);

/**
 * @brief Release DAO resources
 */
void releaseDAO(void);

#endif // DAO_FACTORY_H
