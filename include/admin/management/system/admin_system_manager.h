#ifndef ADMIN_SYSTEM_MANAGER_H
#define ADMIN_SYSTEM_MANAGER_H

#include "../../../admin/admin_auth.h"
#include <stdbool.h>

/**
 * Toggle ATM/Banking service mode between online and offline
 * 
 * @param admin Pointer to the admin user who is changing the service mode
 * @return true if mode changed successfully, false otherwise
 */
bool toggle_service_mode(AdminUser* admin);

/**
 * View system log files (error, audit, transaction, withdrawals)
 * 
 * @param admin Pointer to the admin user who is viewing the logs
 * @return true if operation completed successfully, false otherwise
 */
bool view_system_logs(AdminUser* admin);

/**
 * Manage system configuration settings (view, edit, add)
 * 
 * @param admin Pointer to the admin user who is managing configurations
 * @return true if operation completed successfully, false otherwise
 */
bool manage_system_config(AdminUser* admin);

/**
 * Back up system data files to a timestamped directory
 * 
 * @param admin Pointer to the admin user who is performing the backup
 * @return true if backup completed successfully, false otherwise
 */
bool backup_system_data(AdminUser* admin);

#endif // ADMIN_SYSTEM_MANAGER_H