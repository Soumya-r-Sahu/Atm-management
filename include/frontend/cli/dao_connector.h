/**
 * @file dao_connector.h
 * @brief Frontend DAO connector for Core Banking System
 * @version 1.0
 * @date May 10, 2025
 */

#ifndef DAO_CONNECTOR_H
#define DAO_CONNECTOR_H

#include "../../common/database/dao_interface.h"
#include "../../common/utils/logger.h"

/**
 * @brief Get the DAO instance for frontend operations
 * @return DatabaseAccessObject* Pointer to DAO instance
 */
DatabaseAccessObject* getFrontendDAO(void);

/**
 * @brief Initialize frontend DAO connection
 * @return bool True if initialized successfully
 */
bool initFrontendDAO(void);

/**
 * @brief Close frontend DAO connection
 */
void closeFrontendDAO(void);

#endif // DAO_CONNECTOR_H
