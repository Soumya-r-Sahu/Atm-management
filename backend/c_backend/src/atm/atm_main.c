#include "atm/atm_menu.h"
#include "atm/validation/card_validator.h"
#include "atm/transaction/transaction_processor.h"
#include "common/utils/logger.h"
#include "common/config/config_manager.h"
#include "common/database/database.h"  // Added for close_database() function
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>  // Added for sleep() function

// Function to handle the main menu selection
void process_main_menu(AtmSession* session) {
    bool continue_session = true;
    int choice;
    
    while (continue_session) {
        choice = show_main_menu(session);
        update_session_activity(session);
        
        switch (choice) {
            case 1: // Check Balance
                show_balance_screen(session);
                break;
            case 2: // Withdraw Cash
                show_withdrawal_menu(session);
                break;
            case 3: // Deposit Cash
                show_deposit_menu(session);
                break;
            case 4: // Fund Transfer
                show_transfer_menu(session);
                break;
            case 5: // Change PIN
                show_pin_change_menu(session);
                break;
            case 6: // Mini Statement
                show_mini_statement(session);
                break;
            case 7: // Bill Payment
                show_bill_payment_menu(session);
                break;
            case 8: // Exit
                continue_session = false;
                show_thank_you_screen();
                break;
            default:
                show_error_screen("Invalid option selected. Please try again.");
        }
        
        // If user wants to exit, break out of the loop
        if (!continue_session) {
            break;
        }
        
        // Ask if user wants to continue with other transactions
        continue_session = prompt_continue_session();
    }
}

// Main ATM system entry point
int main(int argc, char** argv) {
    // Initialize logging system first for error tracking
    initializeLogger("logs");
    writeInfoLog("ATM application starting");

    // Initialize ATM system
    if (!atm_initialize()) {
        writeErrorLog("Failed to initialize ATM system. Exiting.");
        printf("Error: Could not initialize the ATM system. Please contact support.\n");
        return EXIT_FAILURE;
    }

    // Main ATM operation loop
    bool running = true;
    while (running) {
        // Show welcome screen and get card number and PIN
        show_welcome_screen();
        
        int card_number, pin;
        bool authenticated = show_pin_entry_screen(&card_number, &pin);
        
        if (authenticated) {
            writeSecurityLog("User authenticated successfully for card %d", card_number);
            
            // Create new session
            AtmSession* session = start_new_session(card_number);
            if (!session) {
                show_error_screen("Failed to start ATM session");
                continue;
            }
            
            // Process main menu
            process_main_menu(session);
            
            // End session
            end_session(session);
            
            // Optional delay before returning to welcome screen
            sleep(3);  // Sleep for 3 seconds
        }
    }
    
    // Clean up resources
    freeConfigs();
    closeLogger();
    
    return EXIT_SUCCESS;
}