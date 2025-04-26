#ifndef CARD_NUM_VALIDATION_H
#define CARD_NUM_VALIDATION_H

#include <stdbool.h>

// Validate if a card number is in the correct format (Luhn algorithm)
bool isValidCardFormat(long long cardNumber);

// Check if a card exists in our system
bool doesCardExist(int cardNumber);

// Function to validate both format and existence
bool validateCardNumber(long long cardNumber);

// Function to get a readable card type name based on the card number
const char* getCardType(long long cardNumber);

// Function to mask a card number for display (e.g., **** **** **** 1234)
void maskCardNumber(long long cardNumber, char* maskedOutput, int maxLength);

#endif // CARD_NUM_VALIDATION_H