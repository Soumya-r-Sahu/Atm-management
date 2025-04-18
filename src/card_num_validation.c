// filepath: src/card_num_validation.c
#include "card_num_validation.h"
#include <stdio.h>
#include <string.h>

int loadCredentials(int cardNumber, int *pin, char *username) {
    FILE *file = fopen("data/credentials.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open credentials file.\n");
        return 0;
    }

    int storedCardNumber, storedPIN;
    char storedUsername[50];
    while (fscanf(file, "%d %d %s", &storedCardNumber, &storedPIN, storedUsername) != EOF) {
        if (storedCardNumber == cardNumber) {
            *pin = storedPIN;
            strcpy(username, storedUsername);
            fclose(file);
            return 1; // Card found
        }
    }

    fclose(file);
    return 0; // Card not found
}