#ifndef PIN_VALIDATION_H
#define PIN_VALIDATION_H

#include <stdbool.h>

// Maximum allowed PIN attempts before lockout
#define MAX_PIN_ATTEMPTS 3

// Duration of lockout period in seconds (5 minutes)
#define PIN_LOCKOUT_DURATION 300

// Basic PIN validation function
int validatePIN(int enteredPin, int actualPin);

// Function to prompt the user for their PIN and validate it
bool promptForPIN(int storedPin);

// Function to record a failed PIN attempt
void recordFailedPINAttempt(int cardNumber);

// Function to reset the PIN attempt counter
void resetPINAttempts(int cardNumber);

// Function to check if a card is temporarily locked out due to failed PIN attempts
bool isCardLockedOut(int cardNumber, int *remainingLockoutTime);

// Function to get the number of remaining PIN attempts
int getRemainingPINAttempts(int cardNumber);

#endif // PIN_VALIDATION_H