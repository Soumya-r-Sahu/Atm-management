#ifndef ADMIN_INTERFACE_H
#define ADMIN_INTERFACE_H

// Admin information structure
typedef struct {
    char adminId[50];
    char username[50];
    char passwordHash[65];
    char role[20];
    char lastLogin[30];
    char status[10];
} AdminInfo;

// Admin interface functions
void processByAdminRole(const char* role);
int handleAdminAuthentication(int atmId);
int handleAtmModeAuthentication(int atmId);

// Admin main function (renamed from main in admin.c)
int admin_main(void);

#endif // ADMIN_INTERFACE_H