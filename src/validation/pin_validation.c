#include "pin_validation.h"
#include "../utils/hash_utils.h"
#include "../utils/logger.h"
#include "../utils/file_utils.h"
#include "../common/paths.h"
#include "../config/config_manager.h" // Added for getConfigValueInt and CONFIG constants
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PIN_ATTEMPTS 3
#define TEMP_PIN_ATTEMPTS_FILE "data/temp/pin_attempts.txt"
#define TEMP_TEST_PIN_ATTEMPTS_FILE "testing/test_pin_attempts.txt"

/**
 * Get the path to the credentials file based on test mode
 */
static const char* getCredentialsPath(int isTestMode) {
    if (isTestMode) {
        return TEST_CREDENTIALS_FILE;
    } else {
        return CREDENTIALS_FILE;
    }
}

/**
 * Get the path to the PIN attempts tracking file based on test mode
 */
static const char* getPINAttemptsPath(int isTestMode) {
    return isTestMode ? TEMP_TEST_PIN_ATTEMPTS_FILE : TEMP_PIN_ATTEMPTS_FILE;
}

/**
 * Get the stored PIN hash for a card number
 * 
 * @param cardNumber The card number to look up
 * @param isTestMode Flag indicating if test mode is active
 * @return A newly allocated string with the hash (must be freed by caller), or NULL if not found
 */
static char* getStoredPINHash(const char* cardNumber, int isTestMode) {
    FILE* file = fopen(getCredentialsPath(isTestMode), "r");
    if (!file) {
        writeErrorLog("Failed to open credentials file");
        return NULL;
    }
    
    char line[256];
    char* storedHash = NULL;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline if present
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        char storedCardNumber[20];
        char hash[65]; // SHA-256 produces 64 hex chars + null terminator
        
        // Format: cardNumber,pinHash
        if (sscanf(line, "%19[^,],%64s", storedCardNumber, hash) == 2) {
            if (strcmp(storedCardNumber, cardNumber) == 0) {
                storedHash = strdup(hash);
                break;
            }
        }
    }
    
    fclose(file);
    return storedHash;
}

int validatePIN(const char* cardNumber, const char* pinStr, int isAdmin) {
    // Get max attempts from config
    int maxAttempts = getConfigValueInt(CONFIG_MAX_WRONG_PIN_ATTEMPTS);
    if (maxAttempts < 0) maxAttempts = 3; // Default if config not found
    
    writeAuditLog("AUTH", "Validating PIN for card");
    
    if (!cardNumber || !pinStr) {
        writeErrorLog("NULL parameters passed to validatePIN");
        return 0;
    }
    
    // Get stored hash
    char* storedHash = getStoredPINHash(cardNumber, isAdmin);
    if (!storedHash) {
        writeErrorLog("No stored PIN hash found for card");
        return 0;
    }
    
    // Hash the provided PIN
    char* inputHash = hashPIN(pinStr);
    if (!inputHash) {
        writeErrorLog("Failed to hash input PIN");
        free(storedHash);
        return 0;
    }
    
    // Use secure hash comparison instead of strcmp
    int result = secure_hash_compare(inputHash, storedHash);
    
    // Reset attempts on successful validation
    if (result) {
        resetPINAttempts(cardNumber, isAdmin);
    }
    
    free(storedHash);
    free(inputHash);
    
    // When tracking attempts, use the config value
    // Changed from getCurrentPinAttempts to getRemainingPINAttempts to use the existing function
    int attempts = MAX_PIN_ATTEMPTS - getRemainingPINAttempts(cardNumber, isAdmin);
    if (attempts >= maxAttempts) {
        // Lock the card
        writeAuditLog("AUTH", "Card blocked due to too many incorrect PIN attempts");
        return 0; // Card is now blocked
    }
    
    return result; // Return the comparison result
}

int changePIN(const char* cardNumber, const char* oldPin, const char* newPin, int isTestMode) {
    writeAuditLog("PIN", "Attempting to change PIN for card");
    
    // Validate the old PIN first
    if (!validatePIN(cardNumber, oldPin, isTestMode)) {
        writeErrorLog("Old PIN validation failed during change request");
        return 0;
    }
    
    // Hash the new PIN
    char* newHash = hashPIN(newPin);
    if (!newHash) {
        writeErrorLog("Failed to hash new PIN");
        return 0;
    }
    
    // Update the credentials file
    const char* credentialsPath = getCredentialsPath(isTestMode);
    FILE* originalFile = fopen(credentialsPath, "r");
    if (!originalFile) {
        writeErrorLog("Failed to open credentials file for reading");
        free(newHash);
        return 0;
    }
    
    char tempPath[256];
    snprintf(tempPath, sizeof(tempPath), "%s.tmp", credentialsPath);
    FILE* tempFile = fopen(tempPath, "w");
    if (!tempFile) {
        writeErrorLog("Failed to create temporary credentials file");
        fclose(originalFile);
        free(newHash);
        return 0;
    }
    
    char line[256];
    int updated = 0;
    
    // Copy and update the credentials file
    while (fgets(line, sizeof(line), originalFile)) {
        char storedCardNumber[20];
        char hash[65];
        
        // Remove newline if present
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        if (sscanf(line, "%19[^,],%64s", storedCardNumber, hash) == 2 && strcmp(storedCardNumber, cardNumber) == 0) {
            // Update this card's PIN hash
            fprintf(tempFile, "%s,%s\n", cardNumber, newHash);
            updated = 1;
        } else {
            // Copy line unchanged (add newline back if removed)
            fprintf(tempFile, "%s\n", line);
        }
    }
    
    fclose(originalFile);
    fclose(tempFile);
    free(newHash);
    
    if (!updated) {
        writeErrorLog("Card number not found in credentials file");
        remove(tempPath); // Delete the temp file
        return 0;
    }
    
    // Replace the original file with the updated one
    if (remove(credentialsPath) != 0) {
        writeErrorLog("Failed to remove original credentials file");
        return 0;
    }
    
    if (rename(tempPath, credentialsPath) != 0) {
        writeErrorLog("Failed to rename temporary credentials file");
        return 0;
    }
    
    writeAuditLog("PIN", "PIN successfully changed for card");
    return 1;
}

int trackPINAttempt(const char* cardNumber, int isTestMode) {
    const char* attemptsPath = getPINAttemptsPath(isTestMode);
    FILE* file = fopen(attemptsPath, "r");
    int attempts = 0;
    
    // If file exists, look for this card's attempts
    if (file) {
        char line[256];
        char tempPath[256];
        snprintf(tempPath, sizeof(tempPath), "%s.tmp", attemptsPath);
        FILE* tempFile = fopen(tempPath, "w");
        
        if (!tempFile) {
            writeErrorLog("Failed to create temporary attempts file");
            fclose(file);
            return 1; // Continue allowing attempts
        }
        
        int found = 0;
        
        while (fgets(line, sizeof(line), file)) {
            char storedCardNumber[20];
            int storedAttempts;
            
            // Remove newline if present
            char* newline = strchr(line, '\n');
            if (newline) *newline = '\0';
            
            if (sscanf(line, "%19[^,],%d", storedCardNumber, &storedAttempts) == 2) {
                if (strcmp(storedCardNumber, cardNumber) == 0) {
                    attempts = storedAttempts + 1;
                    fprintf(tempFile, "%s,%d\n", cardNumber, attempts);
                    found = 1;
                } else {
                    fprintf(tempFile, "%s,%d\n", storedCardNumber, storedAttempts);
                }
            }
        }
        
        if (!found) {
            attempts = 1;
            fprintf(tempFile, "%s,%d\n", cardNumber, attempts);
        }
        
        fclose(file);
        fclose(tempFile);
        
        // Replace original file with updated one
        remove(attemptsPath);
        rename(tempPath, attemptsPath);
    } else {
        // Create new attempts file and add this card
        file = fopen(attemptsPath, "w");
        if (!file) {
            writeErrorLog("Failed to create PIN attempts file");
            return 1; // Continue allowing attempts
        }
        
        attempts = 1;
        fprintf(file, "%s,%d\n", cardNumber, attempts);
        fclose(file);
    }
    
    if (attempts >= MAX_PIN_ATTEMPTS) {
        writeAuditLog("AUTH", "Card blocked due to too many incorrect PIN attempts");
        return 0; // Card is now blocked
    }
    
    return 1; // Attempts still allowed
}

void resetPINAttempts(const char* cardNumber, int isTestMode) {
    const char* attemptsPath = getPINAttemptsPath(isTestMode);
    FILE* file = fopen(attemptsPath, "r");
    
    if (!file) {
        return; // No attempts file, nothing to reset
    }
    
    char tempPath[256];
    snprintf(tempPath, sizeof(tempPath), "%s.tmp", attemptsPath);
    FILE* tempFile = fopen(tempPath, "w");
    
    if (!tempFile) {
        writeErrorLog("Failed to create temporary attempts file during reset");
        fclose(file);
        return;
    }
    
    char line[256];
    
    while (fgets(line, sizeof(line), file)) {
        char storedCardNumber[20];
        int storedAttempts;
        
        // Remove newline if present
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        if (sscanf(line, "%19[^,],%d", storedCardNumber, &storedAttempts) == 2) {
            if (strcmp(storedCardNumber, cardNumber) != 0) {
                // Only keep non-matching cards
                fprintf(tempFile, "%s,%d\n", storedCardNumber, storedAttempts);
            }
        }
    }
    
    fclose(file);
    fclose(tempFile);
    
    // Replace original file with updated one
    remove(attemptsPath);
    rename(tempPath, attemptsPath);
}

/**
 * Check if a card is locked out due to too many failed PIN attempts
 * 
 * @param cardNumber The card number to check
 * @param isTestMode Flag indicating if test mode is active
 * @return 1 if card is locked out, 0 otherwise
 */
int isCardLockedOut(const char* cardNumber, int isTestMode) {
    const char* attemptsPath = getPINAttemptsPath(isTestMode);
    FILE* file = fopen(attemptsPath, "r");
    
    if (!file) {
        return 0; // No attempts file, card is not locked
    }
    
    char line[256];
    int isLocked = 0;
    
    while (fgets(line, sizeof(line), file)) {
        char storedCardNumber[20];
        int storedAttempts;
        
        // Remove newline if present
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        if (sscanf(line, "%19[^,],%d", storedCardNumber, &storedAttempts) == 2) {
            if (strcmp(storedCardNumber, cardNumber) == 0) {
                isLocked = (storedAttempts >= MAX_PIN_ATTEMPTS);
                break;
            }
        }
    }
    
    fclose(file);
    return isLocked;
}

/**
 * Get the number of remaining PIN attempts before lockout
 * 
 * @param cardNumber The card number to check
 * @param isTestMode Flag indicating if test mode is active
 * @return Number of remaining attempts
 */
int getRemainingPINAttempts(const char* cardNumber, int isTestMode) {
    const char* attemptsPath = getPINAttemptsPath(isTestMode);
    FILE* file = fopen(attemptsPath, "r");
    
    if (!file) {
        return MAX_PIN_ATTEMPTS; // No attempts file, all attempts remaining
    }
    
    char line[256];
    int attempts = 0;
    
    while (fgets(line, sizeof(line), file)) {
        char storedCardNumber[20];
        int storedAttempts;
        
        // Remove newline if present
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        if (sscanf(line, "%19[^,],%d", storedCardNumber, &storedAttempts) == 2) {
            if (strcmp(storedCardNumber, cardNumber) == 0) {
                attempts = storedAttempts;
                break;
            }
        }
    }
    
    fclose(file);
    return MAX_PIN_ATTEMPTS - attempts;
}

char* hashPIN(const char* pin) {
    return sha256_hash(pin);
}

// Let's add an overload function to handle both use cases
char* hashPINWithCard(const char* pin, const char* cardNumber) {
    // In this implementation, we're ignoring the cardNumber and just hashing the PIN
    // If you want to incorporate the card number into the hash, you would modify this
    return sha256_hash(pin);
}

// Record a failed PIN attempt - helper function used in menu.c
int recordFailedPINAttempt(int cardNumber) {
    char cardNumberStr[20];
    sprintf(cardNumberStr, "%d", cardNumber);
    return trackPINAttempt(cardNumberStr, 0); // 0 for production mode
}

// Check if PIN format is valid (e.g., 4-6 digits)
int isValidPINFormat(int pin) {
    // PIN should be 4-6 digits (between 1000 and 999999)
    return (pin >= 1000 && pin <= 999999);
}