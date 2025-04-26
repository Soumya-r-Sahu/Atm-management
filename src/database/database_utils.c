#include "database.h"
#include <stdio.h>

// Utility function to fetch balance from file
float fetchBalanceFromFile(int cardNumber) {
    FILE *file = fopen("./../../data/accounting.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open accounting file.\n");
        return -1.0;
    }

    int storedCardNumber;
    float storedBalance;
    while (fscanf(file, "%d | %f", &storedCardNumber, &storedBalance) != EOF) {
        if (storedCardNumber == cardNumber) {
            fclose(file);
            return storedBalance;
        }
    }

    fclose(file);
    return -1.0; // Card not found
}