#ifndef ADMIN_MENU_H
#define ADMIN_MENU_H

#include "../admin_auth.h"

/**
 * Initialize the admin menu system
 */
void admin_menu_initialize();

/**
 * Show the main admin menu with all available options
 * @param admin The authenticated admin user session
 */
void show_admin_main_menu(AdminUser* admin);

/**
 * Show the change password menu for the admin
 * @param admin The authenticated admin user session
 */
void show_change_password_menu(AdminUser* admin);

/**
 * Show the create admin account menu for SuperAdmin users
 * @param admin The authenticated admin user session (must be SuperAdmin)
 */
void show_create_admin_menu(AdminUser* admin);

#endif // ADMIN_MENU_H