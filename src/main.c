#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "pin_validation.h"
#include "card_num_validation.h"

// Function prototypes
void displayMenu();
void handleUserChoice(int choice, float *balance, int *pin, int cardNumber);

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

        balance = fetchBalance(cardNumber);
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
            int exitMenu = 0; // Flag to exit the menu loop
            while (!exitMenu) {
                displayMenu();
                printf("Enter your choice: ");
                scanf("%d", &choice);

                if (choice == 5) {
                    exitATM(cardNumber);
                    exitMenu = 1; // Exit the menu loop
                } else {
                    handleUserChoice(choice, &balance, &pin, cardNumber);
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
    char username[50];
    if (!fetchUsername(cardNumber, username)) {
        printf("Error: Unable to fetch username.\n");
        return;
    }

    switch (choice) {
        case 1:
            checkBalance(*balance);
            break;
        case 2:
            *balance = depositMoney(cardNumber, username);
            break;
        case 3:
            withdrawMoney(balance, cardNumber); // Pass the pointer to balance
            break;
        case 4:
            changePIN(pin);
            savePIN(cardNumber, *pin);
            break;
        default:
            printf("Invalid choice! Please try again.\n");
    }
}

