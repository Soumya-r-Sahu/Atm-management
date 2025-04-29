#include "card_account_management.h"
#include "utils/logger.h"
#include "utils/hash_utils.h"
#include "common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Helper function to generate a unique ID
char* generateUniqueID(const char* prefix, int* counter) {
    char* id = malloc(10);
    if (id == NULL) {
        return NULL;
    }
    
    sprintf(id, "%s%d", prefix, (*counter)++);
    return id;
}

// Helper function to generate expiry date (2 years from now)
char* generateExpiryDate() {
    char* expiryDate = malloc(11);
    if (expiryDate == NULL) {
        return NULL;
    }
    
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    tm_now->tm_year += 2;
    
    strftime(expiryDate, 11, "%Y-%m-%d", tm_now);
    return expiryDate;
}

// Create new account with given details
int createAccount(const char* accountHolderName, int cardNumber, int pin) {
    // Generate PIN hash
    char pinStr[20];
    sprintf(pinStr, "%d", pin);
    char* pinHash = sha256_hash(pinStr);
    if (pinHash == NULL) {
        writeErrorLog("Failed to generate PIN hash while creating account");
        return 0;
    }
    
    // Generate unique IDs
    static int customerCounter = 10001;
    static int accountCounter = 10001;
    static int cardCounter = 10001;
    
    char* customerID = generateUniqueID("C", &customerCounter);
    char* accountID = generateUniqueID("A", &accountCounter);
    char* cardID = generateUniqueID("D", &cardCounter);
    char* expiryDate = generateExpiryDate();
    
    if (!customerID || !accountID || !cardID || !expiryDate) {
        writeErrorLog("Failed to allocate memory for IDs while creating account");
        free(pinHash);
        free(customerID);
        free(accountID);
        free(cardID);
        free(expiryDate);
        return 0;
    }
    
    // Update customer.txt
    FILE* customerFile = fopen(getCustomerFilePath(), "a");
    if (customerFile == NULL) {
        writeErrorLog("Failed to open customer.txt while creating new account");
        free(pinHash);
        free(customerID);
        free(accountID);
        free(cardID);
        free(expiryDate);
        return 0;
    }
    
    fprintf(customerFile, "%s | %s | %-20s | Regular | Active | 0.00\n", 
            customerID, accountID, accountHolderName);
    fclose(customerFile);
    
    // Update card.txt
    FILE* cardFile = fopen(getCardFilePath(), "a");
    if (cardFile == NULL) {
        writeErrorLog("Failed to open card.txt while creating new account");
        free(pinHash);
        free(customerID);
        free(accountID);
        free(cardID);
        free(expiryDate);
        return 0;
    }
    
    fprintf(cardFile, "%s | %s | %-16d | Debit     | %s | Active  | %s\n", 
            cardID, accountID, cardNumber, expiryDate, pinHash);
    fclose(cardFile);
    
    // Clean up allocated memory
    free(pinHash);
    free(customerID);
    free(accountID);
    free(cardID);
    free(expiryDate);
    
    // Log the account creation
    char logMsg[100];
    sprintf(logMsg, "New account created for %s with card number %d", 
            accountHolderName, cardNumber);
    writeAuditLog("ADMIN", logMsg);
    
    return 1;
}

// Block a card by setting its status to "Blocked"
int blockCard(int cardNumber) {
    const char* cardFilePath = getCardFilePath();
    FILE* file = fopen(cardFilePath, "r");
    if (!file) {
        writeErrorLog("Failed to open card file for blocking card");
        return 0;
    }
    
    // Create a temporary file for writing updated data
    char tempFilePath[256];
    snprintf(tempFilePath, sizeof(tempFilePath), "%s.tmp", cardFilePath);
    FILE* tempFile = fopen(tempFilePath, "w");
    if (!tempFile) {
        fclose(file);
        writeErrorLog("Failed to create temporary file for blocking card");
        return 0;
    }
    
    char line[512];
    int found = 0;
    
    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fputs(line, tempFile);
    }
    if (fgets(line, sizeof(line), file)) {
        fputs(line, tempFile);
    }
    
    // Process each entry
    while (fgets(line, sizeof(line), file)) {
        char cardId[20], accountId[20], cardNumberStr[20], cardType[20], expiryDate[20], status[20], pinHash[65];
        
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                  cardId, accountId, cardNumberStr, cardType, expiryDate, status, pinHash) >= 6) {
            int storedCardNumber = atoi(cardNumberStr);
            
            if (storedCardNumber == cardNumber) {
                // Update status to Blocked
                fprintf(tempFile, "%s | %s | %s | %s | %s | Blocked  | %s\n", 
                       cardId, accountId, cardNumberStr, cardType, expiryDate, pinHash);
                found = 1;
                
                // Log the action
                char logMsg[100];
                sprintf(logMsg, "Card %d blocked", cardNumber);
                writeAuditLog("SECURITY", logMsg);
            } else {
                // Copy line unchanged
                fputs(line, tempFile);
            }
        } else {
            // Copy line unchanged for any line that doesn't match the format
            fputs(line, tempFile);
        }
    }
    
    fclose(file);
    fclose(tempFile);
    
    // Replace the original file with the updated one
    if (found) {
        remove(cardFilePath);
        rename(tempFilePath, cardFilePath);
        return 1;
    } else {
        remove(tempFilePath);
        writeErrorLog("Card number not found for blocking");
        return 0;
    }
}

// Unblock a card by setting its status to "Active"
int unblockCard(int cardNumber) {
    const char* cardFilePath = getCardFilePath();
    FILE* file = fopen(cardFilePath, "r");
    if (!file) {
        writeErrorLog("Failed to open card file for unblocking card");
        return 0;
    }
    
    // Create a temporary file for writing updated data
    char tempFilePath[256];
    snprintf(tempFilePath, sizeof(tempFilePath), "%s.tmp", cardFilePath);
    FILE* tempFile = fopen(tempFilePath, "w");
    if (!tempFile) {
        fclose(file);
        writeErrorLog("Failed to create temporary file for unblocking card");
        return 0;
    }
    
    char line[512];
    int found = 0;
    
    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fputs(line, tempFile);
    }
    if (fgets(line, sizeof(line), file)) {
        fputs(line, tempFile);
    }
    
    // Process each entry
    while (fgets(line, sizeof(line), file)) {
        char cardId[20], accountId[20], cardNumberStr[20], cardType[20], expiryDate[20], status[20], pinHash[65];
        
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                  cardId, accountId, cardNumberStr, cardType, expiryDate, status, pinHash) >= 6) {
            int storedCardNumber = atoi(cardNumberStr);
            
            if (storedCardNumber == cardNumber) {
                // Update status to Active
                fprintf(tempFile, "%s | %s | %s | %s | %s | Active  | %s\n", 
                       cardId, accountId, cardNumberStr, cardType, expiryDate, pinHash);
                found = 1;
                
                // Log the action
                char logMsg[100];
                sprintf(logMsg, "Card %d unblocked", cardNumber);
                writeAuditLog("SECURITY", logMsg);
            } else {
                // Copy line unchanged
                fputs(line, tempFile);
            }
        } else {
            // Copy line unchanged for any line that doesn't match the format
            fputs(line, tempFile);
        }
    }
    
    fclose(file);
    fclose(tempFile);
    
    // Replace the original file with the updated one
    if (found) {
        remove(cardFilePath);
        rename(tempFilePath, cardFilePath);
        return 1;
    } else {
        remove(tempFilePath);
        writeErrorLog("Card number not found for unblocking");
        return 0;
    }
}

// Get total daily withdrawals for a card
float getDailyWithdrawals(int cardNumber) {
    const char* withdrawalLogPath = isTestingMode() ? 
        TEST_WITHDRAWALS_LOG_FILE : PROD_WITHDRAWALS_LOG_FILE;
    
    FILE* file = fopen(withdrawalLogPath, "r");
    if (!file) {
        // If file doesn't exist, no withdrawals were made
        return 0.0f;
    }
    
    char line[256];
    float totalWithdrawals = 0.0f;
    
    // Get today's date in YYYY-MM-DD format
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char todayDate[11];
    strftime(todayDate, sizeof(todayDate), "%Y-%m-%d", tm_now);
    
    // Format: cardNumber,date,amount,timestamp
    while (fgets(line, sizeof(line), file)) {
        int storedCardNumber;
        char date[11];
        float amount;
        
        if (sscanf(line, "%d,%10[^,],%f", &storedCardNumber, date, &amount) >= 3) {
            if (storedCardNumber == cardNumber && strcmp(date, todayDate) == 0) {
                totalWithdrawals += amount;
            }
        }
    }
    
    fclose(file);
    return totalWithdrawals;
}

// Check if a card exists
int doesCardExist(int cardNumber) {
    const char* cardFilePath = getCardFilePath();
    FILE* file = fopen(cardFilePath, "r");
    if (!file) {
        writeErrorLog("Failed to open card file to check if card exists");
        return 0;
    }
    
    char line[512];
    int found = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Process each entry
    while (fgets(line, sizeof(line), file) && !found) {
        char cardNumberStr[20];
        
        // Extract the card number field (3rd field)
        if (sscanf(line, "%*s | %*s | %s", cardNumberStr) == 1) {
            int storedCardNumber = atoi(cardNumberStr);
            if (storedCardNumber == cardNumber) {
                found = 1;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

// Check if a card is active
int isCardActive(int cardNumber) {
    const char* cardFilePath = getCardFilePath();
    FILE* file = fopen(cardFilePath, "r");
    if (!file) {
        writeErrorLog("Failed to open card file to check card status");
        return 0;
    }
    
    char line[512];
    int active = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    // Process each entry
    while (fgets(line, sizeof(line), file) && !active) {
        char cardId[20], accountId[20], cardNumberStr[20], cardType[20], expiryDate[20], status[20], pinHash[65];
        
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                  cardId, accountId, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int storedCardNumber = atoi(cardNumberStr);
            
            if (storedCardNumber == cardNumber) {
                // Check if status is "Active"
                active = (strcmp(status, "Active") == 0 || strcmp(status, "Active  ") == 0);
                break;
            }
        }
    }
    
    fclose(file);
    return active;
}

// Get card holder name
void getCardHolderName(int cardNumber, char* name, size_t nameSize) {
    if (!name || nameSize <= 0) {
        writeErrorLog("Invalid parameters passed to getCardHolderName");
        return;
    }
    
    // Default value if not found
    strncpy(name, "Customer", nameSize - 1);
    name[nameSize - 1] = '\0';
    
    // First find the account ID for this card
    const char* cardFilePath = getCardFilePath();
    FILE* cardFile = fopen(cardFilePath, "r");
    if (!cardFile) {
        writeErrorLog("Failed to open card file to get account ID");
        return;
    }
    
    char line[512];
    char accountId[20] = {0};
    
    // Skip header lines
    fgets(line, sizeof(line), cardFile);
    fgets(line, sizeof(line), cardFile);
    
    // Find card and get account ID
    while (fgets(line, sizeof(line), cardFile)) {
        char cardId[20], accId[20], cardNumberStr[20], rest[256];
        
        if (sscanf(line, "%s | %s | %s | %s", cardId, accId, cardNumberStr, rest) >= 3) {
            int storedCardNumber = atoi(cardNumberStr);
            
            if (storedCardNumber == cardNumber) {
                strncpy(accountId, accId, sizeof(accountId) - 1);
                break;
            }
        }
    }
    
    fclose(cardFile);
    
    if (accountId[0] == '\0') {
        writeErrorLog("Failed to find account ID for card");
        return;
    }
    
    // Now look up the customer name using the account ID
    const char* customerFilePath = getCustomerFilePath();
    FILE* customerFile = fopen(customerFilePath, "r");
    if (!customerFile) {
        writeErrorLog("Failed to open customer file to get name");
        return;
    }
    
    // Skip header lines
    fgets(line, sizeof(line), customerFile);
    fgets(line, sizeof(line), customerFile);
    
    // Find account and get customer name
    while (fgets(line, sizeof(line), customerFile)) {
        char custId[20], accId[20], custName[100], rest[256];
        
        if (sscanf(line, "%s | %s | %s | %s", custId, accId, custName, rest) >= 3) {
            if (strcmp(accountId, accId) == 0) {
                strncpy(name, custName, nameSize - 1);
                name[nameSize - 1] = '\0';
                break;
            }
        }
    }
    
    fclose(customerFile);
}

// Get card holder phone number
void getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize) {
    if (!phone || phoneSize <= 0) {
        writeErrorLog("Invalid parameters passed to getCardHolderPhone");
        return;
    }
    
    // Default value if not found
    strncpy(phone, "0000000000", phoneSize - 1);
    phone[phoneSize - 1] = '\0';
    
    // First find the account ID for this card
    const char* cardFilePath = getCardFilePath();
    FILE* cardFile = fopen(cardFilePath, "r");
    if (!cardFile) {
        writeErrorLog("Failed to open card file to get account ID");
        return;
    }
    
    char line[512];
    char accountId[20] = {0};
    
    // Skip header lines
    fgets(line, sizeof(line), cardFile);
    fgets(line, sizeof(line), cardFile);
    
    // Find card and get account ID
    while (fgets(line, sizeof(line), cardFile)) {
        char cardId[20], accId[20], cardNumberStr[20], rest[256];
        
        if (sscanf(line, "%s | %s | %s | %s", cardId, accId, cardNumberStr, rest) >= 3) {
            int storedCardNumber = atoi(cardNumberStr);
            
            if (storedCardNumber == cardNumber) {
                strncpy(accountId, accId, sizeof(accountId) - 1);
                break;
            }
        }
    }
    
    fclose(cardFile);
    
    if (accountId[0] == '\0') {
        writeErrorLog("Failed to find account ID for card");
        return;
    }
    
    // Now look up the customer phone using the account ID
    const char* customerFilePath = getCustomerFilePath();
    FILE* customerFile = fopen(customerFilePath, "r");
    if (!customerFile) {
        writeErrorLog("Failed to open customer file to get phone");
        return;
    }
    
    // Skip header lines
    fgets(line, sizeof(line), customerFile);
    fgets(line, sizeof(line), customerFile);
    
    // Find account and get customer phone
    while (fgets(line, sizeof(line), customerFile)) {
        char custId[20], accId[20], custName[100], custPhone[20], rest[256];
        
        // Format can vary, so try different approaches
        // Phone is expected to be in the 6th field
        char* token = strtok(line, "|");
        int field = 0;
        
        while (token != NULL && field < 6) {
            if (field == 1 && strcmp(accountId, token) == 0) {
                // Account ID matched
                // Continue to find phone field
            }
            
            if (field == 5) {
                // This should be the phone field
                // Trim spaces
                while (*token == ' ') token++;
                char* end = token + strlen(token) - 1;
                while (end > token && (*end == ' ' || *end == '\n')) *end-- = '\0';
                
                strncpy(phone, token, phoneSize - 1);
                phone[phoneSize - 1] = '\0';
                break;
            }
            
            token = strtok(NULL, "|");
            field++;
        }
    }
    
    fclose(customerFile);
}

// Update balance for a card
int updateBalance(int cardNumber, float newBalance) {
    // Need to find the account ID for this card first
    const char* cardFilePath = getCardFilePath();
    FILE* cardFile = fopen(cardFilePath, "r");
    if (!cardFile) {
        writeErrorLog("Failed to open card file to get account ID for balance update");
        return 0;
    }
    
    char line[512];
    char accountId[20] = {0};
    
    // Skip header lines
    fgets(line, sizeof(line), cardFile);
    fgets(line, sizeof(line), cardFile);
    
    // Find card and get account ID
    while (fgets(line, sizeof(line), cardFile)) {
        char cardId[20], accId[20], cardNumberStr[20], rest[256];
        
        if (sscanf(line, "%s | %s | %s | %s", cardId, accId, cardNumberStr, rest) >= 3) {
            int storedCardNumber = atoi(cardNumberStr);
            
            if (storedCardNumber == cardNumber) {
                strncpy(accountId, accId, sizeof(accountId) - 1);
                break;
            }
        }
    }
    
    fclose(cardFile);
    
    if (accountId[0] == '\0') {
        writeErrorLog("Failed to find account ID for card in balance update");
        return 0;
    }
    
    // Now update the balance for this account in the accounting file
    const char* accountingFilePath = isTestingMode() ? 
        TEST_ACCOUNTING_FILE : PROD_ACCOUNTING_FILE;
    FILE* accountingFile = fopen(accountingFilePath, "r");
    if (!accountingFile) {
        writeErrorLog("Failed to open accounting file for balance update");
        return 0;
    }
    
    // Create a temporary file for the update
    char tempFilePath[256];
    snprintf(tempFilePath, sizeof(tempFilePath), "%s.tmp", accountingFilePath);
    FILE* tempFile = fopen(tempFilePath, "w");
    if (!tempFile) {
        fclose(accountingFile);
        writeErrorLog("Failed to create temporary file for balance update");
        return 0;
    }
    
    // Copy header lines
    if (fgets(line, sizeof(line), accountingFile)) {
        fputs(line, tempFile);
    }
    if (fgets(line, sizeof(line), accountingFile)) {
        fputs(line, tempFile);
    }
    
    int found = 0;
    
    // Process each account entry
    while (fgets(line, sizeof(line), accountingFile)) {
        char accId[20], custId[20], balanceStr[30], rest[256];
        
        // Try to extract the account ID and balance
        char* accPtr = strstr(line, accountId);
        if (accPtr && (accPtr == line || *(accPtr-1) == ' ' || *(accPtr-1) == '|')) {
            // This line contains our account ID
            
            // Use a simple approach to update the balance field (3rd field)
            char newLine[512];
            char* parts[10] = {0};
            int partCount = 0;
            
            // Split the line by '|'
            char* token = strtok(strdup(line), "|");
            while (token != NULL && partCount < 10) {
                parts[partCount++] = token;
                token = strtok(NULL, "|");
            }
            
            if (partCount >= 3) {
                // The 3rd part is the balance field
                sprintf(newLine, "%s | %s | %.2f", parts[0], parts[1], newBalance);
                
                // Append the rest of the fields
                for (int i = 3; i < partCount; i++) {
                    strcat(newLine, " | ");
                    strcat(newLine, parts[i]);
                }
                strcat(newLine, "\n");
                
                fputs(newLine, tempFile);
                found = 1;
            } else {
                // Unexpected format, copy as-is
                fputs(line, tempFile);
            }
            
            free(parts[0]); // Free the duplicated string from strdup
        } else {
            // Not our account, copy line unchanged
            fputs(line, tempFile);
        }
    }
    
    fclose(accountingFile);
    fclose(tempFile);
    
    // Replace original file with updated one
    if (found) {
        remove(accountingFilePath);
        rename(tempFilePath, accountingFilePath);
        
        // Log the update
        char logMsg[100];
        sprintf(logMsg, "Updated balance for card %d account %s to $%.2f", 
                cardNumber, accountId, newBalance);
        writeAuditLog("ACCOUNT", logMsg);
        
        return 1;
    } else {
        remove(tempFilePath);
        writeErrorLog("Failed to find account in accounting file for balance update");
        return 0;
    }
}

// Check if a recipient account is valid
int validateRecipientAccount(int cardNumber, const char* accountID, const char* branchCode) {
    // This is a simplified validation that just checks if the card exists
    // and the account is active. In a real system, this would validate 
    // the account ID and branch code match with the card.
    return doesCardExist(cardNumber) && isCardActive(cardNumber);
}