// filepath: src/card_num_validation.c
#include "card_num_validation.h"
#include <stdio.h>
#include <string.h>

// ============================
// Card Number Validation
// ============================

// Function to load credentials for a given card number
int loadCredentials(int cardNumber, int *pin, char *accountHolderName) {
    FILE *file = fopen("data/credentials.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open credentials file.\n");
        return 0;
    }

    char line[256];
    // Skip the header lines
    fgets(line, sizeof(line), file); // Skip the first line (column names)
    fgets(line, sizeof(line), file); // Skip the second line (separator)

    int storedCardNumber, storedPIN;
    char storedUsername[50];
    while (fscanf(file, "%20s | %d | %d", storedUsername, &storedCardNumber, &storedPIN) != EOF) {
        if (storedCardNumber == cardNumber) {
            *pin = storedPIN;
            strcpy(accountHolderName, storedUsername);
            fclose(file);
            return 1; // Card found
        }
    }

    fclose(file);
    return 0; // Card not found
}

// Function to validate if a card number exists
int isCardNumberValid(int cardNumber) {
    FILE *file = fopen("data/credentials.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open credentials file.\n");
        return 0; // File not found
    }

    char line[256];
    // Skip the header lines
    fgets(line, sizeof(line), file); // Skip the first line (column names)
    fgets(line, sizeof(line), file); // Skip the second line (separator)

    int storedCardNumber, storedPIN;
    char storedUsername[50];
    while (fscanf(file, "%20s | %d | %d", storedUsername, &storedCardNumber, &storedPIN) != EOF) {
        if (storedCardNumber == cardNumber) {
            fclose(file);
            return 1; // Card number is valid
        }
    }

    fclose(file);
    return 0; // Card number is invalid
}
