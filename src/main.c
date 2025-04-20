#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "pin_validation.h"
#include "card_num_validation.h" // Ensure this header file declares loadCredentials

// Function prototypes
void displayMenu();
void handleUserChoice(int choice, float *balance, int *pin, int cardNumber);
void savePIN(int cardNumber, int pin);
int verifyPINWithAttempts(int pin, int maxAttempts);
float fetchBalanceFromFile(int cardNumber);

int main() {
    int choice;
    float balance;
    int cardNumber;
    int pin;
    char username[50];
    int enteredPin;
    int attempts = 3;

    while (1) {
        printf("Enter your card number: ");
        scanf("%d", &cardNumber);

        if (!isCardNumberValid(cardNumber)) {
            printf("Invalid card number. Please try again.\n");
            continue;
        }

        if (!loadCredentials(cardNumber, &pin, username)) {
            printf("Card not found. Please try again.\n");
            continue;
        }

        balance = fetchBalanceFromFile(cardNumber);
        if (balance < 0) {
            printf("Error: Unable to fetch balance. Please try again.\n");
            continue;
        }

        printf("Hello, %s! Please enter your PIN to proceed.\n", username);

        while (attempts > 0) {
            printf("Enter your PIN: ");
            scanf("%d", &enteredPin);

            if (validatePIN(enteredPin, pin)) {
                printf("Welcome, %s!\n", username);
                break;
            } else {
                attempts--;
                printf("Incorrect PIN. You have %d attempt(s) remaining.\n", attempts);
            }

            if (attempts == 0) {
                printf("Access denied. Returning to card entry...\n");
                break;
            }
        }

        if (attempts > 0) {
            while (1) {
                displayMenu();
                printf("Enter your choice: ");
                scanf("%d", &choice);

                if (choice == 5) {
                    exitATM(cardNumber);
                    break;
                }

                if (verifyPINWithAttempts(pin, 3)) {
                    handleUserChoice(choice, &balance, &pin, cardNumber);
                } else {
                    printf("Access denied. Returning to card entry...\n");
                    break;
                }
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
    char username[50]; // Add this to fetch the username
    if (!fetchUsername(cardNumber, username)) {
        printf("Error: Unable to fetch username.\n");
        return;
    }

    switch (choice) {
        case 1:
            checkBalance(*balance);
            break;
        case 2:
            *balance = depositMoney(cardNumber, username); // Pass cardNumber and username
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

// Function to verify PIN with multiple attempts
int verifyPINWithAttempts(int pin, int maxAttempts) {
    int enteredPin;
    while (maxAttempts > 0) {
        printf("Re-enter your PIN to proceed: ");
        scanf("%d", &enteredPin);

        if (validatePIN(enteredPin, pin)) {
            return 1; // PIN verified successfully
        } else {
            maxAttempts--;
            printf("Incorrect PIN. You have %d attempt(s) remaining.\n", maxAttempts);
        }
    }
    return 0; // PIN verification failed
}

// Function to fetch the balance from accounting.txt
float fetchBalanceFromFile(int cardNumber) {
    FILE *file = fopen("../data/accounting.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open accounting file.\n");
        return -1.0; // Indicate an error
    }

    int storedCardNumber;
    float storedBalance;
    while (fscanf(file, "%d %f", &storedCardNumber, &storedBalance) != EOF) {
        if (storedCardNumber == cardNumber) {
            fclose(file);
            return storedBalance; // Return the balance for the card
        }
    }

    fclose(file);
    return -1.0; // Card not found
}