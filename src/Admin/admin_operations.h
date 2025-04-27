#ifndef ADMIN_OPERATIONS_H
#define ADMIN_OPERATIONS_H

#include <time.h>

// Admin authentication functions
int authenticateAdmin(const char* adminId, const char* adminPass);
void handleAdminLockout(int* attempts, time_t* lockoutStartTime, int lockoutDuration);

// Core admin operations
void createAccount();
int toggleServiceMode(); // Changed return type from void to int to match admin_db.h
void regenerateCardPin(int cardNumber);
void toggleCardStatus(int cardNumber);

// Service status functions
int getServiceStatus();
int setServiceStatus(int status);

// ATM status management
int updateAtmStatus(const char* atmId, const char* newStatus);

#endif // ADMIN_OPERATIONS_H