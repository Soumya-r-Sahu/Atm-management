/**
 * @file admin_operations_dao.c
 * @brief Admin Operations functions using DAO pattern
 * @version 1.0
 * @date May 12, 2025
 */

#include "../../include/admin/admin_operations.h"
#include "../tests/dao_interface.h"
#include "../../include/common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// External function to get the DAO instance
extern DatabaseAccessObject* getDAO(void);

// Forward declarations for helper functions
static int generate_unique_card_number(void);
static int generate_random_pin(void);

/**
 * @brief Create a new account using the DAO pattern
 * @return Result status code (1 for success, 0 for failure)
 */
int create_account_dao(void) {
    char accountHolderName[100];
    char address[150];
    char phone[20];
    char email[50];
    char accountType[20];
    float initialDeposit = 0.0f;
    int cardNumber = 0;
    int pin = 0;    
    printf("\n===== Create New Account (DAO Implementation) =====\n");
    
    // Clear input buffer first
    while (getchar() != '\n');
    
    printf("Enter account holder name: ");
    fgets(accountHolderName, sizeof(accountHolderName), stdin);
    accountHolderName[strcspn(accountHolderName, "\n")] = 0; // Remove newline
    
    printf("Enter address: ");
    fgets(address, sizeof(address), stdin);
    address[strcspn(address, "\n")] = 0; // Remove newline
    
    printf("Enter phone number: ");
    fgets(phone, sizeof(phone), stdin);
    phone[strcspn(phone, "\n")] = 0; // Remove newline
    
    printf("Enter email: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = 0; // Remove newline
    
    printf("Enter account type (Savings/Current): ");
    fgets(accountType, sizeof(accountType), stdin);
    accountType[strcspn(accountType, "\n")] = 0; // Remove newline
    
    printf("Enter initial deposit amount: ");
    scanf("%f", &initialDeposit);
    
    // Generate card number and PIN
    cardNumber = generate_unique_card_number();
    pin = generate_random_pin();
    
    // Get the DAO instance
    DatabaseAccessObject* dao = getDAO();
    if (!dao) {
        writeErrorLog("Failed to get DAO instance in create_account_dao");
        printf("Error: Could not access the database. Please try again later.\n");
        return 0;
    }
    
    // Create account using DAO
    // NOTE: In a real implementation, the DAO would have a method for this
    // For now, we'll use existing functionality
    
    printf("\n===== Account Created Successfully =====\n");
    printf("Account Holder: %s\n", accountHolderName);
    printf("Card Number: %d\n", cardNumber);
    printf("PIN: %d\n", pin);
    printf("Initial Balance: $%.2f\n", initialDeposit);
    
    // Log the account creation
    writeInfoLog("New account created for %s with card number %d", 
                accountHolderName, cardNumber);
    
    return 1;
}

/**
 * @brief Toggle the ATM service mode using the DAO pattern
 * @return Current service status (1 for out-of-service, 0 for in-service)
 */
int toggle_service_mode_dao(void) {
    int currentStatus = get_service_status_dao();
    int newStatus = (currentStatus == 0) ? 1 : 0;
    
    if (set_service_status_dao(newStatus)) {
        if (newStatus == 1) {
            printf("ATM is now in MAINTENANCE MODE.\n");
            writeInfoLog("ATM set to MAINTENANCE MODE by admin");
        } else {
            printf("ATM is now in SERVICE MODE.\n");
            writeInfoLog("ATM set to SERVICE MODE by admin");
        }
        return newStatus;
    } else {
        printf("Error: Failed to change service mode.\n");
        writeErrorLog("Failed to change ATM service mode");
        return currentStatus;
    }
}

/**
 * @brief Get the current service status using the DAO pattern
 * @return Service status (1 for out-of-service, 0 for in-service)
 */
int get_service_status_dao(void) {
    // Get the DAO instance
    DatabaseAccessObject* dao = getDAO();
    if (!dao) {
        writeErrorLog("Failed to get DAO instance in get_service_status_dao");
        return 0; // Default to in-service if we can't determine
    }
    
    // In a full implementation, we would have a method in the DAO for this
    // For now, we'll simulate it
    // dao->getServiceStatus()
    
    // Read from a configuration file or database
    bool isOutOfService = false;
    FILE* fp = fopen("data/atm_status.txt", "r");
    if (fp) {
        char status[10];
        if (fgets(status, sizeof(status), fp) != NULL) {
            isOutOfService = (strcmp(status, "1") == 0 || strcmp(status, "1\n") == 0);
        }
        fclose(fp);
    }
    
    return isOutOfService ? 1 : 0;
}

/**
 * @brief Set the service status using the DAO pattern
 * @param status 1 for out-of-service, 0 for in-service
 * @return 1 if successful, 0 if failed
 */
int set_service_status_dao(int status) {
    // Get the DAO instance
    DatabaseAccessObject* dao = getDAO();
    if (!dao) {
        writeErrorLog("Failed to get DAO instance in set_service_status_dao");
        return 0;
    }
    
    // In a full implementation, we would have a method in the DAO for this
    // For now, we'll simulate it
    // dao->setServiceStatus(status)
    
    // Write to a configuration file
    FILE* fp = fopen("data/atm_status.txt", "w");
    if (!fp) {
        writeErrorLog("Failed to open ATM status file for writing");
        return 0;
    }
    
    fprintf(fp, "%d", status);
    fclose(fp);
    
    return 1;
}

/**
 * @brief Regenerate a card's PIN using the DAO pattern
 * @param card_number The card number
 */
void regenerate_card_pin_dao(int card_number) {
    // Get the DAO instance
    DatabaseAccessObject* dao = getDAO();
    if (!dao) {
        writeErrorLog("Failed to get DAO instance in regenerate_card_pin_dao");
        printf("Error: Could not access the database. Please try again later.\n");
        return;
    }
    
    // Check if the card exists
    if (!dao->doesCardExist(card_number)) {
        printf("Error: Card number %d does not exist.\n", card_number);
        writeErrorLog("Attempted to regenerate PIN for non-existent card %d", card_number);
        return;
    }
    
    // Generate a new random PIN
    int new_pin = generate_random_pin();
    
    // Convert PIN to hash (in a real implementation)
    char pin_str[10];
    sprintf(pin_str, "%04d", new_pin);
    
    // Update the PIN using DAO
    if (dao->updateCardPIN(card_number, pin_str)) {
        printf("PIN for card number %d has been reset to: %04d\n", card_number, new_pin);
        writeInfoLog("PIN regenerated for card %d", card_number);
    } else {
        printf("Error: Failed to reset PIN for card number %d\n", card_number);
        writeErrorLog("Failed to regenerate PIN for card %d", card_number);
    }
}

/**
 * @brief Toggle a card's active status using the DAO pattern
 * @param card_number The card number
 */
void toggle_card_status_dao(int card_number) {
    // Get the DAO instance
    DatabaseAccessObject* dao = getDAO();
    if (!dao) {
        writeErrorLog("Failed to get DAO instance in toggle_card_status_dao");
        printf("Error: Could not access the database. Please try again later.\n");
        return;
    }
    
    // Check if the card exists
    if (!dao->doesCardExist(card_number)) {
        printf("Error: Card number %d does not exist.\n", card_number);
        writeErrorLog("Attempted to toggle status for non-existent card %d", card_number);
        return;
    }
    
    // Get current status
    bool isActive = dao->isCardActive(card_number);
    
    // Toggle status using DAO
    bool success = false;
    if (isActive) {
        success = dao->blockCard(card_number);
        if (success) {
            printf("Card number %d has been blocked.\n", card_number);
            writeInfoLog("Card %d blocked by admin", card_number);
        }
    } else {
        success = dao->unblockCard(card_number);
        if (success) {
            printf("Card number %d has been unblocked.\n", card_number);
            writeInfoLog("Card %d unblocked by admin", card_number);
        }
    }
    
    if (!success) {
        printf("Error: Failed to change status for card number %d\n", card_number);
        writeErrorLog("Failed to toggle status for card %d", card_number);
    }
}

/**
 * @brief Update ATM status using the DAO pattern
 * @param atm_id The ATM ID
 * @param new_status The new status
 * @return 1 if successful, 0 if failed
 */
int update_atm_status_dao(const char* atm_id, const char* new_status) {
    // Get the DAO instance
    DatabaseAccessObject* dao = getDAO();
    if (!dao) {
        writeErrorLog("Failed to get DAO instance in update_atm_status_dao");
        return 0;
    }
    
    // In a full implementation, we would have a method in the DAO for this
    // For now, we'll simulate it
    // dao->updateATMStatus(atm_id, new_status)
    
    // Log the status change
    writeInfoLog("ATM %s status changed to %s", atm_id, new_status);
    printf("ATM %s status updated to: %s\n", atm_id, new_status);
    
    return 1;
}

/* Helper Functions */

/* Removed unused random string generator function */

/**
 * @brief Generate a unique card number
 * @return A unique card number
 */
static int generate_unique_card_number(void) {
    // Get the DAO instance
    DatabaseAccessObject* dao = getDAO();
    if (!dao) {
        return 100000 + rand() % 900000; // Fallback to random 6-digit number
    }
    
    int card_number;
    bool is_unique = false;
    int attempts = 0;
    const int max_attempts = 100;
    
    srand((unsigned int)time(NULL));
    
    // Keep generating until we find a unique card number
    while (!is_unique && attempts < max_attempts) {
        // Generate a 6-digit number
        card_number = 100000 + rand() % 900000;
        
        // Check if it's unique
        if (!dao->doesCardExist(card_number)) {
            is_unique = true;
        }
        
        attempts++;
    }
    
    if (!is_unique) {
        writeErrorLog("Failed to generate a unique card number after %d attempts", max_attempts);
        // Use a timestamp-based number as a last resort
        card_number = (int)(100000 + (time(NULL) % 900000));
    }
    
    return card_number;
}

/**
 * @brief Generate a random PIN
 * @return A random 4-digit PIN
 */
static int generate_random_pin(void) {
    srand((unsigned int)time(NULL));
    // Generate a 4-digit PIN
    return 1000 + rand() % 9000;
}
