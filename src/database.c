#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "database.h"

// Function to fetch the ATM service status from status.txt
int fetchServiceStatus() {
    FILE *file = fopen("data/status.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open status.txt file.\n");
        return 0; // Default to operational if the file cannot be read
    }

    char status[20];
    if (fgets(status, sizeof(status), file) != NULL) {
        fclose(file);
        if (strstr(status, "Offline") != NULL) {
            return 1; // ATM is out of service
        }
    }

    fclose(file);
    return 0; // ATM is operational
}

// ============================
// Core ATM Functionality
// ============================

// Function to check the balance
void checkBalance(float balance) {
    printf("Your current balance is: $%.2f\n", balance);
}

// Function to deposit money
float depositMoney(int cardNumber, const char *accountHolderName) {
    float amount;
    printf("Enter the amount to deposit: ");
    scanf("%f", &amount);

    if (amount > 0) {
        float balance = fetchBalance(cardNumber);
        if (balance == -1) {
            printf("Error: Unable to fetch balance.\n");
            return -1;
        }

        float oldBalance = balance;
        balance += amount;
        updateBalance(cardNumber, balance);
        printf("Successfully deposited $%.2f. Available balance: $%.2f\n", amount, balance);

        // Log the operation
        char details[200];
        snprintf(details, sizeof(details), "Deposited $%.2f | Old Balance: $%.2f | New Balance: $%.2f", amount, oldBalance, balance);
        writeTransactionLog(accountHolderName, "Deposit Money", details);

        return balance;
    } else {
        printf("Invalid amount. Deposit failed.\n");
        return -1;
    }
}

// Function to withdraw money
void withdrawMoney(float *balance, int cardNumber) {
    float dailyLimit = 1000.0; // Example daily limit
    float dailyWithdrawn = getDailyWithdrawals(cardNumber);

    if (dailyWithdrawn >= dailyLimit) {
        printf("Daily withdrawal limit reached. You cannot withdraw more today.\n");
        return;
    }

    float amount;
    printf("Enter the amount to withdraw: ");
    scanf("%f", &amount);

    if (amount > *balance) {
        printf("Insufficient balance.\n");
        return;
    }

    if (dailyWithdrawn + amount > dailyLimit) {
        printf("This withdrawal exceeds your daily limit. You can withdraw up to %.2f more today.\n", dailyLimit - dailyWithdrawn);
        return;
    }

    *balance -= amount;
    logWithdrawal(cardNumber, amount);
    printf("Withdrawal successful. Your new balance is %.2f.\n", *balance);
}

// Function to change the PIN
void changePIN(int *pin) {
    int newPin;
    printf("Enter your new PIN: ");
    scanf("%d", &newPin);

    if (newPin > 999 && newPin <= 9999) { // Ensure PIN is a 4-digit number
        *pin = newPin; // Update the PIN in memory
        printf("PIN successfully changed.\n");
    } else {
        printf("Invalid PIN. PIN change failed.\n");
    }
}

// Function to exit the ATM
void exitATM(int cardNumber) {
    char accountHolderName[50];
    if (fetchUsername(cardNumber, accountHolderName)) {
        printf("Thank you %s for using Our ATM service. Have a nice day!\n", accountHolderName);
    } else {
        printf("Thank you for using Our ATM service. Have a nice day!\n");
    }
    exit(0); // Properly exit the program
}

// ============================
// Logging and Transaction History
// ============================

// Function to write a transaction log to the file
void writeTransactionLog(const char *accountHolderName, const char *operation, const char *details) {
    FILE *file = fopen("data/transactions.log", "a");
    if (file == NULL) {
        printf("Error: Unable to open transactions log file.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

    fprintf(file, "[%s] Account Holder: %s | Operation: %s | Details: %s\n",
            timestamp, accountHolderName, operation, details);
    fclose(file);
}

// Function to view transaction history for a specific card number
void viewTransactionHistory(int cardNumber) {
    FILE *file = fopen("data/transactions.log", "r");
    if (file == NULL) {
        printf("Error: Unable to open transactions log file.\n");
        return;
    }

    char line[256];
    char cardStr[20];
    sprintf(cardStr, "%d", cardNumber);

    printf("\nTransaction History for Card Number %d:\n", cardNumber);
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, cardStr)) {
            printf("%s", line);
        }
    }
    fclose(file);
}

// ============================
// File Operations and Helpers
// ============================

// Function to fetch the balance for a specific card number
float fetchBalance(int cardNumber) {
    FILE *file = fopen("data/accounting.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open accounting file.\n");
        return -1.0; // Indicate an error
    }

    char line[256];
    int storedCardNumber;
    float storedBalance;
    while (fscanf(file, "%d | %f", &storedCardNumber, &storedBalance) != EOF) {
        if (storedCardNumber == cardNumber) {
            fclose(file);
            return storedBalance; // Return the balance for the card
        }
    }

    fclose(file);
    return -1.0; // Card not found
}

// Function to update the balance for a specific card number
void updateBalance(int cardNumber, float newBalance) {
    FILE *file = fopen("data/accounting.txt", "r+");
    if (file == NULL) {
        printf("Error: Unable to open accounting file.\n");
        return;
    }

    char line[256];
    FILE *tempFile = fopen("data/temp_accounting.txt", "w");
    if (tempFile == NULL) {
        printf("Error: Unable to create temporary file.\n");
        fclose(file);
        return;
    }

    int storedCardNumber;
    float storedBalance;
    int found = 0;
    while (fscanf(file, "%d | %f", &storedCardNumber, &storedBalance) != EOF) {
        if (storedCardNumber == cardNumber) {
            fprintf(tempFile, "%d | %.2f\n", cardNumber, newBalance);
            found = 1;
        } else {
            fprintf(tempFile, "%d | %.2f\n", storedCardNumber, storedBalance);
        }
    }

    if (!found) {
        fprintf(tempFile, "%d | %.2f\n", cardNumber, newBalance);
    }

    fclose(file);
    fclose(tempFile);

    // Replace the original file with the updated file
    remove("data/accounting.txt");
    rename("data/temp_accounting.txt", "data/accounting.txt");
}

// Function to log a withdrawal for a specific card number
void logWithdrawal(int cardNumber, float amount) {
    FILE *file = fopen("data/withdrawals.log", "a");
    if (file == NULL) {
        printf("Error: Unable to log withdrawal.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *today = localtime(&now);

    fprintf(file, "%d | %.2f | %d-%d-%d\n", cardNumber, amount, today->tm_mday, today->tm_mon + 1, today->tm_year + 1900);
    fclose(file);
}

// Function to get the total daily withdrawals for a specific card number
float getDailyWithdrawals(int cardNumber) {
    FILE *file = fopen("data/withdrawals.log", "r");
    if (file == NULL) {
        return 0.0; // No withdrawals yet
    }

    char line[256];
    float total = 0.0;
    time_t now = time(NULL);
    struct tm *today = localtime(&now);

    while (fgets(line, sizeof(line), file)) {
        int storedCardNumber;
        float amount;
        int day, month, year;

        sscanf(line, "%d | %f | %d-%d-%d", &storedCardNumber, &amount, &day, &month, &year);
        if (storedCardNumber == cardNumber && day == today->tm_mday && month == today->tm_mon + 1 && year == today->tm_year + 1900) {
            total += amount;
        }
    }

    fclose(file);
    return total;
}

// Helper function to fetch the username for a specific card number
int fetchUsername(int cardNumber, char *accountHolderName) {
    FILE *file = fopen("data/credentials.txt", "r");
    if (file == NULL) {
        perror("Error opening credentials.txt");
        return 0;
    }

    int storedCardNumber, storedPIN;
    char storedUsername[50];
    while (fscanf(file, "%d %d %s", &storedCardNumber, &storedPIN, storedUsername) != EOF) {
        if (storedCardNumber == cardNumber) {
            strcpy(accountHolderName, storedUsername);
            fclose(file);
            return 1; // Username found
        }
    }

    fclose(file);
    return 0; // Username not found
}

// Encrypt or decrypt a string using XOR encryption
void xorEncryptDecrypt(char *data, const char *key) {
    size_t keyLen = strlen(key);
    for (size_t i = 0; i < strlen(data); i++) {
        data[i] ^= key[i % keyLen];
    }
}