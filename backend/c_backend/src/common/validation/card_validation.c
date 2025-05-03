#include "common/validation/card_validation.h"
#include "common/utils/logger.h"
#include "common/utils/hash_utils.h"
#include "common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Check if a card number meets format requirements
bool is_valid_card_number(int card_number) {
    // Check if the number is 6 digits (between 100000 and 999999)
    return (card_number >= 100000 && card_number <= 999999);
}

// Luhn algorithm to validate credit card numbers
// This is a more comprehensive validation for real-world use
bool validate_card_format(long long cardNumber) {
    if (cardNumber <= 0) return false;
    
    int sum = 0;
    bool alternate = false;
    
    while (cardNumber > 0) {
        int digit = cardNumber % 10;
        
        if (alternate) {
            digit *= 2;
            if (digit > 9) {
                digit = digit - 9;
            }
        }
        
        sum += digit;
        alternate = !alternate;
        cardNumber /= 10;
    }
    
    return (sum % 10 == 0);
}

// Check if a card exists in the database
bool does_card_exist(int cardNumber) {
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card.txt file");
        return false;
    }
    
    char line[256];
    bool found = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], pinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

// Check if a card is active
bool is_card_active(int cardNumber) {
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card.txt file");
        return false;
    }
    
    char line[256];
    bool active = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], pinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                // Check if status is "Active"
                if (strstr(status, "Active") != NULL) {
                    active = true;
                }
                break;
            }
        }
    }
    
    fclose(file);
    return active;
}

// Validate card with PIN (legacy method, for backward compatibility)
bool validate_card(int cardNumber, int pin) {
    char pinStr[20];
    sprintf(pinStr, "%d", pin);
    char* pinHash = sha256_hash(pinStr);
    bool result = validate_card_with_hash(cardNumber, pinHash);
    free(pinHash);
    return result;
}

// Validate card with PIN hash
bool validate_card_with_hash(int cardNumber, const char* pinHash) {
    if (pinHash == NULL) {
        writeErrorLog("NULL PIN hash provided to validate_card_with_hash");
        return false;
    }
    
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card.txt file");
        return false;
    }
    
    char line[256];
    bool valid = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], storedPinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, storedPinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                // Use secure hash comparison instead of strcmp
                if (secure_hash_compare(storedPinHash, pinHash)) {
                    valid = true;
                }
                break;
            }
        }
    }
    
    fclose(file);
    return valid;
}

// Check if a card is expired
bool is_card_expired(int cardNumber) {
    FILE* file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card.txt file");
        return true; // Assume expired if can't verify
    }
    
    char line[256];
    bool is_expired = true; // Default assumption
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Get current date for comparison
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char current_date[11];
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", tm_now);
    
    char cardID[10], accountID[10], cardNumberStr[20], cardType[10], expiryDate[15], status[10], pinHash[65];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                // Simple string comparison (works for YYYY-MM-DD format)
                is_expired = (strcmp(expiryDate, current_date) < 0);
                break;
            }
        }
    }
    
    fclose(file);
    return is_expired;
}

// Get daily withdrawal total for a card
float get_daily_withdrawals(int cardNumber) {
    const char* withdrawalLogPath = isTestingMode() ? 
        "logs/test_withdrawals.log" : "logs/withdrawals.log";
    
    FILE* file = fopen(withdrawalLogPath, "r");
    if (!file) {
        return 0.0f; // No withdrawals if the file doesn't exist
    }
    
    // Get current date
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char today[11];
    strftime(today, sizeof(today), "%Y-%m-%d", tm_now);
    
    char line[256];
    float totalWithdrawals = 0.0f;
    
    while (fgets(line, sizeof(line), file)) {
        int storedCardNumber;
        float amount;
        char date[11];
        
        // Expected format: CardNumber,Date,Amount
        if (sscanf(line, "%d,%10[^,],%f", &storedCardNumber, date, &amount) == 3) {
            if (storedCardNumber == cardNumber && strcmp(date, today) == 0) {
                totalWithdrawals += amount;
            }
        }
    }
    
    fclose(file);
    return totalWithdrawals;
}

// Log a withdrawal transaction for daily limit tracking
void log_withdrawal(int cardNumber, float amount) {
    const char* withdrawalLogPath = isTestingMode() ? 
        "logs/test_withdrawals.log" : "logs/withdrawals.log";
    
    FILE* file = fopen(withdrawalLogPath, "a");
    if (!file) {
        writeErrorLog("Failed to open withdrawals log file");
        return;
    }
    
    // Get current date and time
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char date[11], timestamp[20];
    strftime(date, sizeof(date), "%Y-%m-%d", tm_now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Write withdrawal record
    fprintf(file, "%d,%s,%.2f,%s\n", cardNumber, date, amount, timestamp);
    fclose(file);
}

// Track withdrawals for specific date - useful for backdated processing
void log_withdrawal_for_date(int cardNumber, float amount, const char* date) {
    const char* withdrawalLogPath = isTestingMode() ? 
        "logs/test_withdrawals.log" : "logs/withdrawals.log";
    
    FILE* file = fopen(withdrawalLogPath, "a");
    if (!file) {
        writeErrorLog("Failed to open withdrawals log file");
        return;
    }
    
    // Get current timestamp
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Write withdrawal record with specified date
    fprintf(file, "%d,%s,%.2f,%s\n", cardNumber, date, amount, timestamp);
    fclose(file);
}

// Secure comparison of hashes to prevent timing attacks
bool secure_hash_compare(const char* a, const char* b) {
    if (a == NULL || b == NULL) return false;
    
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    
    if (len_a != len_b) return false;
    
    unsigned char result = 0;
    for (size_t i = 0; i < len_a; i++) {
        result |= a[i] ^ b[i];
    }
    
    return (result == 0);
}

// Implement verify_card_pin to bridge the gap between validation systems
bool verify_card_pin(int card_number, int pin) {
    return validate_card(card_number, pin);
}