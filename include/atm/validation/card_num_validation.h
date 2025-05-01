#ifndef CARD_NUM_VALIDATION_H
#define CARD_NUM_VALIDATION_H

#include <stdbool.h>

// Check if a card number is in a valid format (Luhn algorithm)
bool isValidCardFormat(long long cardNumber);

// Check if a card exists in our database
bool cardExistsInSystem(int cardNumber);

// Validate both format and existence of a card
bool validateCardNumber(long long cardNumber);

// Get the card type based on the card number
const char* getCardType(long long cardNumber);

// Create a masked version of the card number (e.g., **** **** **** 1234)
void maskCardNumber(long long cardNumber, char* maskedOutput, int maxLength);

#endif // CARD_NUM_VALIDATION_H