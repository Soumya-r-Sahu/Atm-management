/**
 * @file cbs_admin_menu.c
 * @brief Implementation of Core Banking System admin menu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include "../../include/admin/admin_auth.h"
#include "../../include/common/database/core_banking_interface.h"
#include "../../include/common/database/card_account_management.h"
#include "../../include/common/utils/logger.h"
#include "../../include/common/reporting/transaction_reports.h"
#include "menu_utils.h"
#include "cbs_admin_menu.h"

// Local function prototypes
static void displayCBSAdminMenu(void);
static void handleCBSAdminMenuChoice(int choice);
static void manageCustomerAccounts(void);
static void manageCards(void);
static void generateReports(void);
static void viewTransactionHistory(void);
static void manageSystemSettings(void);
static void createNewAccount(void);
static void updateExistingAccount(void);
static void closeAccount(void);
static void createNewCard(void);
static void updateCardStatus(void);
static void viewCardDetails(void);
static void updateCardLimits(void);
static void dailyTransactionReport(void);
static void accountStatusReport(void);
static void cardUsageReport(void);

/**
 * @brief Run the Core Banking System admin menu
 */
void runCBSAdminMenu(void) {
    bool running = true;
    
    // Check admin authentication first
    if (!authenticateAdmin()) {
        printf("Authentication failed. Access denied.\n");
        sleep(2);
        return;
    }
    
    writeInfoLog("Admin accessed Core Banking System management menu");
    
    while (running) {
        clearScreen();
        printHeader("CORE BANKING SYSTEM - ADMIN MENU");
        displayCBSAdminMenu();
        
        int choice;
        printf("\nEnter your choice (1-6): ");
        if (scanf("%d", &choice) != 1) {
            // Clear input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            sleep(2);
            continue;
        }
        
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        if (choice == 6) {
            running = false;
            writeInfoLog("Admin exited Core Banking System management menu");
        } else {
            handleCBSAdminMenuChoice(choice);
        }
    }
}

/**
 * @brief Display the CBS admin menu options
 */
static void displayCBSAdminMenu(void) {
    printf("\nCore Banking System Administration:\n");
    printf("1. Customer Account Management\n");
    printf("2. Card Management\n");
    printf("3. Generate Reports\n");
    printf("4. Transaction History\n");
    printf("5. System Settings\n");
    printf("6. Return to Main Menu\n");
}

/**
 * @brief Handle CBS admin menu choices
 * @param choice Menu choice
 */
static void handleCBSAdminMenuChoice(int choice) {
    switch (choice) {
        case 1:
            manageCustomerAccounts();
            break;
        case 2:
            manageCards();
            break;
        case 3:
            generateReports();
            break;
        case 4:
            viewTransactionHistory();
            break;
        case 5:
            manageSystemSettings();
            break;
        default:
            printf("\nInvalid choice. Please try again.\n");
            sleep(2);
            break;
    }
}

/**
 * @brief Manage customer accounts
 */
static void manageCustomerAccounts(void) {
    bool running = true;
    
    while (running) {
        clearScreen();
        printHeader("CUSTOMER ACCOUNT MANAGEMENT");
        
        printf("\nAccount Operations:\n");
        printf("1. Create New Account\n");
        printf("2. Update Existing Account\n");
        printf("3. Close Account\n");
        printf("4. Back to Admin Menu\n");
        
        int choice;
        printf("\nEnter your choice (1-4): ");
        if (scanf("%d", &choice) != 1) {
            // Clear input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            sleep(2);
            continue;
        }
        
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        switch (choice) {
            case 1:
                createNewAccount();
                break;
            case 2:
                updateExistingAccount();
                break;
            case 3:
                closeAccount();
                break;
            case 4:
                running = false;
                break;
            default:
                printf("\nInvalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}

/**
 * @brief Manage cards
 */
static void manageCards(void) {
    bool running = true;
    
    while (running) {
        clearScreen();
        printHeader("CARD MANAGEMENT");
        
        printf("\nCard Operations:\n");
        printf("1. Create New Card\n");
        printf("2. Update Card Status (Block/Unblock)\n");
        printf("3. View Card Details\n");
        printf("4. Update Card Limits\n");
        printf("5. Back to Admin Menu\n");
        
        int choice;
        printf("\nEnter your choice (1-5): ");
        if (scanf("%d", &choice) != 1) {
            // Clear input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            sleep(2);
            continue;
        }
        
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        switch (choice) {
            case 1:
                createNewCard();
                break;
            case 2:
                updateCardStatus();
                break;
            case 3:
                viewCardDetails();
                break;
            case 4:
                updateCardLimits();
                break;
            case 5:
                running = false;
                break;
            default:
                printf("\nInvalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}

/**
 * @brief Generate reports
 */
static void generateReports(void) {
    bool running = true;
    
    while (running) {
        clearScreen();
        printHeader("GENERATE REPORTS");
        
        printf("\nReport Types:\n");
        printf("1. Daily Transaction Report\n");
        printf("2. Account Status Report\n");
        printf("3. Card Usage Report\n");
        printf("4. Back to Admin Menu\n");
        
        int choice;
        printf("\nEnter your choice (1-4): ");
        if (scanf("%d", &choice) != 1) {
            // Clear input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            sleep(2);
            continue;
        }
        
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        switch (choice) {
            case 1:
                dailyTransactionReport();
                break;
            case 2:
                accountStatusReport();
                break;
            case 3:
                cardUsageReport();
                break;
            case 4:
                running = false;
                break;
            default:
                printf("\nInvalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}

/**
 * @brief View transaction history
 */
static void viewTransactionHistory(void) {
    clearScreen();
    printHeader("TRANSACTION HISTORY");
    
    char account_number[25];
    printf("\nEnter account number (or press Enter to search by card): ");
    if (fgets(account_number, sizeof(account_number), stdin) == NULL) {
        printf("Error reading input\n");
        sleep(2);
        return;
    }
    
    // Remove newline character
    account_number[strcspn(account_number, "\n")] = 0;
    
    if (strlen(account_number) == 0) {
        // Search by card instead
        char card_number[20];
        printf("Enter card number: ");
        if (fgets(card_number, sizeof(card_number), stdin) == NULL) {
            printf("Error reading input\n");
            sleep(2);
            return;
        }
        
        // Remove newline character
        card_number[strcspn(card_number, "\n")] = 0;
        
        // Get account number from card
        if (!cbs_get_account_by_card(card_number, account_number, sizeof(account_number))) {
            printf("\nCould not find account for card %s\n", card_number);
            printf("\nPress Enter to continue...");
            getchar();
            return;
        }
    }
    
    // Retrieve transaction history
    TransactionRecord* records = (TransactionRecord*)malloc(50 * sizeof(TransactionRecord));
    if (!records) {
        printf("\nMemory allocation error\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    int count = 0;
    if (!cbs_get_transaction_history(account_number, records, &count, 50)) {
        printf("\nCould not retrieve transaction history\n");
        free(records);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Display transactions
    printf("\nTransaction History for Account: %s\n\n", account_number);
    printf("%-37s %-20s %-15s %-10s %-15s\n", "Transaction ID", "Date", "Type", "Amount", "Status");
    printf("---------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < count; i++) {
        printf("%-37s %-20s %-15s $%-9.2f %-15s\n", 
               records[i].transaction_id, 
               records[i].date, 
               records[i].transaction_type, 
               records[i].amount, 
               records[i].status);
    }
    
    free(records);
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Manage system settings
 */
static void manageSystemSettings(void) {
    clearScreen();
    printHeader("SYSTEM SETTINGS");
    
    printf("\nThis feature is under development.\n");
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Create a new account
 */
static void createNewAccount(void) {
    clearScreen();
    printHeader("CREATE NEW ACCOUNT");
    
    printf("\nThis feature is under development.\n");
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Update existing account
 */
static void updateExistingAccount(void) {
    clearScreen();
    printHeader("UPDATE EXISTING ACCOUNT");
    
    printf("\nThis feature is under development.\n");
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Close an account
 */
static void closeAccount(void) {
    clearScreen();
    printHeader("CLOSE ACCOUNT");
    
    printf("\nThis feature is under development.\n");
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Create a new card
 */
static void createNewCard(void) {
    clearScreen();
    printHeader("CREATE NEW CARD");
    
    printf("\nThis feature is under development.\n");
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Update card status (block/unblock)
 */
static void updateCardStatus(void) {
    clearScreen();
    printHeader("UPDATE CARD STATUS");
    
    int card_number;
    printf("\nEnter card number: ");
    if (scanf("%d", &card_number) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid input. Please enter a number.\n");
        sleep(2);
        return;
    }
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    // Check if card exists
    if (!cbs_card_exists(card_number)) {
        printf("\nCard not found. Please check the card number.\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Get current card status
    bool is_active = cbs_is_card_active(card_number);
    printf("\nCurrent card status: %s\n", is_active ? "ACTIVE" : "BLOCKED");
    
    printf("\nOptions:\n");
    printf("1. Block Card\n");
    printf("2. Unblock Card\n");
    printf("3. Cancel\n");
    
    int choice;
    printf("\nEnter your choice (1-3): ");
    if (scanf("%d", &choice) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid input. Please enter a number.\n");
        sleep(2);
        return;
    }
    
    // Clear input buffer
    while ((c = getchar()) != '\n' && c != EOF);
    
    bool result = false;
    switch (choice) {
        case 1:
            if (is_active) {
                result = cbs_block_card(card_number);
                printf("\n%s\n", result ? "Card blocked successfully" : "Failed to block card");
                writeInfoLog("Admin blocked card: %d, Result: %s", card_number, result ? "Success" : "Failed");
            } else {
                printf("\nCard is already blocked\n");
            }
            break;
        case 2:
            if (!is_active) {
                result = cbs_unblock_card(card_number);
                printf("\n%s\n", result ? "Card unblocked successfully" : "Failed to unblock card");
                writeInfoLog("Admin unblocked card: %d, Result: %s", card_number, result ? "Success" : "Failed");
            } else {
                printf("\nCard is already active\n");
            }
            break;
        case 3:
            printf("\nOperation canceled\n");
            break;
        default:
            printf("\nInvalid choice\n");
            break;
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief View card details
 */
static void viewCardDetails(void) {
    clearScreen();
    printHeader("VIEW CARD DETAILS");
    
    int card_number;
    printf("\nEnter card number: ");
    if (scanf("%d", &card_number) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        printf("Invalid input. Please enter a valid card number.\n");
        sleep(2);
        return;
    }
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    // Get card details from the database
    char holder_name[100] = {0};
    char account_number[25] = {0};
    char expiry_date[10] = {0};
    int is_active = 0;
    double daily_limit = 0.0;
    char card_type[20] = {0};
    
    bool success = cbs_get_card_details(
        card_number, 
        holder_name, sizeof(holder_name),
        account_number, sizeof(account_number),
        expiry_date, sizeof(expiry_date),
        &is_active,
        &daily_limit,
        card_type, sizeof(card_type)
    );
    
    if (success) {
        clearScreen();
        printHeader("CARD DETAILS");
        
        printf("\n%-20s: %d\n", "Card Number", card_number);
        printf("%-20s: %s\n", "Holder Name", holder_name);
        printf("%-20s: %s\n", "Account Number", account_number);
        printf("%-20s: %s\n", "Expiry Date", expiry_date);
        printf("%-20s: %s\n", "Status", is_active ? "Active" : "Blocked");
        printf("%-20s: $%.2f\n", "Daily Limit", daily_limit);
        printf("%-20s: %s\n", "Card Type", card_type);
        
        // Get current balance
        double balance = 0.0;
        if (cbs_get_balance_by_card(card_number, &balance)) {
            printf("%-20s: $%.2f\n", "Current Balance", balance);
        } else {
            printf("%-20s: Unable to retrieve\n", "Current Balance");
        }
        
        // Get recent transactions
        printf("\nRecent Transactions:\n");
        printf("%-36s %-20s %-12s %-20s\n", "Transaction ID", "Type", "Amount", "Date");
        printf("--------------------------------------------------------------------------------\n");
        
        TransactionRecord records[5];
        int count = 0;
        
        if (cbs_get_transaction_history(account_number, records, &count, 5)) {
            if (count > 0) {
                for (int i = 0; i < count; i++) {
                    printf("%-36s %-20s $%-11.2f %-20s\n", 
                        records[i].transaction_id,
                        records[i].transaction_type,
                        records[i].amount,
                        records[i].date);
                }
            } else {
                printf("No recent transactions found.\n");
            }
        } else {
            printf("Unable to retrieve recent transactions.\n");
        }
        
    } else {
        printf("\nFailed to retrieve card details. Card may not exist.\n");
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Update card limits
 */
static void updateCardLimits(void) {
    clearScreen();
    printHeader("UPDATE CARD LIMITS");
    
    int card_number;
    printf("\nEnter card number: ");
    if (scanf("%d", &card_number) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid input. Please enter a number.\n");
        sleep(2);
        return;
    }
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    // Check if card exists
    if (!cbs_card_exists(card_number)) {
        printf("\nCard not found. Please check the card number.\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Get current limits
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", card_number);
    
    double current_atm_limit = 0.0;
    double current_pos_limit = 0.0;
    double current_online_limit = 0.0;
    
    // This function would need to be implemented
    // if (!cbs_get_card_limits(card_number_str, &current_atm_limit, &current_pos_limit, &current_online_limit)) {
    //     printf("\nFailed to retrieve current card limits\n");
    //     printf("\nPress Enter to continue...");
    //     getchar();
    //     return;
    // }
    
    // For now, use placeholder values
    current_atm_limit = 1000.0;
    current_pos_limit = 5000.0;
    current_online_limit = 2000.0;
    
    printf("\nCurrent Card Limits:\n");
    printf("ATM Withdrawal Limit: $%.2f\n", current_atm_limit);
    printf("POS Transaction Limit: $%.2f\n", current_pos_limit);
    printf("Online Transaction Limit: $%.2f\n", current_online_limit);
    
    printf("\nEnter new ATM withdrawal limit (0 to keep current): ");
    double new_atm_limit;
    if (scanf("%lf", &new_atm_limit) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid input.\n");
        sleep(2);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("Enter new POS transaction limit (0 to keep current): ");
    double new_pos_limit;
    if (scanf("%lf", &new_pos_limit) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid input.\n");
        sleep(2);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("Enter new online transaction limit (0 to keep current): ");
    double new_online_limit;
    if (scanf("%lf", &new_online_limit) != 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid input.\n");
        sleep(2);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Clear input buffer
    while ((c = getchar()) != '\n' && c != EOF);
    
    // Use current limits if new values are 0
    if (new_atm_limit == 0) new_atm_limit = current_atm_limit;
    if (new_pos_limit == 0) new_pos_limit = current_pos_limit;
    if (new_online_limit == 0) new_online_limit = current_online_limit;
    
    // This function would need to be implemented
    // bool result = cbs_update_card_limits(card_number_str, new_atm_limit, new_pos_limit, new_online_limit);
    bool result = true; // Placeholder for now
    
    if (result) {
        printf("\nCard limits updated successfully.\n");
        writeInfoLog("Admin updated card limits: Card %d, ATM: %.2f, POS: %.2f, Online: %.2f", 
                    card_number, new_atm_limit, new_pos_limit, new_online_limit);
    } else {
        printf("\nFailed to update card limits.\n");
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Generate daily transaction report
 */
static void dailyTransactionReport(void) {
    clearScreen();
    printHeader("DAILY TRANSACTION REPORT");
    
    // Get report date from user, default to today
    char report_date[11] = {0}; // YYYY-MM-DD format
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    strftime(report_date, sizeof(report_date), "%Y-%m-%d", tm_now);
    
    printf("\nEnter report date (YYYY-MM-DD) [default: today %s]: ", report_date);
    char input_date[20];
    if (fgets(input_date, sizeof(input_date), stdin)) {
        input_date[strcspn(input_date, "\n")] = 0; // Remove newline
        
        // If user entered a date, use it
        if (strlen(input_date) > 0) {
            strncpy(report_date, input_date, sizeof(report_date) - 1);
        }
    }
      printf("\nGenerating transaction report for date: %s\n", report_date);
    printf("Please wait...\n");
    
    // Create reports directory if it doesn't exist
    system("if not exist reports mkdir reports");
    
    // Generate report filename
    char report_path[100];
    sprintf(report_path, "reports/transaction_report_%s.txt", report_date);
    
    // Generate the report
    bool success = cbs_generate_daily_transaction_report(report_date, report_path);
    
    if (success) {
        printf("\nReport generated successfully!\n");
        printf("Report saved to: %s\n", report_path);
        printf("\nWould you like to view the report now? (y/n): ");
        
        char choice;
        scanf("%c", &choice);
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        if (choice == 'y' || choice == 'Y') {
            clearScreen();
            printHeader("VIEWING TRANSACTION REPORT");
            
            // Read and display the report
            FILE *report_file = fopen(report_path, "r");
            if (report_file) {
                char line[256];
                while (fgets(line, sizeof(line), report_file)) {
                    printf("%s", line);
                }
                fclose(report_file);
            } else {
                printf("\nError opening report file.\n");
            }
        }
    } else {
        printf("\nFailed to generate report.\n");
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Generate account status report
 */
static void accountStatusReport(void) {
    clearScreen();
    printHeader("ACCOUNT STATUS REPORT");
    
    printf("\nGenerating account status report...\n");    printf("Please wait...\n");
    
    // Create reports directory if it doesn't exist
    system("if not exist reports mkdir reports");
    
    // Generate report filename with current date
    char report_path[100];
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char date_str[11];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_now);
    sprintf(report_path, "reports/account_status_%s.txt", date_str);
    
    // Generate the report
    bool success = cbs_generate_account_status_report(report_path);
    
    if (success) {
        printf("\nReport generated successfully!\n");
        printf("Report saved to: %s\n", report_path);
        printf("\nWould you like to view the report now? (y/n): ");
        
        char choice;
        scanf("%c", &choice);
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        if (choice == 'y' || choice == 'Y') {
            clearScreen();
            printHeader("VIEWING ACCOUNT STATUS REPORT");
            
            // Read and display the report
            FILE *report_file = fopen(report_path, "r");
            if (report_file) {
                char line[256];
                while (fgets(line, sizeof(line), report_file)) {
                    printf("%s", line);
                }
                fclose(report_file);
            } else {
                printf("\nError opening report file.\n");
            }
        }
    } else {
        printf("\nFailed to generate report.\n");
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Generate card usage report
 */
static void cardUsageReport(void) {
    clearScreen();
    printHeader("CARD USAGE REPORT");
    
    // Get date range from user
    char start_date[11] = {0}; // YYYY-MM-DD format
    char end_date[11] = {0};   // YYYY-MM-DD format
    
    // Default to the last 30 days
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    strftime(end_date, sizeof(end_date), "%Y-%m-%d", tm_now);
    
    // Calculate date 30 days ago
    time_t thirty_days_ago = now - (30 * 24 * 60 * 60);
    struct tm *tm_thirty_days_ago = localtime(&thirty_days_ago);
    strftime(start_date, sizeof(start_date), "%Y-%m-%d", tm_thirty_days_ago);
    
    printf("\nEnter start date (YYYY-MM-DD) [default: %s]: ", start_date);
    char input_date[20];
    if (fgets(input_date, sizeof(input_date), stdin)) {
        input_date[strcspn(input_date, "\n")] = 0; // Remove newline
        
        // If user entered a date, use it
        if (strlen(input_date) > 0) {
            strncpy(start_date, input_date, sizeof(start_date) - 1);
        }
    }
    
    printf("Enter end date (YYYY-MM-DD) [default: %s]: ", end_date);
    if (fgets(input_date, sizeof(input_date), stdin)) {
        input_date[strcspn(input_date, "\n")] = 0; // Remove newline
        
        // If user entered a date, use it
        if (strlen(input_date) > 0) {
            strncpy(end_date, input_date, sizeof(end_date) - 1);
        }
    }
    
    printf("\nGenerating card usage report from %s to %s\n", start_date, end_date);    printf("Please wait...\n");
    
    // Create reports directory if it doesn't exist
    system("if not exist reports mkdir reports");
    
    // Generate report filename
    char report_path[100];
    sprintf(report_path, "reports/card_usage_%s_to_%s.txt", start_date, end_date);
    
    // Generate the report
    bool success = cbs_generate_card_usage_report(start_date, end_date, report_path);
    
    if (success) {
        printf("\nReport generated successfully!\n");
        printf("Report saved to: %s\n", report_path);
        printf("\nWould you like to view the report now? (y/n): ");
        
        char choice;
        scanf("%c", &choice);
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        if (choice == 'y' || choice == 'Y') {
            clearScreen();
            printHeader("VIEWING CARD USAGE REPORT");
            
            // Read and display the report
            FILE *report_file = fopen(report_path, "r");
            if (report_file) {
                char line[256];
                while (fgets(line, sizeof(line), report_file)) {
                    printf("%s", line);
                }
                fclose(report_file);
            } else {
                printf("\nError opening report file.\n");
            }
        }
    } else {
        printf("\nFailed to generate report.\n");
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}
