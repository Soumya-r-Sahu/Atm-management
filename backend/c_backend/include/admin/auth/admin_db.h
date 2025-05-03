#ifndef ADMIN_DB_H
#define ADMIN_DB_H

// Admin credentials operations
int loadAdminCredentials(char *adminId, char *adminPass);
int updateAdminCredentials(const char *newAdminId, const char *newAdminPass);

// ATM account operations
int createNewAccount(const char *accountHolderName, int cardNumber, int pin);
int generateUniqueCardNumber();
int generateRandomPin();
int isCardNumberUnique(int cardNumber);
int updateCardDetails(int cardNumber, int newPIN, const char *newStatus);

// ATM service status operations
int toggleServiceMode();
int getServiceStatus();
int setServiceStatus(int isOutOfService);

#endif // ADMIN_DB_H