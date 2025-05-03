#include "atm/atm_menu.h"
#include "common/utils/logger.h"
#include "common/config/config_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ATM initialization
bool atm_initialize(void) {
    writeInfoLog("ATM system initializing");
    return true;
}

// Buffer clearing utility
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Receipt generation function
void generateReceipt(int card_number, int transaction_type, double amount, double balance, const char* phoneNumber) {
    printf("Generating receipt...\n");
    // In a real implementation, this would format a receipt and possibly print it
    // For now, this is just a stub
    writeInfoLog("Receipt generated");
}

// UI screens
void show_welcome_screen(void) {
    printf("\n===== Welcome to ATM Management System =====\n\n");
}

bool show_pin_entry_screen(int* card_number, int* pin) {
    printf("Enter Card Number: ");
    scanf("%d", card_number);
    printf("Enter PIN: ");
    scanf("%d", pin);
    
    // Stub implementation: Always authenticate for now
    return true;
}

// This function is no longer needed as we're removing the role-based structure
// Keeping it for backward compatibility but with a single return value
UserRole get_user_role(int card_number) {
    // All users now have full access to all features
    return ROLE_CORPORATE; // Return the highest access level for all users
}

void display_atm_main_menu() {
    printf("\n\n\t\t======= ATM MAIN MENU =======\n");
    printf("\t\t1. Check Balance\n");
    printf("\t\t2. Withdraw Cash\n");
    printf("\t\t3. Deposit Cash\n");
    printf("\t\t4. Transfer Funds\n");
    printf("\t\t5. Change PIN\n");
    printf("\t\t6. Mini Statement\n");
    printf("\t\t7. Bill Payment\n");
    // Removed options for netbanking, UPI, corporate service, virtual card management, and role-based functionality
    printf("\t\t8. Language Settings\n");
    printf("\t\t9. Help & Support\n");
    printf("\t\t0. Exit\n");
    printf("\t\t============================\n");
    printf("\t\tEnter your choice: ");
}

void show_balance_screen(AtmSession* session) {
    printf("\n===== Balance Information =====\n");
    printf("Your current balance is: $%.2f\n", session->balance);
    
    // Generate receipt for balance check
    generateReceipt(session->card_number, TRANSACTION_BALANCE, 0.0, session->balance, NULL);
    printf("Receipt generated for this transaction.\n");
}

bool show_withdrawal_menu(AtmSession* session) {
    double amount;
    printf("\n===== Cash Withdrawal =====\n");
    printf("Enter amount to withdraw: $");
    scanf("%lf", &amount);
    
    // Process withdrawal and update balance (simplified here)
    double oldBalance = session->balance;
    session->balance -= amount;
    
    printf("Withdrawal of $%.2f processed successfully.\n", amount);
    
    // Generate receipt for withdrawal
    generateReceipt(session->card_number, TRANSACTION_WITHDRAWAL, amount, session->balance, NULL);
    printf("Receipt generated for this transaction.\n");
    
    return true;
}

bool show_deposit_menu(AtmSession* session) {
    double amount;
    printf("\n===== Cash Deposit =====\n");
    printf("Enter amount to deposit: $");
    scanf("%lf", &amount);
    
    // Process deposit and update balance (simplified here)
    double oldBalance = session->balance;
    session->balance += amount;
    
    printf("Deposit of $%.2f processed successfully.\n", amount);
    
    // Generate receipt for deposit
    generateReceipt(session->card_number, TRANSACTION_DEPOSIT, amount, session->balance, NULL);
    printf("Receipt generated for this transaction.\n");
    
    return true;
}

bool show_transfer_menu(AtmSession* session) {
    int recipient_card;
    double amount;
    
    printf("\n===== Fund Transfer =====\n");
    printf("Enter recipient's card number: ");
    scanf("%d", &recipient_card);
    printf("Enter amount to transfer: $");
    scanf("%lf", &amount);
    
    // Process transfer and update balance (simplified here)
    double oldBalance = session->balance;
    session->balance -= amount;
    
    printf("Transfer of $%.2f to card %d processed successfully.\n", amount, recipient_card);
    
    // Generate receipt for transfer
    generateReceipt(session->card_number, TRANSACTION_TRANSFER, amount, session->balance, NULL);
    printf("Receipt generated for this transaction.\n");
    
    return true;
}

bool show_pin_change_menu(AtmSession* session) {
    int old_pin, new_pin, confirm_pin;
    
    printf("\n===== PIN Change =====\n");
    printf("Enter old PIN: ");
    scanf("%d", &old_pin);
    printf("Enter new PIN: ");
    scanf("%d", &new_pin);
    printf("Confirm new PIN: ");
    scanf("%d", &confirm_pin);
    
    if (new_pin != confirm_pin) {
        printf("PINs do not match. PIN change failed.\n");
        return false;
    }
    
    printf("PIN changed successfully.\n");
    
    // Generate receipt for PIN change
    generateReceipt(session->card_number, TRANSACTION_PIN_CHANGE, 0.0, session->balance, NULL);
    printf("Receipt generated for this transaction.\n");
    
    return true;
}

void show_mini_statement(AtmSession* session) {
    printf("\n===== Mini Statement =====\n");
    printf("Last 5 transactions for your account:\n");
    printf("1. Cash Withdrawal: $50.00\n");
    printf("2. Cash Deposit: $100.00\n");
    printf("3. Fund Transfer: $30.00\n");
    printf("4. Cash Withdrawal: $20.00\n");
    printf("5. ATM Inquiry: $0.00\n");
    printf("Current balance: $%.2f\n", session->balance);
    
    // Generate receipt for mini statement
    generateReceipt(session->card_number, TRANSACTION_BALANCE, 0.0, session->balance, NULL);
    printf("Receipt generated for this transaction.\n");
}

// New functions for extended menu options

bool show_bill_payment_menu(AtmSession* session) {
    int biller_choice;
    double amount;
    
    printf("\n===== Bill Payment =====\n");
    printf("Select biller:\n");
    printf("1. Electricity\n");
    printf("2. Water\n");
    printf("3. Mobile Phone\n");
    printf("4. Internet\n");
    printf("5. Cable TV\n");
    printf("6. Back to main menu\n");
    printf("\nEnter your choice: ");
    scanf("%d", &biller_choice);
    
    if (biller_choice < 1 || biller_choice > 5) {
        return false;
    }
    
    const char* billers[] = {"Electricity", "Water", "Mobile Phone", "Internet", "Cable TV"};
    
    printf("Enter amount for %s bill: $", billers[biller_choice-1]);
    scanf("%lf", &amount);
    
    // Process bill payment and update balance (simplified here)
    double oldBalance = session->balance;
    session->balance -= amount;
    
    printf("Payment of $%.2f for %s bill processed successfully.\n", amount, billers[biller_choice-1]);
    writeInfoLog("Bill payment processed");
    
    // Generate receipt for bill payment
    generateReceipt(session->card_number, TRANSACTION_BILL_PAYMENT, amount, session->balance, NULL);
    printf("Receipt generated for this transaction.\n");
    
    return true;
}

void show_thank_you_screen(void) {
    printf("\n===== Thank You =====\n");
    printf("Thank you for using ATM Management System.\n");
    printf("Please collect your card.\n");
}

void show_error_screen(const char* message) {
    printf("\n===== Error =====\n");
    printf("%s\n", message);
}

void show_transaction_result(const char* title, const char* message, bool success, double amount, double balance) {
    printf("\n===== %s =====\n", title);
    printf("%s\n", message);
    if (success) {
        printf("Amount: $%.2f\n", amount);
        printf("Current Balance: $%.2f\n", balance);
    }
}

// Function to ask user if they want to continue with more transactions
bool prompt_continue_session(void) {
    char choice;
    printf("\nDo you want to continue with another transaction? (y/n): ");
    scanf(" %c", &choice);
    clearInputBuffer();
    
    return (choice == 'y' || choice == 'Y');
}

// Session management
bool is_session_timed_out(AtmSession* session) {
    if (!session) {
        return true;
    }
    
    const char* timeout_str = get_config_value(CONFIG_SESSION_TIMEOUT_SECONDS);
    int timeout = timeout_str ? atoi(timeout_str) : 180; // default 3 minutes
    
    time_t now = time(NULL);
    return (now - session->last_activity) > timeout;
}

void update_session_activity(AtmSession* session) {
    if (session) {
        session->last_activity = time(NULL);
    }
}

AtmSession* start_new_session(int card_number) {
    AtmSession* session = (AtmSession*)malloc(sizeof(AtmSession));
    if (!session) {
        return NULL;
    }
    
    session->card_number = card_number;
    session->authenticated = true;
    session->last_activity = time(NULL);
    session->balance = 1000.00; // Stub implementation: Default balance
    
    return session;
}

void end_session(AtmSession* session) {
    if (session) {
        free(session);
    }
}

// Stub implementations for handler functions
void handle_balance_check(AtmSession* session) {
    show_balance_screen(session);
}

void handle_cash_withdrawal(AtmSession* session) {
    show_withdrawal_menu(session);
}

void handle_cash_deposit(AtmSession* session) {
    show_deposit_menu(session);
}

void handle_fund_transfer(AtmSession* session) {
    show_transfer_menu(session);
}

void handle_pin_change(AtmSession* session) {
    show_pin_change_menu(session);
}

void handle_mini_statement(AtmSession* session) {
    show_mini_statement(session);
}

void handle_bill_payment(AtmSession* session) {
    show_bill_payment_menu(session);
}

// Main menu function
int show_main_menu(AtmSession* session) {
    int choice;
    display_atm_main_menu();
    printf("Enter your choice: ");
    scanf("%d", &choice);
    clearInputBuffer();
    return choice;
}

void handle_main_menu(AtmSession* session) {
    int choice;
    bool exit_menu = false;
    
    while (!exit_menu) {
        display_atm_main_menu();
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: // Check Balance
                handle_balance_check(session);
                break;
            case 2: // Withdraw Cash
                handle_cash_withdrawal(session);
                break;
            case 3: // Deposit Cash
                handle_cash_deposit(session);
                break;
            case 4: // Fund Transfer
                handle_fund_transfer(session);
                break;
            case 5: // Change PIN
                handle_pin_change(session);
                break;
            case 6: // Mini Statement
                handle_mini_statement(session);
                break;
            case 7: // Bill Payment
                handle_bill_payment(session);
                break;
            case 0: // Exit
                printf("\nThank you for using our ATM service.\n");
                exit_menu = true;
                break;
            default:
                printf("\nInvalid choice. Please try again.\n");
        }
    }
}