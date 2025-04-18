#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "database.h"

// Function to check the balance
void checkBalance(float balance) {
    printf("Your current balance is: $%.2f\n", balance);
}

// Function to deposit money
float depositMoney(float balance) {
    float amount;
    printf("Enter the amount to deposit: ");
    scanf("%f", &amount);

    if (amount > 0) {
        balance += amount;
        printf("Successfully deposited $%.2f. New balance: $%.2f\n", amount, balance);
    } else {
        printf("Invalid amount. Deposit failed.\n");
    }

    return balance;
}

// Function to withdraw money
float withdrawMoney(float balance) {
    float amount;
    printf("Enter the amount to withdraw: ");
    scanf("%f", &amount);

    if (amount > 0 && amount <= balance) {
        balance -= amount;
        printf("Successfully withdrew $%.2f. New balance: $%.2f\n", amount, balance);
    } else if (amount > balance) {
        printf("Insufficient balance. Withdrawal failed.\n");
    } else {
        printf("Invalid amount. Withdrawal failed.\n");
    }

    return balance;
}

// Function to change the PIN
void changePIN(int *pin) {
    int newPin;
    printf("Enter your new PIN: ");
    scanf("%d", &newPin);

    if (newPin > 999 && newPin <= 9999) { // Ensure PIN is a 4-digit number
        *pin = newPin;
        printf("PIN successfully changed.\n");
    } else {
        printf("Invalid PIN. PIN change failed.\n");
    }
}

// Function to exit the ATM
void exitATM(int cardNumber) {
    char username[50];
    if (fetchUsername(cardNumber, username)) {
        printf("Thank you %s for using the ATM Have a nice Day!\n", username);
    } else {
        printf("Thank you for using the ATM Have a nice Day!\n");
    }
    exit(0);
}

// Helper function to fetch the username for a specific card number
int fetchUsername(int cardNumber, char *username) {
    FILE *file = fopen("data/credentials.txt", "r");
    if (file == NULL) {
        perror("Error opening credentials.txt");
        return 0;
    }

    int storedCardNumber, storedPIN;
    char storedUsername[50];
    while (fscanf(file, "%d %d %s", &storedCardNumber, &storedPIN, storedUsername) != EOF) {
        if (storedCardNumber == cardNumber) {
            strcpy(username, storedUsername);
            fclose(file);
            return 1; // Username found
        }
    }

    fclose(file);
    return 0; // Username not found
}

// Function to save the PIN for a specific card number
void savePIN(int cardNumber, int pin) {
    FILE *file = fopen("data/credentials.txt", "r+");
    if (file == NULL) {
        // If the file doesn't exist, create it
        file = fopen("data/credentials.txt", "w");
        if (file == NULL) {
            printf("Error: Unable to save PIN.\n");
            return;
        }
    }

    int storedCardNumber, storedPIN;
    char storedUsername[50];
    long position;
    while ((position = ftell(file)) >= 0 && fscanf(file, "%d %d %s", &storedCardNumber, &storedPIN, storedUsername) != EOF) {
        if (storedCardNumber == cardNumber) {
            // Update the PIN for the existing card number
            fseek(file, position, SEEK_SET);
            fprintf(file, "%d %d %s\n", cardNumber, pin, storedUsername);
            fclose(file);
            return;
        }
    }

    // If the card number doesn't exist, append it to the file
    fprintf(file, "%d %d %s\n", cardNumber, pin, "User"); // Default username if not found
    fclose(file);
}

// Function to write a transaction log to the file
void writeTransactionLog(const char *username, const char *operation, const char *details) {
    FILE *file = fopen("data/transactions.log", "a");
    if (file == NULL) {
        perror("Error opening transactions.log");
        return;
    }

    // Get the current timestamp
    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

    // Write the log entry
    fprintf(file, "[%s] User: %s | Operation: %s | Details: %s\n", timestamp, username, operation, details);
    fclose(file);
}

// Function to read all transaction logs from the file
void readTransactionLogs() {
    char line[256];
    FILE *file = fopen("data/transactions.log", "r");
    if (file == NULL) {
        perror("Unable to open transactions.log");
        return;
    }
    printf("\nTransaction Logs:\n");
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}