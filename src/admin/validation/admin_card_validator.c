#include "admin/validation/admin_card_validator.h"
#include "common/validation/card_validation.h"
#include "common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Admin-specific card validation with enhanced security
AdminCardValidationResult validate_card_admin(int card_number, int pin, int security_level) {
    AdminCardValidationResult result = {0};
    
    // First use common validation
    if (!does_card_exist(card_number)) {
        result.status = ADMIN_CARD_NOT_FOUND;
        result.message = "Card not found in the system";
        writeAuditLog("ADMIN", "Attempted to validate non-existent card");
        return result;
    }
    
    if (is_card_expired(card_number)) {
        result.status = ADMIN_CARD_EXPIRED;
        result.message = "Card has expired";
        writeAuditLog("ADMIN", "Attempted to validate expired card");
        return result;
    }
    
    if (!is_card_active(card_number)) {
        result.status = ADMIN_CARD_INACTIVE;
        result.message = "Card is inactive or blocked";
        writeAuditLog("ADMIN", "Attempted to validate inactive/blocked card");
        return result;
    }
    
    // Enhanced security checks for admin interface
    if (security_level > 0) {
        // Check if the card has been reported as suspicious
        if (is_card_flagged(card_number)) {
            result.status = ADMIN_CARD_FLAGGED;
            result.message = "Card has been flagged for suspicious activity";
            writeAuditLog("ADMIN", "Attempted to validate flagged card");
            return result;
        }
    }
    
    // Validate PIN if provided (pin value of -1 means skip PIN validation)
    if (pin != -1) {
        if (!validate_card(card_number, pin)) {
            result.status = ADMIN_CARD_INVALID_PIN;
            result.message = "Invalid PIN provided";
            writeAuditLog("ADMIN", "Failed PIN validation attempt");
            return result;
        }
    }
    
    // Card validation successful
    result.status = ADMIN_CARD_VALID;
    result.message = "Card validated successfully";
    
    // Include additional account information for admin use
    result.account_balance = fetchBalance(card_number);
    get_card_details(card_number, &result.details);
    
    return result;
}

// Check if a card has been flagged for suspicious activity
bool is_card_flagged(int card_number) {
    // This would normally check a security database
    // For this implementation, we'll just check a file
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/security/flagged_cards.txt");
    
    FILE* file = fopen(filepath, "r");
    if (!file) {
        return false; // File doesn't exist, no cards flagged
    }
    
    char line[256];
    bool flagged = false;
    
    while (fgets(line, sizeof(line), file)) {
        int flagged_card;
        if (sscanf(line, "%d", &flagged_card) == 1) {
            if (flagged_card == card_number) {
                flagged = true;
                break;
            }
        }
    }
    
    fclose(file);
    return flagged;
}

// Get detailed card information for admin interface
void get_card_details(int card_number, CardDetails* details) {
    if (!details) {
        return;
    }
    
    // Initialize with empty values
    memset(details, 0, sizeof(CardDetails));
    details->card_number = card_number;
    
    // Read card information from database
    FILE* file = fopen(getCardFilePath(), "r");
    if (!file) {
        return;
    }
    
    char line[512];
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    while (fgets(line, sizeof(line), file)) {
        char cardID[20], accountID[20], cardNumberStr[20], cardType[20], expiryDate[20], status[20], pinHash[100];
        
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                  cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            
            if (storedCardNumber == card_number) {
                // Card found, fill in details
                strncpy(details->card_id, cardID, sizeof(details->card_id) - 1);
                strncpy(details->account_id, accountID, sizeof(details->account_id) - 1);
                strncpy(details->card_type, cardType, sizeof(details->card_type) - 1);
                strncpy(details->expiry_date, expiryDate, sizeof(details->expiry_date) - 1);
                strncpy(details->status, status, sizeof(details->status) - 1);
                
                // Get customer name and phone number
                getCardHolderName(card_number, details->holder_name, sizeof(details->holder_name));
                getCardHolderPhone(card_number, details->phone_number, sizeof(details->phone_number));
                
                break;
            }
        }
    }
    
    fclose(file);
    
    // Get transaction history summary
    details->total_transactions = get_transaction_count(card_number);
    details->last_transaction_date[0] = '\0';
    details->last_transaction_amount = 0.0;
    
    get_last_transaction(card_number, details->last_transaction_date, 
                        sizeof(details->last_transaction_date), &details->last_transaction_amount);
                        
    // Get login history
    details->last_login_date[0] = '\0';
    get_last_login_date(card_number, details->last_login_date, sizeof(details->last_login_date));
    details->failed_login_attempts = get_failed_login_attempts(card_number);
}

// Get transaction count for a card
int get_transaction_count(int card_number) {
    // This would normally query the transaction database
    // For a simple implementation, we'll read from the transaction log
    
    FILE* file = fopen("logs/transactions.log", "r");
    if (!file) {
        return 0;
    }
    
    char line[512];
    int count = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "Card Number") && strstr(line, "Amount")) {
            int transaction_card_number;
            if (sscanf(line, "Card Number: %d", &transaction_card_number) == 1 ||
                sscanf(line, "%*s %*s %d", &transaction_card_number) == 1) {
                if (transaction_card_number == card_number) {
                    count++;
                }
            }
        }
    }
    
    fclose(file);
    return count;
}

// Get the last transaction details for a card
void get_last_transaction(int card_number, char* date, size_t date_size, float* amount) {
    FILE* file = fopen("logs/transactions.log", "r");
    if (!file || !date || !amount) {
        return;
    }
    
    *amount = 0.0;
    date[0] = '\0';
    
    char line[512];
    char last_date[64] = "";
    float last_amount = 0.0;
    
    while (fgets(line, sizeof(line), file)) {
        int transaction_card_number;
        char transaction_date[64];
        float transaction_amount;
        
        if (sscanf(line, "[%[^]]] Card Number: %d, Amount: $%f", 
                   transaction_date, &transaction_card_number, &transaction_amount) >= 3) {
            if (transaction_card_number == card_number) {
                strcpy(last_date, transaction_date);
                last_amount = transaction_amount;
            }
        }
    }
    
    fclose(file);
    
    if (last_date[0] != '\0') {
        strncpy(date, last_date, date_size - 1);
        date[date_size - 1] = '\0';
        *amount = last_amount;
    }
}

// Get the last login date for a card
void get_last_login_date(int card_number, char* date, size_t date_size) {
    FILE* file = fopen("logs/audit.log", "r");
    if (!file || !date) {
        return;
    }
    
    date[0] = '\0';
    
    char line[512];
    char last_date[64] = "";
    
    while (fgets(line, sizeof(line), file)) {
        char log_date[64];
        char log_type[20];
        char log_message[256];
        
        if (sscanf(line, "[%[^]]] [%[^]]] %[^\n]", log_date, log_type, log_message) >= 3) {
            if (strcmp(log_type, "AUTH") == 0) {
                int auth_card_number;
                if (sscanf(log_message, "Successful authentication for card %d", &auth_card_number) == 1) {
                    if (auth_card_number == card_number) {
                        strcpy(last_date, log_date);
                    }
                }
            }
        }
    }
    
    fclose(file);
    
    if (last_date[0] != '\0') {
        strncpy(date, last_date, date_size - 1);
        date[date_size - 1] = '\0';
    }
}

// Get the number of failed login attempts for a card
int get_failed_login_attempts(int card_number) {
    FILE* file = fopen("logs/audit.log", "r");
    if (!file) {
        return 0;
    }
    
    char line[512];
    int count = 0;
    
    // Get today's date
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char today[11];
    strftime(today, sizeof(today), "%Y-%m-%d", tm_now);
    
    while (fgets(line, sizeof(line), file)) {
        char log_date[64];
        char log_type[20];
        char log_message[256];
        
        if (sscanf(line, "[%[^]]] [%[^]]] %[^\n]", log_date, log_type, log_message) >= 3) {
            // Only count today's failed attempts
            if (strncmp(log_date, today, 10) == 0 && strcmp(log_type, "AUTH") == 0) {
                int auth_card_number;
                if (strstr(log_message, "Invalid PIN") && 
                    sscanf(log_message, "%*s %*s %*s %*s %d", &auth_card_number) == 1) {
                    if (auth_card_number == card_number) {
                        count++;
                    }
                }
            }
        }
    }
    
    fclose(file);
    return count;
}