#ifndef ADMIN_OPERATIONS_H
#define ADMIN_OPERATIONS_H

#include <time.h>

// Admin operation functions
int create_account(void);
int toggle_service_mode(void);
void regenerate_card_pin(int card_number);
void toggle_card_status(int card_number);

// Service status functions
int get_service_status(void);
int set_service_status(int status);

// ATM status management
int update_atm_status(const char* atm_id, const char* new_status);

#endif // ADMIN_OPERATIONS_H