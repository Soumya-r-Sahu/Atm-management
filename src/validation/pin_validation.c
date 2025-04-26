#include "pin_validation.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PIN_ATTEMPTS_FILE "../data/pin_attempts.txt"

typedef struct {
    int cardNumber;
    int attempts;
    time_t lockoutTime;
} PinAttemptRecord;

// Helper function to read the PIN attempts from the file
static PinAttemptRecord* readPinAttempts(int *count) {
    FILE *file = fopen(PIN_ATTEMPTS_FILE, "r");
    if (file == NULL) {
        *count = 0;
        return NULL;
    }

    // Count the number of records
    int recordCount = 0;
    char line[100];
    while (fgets(line, sizeof(line), file) != NULL) {
        recordCount++;
    }

    // Allocate memory for records
    PinAttemptRecord *records = (PinAttemptRecord *)malloc(recordCount * sizeof(PinAttemptRecord));
    if (records == NULL) {
        fclose(file);
        *count = 0;
        return NULL;
    }

    // Reset file position to beginning
    rewind(file);

    // Read records
    int index = 0;
    while (fgets(line, sizeof(line), file) != NULL && index < recordCount) {
        if (sscanf(line, "%d|%d|%ld", &records[index].cardNumber, 
                  &records[index].attempts, &records[index].lockoutTime) == 3) {
            index++;
        }
    }

    fclose(file);
    *count = index;
    return records;
}

// Helper function to write PIN attempts back to the file
static void writePinAttempts(PinAttemptRecord *records, int count) {
    FILE *file = fopen(PIN_ATTEMPTS_FILE, "w");
    if (file == NULL) {
        writeErrorLog("Failed to open PIN attempts file for writing");
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%d|%d|%ld\n", records[i].cardNumber, 
               records[i].attempts, records[i].lockoutTime);
    }

    fclose(file);
}

// Basic PIN validation function
int validatePIN(int enteredPin, int actualPin) {
    return enteredPin == actualPin;
}

// Function to prompt the user for their PIN and validate it
bool promptForPIN(int storedPin) {
    int enteredPin;
    int attemptCount = 0;
    const int maxAttempts = 3;

    while (attemptCount < maxAttempts) {
        printf("Please enter your PIN: ");
        if (scanf("%d", &enteredPin) != 1) {
            // Clear input buffer in case of bad input
            while (getchar() != '\n');
            printf("Invalid input. Please enter digits only.\n");
            continue;
        }
        
        // Clear input buffer
        while (getchar() != '\n');

        if (validatePIN(enteredPin, storedPin)) {
            printf("PIN accepted.\n");
            return true;
        }

        attemptCount++;
        printf("Incorrect PIN. %d attempt(s) remaining.\n", maxAttempts - attemptCount);
    }

    printf("Too many incorrect attempts. Card is temporarily locked.\n");
    return false;
}

// Function to record a failed PIN attempt
void recordFailedPINAttempt(int cardNumber) {
    int count;
    PinAttemptRecord *records = readPinAttempts(&count);

    // Find record for this card
    int index = -1;
    for (int i = 0; i < count; i++) {
        if (records[i].cardNumber == cardNumber) {
            index = i;
            break;
        }
    }

    if (index >= 0) {
        // Update existing record
        records[index].attempts++;
        if (records[index].attempts >= MAX_PIN_ATTEMPTS) {
            records[index].lockoutTime = time(NULL) + PIN_LOCKOUT_DURATION;
            
            // Log the lockout
            char logMsg[100];
            sprintf(logMsg, "Card %d has been locked out due to too many failed PIN attempts", cardNumber);
            writeErrorLog(logMsg);
        }
    } else {
        // Create a new record
        PinAttemptRecord *newRecords = (PinAttemptRecord *)realloc(records, 
                                          (count + 1) * sizeof(PinAttemptRecord));
        if (newRecords == NULL) {
            if (records != NULL) free(records);
            writeErrorLog("Memory allocation error in recordFailedPINAttempt");
            return;
        }
        
        records = newRecords;
        records[count].cardNumber = cardNumber;
        records[count].attempts = 1;
        records[count].lockoutTime = 0;
        count++;
    }

    writePinAttempts(records, count);
    free(records);
}

// Function to reset the PIN attempt counter
void resetPINAttempts(int cardNumber) {
    int count;
    PinAttemptRecord *records = readPinAttempts(&count);

    // Find record for this card
    for (int i = 0; i < count; i++) {
        if (records[i].cardNumber == cardNumber) {
            records[i].attempts = 0;
            records[i].lockoutTime = 0;
            break;
        }
    }

    writePinAttempts(records, count);
    if (records != NULL) free(records);
}

// Function to check if a card is temporarily locked out
bool isCardLockedOut(int cardNumber, int *remainingLockoutTime) {
    int count;
    PinAttemptRecord *records = readPinAttempts(&count);
    bool locked = false;

    // Find record for this card
    for (int i = 0; i < count; i++) {
        if (records[i].cardNumber == cardNumber) {
            time_t currentTime = time(NULL);
            if (records[i].lockoutTime > currentTime) {
                locked = true;
                if (remainingLockoutTime != NULL) {
                    *remainingLockoutTime = (int)(records[i].lockoutTime - currentTime);
                }
            } else if (records[i].lockoutTime > 0) {
                // Lockout period has expired, reset it
                records[i].lockoutTime = 0;
                writePinAttempts(records, count);
            }
            break;
        }
    }

    if (records != NULL) free(records);
    return locked;
}

// Function to get the number of remaining PIN attempts
int getRemainingPINAttempts(int cardNumber) {
    int count;
    PinAttemptRecord *records = readPinAttempts(&count);
    int remainingAttempts = MAX_PIN_ATTEMPTS;

    // Find record for this card
    for (int i = 0; i < count; i++) {
        if (records[i].cardNumber == cardNumber) {
            remainingAttempts = MAX_PIN_ATTEMPTS - records[i].attempts;
            if (remainingAttempts < 0) remainingAttempts = 0;
            break;
        }
    }

    if (records != NULL) free(records);
    return remainingAttempts;
}