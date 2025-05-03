#ifndef ADMIN_AUTH_H
#define ADMIN_AUTH_H

#include <stdbool.h>
#include <time.h>

#define MAX_ADMIN_ROLES 5
#define MAX_ROLE_LENGTH 20

/**
 * Structure to represent an admin session
 */
typedef struct {
    char username[50];
    char roles[MAX_ADMIN_ROLES][MAX_ROLE_LENGTH];
    int num_roles;
    time_t login_time;
    time_t last_activity_time;
    bool is_logged_in;
} AdminUser;

/**
 * Initialize the admin authentication system
 * @return true if initialization successful, false otherwise
 */
bool initialize_admin_auth();

/**
 * Attempt to authenticate an admin user
 * @param username Admin username
 * @param password Admin password
 * @param admin Pointer to AdminUser struct to be populated if authentication succeeds
 * @return true if authentication successful, false otherwise
 */
bool authenticate_admin(const char* username, const char* password, AdminUser* admin);

/**
 * Create a new admin session after successful authentication
 * @param username Authenticated admin username
 * @param roles Array of roles assigned to the admin
 * @param num_roles Number of roles
 * @return Pointer to new AdminUser session or NULL if failed
 */
AdminUser* create_admin_session(const char* username, const char* roles[], int num_roles);

/**
 * End an admin session and clean up resources
 * @param admin Pointer to the admin session to end
 * @return true if session was ended successfully, false otherwise
 */
bool end_admin_session(AdminUser* admin);

/**
 * Check if admin session is still valid (not timed out)
 * @param admin Pointer to the admin session to check
 * @return true if session is still valid, false if timed out
 */
bool is_admin_session_valid(AdminUser* admin);

/**
 * Update the last activity time of an admin session
 * @param admin Pointer to the admin session to update
 */
void update_admin_activity(AdminUser* admin);

/**
 * Change an admin's password
 * @param admin Pointer to the admin session
 * @param old_password Current password
 * @param new_password New password
 * @return true if password was changed successfully, false otherwise
 */
bool change_admin_password(AdminUser* admin, const char* old_password, const char* new_password);

/**
 * Check if admin has a specific role
 * @param admin Pointer to the admin session
 * @param role Role to check for
 * @return true if admin has the role, false otherwise
 */
bool admin_has_role(const AdminUser* admin, const char* role);

/**
 * Check if an account is currently locked due to failed login attempts
 * @param username Admin username to check
 * @return true if account is locked, false otherwise
 */
bool is_account_locked(const char* username);

/**
 * Update login attempts counter after a failed login
 * @param username Admin username
 * @return true if update was successful, false otherwise
 */
bool update_login_attempts(const char* username);

/**
 * Reset login attempts counter after successful login
 * @param username Admin username
 * @return true if reset was successful, false otherwise
 */
bool decrement_login_attempts(const char* username);

/**
 * Update admin status (active/inactive)
 * @param admin Pointer to the admin session
 * @param username Admin username to update
 * @param status New status (true for active, false for inactive)
 * @return true if update was successful, false otherwise
 */
bool update_admin_status(AdminUser* admin, const char* username, bool status);

/**
 * Create a new admin account with specified roles
 * @param current_admin Pointer to the current admin session (must be SuperAdmin)
 * @param new_username Username for the new admin
 * @param new_password Password for the new admin
 * @param roles Array of roles to assign to the new admin
 * @param num_roles Number of roles being assigned
 * @return true if admin was created successfully, false otherwise
 */
bool create_admin_account(AdminUser* current_admin, const char* new_username, 
                         const char* new_password, const char* roles[], int num_roles);

/**
 * Check if a username already exists in the admin database
 * @param username Username to check
 * @return true if username exists, false otherwise
 */
bool admin_username_exists(const char* username);

#endif // ADMIN_AUTH_H