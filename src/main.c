#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> // For sleep function on Unix-like systems
#include "database.h" // Ensure this is included
#include "pin_validation.h"
#include "card_num_validation.h"
#include "utils/logger.h" // Include logger.h to use the centralized logError function

// ============================
// Function Prototypes
// ============================
void displayMenu();
void handleUserChoice(int choice, float *balance, int *pin, int cardNumber);
void clearScreen(); // Function to clear the terminal screen
void logActivity(const char *activity); // Log user activities
void checkATMAvailability(); // Function to check ATM availability
void toggleToOutOfServiceInRealTime(); // Function to toggle ATM to Out of Service mode
void logAdminActivity(const char *activity); // Function prototype for logging admin activity

// ============================
// Function to Check ATM Availability
// ============================
void checkATMAvailability() {
    int isOffline = 1; // Flag to track ATM status
    int messagePrinted = 0; // Flag to ensure the message is printed only once

    while (isOffline) {
        FILE *file = fopen("data/status.txt", "r");
        if (file == NULL) {
            printf("Error: Unable to fetch ATM status. Error Code: 701\n");
            logError("Failed to open status.txt while checking ATM availability.");
            sleep(10); // Wait for 10 seconds before retrying
            continue;
        }

        char status[20];
        if (fgets(status, sizeof(status), file) != NULL) {
            fclose(file);
            if (strstr(status, "Offline") != NULL) {
                if (!messagePrinted) {
                    printf("ATM is currently out of service. Please try again later.\n");
                    messagePrinted = 1; // Ensure the message is printed only once
                }
                sleep(10); // Wait for 10 seconds before checking again
                continue;
            } else {
                printf("We are online now!\n");
                for (int i = 5; i > 0; i--) {
                    printf("Starting in %d seconds...\n", i);
                    sleep(1);
                }
                printf("Press any key to continue...\n");
                getchar();
                break; // ATM is online, proceed with operations
            }
        }

        fclose(file);
    }
}

// ============================
// Function to Toggle ATM to Out of Service Mode
// ============================
void toggleToOutOfServiceInRealTime() {
    printf("\nATM is toggling to Out of Service mode. Please wait for ongoing operations to complete.\n");

    // Wait for ongoing operations to complete
    int operationInProgress = 1; // Simulate an operation in progress
    int timeout = 10; // Timeout in seconds

    while (operationInProgress && timeout > 0) {
        printf("\rWaiting for user to complete operation... %d seconds remaining.", timeout);
        fflush(stdout);
        sleep(1);
        timeout--;

        // Check if the operation is completed
        // This is a placeholder. Replace with actual logic to check operation status.
        operationInProgress = 0; // Set to 0 to simulate operation completion
    }

    if (timeout == 0) {
        printf("\nUser did not respond. Aborting all changes made during this session.\n");
        // Logic to abort changes made during the session
        // For example, rollback transactions, reset temporary files, etc.
    }

    // Toggle to Out of Service mode
    FILE *statusFile = fopen("data/status.txt", "w");
    if (statusFile == NULL) {
        printf("Error: Unable to toggle to Out of Service mode.\n");
        logError("Failed to open status.txt while toggling to Out of Service mode.");
        return;
    }

    fprintf(statusFile, "Status: Offline\n");
    fclose(statusFile);

    printf("\nATM is now in Out of Service mode.\n");
    logAdminActivity("ATM toggled to Out of Service mode.");
}

// ============================
// Main Function
// ============================
int main() {
    int choice = 0;
    float balance;
    int cardNumber;
    int pin;
    char username[50];
    int enteredPin;
    int attempts = 6; // Allow up to 6 attempts
    char accountStatus[10]; // To store the account status
    char inputBuffer[100]; // Buffer for user input
    int lockoutAttempts = 3; // Lockout after 3 failed attempts
    int lockoutDuration = 15; // Lockout duration in seconds
    time_t lockoutStartTime = 0;

    checkATMAvailability(); // Check ATM availability before proceeding

    while (1) {
        clearScreen(); // Clear the terminal before entering card number
        printf("Enter your card number: ");
        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL || sscanf(inputBuffer, "%d", &cardNumber) != 1) {
            printf("Invalid input. Please enter a valid card number.\n");
            continue;
        }

        // Clear the input buffer to avoid issues with subsequent inputs
        fflush(stdin);

        if (!isCardNumberValid(cardNumber)) {
            printf("Invalid card number. Please try again.\n");
            logActivity("Invalid card number entered.");
            continue;
        }

        if (!loadCredentials(cardNumber, &pin, username, accountStatus)) {
            printf("Card not found. Please try again.\n");
            logActivity("Card not found.");
            continue;
        }

        // Fetch account status separately
        FILE *file = fopen("data/credentials.txt", "r");
        if (file == NULL) {
            printf("Error: Unable to fetch account status. Error Code: 601\n");
            logError("Failed to open credentials.txt while fetching account status.");
            continue;
        }

        char line[256];
        int storedCardNumber;
        char storedStatus[10];

        // Skip header lines
        fgets(line, sizeof(line), file);
        fgets(line, sizeof(line), file);

        // Search for the card number
        while (fscanf(file, "%*[^|] | %d | %*d | %9s", &storedCardNumber, storedStatus) == 2) {
            if (storedCardNumber == cardNumber) {
                strcpy(accountStatus, storedStatus);
                break;
            }
        }

        fclose(file);

        if (strcmp(accountStatus, "Locked") == 0) {
            if (lockoutStartTime > 0 && difftime(time(NULL), lockoutStartTime) < lockoutDuration) {
                printf("Your card is temporarily locked. Please wait %d seconds and try again.\n", lockoutDuration - (int)difftime(time(NULL), lockoutStartTime));
                continue;
            } else {
                // Unlock the card after the lockout duration
                updateAccountStatus(cardNumber, "Active");
                strcpy(accountStatus, "Active");
                lockoutStartTime = 0;
                printf("Your card is now active. You can try again.\n");
            }
        }

        balance = fetchBalance(cardNumber);
        if (balance < 0) {
            printf("Error: Unable to fetch balance. Error Code: 602\n");
            logError("Failed to fetch balance for the card.");
            logActivity("Failed to fetch balance.");
            continue;
        }

        printf("Hello, %s! Please enter your PIN to proceed.\n", username);

        while (attempts > 0) {
            printf("Enter your PIN: ");
            if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL || sscanf(inputBuffer, "%d", &enteredPin) != 1) {
                printf("Invalid input. Please enter a valid PIN.\n");
                continue;
            }

            if (validatePIN(enteredPin, pin)) {
                printf("Welcome, %s!\n", username);
                logActivity("Successful login.");
                attempts = 6; // Reset attempts after successful login
                lockoutAttempts = 3; // Reset lockout attempts
                break;
            } else {
                attempts--;
                lockoutAttempts--;
                printf("Incorrect PIN. You have %d attempt(s) remaining.\n", attempts);
                logActivity("Failed PIN attempt.");

                if (lockoutAttempts == 0) {
                    printf("You have entered the wrong PIN 3 times. Your card is temporarily locked. Please wait %d seconds and try again later.\n", lockoutDuration);
                    updateAccountStatus(cardNumber, "Locked");
                    logActivity("Card temporarily locked due to 3 failed PIN attempts.");
                    lockoutStartTime = time(NULL);
                    break;
                }

                if (attempts == 3) {
                    printf("Warning: You have 3 more attempts before your card will be permanently blocked.\n");
                }

                if (attempts == 0) {
                    printf("Your card has been permanently locked due to too many failed attempts. Please contact the admin.\n");
                    updateAccountStatus(cardNumber, "Locked");
                    logActivity("Card permanently locked due to failed PIN attempts.");
                }
            }
        }

        if (attempts > 0 && lockoutAttempts > 0) {
            int exitMenu = 0; // Flag to exit the menu loop
            while (!exitMenu) {
                clearScreen(); // Clear the terminal before displaying the menu
                displayMenu();
                printf("Enter your choice: ");
                if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL || sscanf(inputBuffer, "%d", &choice) != 1) {
                    printf("Invalid input. Please enter a valid choice.\n");
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

// ============================
// Menu and User Choice Handling
// ============================
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
        printf("Error: Unable to fetch username. Error Code: 603\n");
        logError("Failed to fetch username for the card.");
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

// ============================
// Utility Functions
// ============================
void logActivity(const char *activity) {
    FILE *file = fopen("data/audit.log", "a");
    if (file == NULL) {
        printf("Error: Unable to open audit log file. Error Code: 604\n");
        logError("Failed to open audit.log while logging activity.");
        return;
    }

    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

    fprintf(file, "[%s] %s\n", timestamp, activity);
    fclose(file);
}

void clearScreen() {
#ifdef _WIN32
    system("cls"); // Windows-specific command to clear the screen
#else
    system("clear"); // Unix/Linux/Mac-specific command to clear the screen
#endif
}