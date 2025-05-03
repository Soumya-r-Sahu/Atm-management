#ifndef ATM_MENU_H
#define ATM_MENU_H

#include <stdbool.h>
#include <time.h>

// User role enum
typedef enum {
    ROLE_REGULAR,
    ROLE_CORPORATE,
    ROLE_PREMIUM
} UserRole;

// Transaction types
#define TRANSACTION_BALANCE_CHECK   1
#define TRANSACTION_WITHDRAWAL      2
#define TRANSACTION_DEPOSIT         3
#define TRANSACTION_MONEY_TRANSFER  4
#define TRANSACTION_PIN_CHANGE      5
#define TRANSACTION_BILL_PAYMENT    6
#define TRANSACTION_MINI_STATEMENT  7

// ATM Session structure
typedef struct {
    int card_number;           // Card number for the session
    bool authenticated;        // Whether user is authenticated
    time_t last_activity;      // Timestamp of last user activity
    double balance;            // Current account balance
} AtmSession;

// Config keys
#define CONFIG_SESSION_TIMEOUT_SECONDS "session_timeout_seconds"

// ATM initialization
bool atm_initialize(void);

// UI screens
void show_welcome_screen(void);
bool show_pin_entry_screen(int* card_number, int* pin);
int show_main_menu(AtmSession* session);
void show_balance_screen(AtmSession* session);
bool show_withdrawal_menu(AtmSession* session);
bool show_deposit_menu(AtmSession* session);
bool show_transfer_menu(AtmSession* session);
bool show_pin_change_menu(AtmSession* session);
void show_mini_statement(AtmSession* session);
bool show_bill_payment_menu(AtmSession* session);
void show_thank_you_screen(void);
void show_error_screen(const char* message);
void show_transaction_result(const char* title, const char* message, bool success, double amount, double balance);

// Receipt generation
void generateReceipt(int card_number, int transaction_type, double amount, double balance, const char* phoneNumber);

// Menu display
void display_atm_main_menu(void);

// User role function
UserRole get_user_role(int card_number);

// Utility functions
void clearInputBuffer(void);

// Handler functions
void handle_balance_check(AtmSession* session);
void handle_cash_withdrawal(AtmSession* session);
void handle_cash_deposit(AtmSession* session);
void handle_fund_transfer(AtmSession* session);
void handle_pin_change(AtmSession* session);
void handle_mini_statement(AtmSession* session);
void handle_bill_payment(AtmSession* session);
void handle_main_menu(AtmSession* session);

// Session management
bool is_session_timed_out(AtmSession* session);
void update_session_activity(AtmSession* session);
AtmSession* start_new_session(int card_number);
void end_session(AtmSession* session);

// Add this function declaration to the header file
bool prompt_continue_session(void);

#endif /* ATM_MENU_H */