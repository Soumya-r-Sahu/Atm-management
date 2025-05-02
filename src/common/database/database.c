#include "common/database/database.h"
#include "common/utils/logger.h"
#include "common/utils/path_manager.h"
#include "common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Initialize the database system
bool initialize_database(void) {
    // Create required data directories
    if (!ensure_directory_exists("data")) {
        write_error_log("Failed to create data directory");
        return false;
    }
    
    if (!ensure_directory_exists("logs")) {
        write_error_log("Failed to create logs directory");
        return false;
    }
    
    // Initialize data files
    bool success = initialize_data_files();
    if (!success) {
        write_error_log("Failed to initialize data files");
        return false;
    }
    
    write_info_log("Database system initialized successfully");
    return true;
}

// Close database connections
void close_database(void) {
    // Nothing to close in our file-based implementation
    write_info_log("Database connections closed");
}

// Check if card exists
bool doesCardExist(int cardNumber) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        write_error_log("Failed to open card file for reading");
        return false;
    }
    
    char line[256];
    int found = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    while (fgets(line, sizeof(line), file)) {
        int stored_card_number;
        if (sscanf(line, "%*s | %*s | %d", &stored_card_number) == 1) {
            if (stored_card_number == cardNumber) {
                found = 1;
                break;
            }
        }
    }
    
    fclose(file);
    return found == 1;
}

// Check if card is active
bool isCardActive(int cardNumber) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        write_error_log("Failed to open card file for reading");
        return false;
    }
    
    char line[256];
    int active = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    while (fgets(line, sizeof(line), file)) {
        int stored_card_number;
        char status[20];
        if (sscanf(line, "%*s | %*s | %d | %*s | %*s | %19s", &stored_card_number, status) >= 2) {
            if (stored_card_number == cardNumber) {
                if (strcmp(status, "Active") == 0) {
                    active = 1;
                }
                break;
            }
        }
    }
    
    fclose(file);
    return active == 1;
}

// Validate card with PIN
bool validateCard(int cardNumber, int pin) {
    char pin_str[20];
    sprintf(pin_str, "%d", pin);
    
    return validateCardWithHash(cardNumber, pin_str); // Using unhashed PIN for simplicity
}

// Validate card with hashed PIN
bool validateCardWithHash(int cardNumber, const char* pinHash) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        write_error_log("Failed to open card file for reading");
        return false;
    }
    
    char line[256];
    int valid = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash
    while (fgets(line, sizeof(line), file)) {
        int stored_card_number;
        char stored_pin_hash[100];
        if (sscanf(line, "%*s | %*s | %d | %*s | %*s | %*s | %99s", &stored_card_number, stored_pin_hash) >= 2) {
            if (stored_card_number == cardNumber) {
                // In a real implementation, would compare hashes securely
                if (strcmp(stored_pin_hash, pinHash) == 0) {
                    valid = 1;
                }
                break;
            }
        }
    }
    
    fclose(file);
    return valid == 1;
}

// Validate recipient account
bool validateRecipientAccount(int recipientCard) {
    // In a real system, this would check if the recipient account exists
    // Here we'll just check if the card exists
    return doesCardExist(recipientCard);
}

// Update PIN
bool updatePIN(int cardNumber, int newPin) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        write_error_log("Failed to open card file for reading");
        return false;
    }
    
    char temp_file_path[256];
    sprintf(temp_file_path, "%s.tmp", get_card_file_path());
    
    FILE* temp_file = fopen(temp_file_path, "w");
    if (!temp_file) {
        fclose(file);
        write_error_log("Failed to open temporary card file for writing");
        return false;
    }
    
    char line[256];
    int updated = 0;
    
    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    
    // Convert new PIN to string (would be hashed in real implementation)
    char new_pin_str[20];
    sprintf(new_pin_str, "%d", newPin);
    
    // Process each line
    while (fgets(line, sizeof(line), file)) {
        char card_id[20], account_id[20], card_number_str[20], card_type[20], expiry_date[20], status[20], pin_hash[100];
        if (sscanf(line, "%19s | %19s | %19s | %19s | %19s | %19s | %99s", 
                   card_id, account_id, card_number_str, card_type, expiry_date, status, pin_hash) >= 7) {
            
            int stored_card_number = atoi(card_number_str);
            if (stored_card_number == cardNumber) {
                // Update PIN hash for this card
                fprintf(temp_file, "%s | %s | %s | %s | %s | %s | %s\n",
                        card_id, account_id, card_number_str, card_type, expiry_date, status, new_pin_str);
                updated = 1;
            } else {
                // Copy line unchanged
                fputs(line, temp_file);
            }
        } else {
            // Copy line that couldn't be parsed
            fputs(line, temp_file);
        }
    }
    
    fclose(file);
    fclose(temp_file);
    
    if (updated) {
        // Replace original file with updated file
        remove(get_card_file_path());
        rename(temp_file_path, get_card_file_path());
        return true;
    } else {
        // Card not found, remove temp file
        remove(temp_file_path);
        return false;
    }
}

// Update PIN hash
bool updatePINHash(int cardNumber, const char* pinHash) {
    // For simplicity, use the same implementation as updatePIN
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        write_error_log("Failed to open card file for reading");
        return false;
    }
    
    char temp_file_path[256];
    sprintf(temp_file_path, "%s.tmp", get_card_file_path());
    
    FILE* temp_file = fopen(temp_file_path, "w");
    if (!temp_file) {
        fclose(file);
        write_error_log("Failed to open temporary card file for writing");
        return false;
    }
    
    char line[256];
    int updated = 0;
    
    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    
    // Process each line
    while (fgets(line, sizeof(line), file)) {
        char card_id[20], account_id[20], card_number_str[20], card_type[20], expiry_date[20], status[20], orig_pin_hash[100];
        if (sscanf(line, "%19s | %19s | %19s | %19s | %19s | %19s | %99s", 
                   card_id, account_id, card_number_str, card_type, expiry_date, status, orig_pin_hash) >= 7) {
            
            int stored_card_number = atoi(card_number_str);
            if (stored_card_number == cardNumber) {
                // Update PIN hash for this card
                fprintf(temp_file, "%s | %s | %s | %s | %s | %s | %s\n",
                        card_id, account_id, card_number_str, card_type, expiry_date, status, pinHash);
                updated = 1;
            } else {
                // Copy line unchanged
                fputs(line, temp_file);
            }
        } else {
            // Copy line that couldn't be parsed
            fputs(line, temp_file);
        }
    }
    
    fclose(file);
    fclose(temp_file);
    
    if (updated) {
        // Replace original file with updated file
        remove(get_card_file_path());
        rename(temp_file_path, get_card_file_path());
        return true;
    } else {
        // Card not found, remove temp file
        remove(temp_file_path);
        return false;
    }
}

// Get card holder name
bool getCardHolderName(int cardNumber, char* name, size_t nameSize) {
    FILE* file = fopen(get_customer_file_path(), "r");
    if (!file) {
        write_error_log("Failed to open customer file for reading");
        return false;
    }
    
    char line[256];
    int found = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Customer ID | Name | Card Number | Address | Phone | Email
    while (fgets(line, sizeof(line), file)) {
        int stored_card_number;
        char customer_name[100];
        if (sscanf(line, "%*s | %99[^|] | %d", customer_name, &stored_card_number) >= 2) {
            if (stored_card_number == cardNumber) {
                // Trim whitespace
                char* end = customer_name + strlen(customer_name) - 1;
                while (end > customer_name && (*end == ' ' || *end == '\t')) *end-- = '\0';
                
                strncpy(name, customer_name, nameSize - 1);
                name[nameSize - 1] = '\0';
                found = 1;
                break;
            }
        }
    }
    
    fclose(file);
    return found == 1;
}

// Get card holder phone
bool getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize) {
    FILE* file = fopen(get_customer_file_path(), "r");
    if (!file) {
        write_error_log("Failed to open customer file for reading");
        return false;
    }
    
    char line[256];
    int found = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Customer ID | Name | Card Number | Address | Phone | Email
    while (fgets(line, sizeof(line), file)) {
        int stored_card_number;
        char customer_phone[100];
        if (sscanf(line, "%*s | %*[^|] | %d | %*[^|] | %99[^|]", &stored_card_number, customer_phone) >= 2) {
            if (stored_card_number == cardNumber) {
                // Trim whitespace
                char* end = customer_phone + strlen(customer_phone) - 1;
                while (end > customer_phone && (*end == ' ' || *end == '\t')) *end-- = '\0';
                
                strncpy(phone, customer_phone, phoneSize - 1);
                phone[phoneSize - 1] = '\0';
                found = 1;
                break;
            }
        }
    }
    
    fclose(file);
    return found == 1;
}

// Fetch account balance
bool fetchBalance(int cardNumber, float* balance) {
    FILE* file = fopen(getAccountingFilePath(), "r");
    if (!file) {
        write_error_log("Failed to open accounting file for reading");
        return false;
    }
    
    char line[256];
    bool found = false;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Format: Account ID | Card Number | Balance | Currency | Status
    while (fgets(line, sizeof(line), file)) {
        int stored_card_number;
        float stored_balance;
        if (sscanf(line, "%*s | %d | %f", &stored_card_number, &stored_balance) >= 2) {
            if (stored_card_number == cardNumber) {
                *balance = stored_balance;
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

// Update account balance
bool updateBalance(int cardNumber, float newBalance) {
    FILE* file = fopen(getAccountingFilePath(), "r");
    if (!file) {
        write_error_log("Failed to open accounting file for reading");
        return false;
    }
    
    char temp_file_path[256];
    sprintf(temp_file_path, "%s.tmp", getAccountingFilePath());
    
    FILE* temp_file = fopen(temp_file_path, "w");
    if (!temp_file) {
        fclose(file);
        write_error_log("Failed to open temporary accounting file for writing");
        return false;
    }
    
    char line[256];
    int updated = 0;
    
    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    
    // Process each line
    while (fgets(line, sizeof(line), file)) {
        char account_id[20], card_number_str[20], balance_str[20], currency[10], status[20];
        if (sscanf(line, "%19s | %19s | %19s | %9s | %19s", 
                   account_id, card_number_str, balance_str, currency, status) >= 5) {
            
            int stored_card_number = atoi(card_number_str);
            if (stored_card_number == cardNumber) {
                // Update balance for this account
                fprintf(temp_file, "%s | %s | %.2f | %s | %s\n",
                        account_id, card_number_str, newBalance, currency, status);
                updated = 1;
            } else {
                // Copy line unchanged
                fputs(line, temp_file);
            }
        } else {
            // Copy line that couldn't be parsed
            fputs(line, temp_file);
        }
    }
    
    fclose(file);
    fclose(temp_file);
    
    if (updated) {
        // Replace original file with updated file
        remove(getAccountingFilePath());
        rename(temp_file_path, getAccountingFilePath());
        return true;
    } else {
        // Card not found, remove temp file
        remove(temp_file_path);
        return false;
    }
}

// Log withdrawal for tracking
void logWithdrawal(int cardNumber, float amount) {
    FILE* file = fopen(getWithdrawalsLogFilePath(), "a");
    if (!file) {
        write_error_log("Failed to open withdrawals log file");
        return;
    }
    
    // Get current date
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char date_str[11];
    char timestamp[20];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Log the withdrawal
    fprintf(file, "%d,%s,%.2f,%s\n", cardNumber, date_str, amount, timestamp);
    fclose(file);
}

// Log withdrawal with specific date for limit tracking
void logWithdrawalForLimit(int cardNumber, float amount, const char* date) {
    FILE* file = fopen(getWithdrawalsLogFilePath(), "a");
    if (!file) {
        write_error_log("Failed to open withdrawals log file");
        return;
    }
    
    // Get current timestamp
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Log the withdrawal with specified date
    fprintf(file, "%d,%s,%.2f,%s\n", cardNumber, date, amount, timestamp);
    fclose(file);
}

// Get daily withdrawal total
float getDailyWithdrawals(int cardNumber) {
    FILE* file = fopen(getWithdrawalsLogFilePath(), "r");
    if (!file) {
        return 0.0f; // No withdrawals if file doesn't exist
    }
    
    char line[256];
    float total = 0.0f;
    
    // Get current date for comparison
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char today[11];
    strftime(today, sizeof(today), "%Y-%m-%d", tm_now);
    
    // Format: CardNumber,Date,Amount,Timestamp
    while (fgets(line, sizeof(line), file)) {
        int stored_card_number;
        char date[11];
        float amount;
        
        if (sscanf(line, "%d,%10[^,],%f", &stored_card_number, date, &amount) >= 3) {
            if (stored_card_number == cardNumber && strcmp(date, today) == 0) {
                total += amount;
            }
        }
    }
    
    fclose(file);
    return total;
}

// Block a card
bool blockCard(int cardNumber) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        write_error_log("Failed to open card file for reading");
        return false;
    }
    
    char temp_file_path[256];
    sprintf(temp_file_path, "%s.tmp", get_card_file_path());
    
    FILE* temp_file = fopen(temp_file_path, "w");
    if (!temp_file) {
        fclose(file);
        write_error_log("Failed to open temporary card file for writing");
        return false;
    }
    
    char line[256];
    int updated = 0;
    
    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    
    // Process each line
    while (fgets(line, sizeof(line), file)) {
        char card_id[20], account_id[20], card_number_str[20], card_type[20], expiry_date[20], status[20], pin_hash[100];
        if (sscanf(line, "%19s | %19s | %19s | %19s | %19s | %19s | %99s", 
                   card_id, account_id, card_number_str, card_type, expiry_date, status, pin_hash) >= 7) {
            
            int stored_card_number = atoi(card_number_str);
            if (stored_card_number == cardNumber) {
                // Block this card
                fprintf(temp_file, "%s | %s | %s | %s | %s | Blocked | %s\n",
                        card_id, account_id, card_number_str, card_type, expiry_date, pin_hash);
                updated = 1;
            } else {
                // Copy line unchanged
                fputs(line, temp_file);
            }
        } else {
            // Copy line that couldn't be parsed
            fputs(line, temp_file);
        }
    }
    
    fclose(file);
    fclose(temp_file);
    
    if (updated) {
        // Replace original file with updated file
        remove(get_card_file_path());
        rename(temp_file_path, get_card_file_path());
        return true;
    } else {
        // Card not found, remove temp file
        remove(temp_file_path);
        return false;
    }
}

// Unblock a card
bool unblockCard(int cardNumber) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        write_error_log("Failed to open card file for reading");
        return false;
    }
    
    char temp_file_path[256];
    sprintf(temp_file_path, "%s.tmp", get_card_file_path());
    
    FILE* temp_file = fopen(temp_file_path, "w");
    if (!temp_file) {
        fclose(file);
        write_error_log("Failed to open temporary card file for writing");
        return false;
    }
    
    char line[256];
    int updated = 0;
    
    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    
    // Process each line
    while (fgets(line, sizeof(line), file)) {
        char card_id[20], account_id[20], card_number_str[20], card_type[20], expiry_date[20], status[20], pin_hash[100];
        if (sscanf(line, "%19s | %19s | %19s | %19s | %19s | %19s | %99s", 
                   card_id, account_id, card_number_str, card_type, expiry_date, status, pin_hash) >= 7) {
            
            int stored_card_number = atoi(card_number_str);
            if (stored_card_number == cardNumber) {
                // Unblock this card
                fprintf(temp_file, "%s | %s | %s | %s | %s | Active | %s\n",
                        card_id, account_id, card_number_str, card_type, expiry_date, pin_hash);
                updated = 1;
            } else {
                // Copy line unchanged
                fputs(line, temp_file);
            }
        } else {
            // Copy line that couldn't be parsed
            fputs(line, temp_file);
        }
    }
    
    fclose(file);
    fclose(temp_file);
    
    if (updated) {
        // Replace original file with updated file
        remove(get_card_file_path());
        rename(temp_file_path, get_card_file_path());
        return true;
    } else {
        // Card not found, remove temp file
        remove(temp_file_path);
        return false;
    }
}

// Log transaction
bool logTransaction(int cardNumber, const char* transactionType, float amount, bool success) {
    FILE* file = fopen(getTransactionsLogFilePath(), "a");
    if (!file) {
        write_error_log("Failed to open transaction log file");
        return false;
    }
    
    // Get current timestamp
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Log the transaction
    fprintf(file, "%s | %d | %s | %.2f | %s\n",
            timestamp, cardNumber, transactionType, amount, success ? "Success" : "Failed");
    fclose(file);
    
    return true;
}