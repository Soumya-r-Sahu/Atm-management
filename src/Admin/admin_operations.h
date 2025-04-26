#ifndef ADMIN_OPERATIONS_H
#define ADMIN_OPERATIONS_H

#include <time.h>

// Admin authentication functions
int authenticateAdmin(const char* adminId, const char* adminPass);
void handleAdminLockout(int* attempts, time_t* lockoutStartTime, int lockoutDuration);

// Core admin operations
void createAccount();
void toggleServiceMode();
void regenerateCardPin(int cardNumber);
void toggleCardStatus(int cardNumber);

// Service status functions
int getServiceStatus();

#endif // ADMIN_OPERATIONS_H