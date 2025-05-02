#include "../../include/common/database/card_account_management.h"
#include "../../include/common/utils/logger.h"
#include "../../include/common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Forward declarations from database.c
extern bool doesCardExist(int cardNumber);
extern bool isCardActive(int cardNumber);

// Internal functions
static bool find_card_in_file(int cardNumber, FILE *file, char *line, size_t line_size, 
                             char *cardID, char *accountID, char *status);

// Block a card by updating its status to "Blocked"
bool blockCard(int cardNumber) {
    FILE *file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card file for blocking card");
        return false;
    }

    char tempPath[256];
    sprintf(tempPath, "%s.temp", getCardFilePath());
    
    FILE *tempFile = fopen(tempPath, "w");
    if (tempFile == NULL) {
        fclose(file);
        writeErrorLog("Failed to create temporary file for card blocking");
        return false;
    }

    char line[512];
    bool found = false;

    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fprintf(tempFile, "%s", line);
    }
    if (fgets(line, sizeof(line), file)) {
        fprintf(tempFile, "%s", line);
    }

    // Process each card line
    while (fgets(line, sizeof(line), file) != NULL) {
        char cardID[20], accountID[20], cardNumberStr[20], cardType[20], expiryDate[20], status[20], pinHash[128];
        
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int currentCardNumber = atoi(cardNumberStr);
            
            if (currentCardNumber == cardNumber) {
                // Update status to "Blocked"
                fprintf(tempFile, "%s | %s | %s | %s | %s | Blocked | %s\n", 
                        cardID, accountID, cardNumberStr, cardType, expiryDate, pinHash);
                found = true;
                
                char logMsg[100];
                sprintf(logMsg, "Card %d has been blocked", cardNumber);
                writeAuditLog("ADMIN", logMsg);
            } else {
                fprintf(tempFile, "%s", line);
            }
        } else {
            fprintf(tempFile, "%s", line);
        }
    }

    fclose(file);
    fclose(tempFile);

    // Replace original file with updated one
    if (!found) {
        remove(tempPath);
        writeErrorLog("Card not found for blocking");
        return false;
    }

    if (remove(getCardFilePath()) != 0 || rename(tempPath, getCardFilePath()) != 0) {
        writeErrorLog("Failed to update card file after blocking card");
        return false;
    }

    return true;
}

// Unblock a card by updating its status to "Active"
bool unblockCard(int cardNumber) {
    FILE *file = fopen(getCardFilePath(), "r");
    if (file == NULL) {
        writeErrorLog("Failed to open card file for unblocking card");
        return false;
    }

    char tempPath[256];
    sprintf(tempPath, "%s.temp", getCardFilePath());
    
    FILE *tempFile = fopen(tempPath, "w");
    if (tempFile == NULL) {
        fclose(file);
        writeErrorLog("Failed to create temporary file for card unblocking");
        return false;
    }

    char line[512];
    bool found = false;

    // Copy header lines
    if (fgets(line, sizeof(line), file)) {
        fprintf(tempFile, "%s", line);
    }
    if (fgets(line, sizeof(line), file)) {
        fprintf(tempFile, "%s", line);
    }

    // Process each card line
    while (fgets(line, sizeof(line), file) != NULL) {
        char cardID[20], accountID[20], cardNumberStr[20], cardType[20], expiryDate[20], status[20], pinHash[128];
        
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int currentCardNumber = atoi(cardNumberStr);
            
            if (currentCardNumber == cardNumber) {
                // Update status to "Active"
                fprintf(tempFile, "%s | %s | %s | %s | %s | Active  | %s\n", 
                        cardID, accountID, cardNumberStr, cardType, expiryDate, pinHash);
                found = true;
                
                char logMsg[100];
                sprintf(logMsg, "Card %d has been unblocked", cardNumber);
                writeAuditLog("ADMIN", logMsg);
            } else {
                fprintf(tempFile, "%s", line);
            }
        } else {
            fprintf(tempFile, "%s", line);
        }
    }

    fclose(file);
    fclose(tempFile);

    // Replace original file with updated one
    if (!found) {
        remove(tempPath);
        writeErrorLog("Card not found for unblocking");
        return false;
    }

    if (remove(getCardFilePath()) != 0 || rename(tempPath, getCardFilePath()) != 0) {
        writeErrorLog("Failed to update card file after unblocking card");
        return false;
    }

    return true;
}

// Helper function to search for a card in the card file
static bool find_card_in_file(int cardNumber, FILE *file, char *line, size_t line_size, 
                              char *cardID, char *accountID, char *status) {
    // Skip header lines
    fgets(line, line_size, file);
    fgets(line, line_size, file);
    
    // Process each card line
    while (fgets(line, line_size, file) != NULL) {
        char cardNumberStr[20], cardType[20], expiryDate[20], pinHash[128];
        
        if (sscanf(line, "%s | %s | %s | %s | %s | %s | %s", 
                   cardID, accountID, cardNumberStr, cardType, expiryDate, status, pinHash) >= 7) {
            int currentCardNumber = atoi(cardNumberStr);
            
            if (currentCardNumber == cardNumber) {
                // Found the card
                return true;
            }
        }
    }
    
    return false;
}