#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================
// Global Variables
// ============================
int isOutOfService = 0; // 0 = Operational, 1 = Out of Service
#define ENCRYPTION_KEY "ADMIN_SECRET_KEY" // Encryption key for sensitive data

// Hashing key
#define HASH_KEY "ATM_HASH_KEY"

// ============================
// Function Prototypes
// ============================
void createAccount();
int generateUniqueCardNumber();
int generateRandomPin();
int isCardNumberUnique(int cardNumber);
void logAdminActivity(const char *activity);
void initializeFiles();
void toggleServiceMode();
void clearScreen();
void resetPIN(int cardNumber);
int loadAdminCredentials(char *adminId, char *adminPass);
void xorEncryptDecrypt(char *data, const char *key);
int validateCredentials(int cardNumber, int enteredPIN);
void hashString(const char *input, char *output);

// ============================
// Main Function
// ============================
int main() {
    initializeFiles();

    char adminId[50], adminPass[50];
    char storedAdminId[50], storedAdminPass[50];
    int attempts = 3; // Maximum login attempts
    int lockoutDuration = 30; // Lockout duration in seconds
    time_t lockoutStartTime = 0;

    // Load admin credentials from the configuration file
    if (!loadAdminCredentials(storedAdminId, storedAdminPass)) {
        printf("Error: Unable to load admin credentials.\n");
        return 1;
    }

    while (attempts > 0) {
        if (lockoutStartTime > 0 && difftime(time(NULL), lockoutStartTime) < lockoutDuration) {
            printf("Admin account is locked. Please wait %d seconds.\n", lockoutDuration - (int)difftime(time(NULL), lockoutStartTime));
            continue;
        }

        printf("Enter Admin ID: ");
        scanf("%s", adminId);
        printf("Enter Admin Password: ");
        scanf("%s", adminPass);

        // Decrypt and validate admin credentials
        if (strcmp(adminId, storedAdminId) == 0 && strcmp(adminPass, storedAdminPass) == 0) {
            printf("\nLogin Successful!\n");
            printf("Welcome to the Admin Panel\n");
            logAdminActivity("Admin logged in");

            int choice;
            do {
                clearScreen();
                printf("\nMenu:\n");
                printf("1. Create Account\n");
                printf("2. Toggle Service Mode\n");
                printf("3. Reset PIN\n");
                printf("4. Exit\n");
                printf("Enter your choice: ");
                scanf("%d", &choice);

                switch (choice) {
                    case 1:
                        clearScreen();
                        createAccount();
                        logAdminActivity("Created a new account");
                        break;
                    case 2:
                        clearScreen();
                        toggleServiceMode();
                        break;
                    case 3: {
                        clearScreen();
                        int cardNumber;
                        printf("Enter Card Number: ");
                        scanf("%d", &cardNumber);
                        resetPIN(cardNumber);
                        logAdminActivity("PIN reset attempted");
                        break;
                    }
                    case 4:
                        clearScreen();
                        printf("Exiting...\n");
                        logAdminActivity("Admin logged out");
                        break;
                    default:
                        printf("Invalid choice. Please try again.\n");
                        logAdminActivity("Invalid menu choice entered");
                }
            } while (choice != 4);
            return 0; // Exit after successful operations
        } else {
            attempts--;
            printf("Invalid Admin ID or Password. You have %d attempt(s) remaining.\n", attempts);
            logAdminActivity("Failed admin login attempt");

            if (attempts == 0) {
                printf("Admin account locked due to too many failed attempts. Please wait %d seconds.\n", lockoutDuration);
                lockoutStartTime = time(NULL);
                logAdminActivity("Admin account locked due to failed login attempts.");
            }
        }
    }

    return 0;
}

// ============================
// Core Admin Operations
// ============================

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
    int existingCardNumber;
    char encryptedPIN[10];

    // Read each line from the file and check if the card number exists
    while (fscanf(credFile, "%49[^|] | %d | %s\n", name, &existingCardNumber, encryptedPIN) == 3) {
        if (existingCardNumber == cardNumber) {
            fclose(credFile);
            return 0; // Card number is not unique
        }
    }

    fclose(credFile);
    return 1; // Card number is unique
}

// Function to toggle the ATM service mode
void toggleServiceMode() {
    FILE *statusFile = fopen("../../data/status.txt", "w");
    if (statusFile == NULL) {
        printf("Error: Unable to open status.txt file.\n");
        logAdminActivity("Error opening status.txt while toggling service mode");
        return;
    }

    if (isOutOfService) {
        isOutOfService = 0;
        fprintf(statusFile, "Status: Online\n"); // Update status to online
        printf("ATM is now operational.\n");
        logAdminActivity("ATM service mode set to operational");
    } else {
        isOutOfService = 1;
        fprintf(statusFile, "Status: Offline\n"); // Update status to offline
        printf("ATM is now out of service.\n");
        logAdminActivity("ATM service mode set to out of service");
    }

    fclose(statusFile);
}

// Function to reset PIN (with hashing)
void resetPIN(int cardNumber) {
    FILE *file = fopen("../../data/credentials.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open credentials file.\n");
        return;
    }

    FILE *tempFile = fopen("../../data/temp_credentials.txt", "w");
    if (tempFile == NULL) {
        printf("Error: Unable to create temporary file.\n");
        fclose(file);
        return;
    }

    int storedCardNumber;
    char storedUsername[50];
    char storedHash[50];
    int newPIN;
    char newHash[50];

    printf("Enter the new PIN for card number %d: ", cardNumber);
    scanf("%d", &newPIN);

    // Hash the new PIN
    char newPinStr[10];
    sprintf(newPinStr, "%d", newPIN);
    hashString(newPinStr, newHash);

    while (fscanf(file, "%49[^|] | %d | %s", storedUsername, &storedCardNumber, storedHash) == 3) {
        if (storedCardNumber == cardNumber) {
            fprintf(tempFile, "%-49s | %d | %s\n", storedUsername, storedCardNumber, newHash);
        } else {
            fprintf(tempFile, "%-49s | %d | %s\n", storedUsername, storedCardNumber, storedHash);
        }
    }

    fclose(file);
    fclose(tempFile);

    // Replace the original file with the updated file
    remove("../../data/credentials.txt");
    rename("../../data/temp_credentials.txt", "../../data/credentials.txt");

    printf("PIN reset successfully for card number %d.\n", cardNumber);
}

// ============================
// Utility Functions
// ============================

// Function to clear the terminal screen
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Function to load admin credentials from a configuration file (with hashing)
int loadAdminCredentials(char *adminId, char *adminPass) {
    FILE *file = fopen("../../data/admin_credentials.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open admin_credentials.txt file.\n");
        return 0;
    }

    char storedAdminId[50], storedAdminHash[50];
    char enteredHash[50];

    if (fscanf(file, "%s %s", storedAdminId, storedAdminHash) != 2) {
        fclose(file);
        printf("Error: Invalid format in admin_credentials.txt.\n");
        return 0;
    }

    // Hash the entered admin password
    hashString(adminPass, enteredHash);

    if (strcmp(adminId, storedAdminId) == 0 && strcmp(enteredHash, storedAdminHash) == 0) {
        fclose(file);
        return 1; // Validation successful
    }

    fclose(file);
    return 0; // Validation failed
}

// Function to encrypt or decrypt a string using XOR encryption
void xorEncryptDecrypt(char *data, const char *key) {
    size_t keyLen = strlen(key);
    for (size_t i = 0; i < strlen(data); i++) {
        data[i] ^= key[i % keyLen];
    }
}

// Function to log admin activity
void logAdminActivity(const char *activity) {
    FILE *file = fopen("../../logs/audit.log", "a");
    if (file == NULL) {
        printf("Error: Unable to open audit log file.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

    fprintf(file, "[%s] ADMIN: %s\n", timestamp, activity);
    fclose(file);
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
}

// Function to validate credentials (card number and PIN)
int validateCredentials(int cardNumber, int enteredPIN) {
    FILE *file = fopen("../../data/credentials.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open credentials file.\n");
        return 0; // Validation failed
    }

    int storedCardNumber;
    char storedUsername[50];
    char storedHash[50];
    char enteredHash[50];

    // Hash the entered PIN
    char enteredPinStr[10];
    sprintf(enteredPinStr, "%d", enteredPIN);
    hashString(enteredPinStr, enteredHash);

    while (fscanf(file, "%49[^|] | %d | %s", storedUsername, &storedCardNumber, storedHash) == 3) {
        if (storedCardNumber == cardNumber && strcmp(storedHash, enteredHash) == 0) {
            fclose(file);
            return 1; // Validation successful
        }
    }

    fclose(file);
    return 0; // Validation failed
}

// Function to hash a string using XOR
void hashString(const char *input, char *output) {
    size_t keyLen = strlen(HASH_KEY);
    size_t inputLen = strlen(input);

    for (size_t i = 0; i < inputLen; i++) {
        output[i] = input[i] ^ HASH_KEY[i % keyLen];
    }
    output[inputLen] = '\0'; // Null-terminate the hashed string
}