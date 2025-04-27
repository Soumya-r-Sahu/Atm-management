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