#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> // For sleep function on Unix-like systems
#include "../database/database.h"
#include "../validation/pin_validation.h"
#include "../validation/card_num_validation.h"
#include "../utils/logger.h"
#include "../utils/language_support.h"
#include "../transaction/transaction_manager.h"
#include "menu.h" // Added menu.h header

// ============================
// Function Prototypes
// ============================
void handleUserChoice(int choice, int cardNumber, const char* username);
void clearScreen(); // Function to clear the terminal screen
void logActivity(const char *activity); // Log user activities
void checkATMAvailability(); // Function to check ATM availability
void toggleToOutOfServiceInRealTime(); // Function to toggle ATM to Out of Service mode
void logAdminActivity(const char *activity); // Function prototype for logging admin activity
void displayLanguageSelection(); // Function to display language selection menu
void printLocalizedMessage(const char* key); // Function to print localized text

// Additional function prototypes for functions called in menu.c but not defined there
void handleBalanceEnquiry(int cardNumber);
void handleCashWithdrawal(int cardNumber);
void handleCashDeposit(int cardNumber);
void handleChangePin(int cardNumber);
void handleMiniStatement(int cardNumber);
void handleLanguageChange();
void handleTransactionHistory(int cardNumber);
void handleAccountDetails(int cardNumber);
void updateCardStatus(int cardNumber, const char* status);
void logTransaction(int cardNumber, int transactionType, float amount, int success);
TransactionResult performWithdrawal(int cardNumber, float amount, const char* username);
TransactionResult performFundTransfer(int cardNumber, int targetCardNumber, float amount, const char* username);
const char* getCustomerPhoneNumber(int cardNumber);
void generateReceipt(int cardNumber, int transactionType, float amount, float newBalance, const char* phoneNumber);

// ============================
// Function to Check ATM Availability
// ============================
void checkATMAvailability() {
    int status = fetchServiceStatus();
    if (status == 1) {
        printf("\n********************************************\n");
        printf("*                                          *\n");
        printf("*              OUT OF SERVICE              *\n");
        printf("*                                          *\n");
        printf("*         We apologize for the             *\n");
        printf("*         inconvenience caused.            *\n");
        printf("*                                          *\n");
        printf("*     Please try again later or visit      *\n");
        printf("*     nearest branch for assistance.       *\n");
        printf("*                                          *\n");
        printf("********************************************\n\n");
        sleep(5); // Display the message for 5 seconds
        exit(0);  // Exit the program
    }
}

// ============================
// Function to Display Language Selection
// ============================
void displayLanguageSelection() {
    clearScreen();
    printf("\n===== Language Selection / भाषा चयन / ଭାଷା ଚୟନ =====\n\n");
    printf("1. English\n");
    printf("2. Hindi (हिंदी)\n");
    printf("3. Odia (ଓଡ଼ିଆ)\n");
    printf("\nPlease select your preferred language (1-3): ");
    
    int choice;
    char inputBuffer[10];
    
    if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
        if (sscanf(inputBuffer, "%d", &choice) == 1) {
            switch(choice) {
                case 1: setLanguage(LANG_ENGLISH); break;
                case 2: setLanguage(LANG_HINDI); break;
                case 3: setLanguage(LANG_ODIA); break;
                default: 
                    printf("Invalid choice. Defaulting to English.\n");
                    setLanguage(LANG_ENGLISH);
            }
        } else {
            printf("Invalid input. Defaulting to English.\n");
            setLanguage(LANG_ENGLISH);
        }
    }
    
    // Print welcome message in selected language
    clearScreen();
    printf("\n");
    printLocalizedMessage("WELCOME");
    printf("\n\n");
    sleep(2); // Show welcome message for 2 seconds
}

// Print localized message
void printLocalizedMessage(const char* key) {
    printf("%s", getLocalizedText(key));
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
    char accountStatus[10]; // To store the account status
    char inputBuffer[100]; // Buffer for user input

    // Initialize language support
    if (!initLanguageSupport()) {
        printf("Warning: Language support initialization failed. Defaulting to English.\n");
        // Continue with default language
    }
    
    // Display language selection menu
    displayLanguageSelection();

    checkATMAvailability(); // Check ATM availability before proceeding

    while (1) {
        clearScreen(); // Clear the terminal before entering card number
        printLocalizedMessage("ENTER_CARD");
        printf(": ");
        
        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL || sscanf(inputBuffer, "%d", &cardNumber) != 1) {
            printLocalizedMessage("INVALID_CARD");
            printf("\n");
            continue;
        }

        // Clear the input buffer to avoid issues with subsequent inputs
        fflush(stdin);

        if (!isCardNumberValid(cardNumber)) {
            printLocalizedMessage("INVALID_CARD");
            printf("\n");
            logActivity("Invalid card number entered.");
            continue;
        }

        if (!loadCredentials(cardNumber, &pin, username, accountStatus)) {
            printLocalizedMessage("INVALID_CARD");
            printf("\n");
            logActivity("Card not found.");
            continue;
        }

        // Fetch account status separately
        FILE *file = fopen("../data/credentials.txt", "r");
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

        // Check if the card is locked in the database
        if (strcmp(accountStatus, "Locked") == 0) {
            printf("Your card has been blocked. Please contact the bank administrator.\n");
            logActivity("Blocked card access attempt.");
            sleep(3); // Pause to show the message
            continue;
        }

        // Check if the card is temporarily locked due to failed PIN attempts
        int remainingLockoutTime;
        if (isCardLockedOut(cardNumber, &remainingLockoutTime)) {
            printf("Your card is temporarily locked due to too many failed attempts.\n");
            printf("Please try again after %d seconds.\n", remainingLockoutTime);
            logActivity("Temporarily locked card access attempt.");
            sleep(3); // Pause to show the message
            continue;
        }

        balance = fetchBalance(cardNumber);
        if (balance < 0) {
            printf("Error: Unable to fetch balance. Error Code: 602\n");
            logError("Failed to fetch balance for the card.");
            logActivity("Failed to fetch balance.");
            continue;
        }

        printf("Hello, %s! ", username);
        printLocalizedMessage("ENTER_PIN");
        printf("\n");

        // Get remaining PIN attempts
        int remainingAttempts = getRemainingPINAttempts(cardNumber);

        // PIN validation loop
        bool authenticated = false;
        while (remainingAttempts > 0) {
            printf("Enter your PIN (%d attempts remaining): ", remainingAttempts);
            if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL || sscanf(inputBuffer, "%d", &enteredPin) != 1) {
                printf("Invalid input. Please enter a valid PIN.\n");
                continue;
            }

            if (validatePIN(enteredPin, pin)) {
                printf("Welcome, %s!\n", username);
                logActivity("Successful login.");
                authenticated = true;
                
                // Reset PIN attempts after successful login
                resetPINAttempts(cardNumber);
                break;
            } else {
                remainingAttempts--;
                printLocalizedMessage("INVALID_PIN");
                printf(" ");
                printf("You have %d attempt(s) remaining.\n", remainingAttempts);
                logActivity("Failed PIN attempt.");
                
                // Record the failed attempt
                recordFailedPINAttempt(cardNumber);
                
                // Check if this attempt has triggered a lockout
                if (isCardLockedOut(cardNumber, &remainingLockoutTime)) {
                    printf("You have entered the wrong PIN too many times.\n");
                    printf("Your card is temporarily locked for %d seconds.\n", remainingLockoutTime);
                    logActivity("Card temporarily locked due to failed PIN attempts.");
                    break;
                }
            }
        }

        if (authenticated) {
            int exitMenu = 0; // Flag to exit the menu loop
            while (!exitMenu) {
                clearScreen(); // Clear the terminal before displaying the menu
                displayMenu();
                printf("Enter your choice: ");
                if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL || sscanf(inputBuffer, "%d", &choice) != 1) {
                    printf("Invalid input. Please enter a valid choice.\n");
                    continue;
                }

                if (choice == 7) { // Exit option
                    printLocalizedMessage("THANK_YOU");
                    printf("\n");
                    logActivity("Exited ATM.");
                    exitMenu = 1; // Exit the menu loop
                } else {
                    handleUserChoice(choice, cardNumber, username);
                }
            }
        }
    }

    return 0;
}

// ============================
// Utility Functions
// ============================
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}