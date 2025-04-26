#include "database.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CREDENTIALS_FILE "../data/credentials.txt"
#define ACCOUNTING_FILE "../data/accounting.txt"
#define WITHDRAWALS_FILE "../data/withdrawals.txt"

// Helper function to get current date as a string (YYYY-MM-DD)
static void getCurrentDate(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d", t);
}

// Fetch the balance for a given card number
float fetchBalance(int cardNumber) {
    FILE *file = fopen(ACCOUNTING_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open accounting file for balance check");
        return -1;
    }

    int storedCardNumber;
    float balance;
    int found = 0;

    while (fscanf(file, "%d | %f", &storedCardNumber, &balance) == 2) {
        if (storedCardNumber == cardNumber) {
            found = 1;
            break;
        }
    }

    fclose(file);
    
    if (!found) {
        char logMsg[100];
        sprintf(logMsg, "Card number %d not found in accounting file", cardNumber);
        writeErrorLog(logMsg);
        return -1;
    }
    
    return balance;
}

// Update the balance for a given card number
int updateBalance(int cardNumber, float newBalance) {
    FILE *file = fopen(ACCOUNTING_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open accounting file for balance update");
        return 0;
    }

    FILE *tempFile = fopen("../data/temp_accounting.txt", "w");
    if (tempFile == NULL) {
        fclose(file);
        writeErrorLog("Failed to create temporary accounting file");
        return 0;
    }

    int storedCardNumber;
    float balance;
    int found = 0;

    while (fscanf(file, "%d | %f", &storedCardNumber, &balance) == 2) {
        if (storedCardNumber == cardNumber) {
            fprintf(tempFile, "%-11d | %.2f\n", cardNumber, newBalance);
            found = 1;
        } else {
            fprintf(tempFile, "%-11d | %.2f\n", storedCardNumber, balance);
        }
    }

    fclose(file);
    fclose(tempFile);

    if (!found) {
        remove("../data/temp_accounting.txt");
        char logMsg[100];
        sprintf(logMsg, "Card number %d not found in accounting file during balance update", cardNumber);
        writeErrorLog(logMsg);
        return 0;
    }

    // Replace the original file with the updated one
    if (remove(ACCOUNTING_FILE) != 0 || 
        rename("../data/temp_accounting.txt", ACCOUNTING_FILE) != 0) {
        writeErrorLog("Failed to update accounting file");
        return 0;
    }

    return 1;
}

// Validate a card number and PIN combination
int validateCard(int cardNumber, int pin) {
    if (!isCardActive(cardNumber)) {
        return 0; // Card is not active
    }
    
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open credentials file for validation");
        return 0;
    }

    char line[256];
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);

    int storedCardNumber;
    int storedPIN;
    char storedUsername[50], storedStatus[10];
    int valid = 0;

    while (fscanf(file, "%49[^|] | %d | %d | %9s", 
                 storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            if (storedPIN == pin && strcmp(storedStatus, "Active") == 0) {
                valid = 1;
            }
            break;
        }
    }

    fclose(file);
    
    // Log the validation attempt
    if (valid) {
        char logMsg[100];
        sprintf(logMsg, "Successful validation for card %d", cardNumber);
        writeAuditLog("VALIDATION", logMsg);
    } else {
        char logMsg[100];
        sprintf(logMsg, "Failed validation for card %d", cardNumber);
        writeErrorLog(logMsg);
    }
    
    return valid;
}

// Check if a card is active
int isCardActive(int cardNumber) {
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open credentials file for status check");
        return 0;
    }

    char line[256];
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);

    int storedCardNumber;
    int storedPIN;
    char storedUsername[50], storedStatus[10];
    int active = 0;

    while (fscanf(file, "%49[^|] | %d | %d | %9s", 
                 storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            active = (strcmp(storedStatus, "Active") == 0);
            break;
        }
    }

    fclose(file);
    return active;
}

// Get the name of the card holder
int getCardHolderName(int cardNumber, char *name, int maxLen) {
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open credentials file for name lookup");
        return 0;
    }

    char line[256];
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);

    int storedCardNumber;
    int storedPIN;
    char storedUsername[50], storedStatus[10];
    int found = 0;

    while (fscanf(file, "%49[^|] | %d | %d | %9s", 
                 storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            found = 1;
            
            // Trim leading and trailing whitespace
            char *start = storedUsername;
            while (*start == ' ') start++;
            
            char *end = start + strlen(start) - 1;
            while (end > start && *end == ' ') end--;
            *(end + 1) = '\0';
            
            strncpy(name, start, maxLen - 1);
            name[maxLen - 1] = '\0'; // Ensure null termination
            break;
        }
    }

    fclose(file);
    return found;
}

// Get the total withdrawals for a card on the current day
float getDailyWithdrawals(int cardNumber) {
    FILE *file = fopen(WITHDRAWALS_FILE, "r");
    if (file == NULL) {
        // File doesn't exist yet, no withdrawals
        return 0.0f;
    }

    char currentDate[20];
    getCurrentDate(currentDate, sizeof(currentDate));

    char line[256];
    float totalWithdrawals = 0.0f;

    while (fgets(line, sizeof(line), file) != NULL) {
        int storedCardNumber;
        char dateStr[20];
        float amount;

        if (sscanf(line, "%d | %19s | %f", &storedCardNumber, dateStr, &amount) == 3) {
            if (storedCardNumber == cardNumber && strcmp(dateStr, currentDate) == 0) {
                totalWithdrawals += amount;
            }
        }
    }

    fclose(file);
    return totalWithdrawals;
}

// Log a withdrawal transaction for the daily limit tracking
void logWithdrawal(int cardNumber, float amount) {
    FILE *file = fopen(WITHDRAWALS_FILE, "a");
    if (file == NULL) {
        writeErrorLog("Failed to open withdrawals log file");
        return;
    }

    char currentDate[20];
    getCurrentDate(currentDate, sizeof(currentDate));

    fprintf(file, "%d | %s | %.2f\n", cardNumber, currentDate, amount);
    fclose(file);
}

// Reset daily withdrawals (typically called at midnight)
void resetDailyWithdrawals() {
    // In a real system, we might archive the old data rather than deleting it
    FILE *file = fopen(WITHDRAWALS_FILE, "w");
    if (file == NULL) {
        writeErrorLog("Failed to reset withdrawals log file");
        return;
    }
    
    fclose(file);
    writeAuditLog("SYSTEM", "Daily withdrawal limits have been reset");
}

// Update PIN for a card
int updatePIN(int cardNumber, int newPIN) {
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open credentials file for PIN update");
        return 0;
    }

    FILE *tempFile = fopen("../data/temp_credentials.txt", "w");
    if (tempFile == NULL) {
        fclose(file);
        writeErrorLog("Failed to create temporary credentials file");
        return 0;
    }

    char line[256];
    int modified = 0;

    // Copy header lines
    if (fgets(line, sizeof(line), file))
        fprintf(tempFile, "%s", line);
    if (fgets(line, sizeof(line), file))
        fprintf(tempFile, "%s", line);

    // Copy or modify each account line
    int storedCardNumber;
    int storedPIN;
    char storedUsername[50], storedStatus[10];

    while (fscanf(file, "%49[^|] | %d | %d | %9s", 
                 storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", 
                   storedUsername, storedCardNumber, newPIN, storedStatus);
            modified = 1;
        } else {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", 
                   storedUsername, storedCardNumber, storedPIN, storedStatus);
        }
    }

    fclose(file);
    fclose(tempFile);

    if (!modified) {
        remove("../data/temp_credentials.txt");
        writeErrorLog("Card not found for PIN update");
        return 0;
    }

    // Replace the original file
    if (remove(CREDENTIALS_FILE) != 0 || 
        rename("../data/temp_credentials.txt", CREDENTIALS_FILE) != 0) {
        writeErrorLog("Failed to update credentials file for PIN change");
        return 0;
    }

    // Log the PIN change
    char logMsg[100];
    sprintf(logMsg, "PIN updated for card %d", cardNumber);
    writeAuditLog("ACCOUNT", logMsg);

    return 1;
}

// Block a card
int blockCard(int cardNumber) {
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open credentials file for card blocking");
        return 0;
    }

    FILE *tempFile = fopen("../data/temp_credentials.txt", "w");
    if (tempFile == NULL) {
        fclose(file);
        writeErrorLog("Failed to create temporary credentials file");
        return 0;
    }

    char line[256];
    int modified = 0;

    // Copy header lines
    if (fgets(line, sizeof(line), file))
        fprintf(tempFile, "%s", line);
    if (fgets(line, sizeof(line), file))
        fprintf(tempFile, "%s", line);

    // Copy or modify each account line
    int storedCardNumber;
    int storedPIN;
    char storedUsername[50], storedStatus[10];

    while (fscanf(file, "%49[^|] | %d | %d | %9s", 
                 storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", 
                   storedUsername, storedCardNumber, storedPIN, "Blocked");
            modified = 1;
        } else {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", 
                   storedUsername, storedCardNumber, storedPIN, storedStatus);
        }
    }

    fclose(file);
    fclose(tempFile);

    if (!modified) {
        remove("../data/temp_credentials.txt");
        writeErrorLog("Card not found for blocking");
        return 0;
    }

    // Replace the original file
    if (remove(CREDENTIALS_FILE) != 0 || 
        rename("../data/temp_credentials.txt", CREDENTIALS_FILE) != 0) {
        writeErrorLog("Failed to update credentials file for card blocking");
        return 0;
    }

    // Log the card blocking
    char logMsg[100];
    sprintf(logMsg, "Card %d has been blocked", cardNumber);
    writeAuditLog("ACCOUNT", logMsg);

    return 1;
}

// Unblock a card
int unblockCard(int cardNumber) {
    FILE *file = fopen(CREDENTIALS_FILE, "r");
    if (file == NULL) {
        writeErrorLog("Failed to open credentials file for card unblocking");
        return 0;
    }

    FILE *tempFile = fopen("../data/temp_credentials.txt", "w");
    if (tempFile == NULL) {
        fclose(file);
        writeErrorLog("Failed to create temporary credentials file");
        return 0;
    }

    char line[256];
    int modified = 0;

    // Copy header lines
    if (fgets(line, sizeof(line), file))
        fprintf(tempFile, "%s", line);
    if (fgets(line, sizeof(line), file))
        fprintf(tempFile, "%s", line);

    // Copy or modify each account line
    int storedCardNumber;
    int storedPIN;
    char storedUsername[50], storedStatus[10];

    while (fscanf(file, "%49[^|] | %d | %d | %9s", 
                 storedUsername, &storedCardNumber, &storedPIN, storedStatus) == 4) {
        if (storedCardNumber == cardNumber) {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", 
                   storedUsername, storedCardNumber, storedPIN, "Active");
            modified = 1;
        } else {
            fprintf(tempFile, "%-20s | %-11d | %-4d | %-9s\n", 
                   storedUsername, storedCardNumber, storedPIN, storedStatus);
        }
    }

    fclose(file);
    fclose(tempFile);

    if (!modified) {
        remove("../data/temp_credentials.txt");
        writeErrorLog("Card not found for unblocking");
        return 0;
    }

    // Replace the original file
    if (remove(CREDENTIALS_FILE) != 0 || 
        rename("../data/temp_credentials.txt", CREDENTIALS_FILE) != 0) {
        writeErrorLog("Failed to update credentials file for card unblocking");
        return 0;
    }

    // Log the card unblocking
    char logMsg[100];
    sprintf(logMsg, "Card %d has been unblocked", cardNumber);
    writeAuditLog("ACCOUNT", logMsg);

    return 1;
}

