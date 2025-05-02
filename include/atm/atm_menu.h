#ifndef ATM_MENU_H
#define ATM_MENU_H

#include <stdbool.h>
#include <time.h>

// ATM Session structure
typedef struct {
    int card_number;           // Card number for the session
    bool authenticated;        // Whether user is authenticated
    time_t last_activity;      // Timestamp of last user activity
    double balance;            // Current account balance
} AtmSession;

// User roles for role-based menu display
typedef enum {
    ROLE_BASIC,       // Basic ATM operations only
    ROLE_NETBANKING,  // Basic + netbanking features
    ROLE_PREMIUM,     // All features including UPI and virtual cards
    ROLE_CORPORATE    // Business account features
} UserRole;

// Config keys
#define CONFIG_SESSION_TIMEOUT_SECONDS "session_timeout_seconds"

// ATM initialization
bool atm_initialize(void);

// Role management
UserRole get_user_role(int card_number);

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
void show_thank_you_screen(void);
void show_error_screen(const char* message);
void show_transaction_result(const char* title, const char* message, bool success, double amount, double balance);

// New extended menu options
bool show_bill_payment_menu(AtmSession* session);
bool show_netbanking_menu(AtmSession* session);
bool show_upi_services_menu(AtmSession* session);
bool show_virtual_card_menu(AtmSession* session);
bool show_corporate_services_menu(AtmSession* session);

// Session management
bool is_session_timed_out(AtmSession* session);
void update_session_activity(AtmSession* session);
AtmSession* start_new_session(int card_number);
void end_session(AtmSession* session);

#endif /* ATM_MENU_H */