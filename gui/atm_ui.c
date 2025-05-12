/**
 * @file atm_ui.c
 * @brief ATM screen user interface
 * 
 * This file provides the implementation for the ATM interface
 * of the Core Banking System.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "../include/global.h"
#include "../include/error_codes.h"

// Menu options
#define MENU_CHECK_BALANCE  1
#define MENU_WITHDRAWAL     2
#define MENU_DEPOSIT        3
#define MENU_TRANSFER       4
#define MENU_PIN_CHANGE     5
#define MENU_MINI_STATEMENT 6
#define MENU_EXIT           0

// ATM state
typedef struct {
    char card_number[20];
    char account_id[MAX_ACCOUNT_ID_LEN];
    int is_authenticated;
    double available_balance;
} ATMState;

/**
 * @brief Displays the ATM welcome screen
 */
void display_welcome_screen() {
    system("cls"); // Clear the screen (Windows-specific)
    
    printf("===============================================\n");
    printf("              WELCOME TO CBS BANK              \n");
    printf("===============================================\n\n");
    printf("Please insert your card and enter your PIN.\n\n");
}

/**
 * @brief Simulates reading a card and PIN entry
 * 
 * @param state The current ATM state
 * @return int 1 if authentication successful, 0 otherwise
 */
int authenticate_card(ATMState* state) {
    char pin[5] = {0};
    int i = 0;
    char ch;
    
    // In a real implementation, this would read from a card reader
    printf("Enter Card Number: ");
    scanf("%19s", state->card_number);
    
    printf("Enter PIN: ");
    
    // Read PIN with masking (showing * instead of numbers)
    while (i < 4) {
        ch = _getch();
        if (ch >= '0' && ch <= '9') {
            pin[i++] = ch;
            printf("*");
        }
    }
    pin[i] = '\0';
    
    printf("\n\nAuthenticating...\n");
    
    // In a real implementation, this would verify against a database
    // For demo purposes, we'll accept any 4-digit PIN
    if (strlen(pin) == 4) {
        printf("Authentication successful!\n");
        
        // For demo purposes
        strcpy(state->account_id, "ACC123456789");
        state->is_authenticated = 1;
        state->available_balance = 1000.0; // Demo balance
        
        printf("Press any key to continue...");
        _getch();
        return 1;
    } else {
        printf("Authentication failed. Invalid PIN.\n");
        printf("Press any key to continue...");
        _getch();
        return 0;
    }
}

/**
 * @brief Displays the main ATM menu
 * 
 * @param state The current ATM state
 */
void display_main_menu(ATMState* state) {
    system("cls"); // Clear the screen
    
    printf("===============================================\n");
    printf("                  CBS BANK ATM                 \n");
    printf("===============================================\n");
    printf("Card: %s | Account: %s\n\n", 
           state->card_number, state->account_id);
    
    printf("Main Menu:\n\n");
    printf("  1. Check Balance\n");
    printf("  2. Withdrawal\n");
    printf("  3. Deposit\n");
    printf("  4. Transfer\n");
    printf("  5. PIN Change\n");
    printf("  6. Mini Statement\n");
    printf("  0. Exit\n\n");
    printf("Enter your choice: ");
}

/**
 * @brief Processes a balance inquiry
 * 
 * @param state The current ATM state
 */
void process_balance_inquiry(ATMState* state) {
    system("cls");
    
    printf("===============================================\n");
    printf("              BALANCE INQUIRY                  \n");
    printf("===============================================\n\n");
    
    printf("Account ID: %s\n", state->account_id);
    printf("Available Balance: $%.2f\n\n", state->available_balance);
    
    printf("Press any key to return to main menu...");
    _getch();
}

/**
 * @brief Processes a withdrawal
 * 
 * @param state The current ATM state
 */
void process_withdrawal(ATMState* state) {
    system("cls");
    double amount = 0.0;
    
    printf("===============================================\n");
    printf("                 WITHDRAWAL                    \n");
    printf("===============================================\n\n");
    
    printf("Account ID: %s\n", state->account_id);
    printf("Available Balance: $%.2f\n\n", state->available_balance);
    
    printf("Enter withdrawal amount: $");
    scanf("%lf", &amount);
    
    // Validate amount
    if (amount <= 0) {
        printf("\nInvalid amount. Please enter a positive value.\n");
    } else if (amount > state->available_balance) {
        printf("\nInsufficient funds. Your available balance is $%.2f\n", state->available_balance);
    } else {
        // Process withdrawal
        state->available_balance -= amount;
        
        printf("\nWithdrawal successful!\n");
        printf("Amount withdrawn: $%.2f\n", amount);
        printf("Remaining balance: $%.2f\n", state->available_balance);
        
        // In a real implementation, this would interact with the transaction module
        printf("\nPlease take your cash and receipt.\n");
    }
    
    printf("\nPress any key to return to main menu...");
    _getch();
}

/**
 * @brief Processes a deposit
 * 
 * @param state The current ATM state
 */
void process_deposit(ATMState* state) {
    system("cls");
    double amount = 0.0;
    
    printf("===============================================\n");
    printf("                  DEPOSIT                      \n");
    printf("===============================================\n\n");
    
    printf("Account ID: %s\n", state->account_id);
    printf("Available Balance: $%.2f\n\n", state->available_balance);
    
    printf("Enter deposit amount: $");
    scanf("%lf", &amount);
    
    // Validate amount
    if (amount <= 0) {
        printf("\nInvalid amount. Please enter a positive value.\n");
    } else {
        // Process deposit
        state->available_balance += amount;
        
        printf("\nDeposit successful!\n");
        printf("Amount deposited: $%.2f\n", amount);
        printf("New balance: $%.2f\n", state->available_balance);
        
        // In a real implementation, this would interact with the transaction module
    }
    
    printf("\nPress any key to return to main menu...");
    _getch();
}

/**
 * @brief Handles the main menu selection
 * 
 * @param state The current ATM state
 * @param choice The user's menu selection
 * @return int 1 to continue, 0 to exit
 */
int handle_main_menu(ATMState* state, int choice) {
    switch (choice) {
        case MENU_CHECK_BALANCE:
            process_balance_inquiry(state);
            return 1;
            
        case MENU_WITHDRAWAL:
            process_withdrawal(state);
            return 1;
            
        case MENU_DEPOSIT:
            process_deposit(state);
            return 1;
            
        case MENU_TRANSFER:
            printf("\nFund transfer not implemented yet.\n");
            printf("Press any key to return to main menu...");
            _getch();
            return 1;
            
        case MENU_PIN_CHANGE:
            printf("\nPIN change not implemented yet.\n");
            printf("Press any key to return to main menu...");
            _getch();
            return 1;
            
        case MENU_MINI_STATEMENT:
            printf("\nMini statement not implemented yet.\n");
            printf("Press any key to return to main menu...");
            _getch();
            return 1;
            
        case MENU_EXIT:
            printf("\nThank you for using CBS Bank ATM.\n");
            printf("Don't forget to take your card.\n");
            return 0;
            
        default:
            printf("\nInvalid choice.\n");
            printf("Press any key to continue...");
            _getch();
            return 1;
    }
}

/**
 * @brief Main function for the ATM interface
 */
int main() {
    // Initialize ATM state
    ATMState state = {
        .card_number = "",
        .account_id = "",
        .is_authenticated = 0,
        .available_balance = 0.0
    };
    
    int running = 1;
    int choice;
    
    // Display welcome screen and authenticate
    display_welcome_screen();
    if (!authenticate_card(&state)) {
        return 1; // Exit if authentication fails
    }
    
    // Main ATM loop
    while (running && state.is_authenticated) {
        display_main_menu(&state);
        scanf("%d", &choice);
        running = handle_main_menu(&state, choice);
    }
    
    return 0;
}
