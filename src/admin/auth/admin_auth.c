#include "../../../include/admin/admin_auth.h"
#include "../../../include/common/utils/logger.h"
#include "../../../include/common/utils/path_manager.h"
#include "../../../include/common/security/hash_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Maximum login attempts before lockout
#define MAX_LOGIN_ATTEMPTS 3
#define SESSION_TIMEOUT_SECONDS 1800 // 30 minutes

// Function to check if an admin has a specific role
bool admin_has_role(const AdminUser* admin, const char* role) {
    if (!admin || !role) {
        return false;
    }
    
    for (int i = 0; i < admin->num_roles; i++) {
        if (strcmp(admin->roles[i], role) == 0) {
            return true;
        }
    }
    
    return false;
}

// Function to initialize admin authentication system
bool initialize_admin_auth() {
    write_info_log("Admin authentication system initialized");
    return true;
}

// Function to get admin credentials from file
bool get_admin_credentials(const char* username, char* password_hash, int max_hash_len, char* roles, int max_roles_len) {
    if (!username || !password_hash || max_hash_len <= 0) {
        write_error_log("Invalid parameters to get_admin_credentials");
        return false;
    }
    
    FILE* file = fopen(getAdminCredentialsFilePath(), "r");
    if (!file) {
        write_error_log("Failed to open admin credentials file");
        return false;
    }
    
    char line[256];
    bool found = false;
    
    // Skip header lines (first two lines)
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Process data lines
    while (fgets(line, sizeof(line), file)) {
        char admin_username[50];
        char admin_password[100];
        char admin_roles[100];
        char status[20];
        
        // Expected format: | USERNAME | PASSWORD_HASH | ROLES | STATUS |
        if (sscanf(line, "| %s | %s | %s | %s |", 
                 admin_username, admin_password, admin_roles, status) >= 4) {
            
            // Check if this is the user we're looking for
            if (strcmp(admin_username, username) == 0) {
                strncpy(password_hash, admin_password, max_hash_len - 1);
                password_hash[max_hash_len - 1] = '\0';
                
                if (roles && max_roles_len > 0) {
                    strncpy(roles, admin_roles, max_roles_len - 1);
                    roles[max_roles_len - 1] = '\0';
                }
                
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

// Function to authenticate an admin user
bool authenticate_admin(const char* username, const char* password, AdminUser* admin) {
    if (!username || !password || !admin) {
        write_error_log("Invalid parameters to authenticate_admin");
        return false;
    }
    
    if (is_account_locked(username)) {
        char log_msg[100];
        sprintf(log_msg, "Authentication attempt for locked account: %s", username);
        write_audit_log("SECURITY", log_msg);
        return false;
    }
    
    char password_hash[100];
    char roles_str[100];
    
    if (!get_admin_credentials(username, password_hash, sizeof(password_hash), 
                              roles_str, sizeof(roles_str))) {
        char log_msg[100];
        sprintf(log_msg, "Authentication failed - user not found: %s", username);
        write_audit_log("SECURITY", log_msg);
        update_login_attempts(username);
        return false;
    }
    
    // Verify password using salted hash
    char password_buffer[200];
    sprintf(password_buffer, "%s", password);
    bool is_valid = verify_password(password_buffer, password_hash);
    
    if (!is_valid) {
        char log_msg[100];
        sprintf(log_msg, "Authentication failed - invalid password for: %s", username);
        write_audit_log("SECURITY", log_msg);
        update_login_attempts(username);
        return false;
    }
    
    // Parse roles from roles_str
    char role_copy[100];
    strcpy(role_copy, roles_str);
    char* roles[MAX_ADMIN_ROLES];
    int num_roles = 0;
    
    char* role_token = strtok(role_copy, ",");
    while (role_token && num_roles < MAX_ADMIN_ROLES) {
        roles[num_roles++] = role_token;
        role_token = strtok(NULL, ",");
    }
    
    // Reset login attempts
    decrement_login_attempts(username);
    
    // Set up admin session
    strncpy(admin->username, username, sizeof(admin->username) - 1);
    admin->username[sizeof(admin->username) - 1] = '\0';
    
    for (int i = 0; i < num_roles && i < MAX_ADMIN_ROLES; i++) {
        strncpy(admin->roles[i], roles[i], MAX_ROLE_LENGTH - 1);
        admin->roles[i][MAX_ROLE_LENGTH - 1] = '\0';
    }
    admin->num_roles = num_roles;
    
    admin->login_time = time(NULL);
    admin->last_activity_time = admin->login_time;
    admin->is_logged_in = true;
    
    char log_msg[100];
    sprintf(log_msg, "Admin authentication successful: %s", username);
    write_audit_log("SECURITY", log_msg);
    
    return true;
}

// Create a new admin session
AdminUser* create_admin_session(const char* username, const char* roles[], int num_roles) {
    if (!username || !roles || num_roles <= 0) {
        write_error_log("Invalid parameters for create_admin_session");
        return NULL;
    }
    
    AdminUser* admin = (AdminUser*)malloc(sizeof(AdminUser));
    if (!admin) {
        write_error_log("Memory allocation failed for admin session");
        return NULL;
    }
    
    strncpy(admin->username, username, sizeof(admin->username) - 1);
    admin->username[sizeof(admin->username) - 1] = '\0';
    
    admin->num_roles = (num_roles > MAX_ADMIN_ROLES) ? MAX_ADMIN_ROLES : num_roles;
    for (int i = 0; i < admin->num_roles; i++) {
        strncpy(admin->roles[i], roles[i], MAX_ROLE_LENGTH - 1);
        admin->roles[i][MAX_ROLE_LENGTH - 1] = '\0';
    }
    
    admin->login_time = time(NULL);
    admin->last_activity_time = admin->login_time;
    admin->is_logged_in = true;
    
    return admin;
}

// End an admin session
bool end_admin_session(AdminUser* admin) {
    if (!admin) {
        return false;
    }
    
    if (admin->is_logged_in) {
        char log_msg[100];
        sprintf(log_msg, "Admin session ended: %s", admin->username);
        write_audit_log("SECURITY", log_msg);
    }
    
    admin->is_logged_in = false;
    admin->num_roles = 0;
    
    return true;
}

// Check if an admin session is still valid
bool is_admin_session_valid(AdminUser* admin) {
    if (!admin || !admin->is_logged_in) {
        return false;
    }
    
    time_t current_time = time(NULL);
    double elapsed = difftime(current_time, admin->last_activity_time);
    
    if (elapsed > SESSION_TIMEOUT_SECONDS) {
        admin->is_logged_in = false;
        char log_msg[100];
        sprintf(log_msg, "Admin session timed out: %s", admin->username);
        write_audit_log("SECURITY", log_msg);
        return false;
    }
    
    return true;
}

// Update admin session activity timestamp
void update_admin_activity(AdminUser* admin) {
    if (admin && admin->is_logged_in) {
        admin->last_activity_time = time(NULL);
    }
}

// Change admin password
bool change_admin_password(AdminUser* admin, const char* old_password, const char* new_password) {
    if (!admin || !admin->is_logged_in || !old_password || !new_password) {
        write_error_log("Invalid parameters for change_admin_password");
        return false;
    }
    
    // Verify old password
    char stored_hash[100];
    if (!get_admin_credentials(admin->username, stored_hash, sizeof(stored_hash), NULL, 0)) {
        write_error_log("Failed to retrieve admin credentials");
        return false;
    }
    
    if (!verify_password(old_password, stored_hash)) {
        write_audit_log("SECURITY", "Password change failed: incorrect old password");
        return false;
    }
    
    // Create hash of new password
    char* new_hash = create_salted_hash(new_password, NULL);
    if (!new_hash) {
        write_error_log("Failed to create hash for new password");
        return false;
    }
    
    // Update password in file
    FILE* file = fopen(getAdminCredentialsFilePath(), "r");
    if (!file) {
        write_error_log("Failed to open admin credentials file");
        free(new_hash);
        return false;
    }
    
    char tempFile[256];
    sprintf(tempFile, "%s.tmp", getAdminCredentialsFilePath());
    
    FILE* temp = fopen(tempFile, "w");
    if (!temp) {
        write_error_log("Failed to create temporary admin credentials file");
        fclose(file);
        free(new_hash);
        return false;
    }
    
    char line[256];
    bool updated = false;
    
    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp);
    }
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp);
    }
    
    // Process and update the needed line
    while (fgets(line, sizeof(line), file)) {
        char username[50];
        char password[100];
        char roles[100];
        char status[20];
        
        if (sscanf(line, "| %s | %s | %s | %s |", username, password, roles, status) >= 4) {
            if (strcmp(username, admin->username) == 0) {
                fprintf(temp, "| %s | %s | %s | %s |\n", 
                       admin->username, new_hash, roles, status);
                updated = true;
            } else {
                fputs(line, temp);
            }
        } else {
            fputs(line, temp);
        }
    }
    
    fclose(file);
    fclose(temp);
    free(new_hash);
    
    if (!updated) {
        write_error_log("Admin user not found in credentials file");
        remove(tempFile);
        return false;
    }
    
    // Replace original with temp file
    if (remove(getAdminCredentialsFilePath()) != 0) {
        write_error_log("Failed to remove original admin credentials file");
        return false;
    }
    
    if (rename(tempFile, getAdminCredentialsFilePath()) != 0) {
        write_error_log("Failed to rename temp file to admin credentials file");
        return false;
    }
    
    write_audit_log("SECURITY", "Admin password changed successfully");
    return true;
}

// Check if account is locked due to too many failed attempts
bool is_account_locked(const char* username) {
    // For a real implementation, you would check a lockout file or database
    // Here's a simplified version that always returns false
    return false;
}

// Update login attempts counter after failed login
bool update_login_attempts(const char* username) {
    // For a real implementation, you would increment a counter in a file or database
    // Here's a simplified version that always returns true
    char log_msg[100];
    sprintf(log_msg, "Failed login attempt for admin user: %s", username);
    write_error_log(log_msg);
    return true;
}

// Reset login attempts counter after successful login
bool decrement_login_attempts(const char* username) {
    // For a real implementation, you would reset a counter in a file or database
    // Here's a simplified version that always returns true
    return true;
}

// Update admin status (active/inactive)
bool update_admin_status(AdminUser* admin, const char* target_username, bool status) {
    if (!admin || !admin->is_logged_in || !target_username) {
        write_error_log("Invalid parameters for update_admin_status");
        return false;
    }
    
    // Check if the admin has the right privileges (must be super_admin)
    if (!admin_has_role(admin, "super_admin")) {
        write_error_log("Insufficient privileges to update admin status");
        return false;
    }
    
    // Open the admin credentials file
    FILE* file = fopen(getAdminCredentialsFilePath(), "r");
    if (!file) {
        write_error_log("Failed to open admin credentials file");
        return false;
    }
    
    char tempFile[256];
    sprintf(tempFile, "%s.tmp", getAdminCredentialsFilePath());
    
    FILE* temp = fopen(tempFile, "w");
    if (!temp) {
        write_error_log("Failed to create temporary admin credentials file");
        fclose(file);
        return false;
    }
    
    char line[256];
    bool updated = false;
    
    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp);
    }
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp);
    }
    
    // Process and update the needed line
    while (fgets(line, sizeof(line), file)) {
        char username[50];
        char password[100];
        char roles[100];
        char status_str[20];
        
        if (sscanf(line, "| %s | %s | %s | %s |", username, password, roles, status_str) >= 4) {
            if (strcmp(username, target_username) == 0) {
                fprintf(temp, "| %s | %s | %s | %s |\n", 
                       username, password, roles, status ? "active" : "inactive");
                updated = true;
            } else {
                fputs(line, temp);
            }
        } else {
            fputs(line, temp);
        }
    }
    
    fclose(file);
    fclose(temp);
    
    if (!updated) {
        write_error_log("Target admin user not found in credentials file");
        remove(tempFile);
        return false;
    }
    
    // Replace original with temp file
    if (remove(getAdminCredentialsFilePath()) != 0) {
        write_error_log("Failed to remove original admin credentials file");
        return false;
    }
    
    if (rename(tempFile, getAdminCredentialsFilePath()) != 0) {
        write_error_log("Failed to rename temp file to admin credentials file");
        return false;
    }
    
    char log_msg[100];
    sprintf(log_msg, "Admin status updated for %s to %s", target_username, status ? "active" : "inactive");
    write_audit_log("ADMIN", log_msg);
    
    return true;
}