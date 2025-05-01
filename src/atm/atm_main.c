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

// Main ATM system entry point
int main(int argc, char** argv) {
    // Initialize logging system first for error tracking
    initialize_logging();
    write_info_log("ATM application starting");

    // Initialize ATM system
    if (!atm_initialize()) {
        write_error_log("Failed to initialize ATM system. Exiting.");
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
            write_audit_log("ACCESS", "User authenticated successfully");
            
            // Create new session
            AtmSession* session = start_new_session(card_number);
            if (!session) {
                show_error_screen("Failed to start ATM session");
                continue;
            }
            
            // Session main menu loop
            bool session_active = true;
            while (session_active) {
                // Check for session timeout
                if (is_session_timed_out(session)) {
                    show_error_screen("Session timed out due to inactivity");
                    session_active = false;
                    continue;
                }
                
                // Show main menu and get user choice
                int choice = show_main_menu(session);
                
                // Handle user choice
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
                        
                    case 7: // Exit
                        session_active = false;
                        break;
                        
                    default:
                        show_error_screen("Invalid option. Please try again.");
                        break;
                }
                
                // Update session activity timestamp
                update_session_activity(session);
            }
            
            // End session and show thank you screen
            end_session(session);
            show_thank_you_screen();
            
            // Optional delay before returning to welcome screen
            #ifdef _WIN32
            sleep(3);  // Sleep for 3 seconds on Windows
            #else
            sleep(3);  // Sleep for 3 seconds on Unix-like systems
            #endif
        }
    }
    
    // Clean up resources
    close_database();
    free_configs();
    close_logs();
    
    return EXIT_SUCCESS;
}