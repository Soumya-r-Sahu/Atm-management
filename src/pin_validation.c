#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // Include for the bool type
#include "pin_validation.h"

// Function to validate the entered PIN against the stored PIN
int validatePIN(int enteredPin, int actualPin) {
    return enteredPin == actualPin;
}

// Function to prompt the user for their PIN and validate it
bool promptForPIN(int storedPin) {
    int enteredPin;
    int attempts = 3; // Maximum attempts for PIN entry

    while (attempts > 0) {
        printf("Enter your PIN: ");
        scanf("%d", &enteredPin);

        if (validatePIN(enteredPin, storedPin)) {
            printf("Access granted!\n");
            return true; // Successful validation
        } else {
            attempts--;
            printf("Incorrect PIN. You have %d attempt(s) remaining.\n", attempts);
        }
    }

    printf("Access denied. Exiting...\n");
    exit(0); // Exit if the maximum attempts are reached
}