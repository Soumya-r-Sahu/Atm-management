#include "card_security.h"
#include "../common/error_handler.h"
#include "../utils/logger.h"
#include "../common/paths.h"
#include "../config/config_manager.h"
#include "../utils/memory_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Card lockout entry structure
typedef struct {
    char cardNumber[20];     // Card number
    int attempts;            // Number of failed attempts
    time_t lockTime;         // When the card was locked (0 if not locked)
    time_t unlockTime;       // When the card will be automatically unlocked (0 if permanent)
    char reason[100];        // Reason for lockout
} CardLockoutEntry;

// In-memory cache of locked cards for fast access
static CardLockoutEntry* lockoutCache = NULL;
static int lockoutCacheSize = 0;
static int lockoutCacheCapacity = 0;

// Get the path to the card lockout file
static const char* getLockoutFilePath(int isTestMode) {
    return isTestMode ? 
        "testing/test_card_lockouts.txt" : 
        "data/card_lockouts.txt";
}

// Load the card lockout file into cache
static void loadLockoutCache(int isTestMode) {
    const char* filePath = getLockoutFilePath(isTestMode);
    FILE* file = fopen(filePath, "r");
    
    if (!file) {
        // File doesn't exist yet, not an error
        return;
    }
    
    // Count the number of entries first
    int entryCount = 0;
    char line[256];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        entryCount++;
    }
    
    // Allocate memory for the cache
    lockoutCache = (CardLockoutEntry*)MALLOC(entryCount * sizeof(CardLockoutEntry), "Card lockout cache");
    if (!lockoutCache) {
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for card lockout cache");
        fclose(file);
        return;
    }
    
    lockoutCacheCapacity = entryCount;
    
    // Reset to beginning of file
    rewind(file);
    
    // Read entries
    while (fgets(line, sizeof(line), file) != NULL && lockoutCacheSize < lockoutCacheCapacity) {
        CardLockoutEntry* entry = &lockoutCache[lockoutCacheSize];
        
        // Parse the line (format: cardNumber,attempts,lockTime,unlockTime,reason)
        if (sscanf(line, "%19[^,],%d,%ld,%ld,%99[^\n]", 
                   entry->cardNumber, &entry->attempts, &entry->lockTime, 
                   &entry->unlockTime, entry->reason) >= 4) {
            lockoutCacheSize++;
        }
    }
    
    fclose(file);
    
    char logMsg[100];
    sprintf(logMsg, "Loaded %d card lockout entries", lockoutCacheSize);
    writeInfoLog(logMsg);
}

// Save the card lockout cache to file
static void saveLockoutCache(int isTestMode) {
    const char* filePath = getLockoutFilePath(isTestMode);
    FILE* file = fopen(filePath, "w");
    
    if (!file) {
        SET_ERROR(ERR_FILE_ACCESS, "Failed to open card lockout file for writing");
        return;
    }
    
    // Write each entry
    for (int i = 0; i < lockoutCacheSize; i++) {
        CardLockoutEntry* entry = &lockoutCache[i];
        fprintf(file, "%s,%d,%ld,%ld,%s\n", 
                entry->cardNumber, entry->attempts, 
                entry->lockTime, entry->unlockTime, entry->reason);
    }
    
    fclose(file);
    
    char logMsg[100];
    sprintf(logMsg, "Saved %d card lockout entries", lockoutCacheSize);
    writeInfoLog(logMsg);
}

// Find a card in the lockout cache
static int findCardInCache(const char* cardNumber) {
    for (int i = 0; i < lockoutCacheSize; i++) {
        if (strcmp(lockoutCache[i].cardNumber, cardNumber) == 0) {
            return i; // Found
        }
    }
    return -1; // Not found
}

// Add a card to the lockout cache
static int addCardToCache(const char* cardNumber, int attempts, time_t lockTime, time_t unlockTime, const char* reason) {
    // Check if cache needs to be expanded
    if (lockoutCacheSize >= lockoutCacheCapacity) {
        int newCapacity = lockoutCacheCapacity == 0 ? 10 : lockoutCacheCapacity * 2;
        CardLockoutEntry* newCache = (CardLockoutEntry*)REALLOC(
            lockoutCache, 
            newCapacity * sizeof(CardLockoutEntry), 
            "Card lockout cache expansion");
        
        if (!newCache) {
            SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to expand card lockout cache");
            return -1;
        }
        
        lockoutCache = newCache;
        lockoutCacheCapacity = newCapacity;
    }
    
    // Add the new entry
    CardLockoutEntry* entry = &lockoutCache[lockoutCacheSize];
    strncpy(entry->cardNumber, cardNumber, sizeof(entry->cardNumber) - 1);
    entry->cardNumber[sizeof(entry->cardNumber) - 1] = '\0';
    
    entry->attempts = attempts;
    entry->lockTime = lockTime;
    entry->unlockTime = unlockTime;
    
    if (reason) {
        strncpy(entry->reason, reason, sizeof(entry->reason) - 1);
        entry->reason[sizeof(entry->reason) - 1] = '\0';
    } else {
        entry->reason[0] = '\0';
    }
    
    lockoutCacheSize++;
    
    return lockoutCacheSize - 1; // Return index of added entry
}

// Remove a card from the lockout cache
static void removeCardFromCache(int index) {
    if (index < 0 || index >= lockoutCacheSize) {
        return;
    }
    
    // Move the last entry to this position (unless it's the last entry)
    if (index < lockoutCacheSize - 1) {
        lockoutCache[index] = lockoutCache[lockoutCacheSize - 1];
    }
    
    lockoutCacheSize--;
}

// Initialize the card security service
int card_security_init(void) {
    // Load the lockout cache for both production and test modes
    loadLockoutCache(0); // Production
    
    // Clean up expired locks
    card_security_cleanup_expired_locks();
    
    return 1;
}

// Record a failed PIN attempt and possibly lock the card
int card_security_record_failed_attempt(const char* cardNumber, int isTestMode) {
    // Get max attempts from config
    int maxAttempts = getConfigValueInt(CONFIG_MAX_WRONG_PIN_ATTEMPTS);
    if (maxAttempts <= 0) maxAttempts = 3; // Default
    
    // Get lockout duration from config (in minutes)
    int lockoutMins = getConfigValueInt(CONFIG_PIN_LOCKOUT_MINUTES);
    if (lockoutMins <= 0) lockoutMins = 30; // Default
    
    // Find the card in the cache
    int index = findCardInCache(cardNumber);
    int attempts = 1;
    
    if (index >= 0) {
        // Card exists in cache, update attempts
        attempts = ++lockoutCache[index].attempts;
        
        // Check if card should be locked
        if (attempts >= maxAttempts) {
            time_t now = time(NULL);
            lockoutCache[index].lockTime = now;
            lockoutCache[index].unlockTime = now + (lockoutMins * 60);
            
            if (!lockoutCache[index].reason[0]) {
                strcpy(lockoutCache[index].reason, "Too many failed PIN attempts");
            }
            
            char logMsg[256];
            sprintf(logMsg, "Card %s locked for %d minutes due to %d failed PIN attempts", 
                    cardNumber, lockoutMins, attempts);
            writeAuditLog("SECURITY", logMsg);
        }
    } else {
        // Card doesn't exist in cache, add it
        index = addCardToCache(cardNumber, 1, 0, 0, NULL);
        
        char logMsg[256];
        sprintf(logMsg, "First failed PIN attempt for card %s recorded", cardNumber);
        writeInfoLog(logMsg);
    }
    
    // Save the updated cache
    saveLockoutCache(isTestMode);
    
    // Return remaining attempts
    return maxAttempts - attempts;
}

// Check if a card is currently locked out
int card_security_is_card_locked(const char* cardNumber, int isTestMode) {
    int index = findCardInCache(cardNumber);
    
    if (index >= 0) {
        // Card exists in cache
        CardLockoutEntry* entry = &lockoutCache[index];
        
        if (entry->lockTime > 0) {
            // Card is locked, check if lock has expired
            time_t now = time(NULL);
            
            if (entry->unlockTime > 0 && now >= entry->unlockTime) {
                // Lock has expired, remove it
                removeCardFromCache(index);
                saveLockoutCache(isTestMode);
                return 0; // Not locked
            }
            
            return 1; // Still locked
        }
    }
    
    return 0; // Not locked
}

// Reset PIN attempts for a card (after successful auth)
int card_security_reset_attempts(const char* cardNumber, int isTestMode) {
    int index = findCardInCache(cardNumber);
    
    if (index >= 0) {
        // Remove the entry (reset attempts to zero)
        removeCardFromCache(index);
        saveLockoutCache(isTestMode);
        
        char logMsg[256];
        sprintf(logMsg, "PIN attempts reset for card %s after successful authentication", cardNumber);
        writeInfoLog(logMsg);
    }
    
    return 1;
}

// Get remaining PIN attempts before lockout
int card_security_get_remaining_attempts(const char* cardNumber, int isTestMode) {
    // Get max attempts from config
    int maxAttempts = getConfigValueInt(CONFIG_MAX_WRONG_PIN_ATTEMPTS);
    if (maxAttempts <= 0) maxAttempts = 3; // Default
    
    int index = findCardInCache(cardNumber);
    
    if (index >= 0) {
        // Card exists in cache, check if it's locked
        CardLockoutEntry* entry = &lockoutCache[index];
        
        if (entry->lockTime > 0) {
            // Card is locked, check if lock has expired
            time_t now = time(NULL);
            
            if (entry->unlockTime > 0 && now >= entry->unlockTime) {
                // Lock has expired, remove it
                removeCardFromCache(index);
                saveLockoutCache(isTestMode);
                return maxAttempts; // All attempts available
            }
            
            return 0; // No attempts remaining while locked
        }
        
        return maxAttempts - entry->attempts;
    }
    
    return maxAttempts; // Card not in cache, all attempts available
}

// Lock a card manually (administrative action)
int card_security_lock_card(const char* cardNumber, const char* reason, int isTestMode) {
    // Find the card in the cache
    int index = findCardInCache(cardNumber);
    time_t now = time(NULL);
    
    if (index >= 0) {
        // Card exists in cache, update it
        CardLockoutEntry* entry = &lockoutCache[index];
        entry->lockTime = now;
        entry->unlockTime = 0; // 0 means permanently locked
        
        if (reason) {
            strncpy(entry->reason, reason, sizeof(entry->reason) - 1);
            entry->reason[sizeof(entry->reason) - 1] = '\0';
        } else {
            strcpy(entry->reason, "Administrative lock");
        }
    } else {
        // Card doesn't exist in cache, add it
        index = addCardToCache(cardNumber, 0, now, 0, reason ? reason : "Administrative lock");
        if (index < 0) {
            return 0; // Failed to add to cache
        }
    }
    
    // Save the updated cache
    saveLockoutCache(isTestMode);
    
    char logMsg[256];
    sprintf(logMsg, "Card %s manually locked by admin: %s", cardNumber, reason ? reason : "No reason provided");
    writeAuditLog("SECURITY", logMsg);
    
    return 1;
}

// Unlock a card manually (administrative action)
int card_security_unlock_card(const char* cardNumber, const char* adminId, const char* reason, int isTestMode) {
    int index = findCardInCache(cardNumber);
    
    if (index >= 0) {
        // Log before removing
        char logMsg[256];
        sprintf(logMsg, "Card %s manually unlocked by admin %s: %s", 
                cardNumber, adminId ? adminId : "unknown", reason ? reason : "No reason provided");
        writeAuditLog("SECURITY", logMsg);
        
        // Remove the entry (fully unlock the card)
        removeCardFromCache(index);
        saveLockoutCache(isTestMode);
        return 1;
    }
    
    // Card not in cache, nothing to unlock
    return 1;
}

// Get the time when a card will be automatically unlocked
long card_security_get_unlock_time(const char* cardNumber, int isTestMode) {
    int index = findCardInCache(cardNumber);
    
    if (index >= 0) {
        return lockoutCache[index].unlockTime;
    }
    
    return 0; // Not locked
}

// Clean up expired card lockouts
int card_security_cleanup_expired_locks(void) {
    int count = 0;
    time_t now = time(NULL);
    
    // Check each entry
    for (int i = lockoutCacheSize - 1; i >= 0; i--) {
        CardLockoutEntry* entry = &lockoutCache[i];
        
        if (entry->lockTime > 0 && entry->unlockTime > 0 && now >= entry->unlockTime) {
            // Lock has expired, remove it
            char logMsg[256];
            sprintf(logMsg, "Card %s automatic unlock after timeout", entry->cardNumber);
            writeInfoLog(logMsg);
            
            removeCardFromCache(i);
            count++;
        }
    }
    
    if (count > 0) {
        // Save if any changes were made
        saveLockoutCache(0); // Production
        saveLockoutCache(1); // Test
        
        char logMsg[100];
        sprintf(logMsg, "Cleaned up %d expired card lockouts", count);
        writeInfoLog(logMsg);
    }
    
    return count;
}