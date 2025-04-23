#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================
// Global Variables
// ============================
int isOutOfService = 0; // 0 = Operational, 1 = Out of Service

// ============================
// Function Prototypes
// ============================
void createAccount();
int generateUniqueCardNumber();
int generateRandomPin();
int isCardNumberUnique(int cardNumber);
void logAdminActivity(const char *activity);
void logError(const char *errorMessage); // New function to log errors
void initializeFiles();
void toggleServiceMode();
void clearScreen();
void resetPIN(int cardNumber);
int loadAdminCredentials(char *adminId, char *adminPass);
int validateCredentials(int cardNumber, int enteredPIN);
void unblockCard();
int isCardNumberValid(int cardNumber); // Function prototype for card number validation
int validateCardNumber(int cardNumber); // Function prototype for card number validation
int updateCardDetails(int cardNumber, int newPIN, const char *newStatus); // Function prototype for updating card details

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

        // Validate admin credentials
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
                printf("3. Regenerate Card Pin\n");
                printf("4. Toggle Card Status\n");
                printf("5. Exit\n");
                printf("Enter your choice: ");
                scanf("%d", &choice);

                switch (choice) {
                    case 1: {
                        clearScreen();
                        createAccount();
                        printf("\nAccount created successfully.\n");
                        printf("\nPress any key to continue to the next operation...\n");
                        getchar();
                        getchar();
                        logAdminActivity("Created a new account");
                        break;
                    }
                    case 2: {
                        clearScreen();
                        toggleServiceMode();
                        printf("\nService mode toggled successfully.\n");
                        printf("\nPress any key to continue to the next operation...\n");
                        getchar();
                        getchar();
                        logAdminActivity("Service mode toggled");
                        break;
                    }
                    case 3: {
                        int cardNumber;
                        printf("Enter Card Number: ");
                        scanf("%d", &cardNumber);

                        // Validate if the card number exists
                        if (!validateCardNumber(cardNumber)) {
                            printf("\nError: Card number does not exist. Operation aborted.\n");
                            printf("\nPress any key to Returning to main menu...\n");
                            getchar();
                            getchar();
                            break;
                        }

                        int newPIN;
                        printf("Enter the new PIN for card number %d: ", cardNumber);
                        scanf("%d", &newPIN);

                        if (!updateCardDetails(cardNumber, newPIN, "Active")) {
                            printf("\nError: Unable to reset PIN. Operation aborted.\n");
                            printf("\nPress any key to Returning to main menu...\n");
                            getchar();
                            getchar();
                            break;
                        }

                        printf("\nPIN reset successfully for card number %d.\n", cardNumber);
                        printf("\nPress any key to continue to the next operation...\n");
                        getchar();
                        getchar();
                        logAdminActivity("PIN reset successfully");
                        break;
                    }

                    case 4: {
                        int cardNumber;
                        printf("Enter the card number to unblock or block: ");
                        scanf("%d", &cardNumber);

                        // Validate if the card number exists
                        if (!validateCardNumber(cardNumber)) {
                            printf("\nError: Card number does not exist. Operation aborted.\n");
                            printf("\nPress any key to Returning to main menu...\n");
                            getchar();
                            getchar();
                            break;
                        }

                        int toggleOption;
                        printf("\nEnter 1 to unblock or 2 to block the card: ");
                        scanf("%d", &toggleOption);

                        if (toggleOption == 1) {
                            int confirm;
                            printf("\nDo you want to proceed with unblocking the card? (1 for Yes, 0 for No): ");
                            scanf("%d", &confirm);

                            if (confirm == 0) {
                                printf("\nOperation aborted. \n");
                                printf("\nPress any key to Returning to main menu...\n");
                                getchar();
                                getchar();
                                break;
                            }

                            int newPIN = rand() % 9000 + 1000; // Generate a new 4-digit PIN
                            if (!updateCardDetails(cardNumber, newPIN, "Active")) {
                                printf("\nError: Unable to unblock card. Operation aborted.\n");
                                printf("\nPress any key to Returning to main menu...\n");
                                getchar();
                                getchar();
                                break;
                            }

                            printf("\nGenerated New PIN: %d\n", newPIN);
                            printf("\nCard unblocked successfully.\n");
                        } else if (toggleOption == 2) {
                            if (!updateCardDetails(cardNumber, -1, "Blocked")) { // -1 indicates no PIN change
                                printf("\nError: Unable to block card. Operation aborted.\n");
                                printf("\nPress any key to Returning to main menu...\n");
                                getchar();
                                getchar();
                                break;
                            }

                            printf("\nCard blocked successfully.\n");
                        } else {
                            printf("\nInvalid option. Operation aborted.\n");
                            printf("\nPress any key to Returning to main menu...\n");
                            getchar();
                            getchar();
                            break;
                        }

                        printf("\nPress any key to continue to the next operation...\n");
                        getchar();
                        getchar();
                        logAdminActivity("Card status toggled");
                        break;
                    }
                    case 5:
                        clearScreen();
                        logAdminActivity("Exited Admin Panel");
                        break;
                    default:
                        printf("Invalid choice. Please try again.\n");
                        logAdminActivity("Invalid menu choice entered");
                }
            } while (choice != 5);
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
        printf("Error: Unable to create account. Error Code: 101\n");
        logError("Failed to open credentials.txt while creating account.");
        return;
    }
    fprintf(credFile, "%-20s | %-11d | %-4d | %-9s\n", accountHolderName, cardNumber, pin, "Active");
    fclose(credFile);

    // Store balance in accounting.txt
    FILE *accountingFile = fopen("../../data/accounting.txt", "a");
    if (accountingFile == NULL) {
        printf("Error: Unable to create account. Error Code: 102\n");
        logError("Failed to open accounting.txt while creating account.");
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
    int pin;
    char status[10];

    // Read each line from the file and check if the card number exists
    while (fscanf(credFile, "%49[^|] | %d | %d | %9s", name, &existingCardNumber, &pin, status) == 4) {
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
        printf("Error: Unable to toggle service mode. Error Code: 401\n");
        logError("Failed to open status.txt while toggling service mode.");
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

// Function to reset PIN
void resetPIN(int cardNumber) {
    FILE *file = fopen("../../data/credentials.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to reset PIN. Error Code: 402\n");
        logError("Failed to open credentials.txt while resetting PIN.");
        return;
    }

    FILE *tempFile = fopen("../../data/temp_credentials.txt", "w");
    if (tempFile == NULL) {
        printf("Error: Unable to reset PIN. Error Code: 403\n");
        logError("Failed to create temporary file while resetting PIN.");
        fclose(file);
        return;
    }

    int storedCardNumber, storedPIN;
    char storedUsername[50], storedStatus[10];
    int newPIN;

    printf("Enter the new PIN for card number %d: ", cardNumber);
    scanf("%d", &newPIN);

    while (fscanf(file, "%49[^|] | %d | %d | %9s", storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", storedUsername, storedCardNumber, newPIN, storedStatus);
        } else {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", storedUsername, storedCardNumber, storedPIN, storedStatus);
        }
    }

    fclose(file);
    fclose(tempFile);

    // Replace the original file with the updated file
    if (remove("../../data/credentials.txt") != 0 || rename("../../data/temp_credentials.txt", "../../data/credentials.txt") != 0) {
        printf("Error: Unable to reset PIN. Error Code: 404\n");
        logError("Failed to replace credentials.txt with updated file while resetting PIN.");
        return;
    }

    printf("PIN reset successfully for card number %d.\n", cardNumber);
    logAdminActivity("PIN reset successfully");
}

// Function to unblock a card
void unblockCard() {
    int cardNumber;
    printf("Enter the card number to unblock: ");
    scanf("%d", &cardNumber);

    FILE *file = fopen("../../data/credentials.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to unblock card. Error Code: 201\n");
        logError("Failed to open credentials.txt while unblocking card.");
        return;
    }

    FILE *tempFile = fopen("../../data/temp_credentials.txt", "w");
    if (tempFile == NULL) {
        printf("Error: Unable to unblock card. Error Code: 202\n");
        logError("Failed to create temporary file while unblocking card.");
        fclose(file);
        return;
    }

    char line[256];
    int storedCardNumber, storedPIN;
    char storedUsername[50], storedStatus[10];
    int newPIN = rand() % 9000 + 1000; // Generate a new 4-digit PIN

    // Write the header lines
    fgets(line, sizeof(line), file);
    fprintf(tempFile, "%s", line);
    fgets(line, sizeof(line), file);
    fprintf(tempFile, "%s", line);

    // Update the status and PIN for the matching card number
    while (fscanf(file, "%49[^|] | %d | %d | %9s", storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", storedUsername, storedCardNumber, newPIN, "Active");
            printf("Card unblocked successfully. New PIN: %d\n", newPIN);
        } else {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", storedUsername, storedCardNumber, storedPIN, storedStatus);
        }
    }

    fclose(file);
    fclose(tempFile);

    // Replace the original file with the updated file
    if (remove("../../data/credentials.txt") != 0 || rename("../../data/temp_credentials.txt", "../../data/credentials.txt") != 0) {
        printf("Error: Unable to unblock card. Error Code: 203\n");
        logError("Failed to replace credentials.txt with updated file while unblocking card.");
        return;
    }

    logAdminActivity("Card unblocked successfully");
}

// Function to update card details
int updateCardDetails(int cardNumber, int newPIN, const char *newStatus) {
    FILE *file = fopen("../../data/credentials.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open credentials file.\n");
        logError("Failed to open credentials.txt while updating card details.");
        return 0;
    }

    FILE *tempFile = fopen("../../data/temp_credentials.txt", "w");
    if (tempFile == NULL) {
        printf("Error: Unable to create temporary file.\n");
        logError("Failed to create temporary file while updating card details.");
        fclose(file);
        return 0;
    }

    char storedUsername[50], storedStatus[10];
    int storedCardNumber, storedPIN;

    while (fscanf(file, "%49[^|] | %d | %d | %9s", storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", storedUsername, storedCardNumber, newPIN == -1 ? storedPIN : newPIN, newStatus);
        } else {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", storedUsername, storedCardNumber, storedPIN, storedStatus);
        }
    }

    fclose(file);
    fclose(tempFile);

    // Replace the original file with the updated file
    if (remove("../../data/credentials.txt") != 0 || rename("../../data/temp_credentials.txt", "../../data/credentials.txt") != 0) {
        printf("Error: Unable to update card details.\n");
        logError("Failed to replace credentials.txt with updated file while updating card details.");
        return 0;
    }

    return 1;
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

// Function to load admin credentials from a configuration file
int loadAdminCredentials(char *adminId, char *adminPass) {
    FILE *file = fopen("../../data/admin_credentials.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to load admin credentials. Error Code: 301\n");
        logError("Failed to open admin_credentials.txt while loading admin credentials.");
        return 0;
    }

    if (fscanf(file, "%s %s", adminId, adminPass) != 2) {
        fclose(file);
        printf("Error: Invalid admin credentials format. Error Code: 302\n");
        logError("Invalid format in admin_credentials.txt.");
        return 0;
    }

    fclose(file);
    return 1; // Validation successful
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

// Function to log errors to error.log
void logError(const char *errorMessage) {
    FILE *file = fopen("../../logs/error.log", "a");
    if (file == NULL) {
        printf("Error: Unable to open error log file.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

    fprintf(file, "[%s] ERROR: %s\n", timestamp, errorMessage);
    fclose(file);
}

void initializeFiles() {
    // Initialize credentials.txt with table headings
    FILE *credFile = fopen("../../data/credentials.txt", "a");
    if (credFile != NULL) {
        fseek(credFile, 0, SEEK_END);
        if (ftell(credFile) == 0) { // File is empty
            fprintf(credFile, "Account Holder Name | Card Number | PIN  | Status\n");
            fprintf(credFile, "--------------------|-------------|------|--------\n");
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

    int storedCardNumber, storedPIN;
    char storedUsername[50], storedStatus[10];

    while (fscanf(file, "%49[^|] | %d | %d | %9s", storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber && storedPIN == enteredPIN) {
            fclose(file);
            return 1; // Validation successful
        }
    }

    fclose(file);
    return 0; // Validation failed
}

// Function to validate if a card number exists
int validateCardNumber(int cardNumber) {
    FILE *file = fopen("../../data/credentials.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open credentials file.\n");
        return 0; // Validation failed
    }

    int storedCardNumber;
    char storedUsername[50], storedStatus[10];
    int storedPIN;

    while (fscanf(file, "%49[^|] | %d | %d | %9s", storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            fclose(file);
            return 1; // Card number exists
        }
    }

    fclose(file);
    return 0; // Card number does not exist
}