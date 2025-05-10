/**
 * @file dao_connector.c
 * @brief Implementation of frontend DAO connector
 */

#include "../../../include/frontend/cli/dao_connector.h"
#include "../../../include/common/database/dao_interface.h"
#include "../../../include/common/database/dao_factory.h"
#include "../../../include/common/utils/logger.h"

// DAO instance
static DatabaseAccessObject* frontendDAO = NULL;

/**
 * @brief Get the DAO instance for frontend operations
 * @return DatabaseAccessObject* Pointer to DAO instance
 */
DatabaseAccessObject* getFrontendDAO(void) {
    if (frontendDAO == NULL) {
        if (!initFrontendDAO()) {
            LOG_ERROR("Failed to initialize frontend DAO");
            return NULL;
        }
    }
    return frontendDAO;
}

/**
 * @brief Initialize frontend DAO connection
 * @return bool True if initialized successfully
 */
bool initFrontendDAO(void) {
    if (frontendDAO != NULL) {
        return true; // Already initialized
    }
    
    frontendDAO = getDAO();
    if (frontendDAO == NULL) {
        LOG_ERROR("Failed to get DAO from factory");
        return false;
    }
    
    LOG_INFO("Frontend DAO initialized successfully");
    return true;
}

/**
 * @brief Close frontend DAO connection
 */
void closeFrontendDAO(void) {
    // The DAO instance is managed by the factory, we don't need to free it
    frontendDAO = NULL;
    LOG_INFO("Frontend DAO connection closed");
}
