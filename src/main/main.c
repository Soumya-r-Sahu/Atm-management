#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../validation/card_num_validation.h"
#include "../validation/pin_validation.h"
#include "../database/database.h"
#include "../utils/logger.h"
#include "../config/config_manager.h"
#include "../common/paths.h"
#include "../utils/language_support.h"
#include "menu.h"
#include "../Admin/admin_operations.h"  // Include admin operations
#include "../Admin/admin_db.h"          // Include admin database operations
#include "../Admin/admin_interface.h"   // Include admin interface for role-based access

// Command-line argument for test mode
#define TEST_MODE_ARG "--test"

// Forward declarations of functions used in this file
extern void displayMainMenu(int cardNumber);
int handleCardAuthentication();
void displayWelcomeBanner();

// Main function
int main(int argc, char *argv[]) {
    bool testMode = false;
    
    // Check for test mode flag
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], TEST_MODE_ARG) == 0) {
            testMode = true;
            printf("Running in TEST MODE - Using test data files\n");
        }
    }
    
    // Initialize data files and directories
    if (!initializeDataFiles()) {
        printf("Error: Failed to initialize required files and directories.\n");
        return 1;
    }
    
    // Initialize language support
    if (!initLanguageSupport()) {
        printf("Warning: Language support could not be fully initialized.\n");
    }
    
    // Set default language
    setLanguage(LANG_ENGLISH);

    if (!initializeConfigs()) {
        printf("Warning: Failed to load system configurations. Using defaults.\n");
    }
    
    // Main application loop
    while (1) {
        displayWelcomeBanner();
        
        // Display language options
        printf("Choose language / भाषा चुनें / ଭାଷା ବାଛନ୍ତୁ:\n");
        printf("1. English\n");
        printf("2. हिन्दी (Hindi)\n");
        printf("3. ଓଡ଼ିଆ (Odia)\n");
        
        int langChoice;
        printf("Enter choice: ");
        scanf("%d", &langChoice);
        
        // Set language based on user choice
        switch (langChoice) {
            case 1:
                setLanguage(LANG_ENGLISH);
                break;
            case 2:
                setLanguage(LANG_HINDI);
                break;
            case 3:
                setLanguage(LANG_ODIA);
                break;
            default:
                setLanguage(LANG_ENGLISH);
                break;
        }
        
        // Mode selection menu
        int choice;
        
        printf("\n===== Mode Selection =====\n");
        printf("1. Admin Mode\n");
        printf("2. ATM Mode\n");
        printf("Enter your choice (1-2): ");
        scanf("%d", &choice);
        
        if (choice == 1) {
            // Admin mode - Ask for admin credentials
            if (handleAdminAuthentication(0)) {
                // Admin authenticated, redirecting to admin menu is handled in handleAdminAuthentication
                printf("\nReturning to main menu...\n");
            }
        } else if (choice == 2) {
            // ATM mode - Ask for ATM ID first
            int atmId;
            printf("\n===== ATM Mode =====\n");
            printf("Enter ATM ID: ");
            scanf("%d", &atmId);
            
            if (handleAtmModeAuthentication(atmId)) {
                // ATM authenticated, now handle regular customer card
                int cardNumber = handleCardAuthentication();
                
                if (cardNumber > 0) {
                    // Regular user authenticated, show main menu
                    displayMainMenu(cardNumber);
                }
            }
        } else {
            printf("\nInvalid selection. Returning to main menu.\n");
        }
        
        // Ask if user wants to continue with another transaction
        char continueChoice;
        printf("\nDo you want to continue? (y/n): ");
        scanf(" %c", &continueChoice);
        
        if (continueChoice != 'y' && continueChoice != 'Y') {
            break;
        }
    }
    
    // Display exit message
    printf("\nThank you for using our ATM service.\n");
    
    // Free resources
    freeConfigs();
    
    return 0;
}

// Display the welcome banner
void displayWelcomeBanner() {
    printf("\n");
    printf(" ____________________________________________________\n");
    printf("|                                                    |\n");
    printf("|              WELCOME TO ATM SYSTEM                 |\n");
    printf("|                                                    |\n");
    printf("|____________________________________________________|\n\n");
}

// Handle card authentication
int handleCardAuthentication() {
    int cardNumInt;
    char cardNumber[20]; // String representation for functions that expect strings
    char pinStr[10];
    
    printf("\n===== Customer Authentication =====\n");
    printf("Please enter your card number: ");
    scanf("%d", &cardNumInt);
    
    // Convert card number to string
    sprintf(cardNumber, "%d", cardNumInt);
    
    // Check if card number is valid
    if (!doesCardExist(cardNumInt)) {
        writeErrorLog("Invalid card number entered");
        printf("Invalid card number. Please try again.\n");
        return -1;
    }
    
    // Check if card is active
    if (!isCardActive(cardNumInt)) {
        writeErrorLog("Attempt to use inactive/blocked card");
        printf("This card is not active or has been blocked. Please contact customer service.\n");
        return -1;
    }
    
    // Check if card is temporarily locked due to too many PIN attempts
    if (isCardLockedOut(cardNumber, 0)) {
        int attempts = getRemainingPINAttempts(cardNumber, 0);
        printf("This card is temporarily locked due to too many incorrect PIN attempts.\n");
        return -1;
    }
    
    // Get PIN
    printf("Please enter your PIN: ");
    scanf("%s", pinStr);
    
    // Validate PIN
    if (!validatePIN(cardNumber, pinStr, 0)) {
        // Track failed PIN attempt
        int attemptsLeft = trackPINAttempt(cardNumber, 0);
        if (attemptsLeft > 0) {
            printf("Invalid PIN. You have %d attempts remaining.\n", attemptsLeft);
        }
        writeErrorLog("Invalid PIN entered");
        return -1;
    }
    
    // Reset PIN attempts on successful validation
    resetPINAttempts(cardNumber, 0);
    
    return cardNumInt;
}