#ifndef ADMIN_MENU_H
#define ADMIN_MENU_H

#include "admin_auth.h"
#include <stdbool.h>

// Initialize the admin interface
bool admin_menu_initialize(void);

// Display admin login screen
bool show_admin_login_screen(char* username, char* password);

// Display admin welcome screen
void show_admin_welcome_screen(AdminUser* admin);

// Display main admin menu and handle selection
int show_admin_main_menu(AdminUser* admin);

// Display card management menu
void show_card_management_menu(AdminUser* admin);

// Display customer management menu
void show_customer_management_menu(AdminUser* admin);

// Display transaction management menu
void show_transaction_management_menu(AdminUser* admin);

// Display system configuration menu
void show_system_config_menu(AdminUser* admin);

// Display user management menu
void show_user_management_menu(AdminUser* admin);

// Display reports menu
void show_reports_menu(AdminUser* admin);

// Display activity logs
void show_activity_logs(AdminUser* admin);

// Display change password screen
bool show_change_password_screen(AdminUser* admin);

// Display error screen
void show_admin_error_screen(const char* message);

// Display success screen
void show_admin_success_screen(const char* message);

// Clear screen (cross-platform)
void clear_admin_screen(void);

// Show confirmation dialog
bool show_admin_confirmation(const char* message);

#endif // ADMIN_MENU_H