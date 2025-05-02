#include "atm/atm_menu.h"
#include "common/utils/logger.h"
#include "common/config/config_manager.h"
#include "netbanking/netbanking.h"
#include "upi_transaction/upi_transaction.h"
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

// Get user role from database based on card number
// This is a stub implementation that would typically query a database
UserRole get_user_role(int card_number) {
    // For demo purposes:
    // Cards ending in 1-3: Basic role
    // Cards ending in 4-6: Netbanking role
    // Cards ending in 7-8: Premium role
    // Cards ending in 9-0: Corporate role
    
    int last_digit = card_number % 10;
    
    if (last_digit >= 1 && last_digit <= 3) {
        return ROLE_BASIC;
    } else if (last_digit >= 4 && last_digit <= 6) {
        return ROLE_NETBANKING;
    } else if (last_digit >= 7 && last_digit <= 8) {
        return ROLE_PREMIUM;
    } else {
        return ROLE_CORPORATE;
    }
}

int show_main_menu(AtmSession* session) {
    int choice = 0;
    UserRole role = get_user_role(session->card_number);
    
    printf("\n===== ATM Main Menu =====\n");
    
    // Basic operations - available to all users
    printf("1. Check Balance\n");
    printf("2. Withdraw Cash\n");
    printf("3. Deposit Cash\n");
    printf("4. Fund Transfer\n");
    printf("5. Change PIN\n");
    printf("6. Mini Statement\n");
    
    // Netbanking features - available to netbanking, premium and corporate roles
    if (role >= ROLE_NETBANKING) {
        printf("7. Bill Payment\n");
        printf("8. Netbanking Services\n");
    }
    
    // Premium features - available to premium and corporate roles
    if (role >= ROLE_PREMIUM) {
        printf("9. UPI Services\n");
        printf("10. Virtual Card Management\n");
    }
    
    // Corporate features - available only to corporate role
    if (role == ROLE_CORPORATE) {
        printf("11. Corporate Services\n");
    }
    
    // Exit option - always last
    printf("%d. Exit\n", (role == ROLE_BASIC) ? 7 : 
                          (role == ROLE_NETBANKING) ? 9 : 
                          (role == ROLE_PREMIUM) ? 11 : 12);
    
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
    
    printf("Payment of $%.2f for %s bill processed successfully.\n", amount, billers[biller_choice-1]);
    write_info_log("Bill payment processed");
    return true;
}

bool show_netbanking_menu(AtmSession* session) {
    int choice;
    
    printf("\n===== Netbanking Services =====\n");
    printf("1. Check Netbanking Status\n");
    printf("2. Update Mobile Number\n");
    printf("3. Update Email Address\n");
    printf("4. Set Transaction Limits\n");
    printf("5. Back to main menu\n");
    printf("\nEnter your choice: ");
    scanf("%d", &choice);
    
    switch (choice) {
        case 1:
            printf("Netbanking is active for your account.\n");
            break;
        case 2:
            printf("Mobile number updated successfully.\n");
            break;
        case 3:
            printf("Email address updated successfully.\n");
            break;
        case 4:
            printf("Transaction limits updated successfully.\n");
            break;
        case 5:
        default:
            return false;
    }
    
    write_info_log("Netbanking service accessed");
    return true;
}

bool show_upi_services_menu(AtmSession* session) {
    int choice;
    
    printf("\n===== UPI Services =====\n");
    printf("1. Activate UPI\n");
    printf("2. Reset UPI PIN\n");
    printf("3. Link New Account to UPI\n");
    printf("4. View UPI Transaction History\n");
    printf("5. Back to main menu\n");
    printf("\nEnter your choice: ");
    scanf("%d", &choice);
    
    switch (choice) {
        case 1:
            printf("UPI activated successfully.\n");
            break;
        case 2:
            printf("UPI PIN reset instructions sent to your registered mobile.\n");
            break;
        case 3:
            printf("New account linked to UPI successfully.\n");
            break;
        case 4:
            printf("\nUPI Transaction History:\n");
            printf("1. Paid Rs. 500 to merchant1@upi\n");
            printf("2. Received Rs. 1000 from friend@upi\n");
            break;
        case 5:
        default:
            return false;
    }
    
    write_info_log("UPI service accessed");
    return true;
}

bool show_virtual_card_menu(AtmSession* session) {
    int choice;
    
    printf("\n===== Virtual Card Management =====\n");
    printf("1. View Virtual Cards\n");
    printf("2. Generate New Virtual Card\n");
    printf("3. Block Virtual Card\n");
    printf("4. Set Virtual Card Limits\n");
    printf("5. Back to main menu\n");
    printf("\nEnter your choice: ");
    scanf("%d", &choice);
    
    switch (choice) {
        case 1:
            printf("\nYour Virtual Cards:\n");
            printf("1. Card ending with 4321 (Active)\n");
            printf("2. Card ending with 8765 (Blocked)\n");
            break;
        case 2:
            printf("New virtual card generated successfully.\n");
            break;
        case 3:
            printf("Virtual card blocked successfully.\n");
            break;
        case 4:
            printf("Virtual card limits updated successfully.\n");
            break;
        case 5:
        default:
            return false;
    }
    
    write_info_log("Virtual card service accessed");
    return true;
}

bool show_corporate_services_menu(AtmSession* session) {
    int choice;
    
    printf("\n===== Corporate Services =====\n");
    printf("1. Bulk Transfer\n");
    printf("2. Corporate Account Statement\n");
    printf("3. Manage Employee Accounts\n");
    printf("4. Tax Payment Services\n");
    printf("5. Back to main menu\n");
    printf("\nEnter your choice: ");
    scanf("%d", &choice);
    
    switch (choice) {
        case 1:
            printf("Bulk transfer initiated. Please check email for authorization.\n");
            break;
        case 2:
            printf("Corporate account statement sent to registered email.\n");
            break;
        case 3:
            printf("Please log in to web portal to manage employee accounts.\n");
            break;
        case 4:
            printf("Tax payment portal accessed. Please select tax type online.\n");
            break;
        case 5:
        default:
            return false;
    }
    
    write_info_log("Corporate service accessed");
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