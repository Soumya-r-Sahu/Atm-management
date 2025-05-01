#include "../../../include/atm/validation/card_validator.h"
#include "../../../include/common/utils/logger.h"
#include "../../../include/common/utils/path_manager.h"
#include "../../../include/common/config/config_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// Validate card number format (16 digits, may contain dashes or spaces)
bool validate_card_format(const char* cardNumber) {
    if (!cardNumber) {
        return false;
    }
    
    int digits = 0;
    for (int i = 0; cardNumber[i] != '\0'; i++) {
        if (isdigit(cardNumber[i])) {
            digits++;
        } else if (cardNumber[i] != '-' && cardNumber[i] != ' ') {
            return false; // Invalid character
        }
    }
    
    return digits == 16; // Must have exactly 16 digits
}

// Validate a card based on card number
CardValidationStatus validate_card(const char* cardNumber) {
    if (!validate_card_format(cardNumber)) {
        return CARD_INVALID_FORMAT;
    }
    
    CardData* card = get_card_data(cardNumber);
    if (!card) {
        return CARD_NOT_FOUND;
    }
    
    // Check if card is blocked
    CardValidationStatus status = card->is_blocked ? CARD_BLOCKED : CARD_VALID;
    free(card);
    
    return status;
}

// Validate a virtual transaction with card number, CVV, and expiry date
CardValidationStatus validate_virtual_transaction(const char* cardNumber, int cvv, const char* expiryDate) {
    if (!validate_card_format(cardNumber)) {
        return CARD_INVALID_FORMAT;
    }
    
    CardData* card = get_card_data(cardNumber);
    if (!card) {
        return CARD_NOT_FOUND;
    }
    
    // Check if card is blocked
    if (card->is_blocked) {
        free(card);
        return CARD_BLOCKED;
    }
    
    // Check CVV
    if (card->cvv != cvv) {
        free(card);
        return CARD_CVV_INVALID;
    }
    
    // Check expiry date
    if (strcmp(card->expiry_date, expiryDate) != 0) {
        free(card);
        return CARD_EXPIRED;
    }
    
    free(card);
    return CARD_VALID;
}

// Get card data from card number
CardData* get_card_data(const char* cardNumber) {
    if (!cardNumber) {
        writeErrorLog("NULL card number provided to get_card_data");
        return NULL;
    }
    
    // Remove any dashes or spaces from the input
    char cleanCardNumber[17] = {0}; // 16 digits + null terminator
    int j = 0;
    for (int i = 0; cardNumber[i] != '\0' && j < 16; i++) {
        if (isdigit(cardNumber[i])) {
            cleanCardNumber[j++] = cardNumber[i];
        }
    }
    cleanCardNumber[j] = '\0';
    
    FILE* file = fopen(getCardFilePath(), "r");
    if (!file) {
        writeErrorLog("Failed to open card file");
        return NULL;
    }
    
    char line[256];
    CardData* card = NULL;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: CARD_ID | CARD_NUMBER | CVV | EXPIRY | PIN | IS_BLOCKED | CUSTOMER_ID
    while (fgets(line, sizeof(line), file)) {
        int cardId, cvv, isBlocked, customerId;
        char cardNumberStr[30], expiryDate[15], pinHash[70];
        
        if (sscanf(line, "| %d | %[^|] | %d | %[^|] | %[^|] | %d | %d |", 
                  &cardId, cardNumberStr, &cvv, expiryDate, pinHash, &isBlocked, &customerId) >= 7) {
            
            // Clean up the stored card number for comparison
            char cleanStoredCard[17] = {0};
            int k = 0;
            for (int i = 0; cardNumberStr[i] != '\0' && k < 16; i++) {
                if (isdigit(cardNumberStr[i])) {
                    cleanStoredCard[k++] = cardNumberStr[i];
                }
            }
            cleanStoredCard[k] = '\0';
            
            // Compare the clean card numbers
            if (strcmp(cleanCardNumber, cleanStoredCard) == 0) {
                // Found the card
                card = (CardData*)malloc(sizeof(CardData));
                if (!card) {
                    writeErrorLog("Memory allocation failed for CardData");
                    fclose(file);
                    return NULL;
                }
                
                card->card_id = cardId;
                card->customer_id = customerId;
                strncpy(card->card_number, cardNumberStr, sizeof(card->card_number) - 1);
                card->card_number[sizeof(card->card_number) - 1] = '\0';
                card->cvv = cvv;
                strncpy(card->expiry_date, expiryDate, sizeof(card->expiry_date) - 1);
                card->expiry_date[sizeof(card->expiry_date) - 1] = '\0';
                card->is_blocked = isBlocked;
                
                break;
            }
        }
    }
    
    fclose(file);
    return card;
}

// Check if virtual transactions have exceeded daily limits
bool is_virtual_transaction_limit_exceeded(const char* cardNumber, float amount) {
    if (!cardNumber) {
        return true; // If no card number, assume limit exceeded
    }
    
    // Get daily transaction limit from config
    const char* dailyLimitStr = get_config_value("daily_transaction_limit");
    float dailyLimit = dailyLimitStr ? atof(dailyLimitStr) : 50000.0f; // Default 50,000
    
    // Get the card ID to track transactions
    CardData* card = get_card_data(cardNumber);
    if (!card) {
        return true; // If card not found, assume limit exceeded
    }
    
    int cardId = card->card_id;
    free(card); // Free the card data after we get the ID
    
    // Get current date for comparison
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char today[11]; // YYYY-MM-DD
    strftime(today, sizeof(today), "%Y-%m-%d", tm_now);
    
    // Open transaction log to check daily total
    FILE* logFile = fopen(getTransactionLogFilePath(), "r");
    if (!logFile) {
        // If can't open log, assume safe transaction
        return false;
    }
    
    char line[256];
    float todayTotal = 0.0f;
    
    // Looking for today's transactions for this card
    while (fgets(line, sizeof(line), logFile)) {
        int logCardId;
        float txnAmount;
        char txnDate[11], txnType[20];
        
        // Try to parse line for transaction info
        if (strstr(line, today) && 
            sscanf(line, "%*[^|] | %d | %[^|] | %f | %[^|]", 
                   &logCardId, txnType, &txnAmount, txnDate) >= 4) {
            
            if (logCardId == cardId && 
                (strstr(txnType, "WITHDRAWAL") || strstr(txnType, "TRANSFER"))) {
                todayTotal += txnAmount;
            }
        }
    }
    
    fclose(logFile);
    
    // Check if this transaction would exceed the daily limit
    return (todayTotal + amount > dailyLimit);
}

// Check if virtual ATM features are enabled
bool is_virtual_atm_enabled() {
    const char* enabledStr = get_config_value("enable_virtual_atm");
    
    // Default to disabled if setting not found
    if (!enabledStr) {
        return false;
    }
    
    // Check if the value is "true", "yes", "1", or "enabled"
    return (strcasecmp(enabledStr, "true") == 0 || 
            strcasecmp(enabledStr, "yes") == 0 || 
            strcasecmp(enabledStr, "1") == 0 ||
            strcasecmp(enabledStr, "enabled") == 0);
}