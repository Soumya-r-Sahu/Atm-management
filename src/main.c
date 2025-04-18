#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "pin_validation.h"

// Function prototypes
void displayMenu();
void handleUserChoice(int choice, float *balance, int *pin, int cardNumber);
int loadCredentials(int cardNumber, int *pin, char *username);
void savePIN(int cardNumber, int pin);

int main() {
    int choice;
    float balance = 1000.0; // Initial balance
    int cardNumber;
    int pin;
    char username[50]; // To store the username
    int enteredPin;
    int attempts = 3; // Maximum attempts for PIN entry

    // Prompt for card number
    printf("Enter your card number: ");
    scanf("%d", &cardNumber);

    // Load the credentials (PIN and username) for the entered card number
    if (!loadCredentials(cardNumber, &pin, username)) {
        printf("Card not found. Exiting...\n");
        exit(0);
    }

    // Greet the user with their name
    printf("Hello, %s! Please enter your PIN to proceed.\n", username);

    // PIN verification
    while (attempts > 0) {
        printf("Enter your PIN: ");
        scanf("%d", &enteredPin);

        if (validatePIN(enteredPin, pin)) {
            printf("Welcome, %s!\n", username); // Greet the user after successful PIN entry
            break;
        } else {
            attempts--;
            printf("Incorrect PIN. You have %d attempt(s) remaining.\n", attempts);
        }

        if (attempts == 0) {
            printf("Access denied. Exiting...\n");
            exit(0);
        }
    }

    while (1) {
        displayMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);

        if (choice == 5) {
            // Directly exit without PIN validation
            exitATM();
        }

        // Require PIN validation before handling other menu options
        attempts = 3; // Reset attempts for each menu option
        while (attempts > 0) {
            printf("Re-enter your PIN to proceed: ");
            scanf("%d", &enteredPin);

            if (validatePIN(enteredPin, pin)) {
                handleUserChoice(choice, &balance, &pin, cardNumber);
                break;
            } else {
                attempts--;
                printf("Incorrect PIN. You have %d attempt(s) remaining.\n", attempts);
            }

            if (attempts == 0) {
                printf("Access denied. Exiting...\n");
                exit(0);
            }
        }
    }

    return 0;
}

void displayMenu() {
    printf("\n===== ATM Menu =====\n");
    printf("1. Check Balance\n");
    printf("2. Deposit Money\n");
    printf("3. Withdraw Money\n");
    printf("4. Change PIN\n");
    printf("5. Exit\n");
    printf("=====================\n");
}

void handleUserChoice(int choice, float *balance, int *pin, int cardNumber) {
    switch (choice) {
        case 1:
            checkBalance(*balance);
            break;
        case 2:
            *balance = depositMoney(*balance);
            break;
        case 3:
            *balance = withdrawMoney(*balance);
            break;
        case 4:
            changePIN(pin);
            savePIN(cardNumber, *pin); // Save the updated PIN for the card number
            break;
        case 5:
            exitATM(cardNumber);
            break;
        default:
            printf("Invalid choice! Please try again.\n");
    }
}

// Function to load the credentials (PIN and username) for a specific card number
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
            strcpy(username, storedUsername); // Copy the username
            fclose(file);
            return 1; // Card found
        }
    }

    fclose(file);
    return 0; // Card not found
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