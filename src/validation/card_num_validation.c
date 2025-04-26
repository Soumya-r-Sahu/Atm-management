#include "card_num_validation.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CREDENTIALS_FILE "../data/credentials.txt"

// Validate if a card number is in the correct format using Luhn algorithm
bool isValidCardFormat(long long cardNumber) {
    int sum = 0;
    bool alternate = false;
    
    // Convert number to string for easier processing
    char cardStr[20];
    sprintf(cardStr, "%lld", cardNumber);
    int len = strlen(cardStr);
    
    // Check card length - most cards are between 13-19 digits
    if (len < 13 || len > 19) {
        return false;
    }
    
    // Apply Luhn algorithm
    for (int i = len - 1; i >= 0; i--) {
        int n = cardStr[i] - '0';
        if (alternate) {
            n *= 2;
            if (n > 9) {
                n = (n % 10) + 1;
            }
        }
        sum += n;
        alternate = !alternate;
    }
    
    return (sum % 10 == 0);
}

// Check if a card exists in our system
bool doesCardExist(int cardNumber) {
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open credentials file for card existence check");
        return false;
    }
    
    char line[256];
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    int storedCardNumber;
    int storedPIN;
    char storedUsername[50], storedStatus[10];
    bool found = false;
    
    while (fscanf(file, "%49[^|] | %d | %d | %9s", 
                 storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            found = true;
            break;
        }
    }
    
    fclose(file);
    return found;
}

// Function to validate both format and existence
bool validateCardNumber(long long cardNumber) {
    // First check format
    if (!isValidCardFormat(cardNumber)) {
        char logMsg[100];
        sprintf(logMsg, "Card number %lld has invalid format", cardNumber);
        writeErrorLog(logMsg);
        return false;
    }
    
    // Then check existence in our system
    if (!doesCardExist((int)cardNumber)) {
        char logMsg[100];
        sprintf(logMsg, "Card number %lld not found in system", cardNumber);
        writeErrorLog(logMsg);
        return false;
    }
    
    return true;
}

// Function to get a readable card type name based on the card number
const char* getCardType(long long cardNumber) {
    char cardStr[20];
    sprintf(cardStr, "%lld", cardNumber);
    int len = strlen(cardStr);
    
    // Get the first digit
    int firstDigit = cardStr[0] - '0';
    
    // Get the first two digits
    int firstTwoDigits = 0;
    if (len >= 2) {
        firstTwoDigits = (cardStr[0] - '0') * 10 + (cardStr[1] - '0');
    }
    
    // Determine card type based on industry standards
    if (firstDigit == 4) {
        return "Visa";
    } else if ((firstTwoDigits >= 51 && firstTwoDigits <= 55) || 
               (firstTwoDigits >= 22 && firstTwoDigits <= 27)) {
        return "MasterCard";
    } else if (firstTwoDigits == 34 || firstTwoDigits == 37) {
        return "American Express";
    } else if (firstTwoDigits == 36 || firstTwoDigits == 38 || firstTwoDigits == 39) {
        return "Diner's Club";
    } else if (firstTwoDigits == 62 || firstTwoDigits == 88) {
        return "China UnionPay";
    } else {
        return "Unknown";
    }
}

// Function to mask a card number for display (e.g., **** **** **** 1234)
void maskCardNumber(long long cardNumber, char* maskedOutput, int maxLength) {
    char cardStr[20];
    sprintf(cardStr, "%lld", cardNumber);
    int len = strlen(cardStr);
    
    // Only show the last 4 digits
    if (len <= 4) {
        // Card number is too short, just copy it as is
        strncpy(maskedOutput, cardStr, maxLength - 1);
    } else {
        int i, j;
        
        // Fill with asterisks, grouped by 4 with spaces between groups
        for (i = 0, j = 0; i < len - 4 && j < maxLength - 5; i++) {
            if (i > 0 && i % 4 == 0) {
                maskedOutput[j++] = ' ';
            }
            maskedOutput[j++] = '*';
        }
        
        // Add a space before the last 4 digits if we have room
        if (j < maxLength - 5 && (len - 4) % 4 == 0) {
            maskedOutput[j++] = ' ';
        }
        
        // Add the last 4 digits
        for (i = len - 4; i < len && j < maxLength - 1; i++) {
            maskedOutput[j++] = cardStr[i];
        }
        
        maskedOutput[j] = '\0';
    }
}
