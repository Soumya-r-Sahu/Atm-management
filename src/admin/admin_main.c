#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/admin/admin_operations.h"
#include "../../include/admin/admin_menu.h"
#include "../../include/admin/admin_auth.h"
#include "../../include/common/utils/logger.h"
#include "../../include/common/database/database.h"
#include "../../include/common/config/config_manager.h"
#include "../../include/admin/ui/admin_menu.h"
#include "../../include/common/utils/path_manager.h"

// Constants
#define ADMIN_LOCKOUT_DURATION 60 // seconds
#define MAX_ADMIN_ATTEMPTS 3

// Main function for the admin application
int main(int argc, char** argv) {
    // Initialize necessary components
    if (!initialize_database()) {
        fprintf(stderr, "Failed to initialize database\n");
        return EXIT_FAILURE;
    }

    if (!initialize_admin_auth()) {
        fprintf(stderr, "Failed to initialize admin authentication system\n");
        return EXIT_FAILURE;
    }

    writeAuditLog("ADMIN", "Admin application started");

    // Process command line arguments if any
    bool testMode = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test") == 0) {
            testMode = true;
            setTestingMode(1);
            writeAuditLog("ADMIN", "Test mode enabled");
        }
    }

    // Authentication
    char username[50], password[50];
    AdminUser* admin = NULL;
    int attempts = 0;
    const int MAX_ATTEMPTS = 3;
    bool authenticated = false;

    printf("=======================================\n");
    printf("=          ADMIN CONSOLE             =\n");
    printf("=======================================\n\n");

    while (attempts < MAX_ATTEMPTS && !authenticated) {
        printf("\nUsername: ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = 0; // Remove newline

        printf("Password: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = 0; // Remove newline

        // Create admin user for session
        admin = (AdminUser*)malloc(sizeof(AdminUser));
        if (!admin) {
            fprintf(stderr, "Memory allocation failed\n");
            writeErrorLog("Memory allocation failed for admin user");
            return EXIT_FAILURE;
        }

        // Authenticate
        authenticated = authenticate_admin(username, password, admin);
        if (!authenticated) {
            printf("\nInvalid username or password.\n");
            free(admin);
            admin = NULL;
            attempts++;

            if (attempts < MAX_ATTEMPTS) {
                printf("Attempts remaining: %d\n", MAX_ATTEMPTS - attempts);
            }
        } else {
            writeAuditLog("ADMIN", "Admin login successful");
        }
    }

    if (!authenticated) {
        printf("\nToo many failed login attempts. Exiting.\n");
        writeAuditLog("ADMIN", "Login failed after maximum attempts");
        return EXIT_FAILURE;
    }

    // Enter admin menu
    admin_menu_initialize();
    show_admin_main_menu(admin);

    // Cleanup
    if (admin) {
        end_admin_session(admin);
        free(admin);
    }

    writeAuditLog("ADMIN", "Admin application shutting down");
    return EXIT_SUCCESS;
}