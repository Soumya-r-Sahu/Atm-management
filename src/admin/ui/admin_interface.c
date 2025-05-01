#include "../../include/admin/admin_interface.h"
#include "../../include/admin/admin_auth.h"
#include "../../utils/logger.h"
#include <stdio.h>
#include <string.h>

// Implementation of handleAdminAuthentication
int handleAdminAuthentication(int atmId) {
    printf("\n===== Admin Authentication =====\n");
    printf("Redirecting to admin panel...\n");
    
    // Call the admin_main function that we renamed from main in admin.c
    return admin_main();
}

// Implementation of handleAtmModeAuthentication
int handleAtmModeAuthentication(int atmId) {
    printf("\n===== ATM Authentication =====\n");
    printf("Authenticating ATM ID: %d\n", atmId);
    
    // Perform ATM authentication logic here
    // For this simple implementation, we'll accept any ATM ID
    if (atmId > 0) {
        printf("ATM authenticated successfully.\n");
        writeAuditLog("SYSTEM", "ATM authenticated");
        return 1; // Success
    } else {
        printf("Invalid ATM ID.\n");
        writeErrorLog("Invalid ATM ID entered");
        return 0; // Failure
    }
}

// Implementation of processByAdminRole
void processByAdminRole(const char* role) {
    printf("\n===== Processing by Admin Role =====\n");
    printf("Role: %s\n", role);
    
    if (strcmp(role, "Manager") == 0) {
        printf("Manager access granted: Full system access\n");
    } else if (strcmp(role, "Operator") == 0) {
        printf("Operator access granted: Limited system access\n");
    } else if (strcmp(role, "Auditor") == 0) {
        printf("Auditor access granted: Read-only access\n");
    } else {
        printf("Standard access granted: Basic functionality\n");
    }
}