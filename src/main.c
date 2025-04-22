#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "database.h"
#include "pin_validation.h"
#include "card_num_validation.h"

// Function prototypes
void displayMenu();
void handleUserChoice(int choice, float *balance, int *pin, int cardNumber);
void clearScreen(); // Function to clear the terminal screen
void logActivity(const char *activity); // Log user activities

int main() {
    int choice = 0;
    float balance;
    int cardNumber;
    int pin;
    char username[50];
    int enteredPin;
    int attempts = 3;
    int lockoutDuration = 30; // Lockout duration in seconds
    time_t lockoutStartTime = 0;

    while (1) {
        clearScreen(); // Clear the terminal before entering card number
        printf("Enter your card number: ");
        if (scanf("%d", &cardNumber) != 1) {
            printf("Invalid input. Please enter a valid card number.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        if (!isCardNumberValid(cardNumber)) {
            printf("Invalid card number. Please try again.\n");
            logActivity("Invalid card number entered.");
            continue;
        }

        if (!loadCredentials(cardNumber, &pin, username)) {
            printf("Card not found. Please try again.\n");
            logActivity("Card not found.");
            continue;
        }

        balance = fetchBalance(cardNumber);
        if (balance < 0) {
            printf("Error: Unable to fetch balance. Please try again.\n");
            logActivity("Failed to fetch balance.");
            continue;
        }

        printf("Hello, %s! Please enter your PIN to proceed.\n", username);

        while (attempts > 0) {
            if (lockoutStartTime > 0 && difftime(time(NULL), lockoutStartTime) < lockoutDuration) {
                printf("Account is locked. Please wait %d seconds.\n", lockoutDuration - (int)difftime(time(NULL), lockoutStartTime));
                continue;
            }

            printf("Enter your PIN: ");
            if (scanf("%d", &enteredPin) != 1) {
                printf("Invalid input. Please enter a valid PIN.\n");
                while (getchar() != '\n'); // Clear input buffer
                continue;
            }

            if (validatePIN(enteredPin, pin)) {
                printf("Welcome, %s!\n", username);
                logActivity("Successful login.");
                attempts = 3; // Reset attempts after successful login
                lockoutStartTime = 0; // Reset lockout
                break;
            } else {
                attempts--;
                printf("Incorrect PIN. You have %d attempt(s) remaining.\n", attempts);
                logActivity("Failed PIN attempt.");

                if (attempts == 0) {
                    printf("Account locked due to too many failed attempts. Please wait %d seconds.\n", lockoutDuration);
                    lockoutStartTime = time(NULL);
                    logActivity("Account locked due to failed PIN attempts.");
                }
            }
        }

        if (attempts > 0) {
            int exitMenu = 0; // Flag to exit the menu loop
            while (!exitMenu) {
                clearScreen(); // Clear the terminal before displaying the menu
                displayMenu();
                printf("Enter your choice: ");
                if (scanf("%d", &choice) != 1) {
                    printf("Invalid input. Please enter a valid choice.\n");
                    while (getchar() != '\n'); // Clear input buffer
                    continue;
                }

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
    char accountHolderName[50]; // Add this to fetch the username
    if (!fetchUsername(cardNumber, accountHolderName)) {
        printf("Error: Unable to fetch username.\n");
        logActivity("Failed to fetch username.");
        return;
    }

    switch (choice) {
        case 1:
            checkBalance(*balance);
            logActivity("Checked balance.");
            break;
        case 2:
            *balance = depositMoney(cardNumber, accountHolderName); // Pass cardNumber and username
            logActivity("Deposited money.");
            break;
        case 3:
            withdrawMoney(balance, cardNumber);
            logActivity("Withdrew money.");
            break;
        case 4:
            changePIN(pin);
            savePIN(cardNumber, *pin); // Save the updated PIN for the card number
            logActivity("Changed PIN.");
            break;
        case 5:
            exitATM(cardNumber);
            logActivity("Exited ATM.");
            break;
        case 6:
            viewTransactionHistory(cardNumber);
            logActivity("Viewed transaction history.");
            break;
        default:
            printf("Invalid choice! Please try again.\n");
            logActivity("Invalid menu choice.");
    }
}

// Function to log user activities
void logActivity(const char *activity) {
    FILE *file = fopen("data/audit.log", "a");
    if (file == NULL) {
        printf("Error: Unable to open audit log file.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

    fprintf(file, "[%s] %s\n", timestamp, activity);
    fclose(file);
}

// Function to clear the terminal screen
void clearScreen() {
#ifdef _WIN32
    system("cls"); // Windows-specific command to clear the screen
#else
    system("clear"); // Unix/Linux/Mac-specific command to clear the screen
#endif
}