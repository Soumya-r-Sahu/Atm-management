#include "atm/atm_menu.h"
#include "common/utils/logger.h"
#include "common/config/config_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ATM initialization
bool atm_initialize(void) {
    write_info_log("ATM system initializing");
    return true;
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

int show_main_menu(AtmSession* session) {
    int choice = 0;
    
    printf("\n===== ATM Main Menu =====\n");
    printf("1. Check Balance\n");
    printf("2. Withdraw Cash\n");
    printf("3. Deposit Cash\n");
    printf("4. Fund Transfer\n");
    printf("5. Change PIN\n");
    printf("6. Mini Statement\n");
    printf("7. Exit\n");
    printf("\nEnter your choice: ");
    scanf("%d", &choice);
    
    return choice;
}

void show_balance_screen(AtmSession* session) {
    printf("\n===== Balance Information =====\n");
    printf("Your current balance is: $%.2f\n", session->balance);
}

bool show_withdrawal_menu(AtmSession* session) {
    double amount;
    printf("\n===== Cash Withdrawal =====\n");
    printf("Enter amount to withdraw: $");
    scanf("%lf", &amount);
    
    printf("Withdrawal of $%.2f processed successfully.\n", amount);
    return true;
}

bool show_deposit_menu(AtmSession* session) {
    double amount;
    printf("\n===== Cash Deposit =====\n");
    printf("Enter amount to deposit: $");
    scanf("%lf", &amount);
    
    printf("Deposit of $%.2f processed successfully.\n", amount);
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
    
    printf("Transfer of $%.2f to card %d processed successfully.\n", amount, recipient_card);
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