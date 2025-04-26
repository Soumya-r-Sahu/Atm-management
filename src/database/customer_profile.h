#ifndef CUSTOMER_PROFILE_H
#define CUSTOMER_PROFILE_H

#include <stdbool.h>

// Customer profile structure
typedef struct {
    int cardNumber;
    char name[100];
    float balance;
    char status[20];
    char lastLoginDate[20];
    int pin;
    char phoneNumber[20]; // Added phoneNumber field
} CustomerProfile;

// Function to retrieve a customer profile by card number
bool getCustomerProfile(int cardNumber, CustomerProfile *profile);

// Function to update a customer's profile
bool updateCustomerProfile(const CustomerProfile *profile);

// Function to create a new customer profile
bool createCustomerProfile(const CustomerProfile *profile);

// Function to change a customer's PIN
bool changeCustomerPIN(int cardNumber, int newPIN);

// Function to block/unblock a customer's card
bool setCardStatus(int cardNumber, const char *status);

// Function to update customer's last login date
bool updateLastLogin(int cardNumber);

// Function to print customer profile summary
void printCustomerSummary(const CustomerProfile *profile);

// Function to retrieve a customer's phone number by card number
const char* getCustomerPhoneNumber(int cardNumber);

#endif // CUSTOMER_PROFILE_H