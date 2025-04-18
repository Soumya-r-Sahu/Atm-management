#ifndef PIN_VALIDATION_H
#define PIN_VALIDATION_H

#include <stdbool.h>

// Function prototypes
bool validatePIN(int enteredPin, int storedPin);
void changePIN(int *storedPin);

#endif // PIN_VALIDATION_H