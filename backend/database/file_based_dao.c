/**
 * @file file_based_dao.c
 * @brief File-based Database Access Object Implementation
 * @version 1.0
 * @date May 11, 2025
 */

#include "../../include/common/database/dao_interface.h"
#include "../../include/common/utils/logger.h"
#include "../../include/common/database/db_unified_config.h"
#include "../../include/common/paths.h"
#include "../../include/common/utils/path_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Private function declarations
static bool file_doesCardExist(int cardNumber);
static bool file_isCardActive(int cardNumber);
static bool file_validateCard(int cardNumber, int pin);
static bool file_validateCardWithHash(int cardNumber, const char* pinHash);
static bool file_validateCardCVV(int cardNumber, int cvv);
static bool file_blockCard(int cardNumber);
static bool file_unblockCard(int cardNumber);
static bool file_updateCardPIN(int cardNumber, const char* newPINHash);
static bool file_getCardHolderName(int cardNumber, char* name, size_t nameSize);
static bool file_getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize);
static float file_fetchBalance(int cardNumber);
static bool file_updateBalance(int cardNumber, float newBalance);
static float file_getDailyWithdrawals(int cardNumber);
static void file_logWithdrawal(int cardNumber, float amount);
static bool file_logTransaction(int cardNumber, const char* transactionType, float amount, bool success);
static bool file_getMiniStatement(int cardNumber, Transaction* transactions, int maxTransactions, int* count);
static void* file_getConnection(void);
static void file_releaseConnection(void* conn);

// Dummy connection object - for file-based operations, we don't need a real connection
typedef struct {
    int dummy;  // Just a placeholder
} FileConnection;

static FileConnection dummyConn = { 0 };

/**
 * @brief Get file-based connection (dummy)
 * @return void* Dummy connection object
 */
static void* file_getConnection(void) {
    writeInfoLog("Getting file-based connection (dummy)");
    return &dummyConn;
}

/**
 * @brief Release file-based connection (dummy)
 * @param conn Dummy connection object
 */
static void file_releaseConnection(void* conn) {
    writeInfoLog("Releasing file-based connection (dummy)");
    return;
}

/**
 * @brief Check if a card exists in the file-based storage system
 * @param cardNumber The card number to check
 * @return bool True if card exists, false otherwise
 */
static bool file_doesCardExist(int cardNumber) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        writeErrorLog("Failed to open card file for reading");
        return false;
    }
    
    char line[256];
    bool found = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), file) == NULL || fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return false;
    }
    
    // Optimized card search - scan each line for the card number
    while (fgets(line, sizeof(line), file)) {
        int fileCardNumber;
        // Parse just the card number (3rd field)
        if (sscanf(line, "%*s | %*s | %d", &fileCardNumber) == 1) {
            if (fileCardNumber == cardNumber) {
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

/**
 * @brief Check if a card is active in the file-based storage system
 * @param cardNumber The card number to check
 * @return bool True if card is active, false otherwise
 */
static bool file_isCardActive(int cardNumber) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        writeErrorLog("Failed to open card file for reading");
        return false;
    }
    
    char line[256];
    bool isActive = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), file) == NULL || fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return false;
    }
    
    // Search for the card and check if it's active
    while (fgets(line, sizeof(line), file)) {
        int fileCardNumber;
        char status[20];
        
        // Parse the card number (3rd field) and status (6th field)
        if (sscanf(line, "%*s | %*s | %d | %*s | %*s | %19s", &fileCardNumber, status) == 2) {
            if (fileCardNumber == cardNumber) {
                // Check if status is "Active"
                isActive = (strcmp(status, "Active") == 0 || strcmp(status, "Active ") == 0);
                break;
            }
        }
    }
    
    fclose(file);
    return isActive;
}

/**
 * @brief Validate a card's PIN against the stored value
 * @param cardNumber The card number
 * @param pin The PIN to validate
 * @return bool True if PIN is valid, false otherwise
 */
static bool file_validateCard(int cardNumber, int pin) {
    char pinStr[10];
    sprintf(pinStr, "%04d", pin);  // Convert PIN to string
    
    // Convert PIN to hash - in a real implementation, use a proper hash function
    // For now, this is just a placeholder. The actual implementation would use a secure hash.
    char pinHash[100];
    sprintf(pinHash, "hash_%s", pinStr);  // Dummy hash
    
    return file_validateCardWithHash(cardNumber, pinHash);
}

/**
 * @brief Validate a card's PIN hash against the stored hash
 * @param cardNumber The card number
 * @param pinHash The PIN hash to validate
 * @return bool True if hash is valid, false otherwise
 */
static bool file_validateCardWithHash(int cardNumber, const char* pinHash) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        writeErrorLog("Failed to open card file for reading");
        return false;
    }
    
    char line[512];  // Larger buffer for PIN hash
    bool isValid = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), file) == NULL || fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return false;
    }
    
    // Search for the card and compare the PIN hash
    while (fgets(line, sizeof(line), file)) {
        int fileCardNumber;
        char storedPinHash[256];  // Larger to accommodate hash
        
        // Parse card number and PIN hash
        if (sscanf(line, "%*s | %*s | %d | %*s | %*s | %*s | %255s", &fileCardNumber, storedPinHash) == 2) {
            if (fileCardNumber == cardNumber) {
                // In a real implementation, use a secure comparison function
                isValid = (strcmp(storedPinHash, pinHash) == 0);
                break;
            }
        }
    }
    
    fclose(file);
    return isValid;
}

/**
 * @brief Validate a card's CVV
 * @param cardNumber The card number
 * @param cvv The CVV to validate
 * @return bool True if CVV is valid, false otherwise
 */
static bool file_validateCardCVV(int cardNumber, int cvv) {
    // In a real implementation, this would validate against a stored CVV
    // For this example, we'll just return true as CVV is not stored in our file system
    writeInfoLog("CVV validation not implemented in file-based storage");
    return true;
}

/**
 * @brief Block a card
 * @param cardNumber The card number to block
 * @return bool True if successfully blocked, false otherwise
 */
static bool file_blockCard(int cardNumber) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        writeErrorLog("Failed to open card file for reading");
        return false;
    }
    
    char temp_file_path[256];
    sprintf(temp_file_path, "%s.tmp", get_card_file_path());
    
    FILE* temp_file = fopen(temp_file_path, "w");
    if (!temp_file) {
        fclose(file);
        writeErrorLog("Failed to open temporary card file for writing");
        return false;
    }
    
    char line[256];
    bool updated = false;
    
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
            
            // If this is the card we want to block, update its status
            if (stored_card_number == cardNumber) {
                fprintf(temp_file, "%s | %s | %s | %s | %s | %s  | %s\n", 
                        card_id, account_id, card_number_str, card_type, expiry_date, "Blocked", pin_hash);
                updated = true;
            } else {
                // Just copy the line as is
                fputs(line, temp_file);
            }
        } else {
            // Malformed line, just copy it
            fputs(line, temp_file);
        }
    }
    
    fclose(file);
    fclose(temp_file);
    
    // Replace the original file with the updated one if changes were made
    if (updated) {
        remove(get_card_file_path());
        rename(temp_file_path, get_card_file_path());
        writeInfoLog("Card blocked successfully");
    } else {
        remove(temp_file_path);  // Clean up the temp file
        writeWarningLog("Card not found for blocking");
    }
    
    return updated;
}

/**
 * @brief Unblock a card
 * @param cardNumber The card number to unblock
 * @return bool True if successfully unblocked, false otherwise
 */
static bool file_unblockCard(int cardNumber) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        writeErrorLog("Failed to open card file for reading");
        return false;
    }
    
    char temp_file_path[256];
    sprintf(temp_file_path, "%s.tmp", get_card_file_path());
    
    FILE* temp_file = fopen(temp_file_path, "w");
    if (!temp_file) {
        fclose(file);
        writeErrorLog("Failed to open temporary card file for writing");
        return false;
    }
    
    char line[256];
    bool updated = false;
    
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
            
            // If this is the card we want to unblock, update its status
            if (stored_card_number == cardNumber) {
                fprintf(temp_file, "%s | %s | %s | %s | %s | %s  | %s\n", 
                        card_id, account_id, card_number_str, card_type, expiry_date, "Active", pin_hash);
                updated = true;
            } else {
                // Just copy the line as is
                fputs(line, temp_file);
            }
        } else {
            // Malformed line, just copy it
            fputs(line, temp_file);
        }
    }
    
    fclose(file);
    fclose(temp_file);
    
    // Replace the original file with the updated one if changes were made
    if (updated) {
        remove(get_card_file_path());
        rename(temp_file_path, get_card_file_path());
        writeInfoLog("Card unblocked successfully");
    } else {
        remove(temp_file_path);  // Clean up the temp file
        writeWarningLog("Card not found for unblocking");
    }
    
    return updated;
}

/**
 * @brief Update a card's PIN
 * @param cardNumber The card number
 * @param newPINHash The new PIN hash
 * @return bool True if successfully updated, false otherwise
 */
static bool file_updateCardPIN(int cardNumber, const char* newPINHash) {
    FILE* file = fopen(get_card_file_path(), "r");
    if (!file) {
        writeErrorLog("Failed to open card file for reading");
        return false;
    }
    
    char temp_file_path[256];
    sprintf(temp_file_path, "%s.tmp", get_card_file_path());
    
    FILE* temp_file = fopen(temp_file_path, "w");
    if (!temp_file) {
        fclose(file);
        writeErrorLog("Failed to open temporary card file for writing");
        return false;
    }
    
    char line[512];  // Larger buffer for PIN hash
    bool updated = false;
    
    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp_file);
    }
    
    // Process each line
    while (fgets(line, sizeof(line), file)) {
        char card_id[20], account_id[20], card_number_str[20], card_type[20], expiry_date[20], status[20], pin_hash[256];
        if (sscanf(line, "%19s | %19s | %19s | %19s | %19s | %19s | %255s", 
                   card_id, account_id, card_number_str, card_type, expiry_date, status, pin_hash) >= 7) {
            
            int stored_card_number = atoi(card_number_str);
            
            // If this is the card we want to update, change the PIN hash
            if (stored_card_number == cardNumber) {
                fprintf(temp_file, "%s | %s | %s | %s | %s | %s | %s\n", 
                        card_id, account_id, card_number_str, card_type, expiry_date, status, newPINHash);
                updated = true;
            } else {
                // Just copy the line as is
                fputs(line, temp_file);
            }
        } else {
            // Malformed line, just copy it
            fputs(line, temp_file);
        }
    }
    
    fclose(file);
    fclose(temp_file);
    
    // Replace the original file with the updated one if changes were made
    if (updated) {
        remove(get_card_file_path());
        rename(temp_file_path, get_card_file_path());
        writeInfoLog("Card PIN updated successfully");
    } else {
        remove(temp_file_path);  // Clean up the temp file
        writeWarningLog("Card not found for PIN update");
    }
    
    return updated;
}

/**
 * @brief Get the card holder's name
 * @param cardNumber The card number
 * @param name Buffer to store the name
 * @param nameSize Size of the name buffer
 * @return bool True if name was found, false otherwise
 */
static bool file_getCardHolderName(int cardNumber, char* name, size_t nameSize) {
    if (!name || nameSize == 0) {
        writeErrorLog("Invalid parameters for getCardHolderName");
        return false;
    }
    
    // First, find the account ID for this card
    FILE* cardFile = fopen(get_card_file_path(), "r");
    if (!cardFile) {
        writeErrorLog("Failed to open card file for reading");
        return false;
    }
    
    char line[256];
    char account_id[20] = {0};
    bool foundCard = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), cardFile) == NULL || fgets(line, sizeof(line), cardFile) == NULL) {
        fclose(cardFile);
        return false;
    }
    
    // Find the card and get its account ID
    while (fgets(line, sizeof(line), cardFile)) {
        char card_id[20], acc_id[20], card_number_str[20], card_type[20], expiry_date[20], status[20], pin_hash[100];
        if (sscanf(line, "%19s | %19s | %19s | %19s | %19s | %19s | %99s", 
                   card_id, acc_id, card_number_str, card_type, expiry_date, status, pin_hash) >= 7) {
            
            int stored_card_number = atoi(card_number_str);
            
            if (stored_card_number == cardNumber) {
                strncpy(account_id, acc_id, sizeof(account_id) - 1);
                foundCard = true;
                break;
            }
        }
    }
    
    fclose(cardFile);
    
    if (!foundCard || account_id[0] == '\0') {
        writeWarningLog("Card not found or account ID missing");
        return false;
    }
    
    // Now look up the customer name using the account ID
    FILE* customerFile = fopen(get_customer_file_path(), "r");
    if (!customerFile) {
        writeErrorLog("Failed to open customer file for reading");
        return false;
    }
    
    bool foundCustomer = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), customerFile) == NULL || fgets(line, sizeof(line), customerFile) == NULL) {
        fclose(customerFile);
        return false;
    }
    
    // Find the customer record with matching account ID
    while (fgets(line, sizeof(line), customerFile)) {
        char customer_id[20], acc_id[20], customer_name[100];
        if (sscanf(line, "%19s | %19s | %99[^|]", customer_id, acc_id, customer_name) >= 3) {
            if (strcmp(acc_id, account_id) == 0) {
                // Remove trailing spaces from the name
                char* end = customer_name + strlen(customer_name) - 1;
                while (end > customer_name && isspace((unsigned char)*end)) {
                    *end-- = '\0';
                }
                
                strncpy(name, customer_name, nameSize - 1);
                name[nameSize - 1] = '\0';  // Ensure null-termination
                foundCustomer = true;
                break;
            }
        }
    }
    
    fclose(customerFile);
    return foundCustomer;
}

/**
 * @brief Get the card holder's phone number
 * @param cardNumber The card number
 * @param phone Buffer to store the phone number
 * @param phoneSize Size of the phone buffer
 * @return bool True if phone number was found, false otherwise
 */
static bool file_getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize) {
    // In our simplified file structure, phone might not be directly available
    // For the example implementation, we'll return a placeholder
    if (!phone || phoneSize == 0) {
        return false;
    }
    
    strncpy(phone, "Not available", phoneSize - 1);
    phone[phoneSize - 1] = '\0';
    
    writeInfoLog("Card holder phone retrieval not fully implemented in file-based storage");
    return true;
}

/**
 * @brief Fetch the account balance for a card
 * @param cardNumber The card number
 * @return float The account balance, or -1 on error
 */
static float file_fetchBalance(int cardNumber) {
    // First, find the account ID for this card
    FILE* cardFile = fopen(get_card_file_path(), "r");
    if (!cardFile) {
        writeErrorLog("Failed to open card file for reading");
        return -1.0f;
    }
    
    char line[256];
    char account_id[20] = {0};
    bool foundCard = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), cardFile) == NULL || fgets(line, sizeof(line), cardFile) == NULL) {
        fclose(cardFile);
        return -1.0f;
    }
    
    // Find the card and get its account ID
    while (fgets(line, sizeof(line), cardFile)) {
        char card_id[20], acc_id[20], card_number_str[20], card_type[20], expiry_date[20], status[20], pin_hash[100];
        if (sscanf(line, "%19s | %19s | %19s | %19s | %19s | %19s | %99s", 
                   card_id, acc_id, card_number_str, card_type, expiry_date, status, pin_hash) >= 7) {
            
            int stored_card_number = atoi(card_number_str);
            
            if (stored_card_number == cardNumber) {
                strncpy(account_id, acc_id, sizeof(account_id) - 1);
                foundCard = true;
                break;
            }
        }
    }
    
    fclose(cardFile);
    
    if (!foundCard || account_id[0] == '\0') {
        writeWarningLog("Card not found or account ID missing");
        return -1.0f;
    }
    
    // Now look up the balance in the accounting file
    FILE* accountingFile = fopen(getAccountingFilePath(), "r");
    if (!accountingFile) {
        writeErrorLog("Failed to open accounting file for reading");
        return -1.0f;
    }
    
    float balance = -1.0f;
    
    // Skip header lines
    if (fgets(line, sizeof(line), accountingFile) == NULL || fgets(line, sizeof(line), accountingFile) == NULL) {
        fclose(accountingFile);
        return -1.0f;
    }
    
    // Find the account record with matching account ID
    while (fgets(line, sizeof(line), accountingFile)) {
        char acc_id[20], card_number_str[20], balance_str[20], currency[10], status[20];
        if (sscanf(line, "%19s | %19s | %19s | %9s | %19s", 
                   acc_id, card_number_str, balance_str, currency, status) >= 5) {
            
            if (strcmp(acc_id, account_id) == 0) {
                balance = atof(balance_str);
                break;
            }
        }
    }
    
    fclose(accountingFile);
    return balance;
}

/**
 * @brief Update the account balance for a card
 * @param cardNumber The card number
 * @param newBalance The new balance
 * @return bool True if successfully updated, false otherwise
 */
static bool file_updateBalance(int cardNumber, float newBalance) {
    // First, find the account ID for this card
    FILE* cardFile = fopen(get_card_file_path(), "r");
    if (!cardFile) {
        writeErrorLog("Failed to open card file for reading");
        return false;
    }
    
    char line[256];
    char account_id[20] = {0};
    bool foundCard = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), cardFile) == NULL || fgets(line, sizeof(line), cardFile) == NULL) {
        fclose(cardFile);
        return false;
    }
    
    // Find the card and get its account ID
    while (fgets(line, sizeof(line), cardFile)) {
        char card_id[20], acc_id[20], card_number_str[20], card_type[20], expiry_date[20], status[20], pin_hash[100];
        if (sscanf(line, "%19s | %19s | %19s | %19s | %19s | %19s | %99s", 
                   card_id, acc_id, card_number_str, card_type, expiry_date, status, pin_hash) >= 7) {
            
            int stored_card_number = atoi(card_number_str);
            
            if (stored_card_number == cardNumber) {
                strncpy(account_id, acc_id, sizeof(account_id) - 1);
                foundCard = true;
                break;
            }
        }
    }
    
    fclose(cardFile);
    
    if (!foundCard || account_id[0] == '\0') {
        writeWarningLog("Card not found or account ID missing");
        return false;
    }
    
    // Now update the balance in the accounting file
    FILE* accountingFile = fopen(getAccountingFilePath(), "r");
    if (!accountingFile) {
        writeErrorLog("Failed to open accounting file for reading");
        return false;
    }
    
    char temp_file_path[256];
    sprintf(temp_file_path, "%s.tmp", getAccountingFilePath());
    
    FILE* temp_file = fopen(temp_file_path, "w");
    if (!temp_file) {
        fclose(accountingFile);
        writeErrorLog("Failed to open temporary accounting file for writing");
        return false;
    }
    
    bool updated = false;
    
    // Copy header lines
    if (fgets(line, sizeof(line), accountingFile)) {
        fputs(line, temp_file);
    }
    if (fgets(line, sizeof(line), accountingFile)) {
        fputs(line, temp_file);
    }
    
    // Process each line
    while (fgets(line, sizeof(line), accountingFile)) {
        char acc_id[20], card_number_str[20], balance_str[20], currency[10], status[20];
        if (sscanf(line, "%19s | %19s | %19s | %9s | %19s", 
                   acc_id, card_number_str, balance_str, currency, status) >= 5) {
            
            if (strcmp(acc_id, account_id) == 0) {
                // Update the balance
                fprintf(temp_file, "%s | %s | %.2f | %s | %s\n", 
                       acc_id, card_number_str, newBalance, currency, status);
                updated = true;
            } else {
                // Just copy the line as is
                fputs(line, temp_file);
            }
        } else {
            // Malformed line, just copy it
            fputs(line, temp_file);
        }
    }
    
    fclose(accountingFile);
    fclose(temp_file);
    
    // Replace the original file with the updated one if changes were made
    if (updated) {
        remove(getAccountingFilePath());
        rename(temp_file_path, getAccountingFilePath());
        writeInfoLog("Account balance updated successfully");
    } else {
        remove(temp_file_path);  // Clean up the temp file
        writeWarningLog("Account not found for balance update");
    }
    
    return updated;
}

/**
 * @brief Get total daily withdrawals for a card
 * @param cardNumber The card number
 * @return float The total amount withdrawn today
 */
static float file_getDailyWithdrawals(int cardNumber) {
    // Get today's date
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char today[11];
    strftime(today, sizeof(today), "%Y-%m-%d", tm_now);
    
    // Open transactions log
    FILE* file = fopen(getTransactionsLogPath(), "r");
    if (!file) {
        writeErrorLog("Failed to open transactions log file");
        return 0.0f;
    }
    
    char line[256];
    float totalWithdrawals = 0.0f;
    
    // Skip header lines if they exist
    if (fgets(line, sizeof(line), file) != NULL && strstr(line, "Transaction ID") != NULL) {
        // This was the header line, continue
    } else {
        // Not a header line, rewind to start of file
        rewind(file);
    }
    
    // Process each transaction
    while (fgets(line, sizeof(line), file)) {
        char transactionID[20], accountID[20], transactionType[20], amountStr[20], timestamp[30], status[20], remarks[50];
        if (sscanf(line, "%19s | %19s | %19s | %19s | %29s | %19s | %49[^\n]", 
                  transactionID, accountID, transactionType, amountStr, timestamp, status, remarks) >= 6) {
            
            // Check if this is a withdrawal for this card made today
            if (strcmp(transactionType, "Withdrawal") == 0 && 
                strcmp(status, "Success") == 0 &&
                strncmp(timestamp, today, 10) == 0) {
                
                // Check if this is for our card number (may need to cross-reference with card file)
                // For simplicity, we check if the card number appears in the remarks
                char cardNumberStr[20];
                sprintf(cardNumberStr, "%d", cardNumber);
                
                if (strstr(remarks, cardNumberStr) != NULL) {
                    float amount = atof(amountStr);
                    totalWithdrawals += amount;
                }
            }
        }
    }
    
    fclose(file);
    return totalWithdrawals;
}

/**
 * @brief Log a withdrawal transaction
 * @param cardNumber The card number
 * @param amount The withdrawal amount
 */
static void file_logWithdrawal(int cardNumber, float amount) {
    // Forward to the more general transaction logging function
    file_logTransaction(cardNumber, "Withdrawal", amount, true);
}

/**
 * @brief Log a transaction
 * @param cardNumber The card number
 * @param transactionType Type of transaction
 * @param amount Transaction amount
 * @param success Whether transaction was successful
 * @return bool True if successfully logged, false otherwise
 */
static bool file_logTransaction(int cardNumber, const char* transactionType, float amount, bool success) {
    // First, find the account ID for this card
    FILE* cardFile = fopen(get_card_file_path(), "r");
    if (!cardFile) {
        writeErrorLog("Failed to open card file for reading");
        return false;
    }
    
    char line[256];
    char account_id[20] = {0};
    bool foundCard = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), cardFile) == NULL || fgets(line, sizeof(line), cardFile) == NULL) {
        fclose(cardFile);
        return false;
    }
    
    // Find the card and get its account ID
    while (fgets(line, sizeof(line), cardFile)) {
        char card_id[20], acc_id[20], card_number_str[20], card_type[20], expiry_date[20], status[20], pin_hash[100];
        if (sscanf(line, "%19s | %19s | %19s | %19s | %19s | %19s | %99s", 
                   card_id, acc_id, card_number_str, card_type, expiry_date, status, pin_hash) >= 7) {
            
            int stored_card_number = atoi(card_number_str);
            
            if (stored_card_number == cardNumber) {
                strncpy(account_id, acc_id, sizeof(account_id) - 1);
                foundCard = true;
                break;
            }
        }
    }
    
    fclose(cardFile);
    
    if (!foundCard || account_id[0] == '\0') {
        writeWarningLog("Card not found or account ID missing");
        return false;
    }
    
    // Generate transaction ID (timestamp-based)
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char transactionID[20];
    sprintf(transactionID, "TXN%d%02d%02d%02d%02d", 
            tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
            tm_now->tm_hour, tm_now->tm_min);
    
    // Format timestamp
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Prepare remarks
    char remarks[50];
    sprintf(remarks, "Card %d %s", cardNumber, transactionType);
    
    // Open the transactions log file for appending
    FILE* logFile = fopen(getTransactionsLogPath(), "a");
    if (!logFile) {
        writeErrorLog("Failed to open transactions log file for writing");
        return false;
    }
    
    // Write the transaction record
    fprintf(logFile, "%s | %s | %-15s | %-8.2f | %-19s | %-17s | %s\n", 
            transactionID, account_id, transactionType, amount, timestamp, 
            success ? "Success" : "Failed", remarks);
    
    fclose(logFile);
    
    char logMsg[150];
    sprintf(logMsg, "Transaction logged: %s %s for card %d, amount: %.2f, status: %s", 
            transactionID, transactionType, cardNumber, amount, success ? "Success" : "Failed");
    writeInfoLog(logMsg);
    
    return true;
}

/**
 * @brief Get a mini statement of recent transactions
 * @param cardNumber The card number
 * @param transactions Array to store transactions
 * @param maxTransactions Maximum number of transactions to retrieve
 * @param count Output parameter for number of transactions retrieved
 * @return bool True if successful, false otherwise
 */
static bool file_getMiniStatement(int cardNumber, Transaction* transactions, int maxTransactions, int* count) {
    if (!transactions || maxTransactions <= 0 || !count) {
        writeErrorLog("Invalid parameters for getMiniStatement");
        return false;
    }
    
    // Initialize count
    *count = 0;
    
    // First, find the account ID for this card
    FILE* cardFile = fopen(get_card_file_path(), "r");
    if (!cardFile) {
        writeErrorLog("Failed to open card file for reading");
        return false;
    }
    
    char line[256];
    char account_id[20] = {0};
    bool foundCard = false;
    
    // Skip header lines
    if (fgets(line, sizeof(line), cardFile) == NULL || fgets(line, sizeof(line), cardFile) == NULL) {
        fclose(cardFile);
        return false;
    }
    
    // Find the card and get its account ID
    while (fgets(line, sizeof(line), cardFile)) {
        char card_id[20], acc_id[20], card_number_str[20], card_type[20], expiry_date[20], status[20], pin_hash[100];
        if (sscanf(line, "%19s | %19s | %19s | %19s | %19s | %19s | %99s", 
                   card_id, acc_id, card_number_str, card_type, expiry_date, status, pin_hash) >= 7) {
            
            int stored_card_number = atoi(card_number_str);
            
            if (stored_card_number == cardNumber) {
                strncpy(account_id, acc_id, sizeof(account_id) - 1);
                foundCard = true;
                break;
            }
        }
    }
    
    fclose(cardFile);
    
    if (!foundCard || account_id[0] == '\0') {
        writeWarningLog("Card not found or account ID missing");
        return false;
    }
    
    // Open transactions log
    FILE* file = fopen(getTransactionsLogPath(), "r");
    if (!file) {
        writeErrorLog("Failed to open transactions log file");
        return false;
    }
    
    // We'll store transactions temporarily to sort them later
    Transaction tempTransactions[100];  // Assuming we won't find more than 100 transactions
    int tempCount = 0;
    
    // Skip header if it exists
    if (fgets(line, sizeof(line), file) != NULL && strstr(line, "Transaction ID") != NULL) {
        // This was a header line, continue
    } else {
        // Not a header, rewind
        rewind(file);
    }
    
    // Process each transaction
    while (fgets(line, sizeof(line), file) && tempCount < 100) {
        char transactionID[20], file_account_id[20], transactionType[30], amountStr[20], timestamp[30], status[20], remarks[100];
        
        if (sscanf(line, "%19s | %19s | %29s | %19s | %29s | %19s | %99[^\n]", 
                  transactionID, file_account_id, transactionType, amountStr, timestamp, status, remarks) >= 6) {
            
            // Check if this is a transaction for our account
            if (strcmp(file_account_id, account_id) == 0) {
                // Add to temporary array
                strncpy(tempTransactions[tempCount].type, transactionType, sizeof(tempTransactions[tempCount].type) - 1);
                tempTransactions[tempCount].type[sizeof(tempTransactions[tempCount].type) - 1] = '\0';
                
                tempTransactions[tempCount].amount = atof(amountStr);
                
                strncpy(tempTransactions[tempCount].status, 
                        (strcmp(status, "Success") == 0) ? "Success" : "Failed", 
                        sizeof(tempTransactions[tempCount].status) - 1);
                tempTransactions[tempCount].status[sizeof(tempTransactions[tempCount].status) - 1] = '\0';
                
                strncpy(tempTransactions[tempCount].timestamp, timestamp, sizeof(tempTransactions[tempCount].timestamp) - 1);
                tempTransactions[tempCount].timestamp[sizeof(tempTransactions[tempCount].timestamp) - 1] = '\0';
                
                tempCount++;
            }
        }
    }
    
    fclose(file);
    
    // Sort transactions by timestamp (most recent first)
    // This is a simple bubble sort - in a real implementation, use a more efficient algorithm
    for (int i = 0; i < tempCount - 1; i++) {
        for (int j = 0; j < tempCount - i - 1; j++) {
            if (strcmp(tempTransactions[j].timestamp, tempTransactions[j + 1].timestamp) < 0) {
                // Swap
                Transaction temp = tempTransactions[j];
                tempTransactions[j] = tempTransactions[j + 1];
                tempTransactions[j + 1] = temp;
            }
        }
    }
    
    // Copy the most recent transactions to the output array
    int numToCopy = (tempCount < maxTransactions) ? tempCount : maxTransactions;
    for (int i = 0; i < numToCopy; i++) {
        transactions[i] = tempTransactions[i];
    }
    
    *count = numToCopy;
    return true;
}

/**
 * @brief Create a file-based DAO
 * @return DatabaseAccessObject* The created DAO
 */
DatabaseAccessObject* createFileBasedDAO(void) {
    // Allocate memory for the DAO
    DatabaseAccessObject* dao = (DatabaseAccessObject*)malloc(sizeof(DatabaseAccessObject));
    if (!dao) {
        writeErrorLog("Failed to allocate memory for file-based DAO");
        return NULL;
    }
    
    // Initialize the function pointers
    dao->doesCardExist = file_doesCardExist;
    dao->isCardActive = file_isCardActive;
    dao->validateCard = file_validateCard;
    dao->validateCardWithHash = file_validateCardWithHash;
    dao->validateCardCVV = file_validateCardCVV;
    dao->blockCard = file_blockCard;
    dao->unblockCard = file_unblockCard;
    dao->updateCardPIN = file_updateCardPIN;
    dao->getCardHolderName = file_getCardHolderName;
    dao->getCardHolderPhone = file_getCardHolderPhone;
    dao->fetchBalance = file_fetchBalance;
    dao->updateBalance = file_updateBalance;
    dao->getDailyWithdrawals = file_getDailyWithdrawals;
    dao->logWithdrawal = file_logWithdrawal;
    dao->logTransaction = file_logTransaction;
    dao->getMiniStatement = file_getMiniStatement;
    dao->getConnection = file_getConnection;
    dao->releaseConnection = file_releaseConnection;
    
    return dao;
}
