/**
 * @file admin_operations_dao.h
 * @brief Admin Operations functions using DAO pattern
 * @version 1.0
 * @date May 12, 2025
 */

#ifndef ADMIN_OPERATIONS_DAO_H
#define ADMIN_OPERATIONS_DAO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new account using the DAO pattern
 * @return Result status code (1 for success, 0 for failure)
 */
int create_account_dao(void);

/**
 * @brief Toggle the ATM service mode using the DAO pattern
 * @return Current service status (1 for out-of-service, 0 for in-service)
 */
int toggle_service_mode_dao(void);

/**
 * @brief Get the current service status using the DAO pattern
 * @return Service status (1 for out-of-service, 0 for in-service)
 */
int get_service_status_dao(void);

/**
 * @brief Set the service status using the DAO pattern
 * @param status 1 for out-of-service, 0 for in-service
 * @return 1 if successful, 0 if failed
 */
int set_service_status_dao(int status);

/**
 * @brief Regenerate a card's PIN using the DAO pattern
 * @param card_number The card number
 */
void regenerate_card_pin_dao(int card_number);

/**
 * @brief Toggle a card's active status using the DAO pattern
 * @param card_number The card number
 */
void toggle_card_status_dao(int card_number);

/**
 * @brief Update ATM status using the DAO pattern
 * @param atm_id The ATM ID
 * @param new_status The new status
 * @return 1 if successful, 0 if failed
 */
int update_atm_status_dao(const char* atm_id, const char* new_status);

#ifdef __cplusplus
}
#endif

#endif /* ADMIN_OPERATIONS_DAO_H */
