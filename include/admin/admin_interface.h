#ifndef ADMIN_INTERFACE_H
#define ADMIN_INTERFACE_H

// Admin information structure
typedef struct {
    char admin_id[50];
    char username[50];
    char password_hash[65];
    char role[20];
    char last_login[30];
    char status[10];
} AdminInfo;

// Admin interface functions
void process_by_admin_role(const char* role);
int handle_admin_authentication(int atm_id);
int handle_atm_mode_authentication(int atm_id);

// Admin main function
int admin_main(void);

#endif // ADMIN_INTERFACE_H