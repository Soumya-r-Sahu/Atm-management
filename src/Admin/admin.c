#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ADMIN_ID "admin123"
#define ADMIN_PASS "pass456"

void createAccount();
int generateUniqueCardNumber();
int generateRandomPin();
int isCardNumberUnique(int cardNumber);
void logAdminActivity(const char *activity); // Function prototype
void initializeFiles(); // Function to initialize files with table headings

int main() {
    // Ensure the files are initialized
    initializeFiles();

    char adminId[50], adminPass[50];

    printf("Enter Admin ID: ");
    scanf("%s", adminId);
    printf("Enter Admin Password: ");
    scanf("%s", adminPass);

    if (strcmp(adminId, ADMIN_ID) == 0 && strcmp(adminPass, ADMIN_PASS) == 0) {
        printf("\nLogin Successful!\n");
        printf("Welcome to the Admin Panel\n");
        logAdminActivity("Admin logged in");

        int choice;
        do {
            printf("\nMenu:\n");
            printf("1. Create Account\n");
            printf("2. Exit\n");
            printf("Enter your choice: ");
            scanf("%d", &choice);

            switch (choice) {
                case 1:
                    createAccount();
                    logAdminActivity("Created a new account");
                    break;
                case 2:
                    printf("Exiting...\n");
                    logAdminActivity("Admin logged out");
                    break;
                default:
                    printf("Invalid choice. Please try again.\n");
                    logAdminActivity("Invalid menu choice entered");
            }
        } while (choice != 2);
    } else {
        printf("\nInvalid Admin ID or Password. Access Denied.\n");
        logAdminActivity("Failed admin login attempt");
    }

    return 0;
}

void initializeFiles() {
    // Initialize credentials.txt with table headings
    FILE *credFile = fopen("../../data/credentials.txt", "a");
    if (credFile != NULL) {
        fseek(credFile, 0, SEEK_END);
        if (ftell(credFile) == 0) { // File is empty
            fprintf(credFile, "Account Holder Name | Card Number | PIN\n");
            fprintf(credFile, "--------------------|-------------|-----\n");
        }
        fclose(credFile);
    }

    // Initialize accounting.txt with table headings
    FILE *accountingFile = fopen("../../data/accounting.txt", "a");
    if (accountingFile != NULL) {
        fseek(accountingFile, 0, SEEK_END);
        if (ftell(accountingFile) == 0) { // File is empty
            fprintf(accountingFile, "Card Number | Balance\n");
            fprintf(accountingFile, "------------|--------\n");
        }
        fclose(accountingFile);
    }

    // Initialize admin.log if it doesn't exist
    FILE *logFile = fopen("../../data/admin.log", "a");
    if (logFile != NULL) {
        fclose(logFile);
    }
}

void createAccount() {
    char accountHolderName[100];
    float initialDeposit;
    int cardNumber, pin;

    printf("\nEnter Account Holder Name: ");
    scanf(" %[^\n]", accountHolderName);
    printf("Enter Initial Deposit: ");
    scanf("%f", &initialDeposit);

    cardNumber = generateUniqueCardNumber();
    pin = generateRandomPin();

    printf("\nAccount Created Successfully!\n");
    printf("Account Holder Name: %s\n", accountHolderName);
    printf("Card Number: %d\n", cardNumber);
    printf("PIN: %d\n", pin);
    printf("Balance: %.2f\n", initialDeposit);

    // Store credentials in credentials.txt
    FILE *credFile = fopen("../../data/credentials.txt", "a");
    if (credFile == NULL) {
        printf("Error opening credentials.txt file.\n");
        logAdminActivity("Error opening credentials.txt while creating account");
        return;
    }
    fprintf(credFile, "%-20s | %-11d | %-4d\n", accountHolderName, cardNumber, pin);
    fclose(credFile);

    // Store balance in accounting.txt
    FILE *accountingFile = fopen("../../data/accounting.txt", "a");
    if (accountingFile == NULL) {
        printf("Error opening accounting.txt file.\n");
        logAdminActivity("Error opening accounting.txt while creating account");
        return;
    }
    fprintf(accountingFile, "%-12d | %-8.2f\n", cardNumber, initialDeposit);
    fclose(accountingFile);

    logAdminActivity("Account created successfully");
}

int generateUniqueCardNumber() {
    int cardNumber;
    do {
        cardNumber = rand() % 900000 + 100000; // Generate a 6-digit number
    } while (!isCardNumberUnique(cardNumber));
    return cardNumber;
}

int generateRandomPin() {
    return rand() % 9000 + 1000; // Generate a 4-digit number
}

int isCardNumberUnique(int cardNumber) {
    FILE *credFile = fopen("../../data/credentials.txt", "r");
    if (credFile == NULL) {
        return 1; // If file doesn't exist, card number is unique
    }

    char name[100];
    int existingCardNumber, pin;
    while (fscanf(credFile, "%s %d %d", name, &existingCardNumber, &pin) != EOF) {
        if (existingCardNumber == cardNumber) {
            fclose(credFile);
            return 0; // Card number is not unique
        }
    }

    fclose(credFile);
    return 1; // Card number is unique
}

// Function to log admin activity
void logAdminActivity(const char *activity) {
    FILE *logFile = fopen("../../data/admin.log", "a");
    if (logFile == NULL) {
        printf("Error: Unable to open admin.log file.\n");
        return;
    }

    // Get the current timestamp
    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);

    // Write the activity with a timestamp to the log file
    fprintf(logFile, "[%02d-%02d-%04d %02d:%02d:%02d] %s\n",
            localTime->tm_mday, localTime->tm_mon + 1, localTime->tm_year + 1900,
            localTime->tm_hour, localTime->tm_min, localTime->tm_sec,
            activity);

    fclose(logFile);
}