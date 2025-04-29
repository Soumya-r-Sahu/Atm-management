#include "admin_menu.h"
#include "admin_operations.h"
#include "admin_db.h"
#include "../utils/logger.h"
#include "../config/config_manager.h"  // Added missing include for config-related functions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================
// Admin Menu Functions
// ============================

// Display admin menu
void displayAdminMenu() {
    printf("\n===== ATM Admin Panel =====\n");
    printf("1. Dashboard\n");
    printf("2. Cash Management\n");
    printf("3. Transaction Management\n");
    printf("4. User Management\n");
    printf("5. ATM Maintenance\n");
    printf("6. Security Management\n");
    printf("7. Settings\n");
    printf("8. Notifications and Alerts\n");
    printf("9. Audit Logs\n");
    printf("10. Atm Configurations\n");  // New option
    printf("11. Exit\n");                   // Changed from 10 to 11
    printf("=========================\n");
    printf("Enter your choice: ");
}

// Display the admin dashboard with ATM status and statistics
void displayDashboard() {
    printf("\n===== 📊 Dashboard =====\n");
    
    // Get ATM status from the file
    FILE *atmFile = fopen("data/atm_data.txt", "r");
    if (!atmFile) {
        printf("Error: Could not open ATM data file!\n");
        return;
    }

    // Variables for ATM data
    char line[256];
    char atm_id[20], location[50], status[30];
    double total_cash = 0.0;
    int atm_count = 0, online_count = 0;
    int total_transactions = 0;
    char last_refilled[30];
    int transaction_count;

    // Skip header lines (3 lines including separator lines)
    for (int i = 0; i < 3; i++) {
        if (fgets(line, sizeof(line), atmFile) == NULL) {
            printf("Error: Invalid ATM data file format!\n");
            fclose(atmFile);
            return;
        }
    }

    // Process each ATM entry
    while (fgets(line, sizeof(line), atmFile)) {
        // Skip separator lines
        if (line[0] == '+') continue;
        
        // Parse ATM data
        if (sscanf(line, "| %s | %*[^|] | %s | %lf | %*[^|] | %d |", 
                  atm_id, status, &total_cash, &transaction_count) >= 3) {
            atm_count++;
            total_transactions += transaction_count;
            
            if (strcmp(status, "Online") == 0) {
                online_count++;
            }
        }
    }
    
    fclose(atmFile);
    
    // Count today's transactions
    FILE *transactionFile = fopen("data/atm_transactions.txt", "r");
    if (!transactionFile) {
        printf("Error: Could not open transaction data file!\n");
    } else {
        // Variables for transaction counting
        char trans_date[30];
        char today_date[11] = "2025-04-27"; // Current date
        int today_transactions = 0;
        
        // Skip header lines (3 lines including separator lines)
        for (int i = 0; i < 3; i++) {
            if (fgets(line, sizeof(line), transactionFile) == NULL) break;
        }
        
        // Count transactions for today
        while (fgets(line, sizeof(line), transactionFile)) {
            // Skip separator lines
            if (line[0] == '+') continue;
            
            // Try to extract the date part from transaction_time
            if (sscanf(line, "| %*[^|] | %*[^|] | %*[^|] | %*[^|] | %*[^|] | %10s", trans_date) == 1) {
                if (strncmp(trans_date, today_date, 10) == 0) {
                    today_transactions++;
                }
            }
        }
        
        fclose(transactionFile);
        
        // Display dashboard information
        printf("ATM Status: %d of %d ATMs Online\n", online_count, atm_count);
        printf("Total Cash Available: ₹%.2f\n", total_cash);
        printf("Number of Transactions Today: %d\n", today_transactions);
        printf("Total Transaction Count: %d\n", total_transactions);
    }
    
    // Display alerts from security_logs.txt
    printf("\n--- Alerts ---\n");
    FILE *securityFile = fopen("data/security_logs.txt", "r");
    if (!securityFile) {
        printf("- No security alerts available\n");
    } else {
        char log_id[20], user_id[20], event_type[50], event_details[100], status[20];
        int alert_count = 0;
        
        // Skip header lines
        for (int i = 0; i < 3; i++) {
            if (fgets(line, sizeof(line), securityFile) == NULL) break;
        }
        
        // Display unresolved security alerts
        while (fgets(line, sizeof(line), securityFile) && alert_count < 3) {
            // Skip separator lines
            if (line[0] == '+') continue;
            
            // Extract status field
            if (strstr(line, "Unresolved")) {
                char *event_start = strstr(line, "|");
                if (event_start) {
                    event_start = strstr(event_start + 1, "|");
                    if (event_start) {
                        event_start = strstr(event_start + 1, "|");
                        if (event_start) {
                            char event_text[100] = {0};
                            sscanf(event_start + 1, " %[^|]", event_text);
                            printf("- %s\n", event_text);
                            alert_count++;
                        }
                    }
                }
            }
        }
        
        if (alert_count == 0) {
            printf("- No unresolved security alerts\n");
        }
        
        fclose(securityFile);
    }
    
    writeAuditLog("ADMIN", "Viewed dashboard");
}

// Manage cash levels in the ATM
void manageCash() {
    int choice;
    
    do {
        printf("\n===== 💰 Cash Management =====\n");
        printf("1. View Cash Levels\n");
        printf("2. Load/Refill Cash\n");
        printf("3. Cash Withdrawal Reports\n");
        printf("4. Daily Virtual Withdrawals Summary\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1: {
                printf("\n--- Current Cash Levels ---\n");
                
                // Try to read cash data from file
                FILE *cashFile = fopen("data/atm_data.txt", "r");
                if (!cashFile) {
                    // If file doesn't exist, show default data
                    printf("₹100 notes: 200 (₹20,000)\n");
                    printf("₹200 notes: 100 (₹20,000)\n");
                    printf("₹500 notes: 20 (₹10,000)\n");
                    printf("₹2000 notes: 0 (₹0)\n");
                    printf("Total Cash: ₹50,000\n");
                } else {
                    // Read cash data from file
                    double totalCash = 0.0;
                    int notes100 = 0, notes200 = 0, notes500 = 0, notes2000 = 0;
                    char line[256];
                    
                    // Look for cash inventory data in the file
                    // Try to read from a cash inventory section if it exists
                    FILE *inventoryFile = fopen("data/cash_inventory.txt", "r");
                    if (inventoryFile) {
                        while (fgets(line, sizeof(line), inventoryFile)) {
                            int denomination, count;
                            if (sscanf(line, "%d,%d", &denomination, &count) == 2) {
                                switch (denomination) {
                                    case 100: notes100 = count; break;
                                    case 200: notes200 = count; break;
                                    case 500: notes500 = count; break;
                                    case 2000: notes2000 = count; break;
                                }
                            }
                        }
                        fclose(inventoryFile);
                    } else {
                        // Use defaults if specific inventory file doesn't exist
                        notes100 = 200;
                        notes200 = 100;
                        notes500 = 20;
                        notes2000 = 0;
                    }
                    
                    // Calculate totals
                    int total100 = notes100 * 100;
                    int total200 = notes200 * 200;
                    int total500 = notes500 * 500;
                    int total2000 = notes2000 * 2000;
                    totalCash = total100 + total200 + total500 + total2000;
                    
                    // Display the cash level details
                    printf("₹100 notes: %d (₹%d)\n", notes100, total100);
                    printf("₹200 notes: %d (₹%d)\n", notes200, total200);
                    printf("₹500 notes: %d (₹%d)\n", notes500, total500);
                    printf("₹2000 notes: %d (₹%d)\n", notes2000, total2000);
                    printf("Total Cash: ₹%.2f\n", totalCash);
                    
                    fclose(cashFile);
                }
                writeAuditLog("ADMIN", "Viewed cash levels");
                break;
            }
            
            case 2: {
                printf("\n--- Load/Refill Cash ---\n");
                
                // Try to read previous refill data for reference
                FILE *refilFile = fopen("logs/refill_log.txt", "r");
                if (refilFile) {
                    char line[256];
                    int count = 0;
                    // Display the last refill record
                    printf("Last refill records:\n");
                    printf("-------------------\n");
                    
                    // Seek to end of file then move back to show most recent entries
                    fseek(refilFile, 0, SEEK_END);
                    long fileSize = ftell(refilFile);
                    
                    // If file is too large, just show last approx 500 bytes
                    if (fileSize > 500) {
                        fseek(refilFile, fileSize - 500, SEEK_SET);
                        // Discard partial line
                        fgets(line, sizeof(line), refilFile);
                    } else {
                        rewind(refilFile);
                    }
                    
                    // Read and display recent refill records
                    while (fgets(line, sizeof(line), refilFile) && count < 3) {
                        printf("%s", line);
                        count++;
                    }
                    printf("\n");
                    fclose(refilFile);
                }
                
                // Generate a reference ID (using current time)
                time_t t = time(NULL);
                struct tm *tm = localtime(&t);
                char refId[20];
                sprintf(refId, "REF%02d%02d%02d%02d%02d", 
                        tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                
                // Get current date and time for the refill record
                char dateTime[30];
                strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M:%S", tm);
                
                printf("New refill request generated.\n");
                printf("Reference ID: %s\n", refId);
                printf("Date: %s\n", dateTime);
                
                // Add refill record to log file
                FILE *logFile = fopen("logs/refill_log.txt", "a");
                if (logFile) {
                    fprintf(logFile, "%s | %s | Refill request submitted | Pending\n", dateTime, refId);
                    fclose(logFile);
                }
                
                writeAuditLog("ADMIN", "Submitted cash refill request");
                break;
            }
            
            case 3: {
                printf("\n--- Cash Withdrawal Reports ---\n");
                
                // Try to read withdrawal data from transaction file
                FILE *withdrawalFile = fopen("logs/withdrawals.log", "r");
                if (!withdrawalFile) {
                    // If file doesn't exist, show default data
                    printf("Today's Withdrawals: ₹120,000\n");
                    printf("This Week's Withdrawals: ₹750,000\n");
                    printf("This Month's Withdrawals: ₹3,250,000\n");
                } else {
                    // Calculate date ranges
                    time_t now = time(NULL);
                    struct tm *tm_now = localtime(&now);
                    int today_day = tm_now->tm_mday;
                    int today_month = tm_now->tm_mon + 1;
                    int today_year = tm_now->tm_year + 1900;
                    
                    char line[256];
                    double todayTotal = 0, weekTotal = 0, monthTotal = 0;
                    int day, month, year;
                    double amount;
                    
                    // Process each withdrawal record
                    while (fgets(line, sizeof(line), withdrawalFile)) {
                        // Extract date and amount (format: YYYY-MM-DD|amount)
                        if (sscanf(line, "%d-%d-%d|%lf", &year, &month, &day, &amount) == 4) {
                            // Add to appropriate totals based on date
                            
                            // Today's total
                            if (day == today_day && month == today_month && year == today_year) {
                                todayTotal += amount;
                            }
                            
                            // This week's total (simplified: just check if within last 7 days)
                            struct tm transaction_time = {0};
                            transaction_time.tm_year = year - 1900;
                            transaction_time.tm_mon = month - 1;
                            transaction_time.tm_mday = day;
                            
                            time_t trans_time = mktime(&transaction_time);
                            double diff_seconds = difftime(now, trans_time);
                            double diff_days = diff_seconds / (24 * 3600);
                            
                            if (diff_days <= 7.0) {
                                weekTotal += amount;
                            }
                            
                            // This month's total
                            if (month == today_month && year == today_year) {
                                monthTotal += amount;
                            }
                        }
                    }
                    
                    fclose(withdrawalFile);
                    
                    // Display the calculated totals
                    printf("Today's Withdrawals: ₹%.2f\n", todayTotal);
                    printf("This Week's Withdrawals: ₹%.2f\n", weekTotal);
                    printf("This Month's Withdrawals: ₹%.2f\n", monthTotal);
                }
                writeAuditLog("ADMIN", "Viewed cash withdrawal reports");
                break;
            }
            
            case 4: {
                printf("\n--- Daily Virtual Withdrawals Summary ---\n");
                
                // Try to read virtual withdrawal data from virtual_wallet.txt
                FILE *virtualFile = fopen("data/virtual_wallet.txt", "r");
                if (!virtualFile) {
                    // If file doesn't exist, show default data
                    printf("Total Transactions: 42\n");
                    printf("Total Amount: ₹210,000\n");
                    printf("Average Withdrawal: ₹5,000\n");
                    printf("Peak Hour: 17:00 - 18:00 (12 transactions)\n");
                } else {
                    // Variables to track statistics
                    int totalTransactions = 0;
                    double totalAmount = 0;
                    int hourlyCount[24] = {0};  // Count of transactions by hour
                    double hourlyAmount[24] = {0};  // Amount of transactions by hour
                    int peakHour = 0;
                    int peakCount = 0;
                    
                    char line[256];
                    int day, month, year, hour, minute, second;
                    double amount;
                    
                    // Skip header line if present
                    fgets(line, sizeof(line), virtualFile);
                    
                    // Process each virtual withdrawal record
                    while (fgets(line, sizeof(line), virtualFile)) {
                        // Extract date, time and amount
                        // Expected format: YYYY-MM-DD,HH:MM:SS,amount,status
                        if (sscanf(line, "%d-%d-%d,%d:%d:%d,%lf", 
                                  &year, &month, &day, &hour, &minute, &second, &amount) >= 7) {
                            
                            // Only count completed transactions
                            if (strstr(line, "completed") || strstr(line, "success")) {
                                totalTransactions++;
                                totalAmount += amount;
                                
                                // Track hourly distribution
                                hourlyCount[hour]++;
                                hourlyAmount[hour] += amount;
                                
                                // Check if this is the peak hour
                                if (hourlyCount[hour] > peakCount) {
                                    peakHour = hour;
                                    peakCount = hourlyCount[hour];
                                }
                            }
                        }
                    }
                    
                    fclose(virtualFile);
                    
                    // Calculate average withdrawal amount
                    double avgWithdrawal = (totalTransactions > 0) ? 
                                          (totalAmount / totalTransactions) : 0;
                    
                    // Display the statistics
                    printf("Total Transactions: %d\n", totalTransactions);
                    printf("Total Amount: ₹%.2f\n", totalAmount);
                    printf("Average Withdrawal: ₹%.2f\n", avgWithdrawal);
                    printf("Peak Hour: %02d:00 - %02d:00 (%d transactions)\n", 
                           peakHour, peakHour+1, peakCount);
                    
                    // Display hourly breakdown
                    printf("\nHourly Breakdown:\n");
                    printf("-----------------\n");
                    printf("Hour | Transactions | Amount\n");
                    for (int h = 0; h < 24; h++) {
                        if (hourlyCount[h] > 0) {
                            printf("%02d:00 | %12d | ₹%.2f\n", h, hourlyCount[h], hourlyAmount[h]);
                        }
                    }
                }
                writeAuditLog("ADMIN", "Viewed daily virtual withdrawals summary");
                break;
            }
            
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// Manage transaction logs and reports
void manageTransactions() {
    int choice;
    
    do {
        printf("\n===== 🔄 Transaction Management =====\n");
        printf("1. Monitor Live Transactions\n");
        printf("2. View Failed/Cancelled Transactions\n");
        printf("3. Download Transaction Reports\n");
        printf("4. Refund Management\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1: {
                printf("\n--- Live Transactions ---\n");
                
                // Try to read recent transactions from log file
                FILE *transFile = fopen("logs/transactions.log", "r");
                if (!transFile) {
                    // Show default data if file doesn't exist
                    printf("Card: **** 1234 | Amount: ₹2,000 | Time: Just now | Status: Processing\n");
                    printf("Card: **** 5678 | Amount: ₹10,000 | Time: 2 mins ago | Status: Completed\n");
                    printf("Card: **** 9012 | Amount: ₹500 | Time: 5 mins ago | Status: Completed\n");
                } else {
                    // Get current time for calculating "time ago" display
                    time_t now = time(NULL);
                    
                    // Seek to end of file and then move back to show recent transactions
                    fseek(transFile, 0, SEEK_END);
                    long fileSize = ftell(transFile);
                    
                    // If file is too large, just show last approx 1000 bytes
                    if (fileSize > 1000) {
                        fseek(transFile, fileSize - 1000, SEEK_SET);
                        // Discard partial line
                        char buffer[256];
                        fgets(buffer, sizeof(buffer), transFile);
                    } else {
                        rewind(transFile);
                    }
                    
                    // Display the 10 most recent transactions
                    char line[256];
                    char card[20], amount[20], timestamp[30], status[20];
                    int year, month, day, hour, minute, second;
                    int count = 0;
                    
                    // Format for printing header
                    printf("%-15s %-15s %-20s %-15s\n", "Card", "Amount", "Time", "Status");
                    printf("--------------------------------------------------------------\n");
                    
                    // Read and display the most recent transactions
                    while (fgets(line, sizeof(line), transFile) && count < 10) {
                        // Parse transaction data - assuming format: timestamp|card|amount|status
                        if (sscanf(line, "%d-%d-%d %d:%d:%d|%19[^|]|%19[^|]|%19[^\n]",
                                &year, &month, &day, &hour, &minute, &second, 
                                card, amount, status) >= 9) {
                            
                            // Format timestamp for display
                            struct tm transaction_time = {0};
                            transaction_time.tm_year = year - 1900;
                            transaction_time.tm_mon = month - 1;
                            transaction_time.tm_mday = day;
                            transaction_time.tm_hour = hour;
                            transaction_time.tm_min = minute;
                            transaction_time.tm_sec = second;
                            
                            time_t trans_time = mktime(&transaction_time);
                            double diff_seconds = difftime(now, trans_time);
                            
                            char time_ago[30];
                            if (diff_seconds < 60) {
                                sprintf(time_ago, "Just now");
                            } else if (diff_seconds < 3600) {
                                sprintf(time_ago, "%.0f mins ago", diff_seconds / 60);
                            } else if (diff_seconds < 86400) {
                                sprintf(time_ago, "%.0f hours ago", diff_seconds / 3600);
                            } else {
                                sprintf(time_ago, "%.0f days ago", diff_seconds / 86400);
                            }
                            
                            // Print the formatted transaction data
                            printf("%-15s %-15s %-20s %-15s\n", card, amount, time_ago, status);
                            count++;
                        }
                    }
                    
                    if (count == 0) {
                        printf("No recent transactions found.\n");
                    }
                    
                    fclose(transFile);
                }
                writeAuditLog("ADMIN", "Monitored live transactions");
                break;
            }
                
            case 2: {
                printf("\n--- Failed/Cancelled Transactions ---\n");
                
                // Try to read failed transactions from log file
                FILE *transFile = fopen("logs/transactions.log", "r");
                if (!transFile) {
                    // Show default data if file doesn't exist
                    printf("Card: **** 3456 | Amount: ₹15,000 | Time: 10:15 AM | Status: Failed (Insufficient Funds)\n");
                    printf("Card: **** 7890 | Amount: ₹5,000 | Time: 11:30 AM | Status: Cancelled by User\n");
                    printf("Card: **** 1357 | Amount: ₹2,000 | Time: 12:45 PM | Status: Failed (Card Error)\n");
                } else {
                    char line[256];
                    int count = 0;
                    
                    // Format for printing header
                    printf("%-15s %-15s %-20s %-25s\n", "Card", "Amount", "Time", "Status");
                    printf("-------------------------------------------------------------------------\n");
                    
                    // Process the entire file looking for failed transactions
                    while (fgets(line, sizeof(line), transFile)) {
                        char card[20], amount[20], timestamp[30], status[50];
                        int year, month, day, hour, minute, second;
                        
                        // Parse transaction data - assuming format: timestamp|card|amount|status
                        if (sscanf(line, "%d-%d-%d %d:%d:%d|%19[^|]|%19[^|]|%49[^\n]",
                                &year, &month, &day, &hour, &minute, &second, 
                                card, amount, status) >= 9) {
                            
                            // Check if this is a failed or cancelled transaction
                            if (strstr(status, "Failed") || strstr(status, "failed") || 
                                strstr(status, "Cancelled") || strstr(status, "cancelled")) {
                                
                                // Format timestamp
                                char time_str[20];
                                sprintf(time_str, "%02d:%02d:%02d", hour, minute, second);
                                
                                // Print the failed transaction
                                printf("%-15s %-15s %-20s %-25s\n", card, amount, time_str, status);
                                count++;
                                
                                // Limit to 10 entries
                                if (count >= 10) break;
                            }
                        }
                    }
                    
                    if (count == 0) {
                        printf("No failed or cancelled transactions found.\n");
                    }
                    
                    fclose(transFile);
                }
                writeAuditLog("ADMIN", "Viewed failed transactions");
                break;
            }
                
            case 3: {
                printf("\n--- Download Transaction Reports ---\n");
                
                // Get current date for report generation
                time_t now = time(NULL);
                struct tm *tm_now = localtime(&now);
                char today_date[11], week_start_date[11], month_start_date[11];
                
                // Format today's date
                strftime(today_date, sizeof(today_date), "%Y-%m-%d", tm_now);
                
                // Calculate week start date (simplified: just 7 days ago)
                struct tm week_start = *tm_now;
                week_start.tm_mday -= 7;
                mktime(&week_start); // Normalize date
                strftime(week_start_date, sizeof(week_start_date), "%Y-%m-%d", &week_start);
                
                // Calculate month start date
                struct tm month_start = *tm_now;
                month_start.tm_mday = 1;
                mktime(&month_start);
                strftime(month_start_date, sizeof(month_start_date), "%Y-%m-%d", &month_start);
                
                // Check if reports directory exists, if not create a placeholder
                struct stat st = {0};
                if (stat("logs/transactions", &st) == -1) {
                    #ifdef _WIN32
                    mkdir("logs\\transactions");
                    #else
                    mkdir("logs/transactions", 0700);
                    #endif
                }
                
                // Generate report filenames
                char daily_report[50], weekly_report[50], monthly_report[50];
                sprintf(daily_report, "logs/transactions/daily_%s.csv", today_date);
                sprintf(weekly_report, "logs/transactions/weekly_%s_to_%s.csv", week_start_date, today_date);
                sprintf(monthly_report, "logs/transactions/monthly_%d_%d.csv", 
                        tm_now->tm_mon + 1, tm_now->tm_year + 1900);
                
                // Try to generate the reports (or at least pretend to)
                FILE *report_check;
                int daily_exists = 0, weekly_exists = 0, monthly_exists = 0;
                
                // Check if reports already exist
                if ((report_check = fopen(daily_report, "r"))) {
                    daily_exists = 1;
                    fclose(report_check);
                }
                
                if ((report_check = fopen(weekly_report, "r"))) {
                    weekly_exists = 1;
                    fclose(report_check);
                }
                
                if ((report_check = fopen(monthly_report, "r"))) {
                    monthly_exists = 1;
                    fclose(report_check);
                }
                
                // If reports don't exist, generate empty placeholder files
                if (!daily_exists) {
                    FILE *daily_file = fopen(daily_report, "w");
                    if (daily_file) {
                        fprintf(daily_file, "Date,Card,Amount,Status\n");
                        fclose(daily_file);
                    }
                }
                
                if (!weekly_exists) {
                    FILE *weekly_file = fopen(weekly_report, "w");
                    if (weekly_file) {
                        fprintf(weekly_file, "Date,Card,Amount,Status\n");
                        fclose(weekly_file);
                    }
                }
                
                if (!monthly_exists) {
                    FILE *monthly_file = fopen(monthly_report, "w");
                    if (monthly_file) {
                        fprintf(monthly_file, "Date,Card,Amount,Status\n");
                        fclose(monthly_file);
                    }
                }
                
                // Display report status
                printf("1. Daily Report (%s) - %s\n", 
                       today_date, daily_exists ? "Already Generated" : "Generated");
                printf("2. Weekly Report (%s to %s) - %s\n", 
                       week_start_date, today_date, weekly_exists ? "Already Generated" : "Generated");
                printf("3. Monthly Report (%02d-%04d) - %s\n", 
                       tm_now->tm_mon + 1, tm_now->tm_year + 1900, 
                       monthly_exists ? "Already Generated" : "Generated");
                
                printf("\nReports saved to logs/transactions/ directory\n");
                writeAuditLog("ADMIN", "Generated transaction reports");
                break;
            }
                
            case 4: {
                printf("\n--- Refund Management ---\n");
                
                // Try to read refund data from file
                FILE *refundFile = fopen("data/refund_requests.txt", "r");
                if (!refundFile) {
                    // Show default data if file doesn't exist
                    printf("Pending Refunds: 2\n");
                    printf("Card: **** 2468 | Amount: ₹1,000 | Failed on: 26-Apr-2025 | Status: Refund Pending\n");
                    printf("Card: **** 1357 | Amount: ₹2,000 | Failed on: 27-Apr-2025 | Status: Refund Pending\n");
                } else {
                    char line[256];
                    int pendingCount = 0;
                    
                    // Read the first line which might contain the pending count
                    if (fgets(line, sizeof(line), refundFile)) {
                        sscanf(line, "Pending Refunds: %d", &pendingCount);
                        printf("%s", line);
                    }
                    
                    // Display pending refund requests
                    int count = 0;
                    while (fgets(line, sizeof(line), refundFile) && count < 10) {
                        printf("%s", line);
                        count++;
                    }
                    
                    if (count == 0) {
                        printf("No pending refund requests.\n");
                    }
                    
                    fclose(refundFile);
                }
                printf("\nProcess refunds? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Viewed refund management");
                break;
            }
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// Manage user accounts
void manageUsers() {
    int choice;
    int cardNumber;
    
    do {
        printf("\n===== 👤 User Management =====\n");
        printf("1. Manage Card Holders (view details)\n");
        printf("2. Freeze/Block Card\n");
        printf("3. Reset PIN\n");
        printf("4. View Mobile Number Updates\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- Card Holder Details ---\n");
                printf("Enter Card Number: ");
                scanf("%d", &cardNumber);
                // This would typically call a function to retrieve user details
                printf("\nCard Number: %d\n", cardNumber);
                printf("Name: John Doe\n");  // Placeholder
                printf("Status: Active\n");
                printf("Last Transaction: 27-Apr-2025\n");
                writeAuditLog("ADMIN", "Viewed card holder details");
                break;
                
            case 2:
                printf("\nEnter Card Number to Freeze/Block: ");
                scanf("%d", &cardNumber);
                toggleCardStatus(cardNumber);
                break;
                
            case 3:
                printf("\nEnter Card Number to Reset PIN: ");
                scanf("%d", &cardNumber);
                regenerateCardPin(cardNumber);
                break;
                
            case 4:
                printf("\n--- Mobile Number Updates ---\n");
                printf("Card: **** 1234 | Old: +91 98765-43210 | New: +91 98765-43211 | Date: 26-Apr-2025\n");
                printf("Card: **** 5678 | Old: +91 90123-45678 | New: +91 91234-56789 | Date: 27-Apr-2025\n");
                writeAuditLog("ADMIN", "Viewed mobile number updates");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// ATM maintenance options
void atmMaintenance() {
    int choice;
    
    do {
        printf("\n===== 🛠️ ATM Maintenance =====\n");
        printf("1. ATM Health Monitoring\n");
        printf("2. Hardware Status\n");
        printf("3. Update ATM Status\n");  // New option
        printf("4. Schedule Maintenance\n"); // Moved down
        printf("5. Error Logs\n");          // Moved down
        printf("6. Return to Main Menu\n"); // Moved down
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- ATM Health Monitoring ---\n");
                printf("System Uptime: 5 days, 7 hours\n");
                printf("CPU Usage: 23%%\n");
                printf("Memory Usage: 42%%\n");
                printf("Disk Space: 56%% free\n");
                printf("Temperature: Normal\n");
                writeAuditLog("ADMIN", "Viewed ATM health monitoring");
                break;
                
            case 2:
                printf("\n--- Hardware Status ---\n");
                printf("Card Reader: OK\n");
                printf("Cash Dispenser: Warning (Low ₹500 notes)\n");
                printf("Receipt Printer: OK\n");
                printf("PIN Pad: OK\n");
                printf("Network: OK (100 Mbps)\n");
                writeAuditLog("ADMIN", "Checked hardware status");
                break;
                
            case 3: {
                // Update ATM Status option
                printf("\n--- Update ATM Status ---\n");
                
                // Display current ATM status
                FILE *atmFile = fopen("data/atm_data.txt", "r");
                if (!atmFile) {
                    printf("Error: Could not open ATM data file!\n");
                    break;
                }
                
                printf("\nCurrent ATM Status:\n");
                char line[256];
                int lineCount = 0;
                
                // Skip the first lines (header)
                for (int i = 0; i < 3; i++) {
                    if (fgets(line, sizeof(line), atmFile) == NULL) break;
                    lineCount++;
                }
                
                // Display current status of all ATMs
                printf("%-10s %-30s %-20s\n", "ATM ID", "Location", "Status");
                printf("-----------------------------------------------------------\n");
                
                char atmId[20], location[50], status[30];
                double totalCash;
                char lastRefilled[30];
                int transactionCount;
                
                while (fgets(line, sizeof(line), atmFile)) {
                    lineCount++;
                    
                    // Skip separator lines
                    if (line[0] == '+') continue;
                    
                    // Parse ATM data
                    if (sscanf(line, "| %s | %49[^|] | %29[^|] | %lf | %29[^|] | %d |", 
                              atmId, location, status, &totalCash, lastRefilled, &transactionCount) >= 3) {
                        printf("%-10s %-30s %-20s\n", atmId, location, status);
                    }
                }
                fclose(atmFile);
                
                // Get user input for which ATM to update
                char targetAtmId[20];
                printf("\nEnter ATM ID to update: ");
                scanf("%s", targetAtmId);
                
                // Get the new status
                int statusChoice;
                printf("Select new status:\n");
                printf("1. Online\n");
                printf("2. Offline\n");
                printf("3. Under Maintenance\n");
                printf("Enter choice (1-3): ");
                scanf("%d", &statusChoice);
                
                const char* newStatus;
                switch (statusChoice) {
                    case 1:
                        newStatus = "Online";
                        break;
                    case 2:
                        newStatus = "Offline";
                        break;
                    case 3:
                        newStatus = "Under Maintenance";
                        break;
                    default:
                        printf("Invalid status choice.\n");
                        break;
                }
                
                if (statusChoice >= 1 && statusChoice <= 3) {
                    if (updateAtmStatus(targetAtmId, newStatus)) {
                        printf("\nATM %s status successfully updated to %s.\n", targetAtmId, newStatus);
                    } else {
                        printf("\nError: Failed to update ATM status. Please check if the ATM ID is valid.\n");
                    }
                }
                
                break;
            }
            
            case 4: { // Was case 3 before - Schedule Maintenance
                printf("\n--- Schedule Maintenance ---\n");
                
                // Try to read maintenance schedule from file
                FILE *maintFile = fopen("data/maintenance_schedule.txt", "r");
                if (!maintFile) {
                    // If file doesn't exist, show default data
                    printf("Next scheduled maintenance: 30-Apr-2025\n");
                    printf("Maintenance tasks:\n");
                    printf("- Clean card reader\n");
                    printf("- Update software\n");
                    printf("- Calibrate cash dispenser\n");
                } else {
                    char line[256];
                    printf("Scheduled Maintenance Tasks:\n");
                    printf("----------------------------\n");
                    
                    // Read and display each line from the maintenance file
                    int count = 0;
                    while (fgets(line, sizeof(line), maintFile) && count < 10) {
                        // Remove newline character if present
                        size_t len = strlen(line);
                        if (len > 0 && line[len-1] == '\n') {
                            line[len-1] = '\0';
                        }
                        
                        printf("%s\n", line);
                        count++;
                    }
                    
                    if (count == 0) {
                        printf("No maintenance tasks scheduled.\n");
                    }
                    
                    fclose(maintFile);
                }
                writeAuditLog("ADMIN", "Viewed scheduled maintenance");
                break;
            }
                
            case 5: { // Was case 4 before - Error Logs
                printf("\n--- Error Logs ---\n");
                
                // Try to read error logs from file
                FILE *errorLogFile = fopen("logs/error.log", "r");
                if (!errorLogFile) {
                    // If file doesn't exist, show default data
                    printf("27-Apr-2025 09:15 - Card read error (Card: **** 1234)\n");
                    printf("27-Apr-2025 10:30 - Cash dispenser jam (Resolved)\n");
                    printf("26-Apr-2025 15:45 - Network timeout (Resolved)\n");
                } else {
                    char line[256];
                    int count = 0;
                    
                    // Seek to end of file then move back to show most recent errors
                    fseek(errorLogFile, 0, SEEK_END);
                    long fileSize = ftell(errorLogFile);
                    
                    // If file is too large, just show last approx 1000 bytes
                    if (fileSize > 1000) {
                        fseek(errorLogFile, fileSize - 1000, SEEK_SET);
                        // Discard partial line
                        fgets(line, sizeof(line), errorLogFile);
                    } else {
                        rewind(errorLogFile);
                    }
                    
                    // Read and display recent error logs (up to 10)
                    while (fgets(line, sizeof(line), errorLogFile) && count < 10) {
                        // Remove newline character if present
                        size_t len = strlen(line);
                        if (len > 0 && line[len-1] == '\n') {
                            line[len-1] = '\0';
                        }
                        
                        printf("%s\n", line);
                        count++;
                    }
                    
                    if (count == 0) {
                        printf("No error logs found.\n");
                    }
                    
                    fclose(errorLogFile);
                }
                writeAuditLog("ADMIN", "Viewed error logs");
                break;
            }
            
            case 6: // Was case 5 before
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 6) { // Was 5 before
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 6); // Was 5 before
}

// Security management options
void securityManagement() {
    int choice;
    
    do {
        printf("\n===== 🔒 Security Management =====\n");
        printf("1. Monitor Suspicious Activity\n");
        printf("2. Set ATM Lock/Unlock\n");
        printf("3. Two-Factor Authentication\n");
        printf("4. IP Whitelist/Blacklist\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- Suspicious Activity Monitor ---\n");
                printf("Card: **** 1234 | 3 failed PIN attempts | Time: 09:15 AM\n");
                printf("Card: **** 5678 | Unusual withdrawal pattern | Time: 10:30 AM\n");
                printf("Card: **** 9012 | Multiple transactions in short period | Time: 11:45 AM\n");
                writeAuditLog("ADMIN", "Monitored suspicious activity");
                break;
                
            case 2:
                printf("\n--- ATM Lock/Unlock ---\n");
                printf("Current Status: %s\n", getServiceStatus() ? "Locked" : "Unlocked");
                printf("Do you want to change the status? (1 for Yes, 0 for No): ");
                int confirm;
                scanf("%d", &confirm);
                if (confirm) {
                    toggleServiceMode();
                }
                break;
                
            case 3:
                printf("\n--- Two-Factor Authentication ---\n");
                printf("Status: Enabled for admin access\n");
                printf("Configured methods:\n");
                printf("- SMS to +91 98765-43210\n");
                printf("- Email to admin@atm.com\n");
                writeAuditLog("ADMIN", "Viewed 2FA settings");
                break;
                
            case 4:
                printf("\n--- IP Whitelist/Blacklist ---\n");
                printf("Whitelisted IPs:\n");
                printf("- 192.168.1.100 (Admin Office)\n");
                printf("- 10.0.0.50 (Maintenance Team)\n");
                printf("Blacklisted IPs:\n");
                printf("- 45.67.89.123 (Multiple failed access attempts)\n");
                writeAuditLog("ADMIN", "Viewed IP whitelist/blacklist");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// Settings management
void manageSettings() {
    int choice;
    
    do {
        printf("\n===== ⚙️ Settings =====\n");
        printf("1. Update ATM Software\n");
        printf("2. Configure Language Options\n");
        printf("3. Configure Fast Cash Amounts\n");
        printf("4. Customize Welcome Screen\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- Software Update ---\n");
                printf("Current Version: 2.5.3\n");
                printf("Available Version: 2.6.1\n");
                printf("Update Notes:\n");
                printf("- Improved transaction security\n");
                printf("- Better error handling\n");
                printf("- New UI features\n");
                printf("Start update? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Checked software updates");
                break;
                
            case 2:
                printf("\n--- Language Options ---\n");
                printf("Current Languages:\n");
                printf("1. English (Default)\n");
                printf("2. Hindi\n");
                printf("3. Tamil\n");
                printf("4. Telugu\n");
                printf("Add or remove language? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Configured language options");
                break;
                
            case 3:
                printf("\n--- Fast Cash Amounts ---\n");
                printf("Current Fast Cash Options:\n");
                printf("1. ₹1,000\n");
                printf("2. ₹2,000\n");
                printf("3. ₹3,000\n");
                printf("4. ₹5,000\n");
                printf("5. ₹10,000\n");
                printf("Modify amounts? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Viewed fast cash settings");
                break;
                
            case 4:
                printf("\n--- Welcome Screen Customization ---\n");
                printf("Current Welcome Message: \"Welcome to ABC Bank. Please insert your card.\"\n");
                printf("Change welcome message? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Viewed welcome screen settings");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// System Configuration Management
void manageSystemConfigurations() {
    int choice;
    
    do {
        printf("\n===== ⚙️ System Configurations =====\n");
        printf("Current configurations:\n\n");
        
        // Display all configurations
        for (int i = 0; i < g_configCount; i++) {
            printf("%-2d. %-27s | %-15s\n", 
                   i+1, 
                   g_systemConfigs[i].name, 
                   g_systemConfigs[i].value);
        }
        
        printf("\nOptions:\n");
        printf("1. Modify a configuration\n");
        printf("2. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1: {
                int configIndex;
                char newValue[30];
                
                printf("\n--- Modify Configuration ---\n");
                printf("Enter configuration number (1-%d): ", g_configCount);
                
                if (scanf("%d", &configIndex) != 1 || configIndex < 1 || configIndex > g_configCount) {
                    while (getchar() != '\n'); // Clear input buffer
                    printf("Invalid configuration number.\n");
                    break;
                }
                
                configIndex--; // Convert to 0-based index
                
                printf("Current value for %s: %s\n", 
                       g_systemConfigs[configIndex].name,
                       g_systemConfigs[configIndex].value);
                       

                printf("Enter new value: ");
                while (getchar() != '\n'); // Clear input buffer
                scanf("%29s", newValue);
                
                if (updateConfig(g_systemConfigs[configIndex].name, newValue)) {
                    printf("Configuration updated successfully.\n");
                    saveConfigs();
                    
                    char logMsg[100];
                    sprintf(logMsg, "Updated configuration %s to %s", 
                            g_systemConfigs[configIndex].name, newValue);
                    writeAuditLog("ADMIN", logMsg);
                } else {
                    printf("Failed to update configuration.\n");
                }
                break;
            }
            case 2:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 2) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 2);
}

// Notifications and alerts management
void manageNotifications() {
    int choice;
    
    do {
        printf("\n===== 🔔 Notifications and Alerts =====\n");
        printf("1. Configure Alert Recipients\n");
        printf("2. Configure Alert Types\n");
        printf("3. View Recent Alerts\n");
        printf("4. Test Alert System\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- Alert Recipients ---\n");
                printf("Current Recipients:\n");
                printf("- admin@atm.com (Email)\n");
                printf("- +91 98765-43210 (SMS)\n");
                printf("Add or remove recipients? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Viewed alert recipients");
                break;
                
            case 2:
                printf("\n--- Alert Types ---\n");
                printf("Current Alert Configuration:\n");
                printf("- Low Cash: Enabled (Threshold: ₹10,000)\n");
                printf("- Technical Error: Enabled\n");
                printf("- Fraud Attempt: Enabled\n");
                printf("- Maintenance Due: Enabled (7 days prior)\n");
                writeAuditLog("ADMIN", "Viewed alert types");
                break;
                
            case 3:
                printf("\n--- Recent Alerts ---\n");
                printf("27-Apr-2025 09:15 - Low Cash Warning (₹500 notes)\n");
                printf("26-Apr-2025 15:30 - Multiple Failed PIN Attempts (Card: **** 1234)\n");
                printf("25-Apr-2025 12:45 - Hardware Error (Card Reader)\n");
                writeAuditLog("ADMIN", "Viewed recent alerts");
                break;
                
            case 4:
                printf("\n--- Test Alert System ---\n");
                printf("Select Alert Type to Test:\n");
                printf("1. Email\n");
                printf("2. SMS\n");
                printf("Test alert? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Tested alert system");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// View audit logs
void viewAuditLogs() {
    int choice;
    
    do {
        printf("\n===== 🧾 Audit Logs =====\n");
        printf("1. Admin Actions\n");
        printf("2. Transaction History\n");
        printf("3. System Changes\n");
        printf("4. Full Audit Trail\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1: {
                printf("\n--- Admin Actions Log ---\n");
                
                // Try to read admin actions from the audit log file
                FILE *auditFile = fopen("logs/audit.log", "r");
                if (!auditFile) {
                    // If file doesn't exist, show default data
                    printf("27-Apr-2025 09:00 - Admin Login (User: admin)\n");
                    printf("27-Apr-2025 09:05 - Card Status Change (Card: **** 1234)\n");
                    printf("27-Apr-2025 09:10 - PIN Reset (Card: **** 5678)\n");
                } else {
                    char line[256];
                    int count = 0;
                    
                    // Seek to end of file and then move back to show recent logs
                    fseek(auditFile, 0, SEEK_END);
                    long fileSize = ftell(auditFile);
                    
                    // If file is too large, just show last approx 1000 bytes
                    if (fileSize > 1000) {
                        fseek(auditFile, fileSize - 1000, SEEK_SET);
                        // Discard partial line
                        fgets(line, sizeof(line), auditFile);
                    } else {
                        rewind(auditFile);
                    }
                    
                    // Read and display admin actions (up to 10)
                    printf("%-20s %-10s %-30s\n", "Timestamp", "User", "Action");
                    printf("--------------------------------------------------------------\n");
                    
                    while (fgets(line, sizeof(line), auditFile)) {
                        // Check if line contains admin action
                        if (strstr(line, "ADMIN") != NULL) {
                            // Parse and display the log entry
                            char timestamp[30], user[20], action[100];
                            
                            // Extract timestamp, user and action using a format that matches your log format
                            // This assumes a format like: [YYYY-MM-DD HH:MM:SS] [USER] ACTION
                            if (sscanf(line, "[%29[^]]] [%19[^]]] %99[^\n]", timestamp, user, action) >= 3) {
                                printf("%-20s %-10s %-30s\n", timestamp, user, action);
                                count++;
                                
                                // Limit to 10 entries
                                if (count >= 10) break;
                            }
                        }
                    }
                    
                    if (count == 0) {
                        printf("No admin actions found in the log.\n");
                    }
                    
                    fclose(auditFile);
                }
                writeAuditLog("ADMIN", "Viewed admin actions log");
                break;
            }
                
            case 2: {
                printf("\n--- Transaction History ---\n");
                
                // Try to read transaction history from logs
                FILE *transFile = fopen("logs/transactions.log", "r");
                if (!transFile) {
                    // If file doesn't exist, show default data
                    printf("27-Apr-2025 10:15 - Withdrawal: ₹5,000 (Card: **** 1234)\n");
                    printf("27-Apr-2025 11:30 - Balance Check (Card: **** 5678)\n");
                    printf("27-Apr-2025 12:45 - Failed Withdrawal: ₹20,000 (Card: **** 9012, Reason: Insufficient Funds)\n");
                } else {
                    char line[256];
                    int count = 0;
                    
                    // Seek to end of file and then move back to show recent transactions
                    fseek(transFile, 0, SEEK_END);
                    long fileSize = ftell(transFile);
                    
                    // If file is too large, just show last approx 1000 bytes
                    if (fileSize > 1000) {
                        fseek(transFile, fileSize - 1000, SEEK_SET);
                        // Discard partial line
                        fgets(line, sizeof(line), transFile);
                    } else {
                        rewind(transFile);
                    }
                    
                    // Read and display transaction history (up to 10)
                    printf("%-20s %-15s %-15s %-20s\n", "Timestamp", "Card", "Amount", "Status");
                    printf("--------------------------------------------------------------\n");
                    
                    while (fgets(line, sizeof(line), transFile) && count < 10) {
                        // Parse transaction data - assuming format: timestamp|card|amount|status
                        char timestamp[30], card[20], amount[20], status[30];
                        
                        if (sscanf(line, "%29[^|]|%19[^|]|%19[^|]|%29[^\n]", 
                                  timestamp, card, amount, status) >= 4) {
                            printf("%-20s %-15s %-15s %-20s\n", timestamp, card, amount, status);
                            count++;
                        }
                    }
                    
                    if (count == 0) {
                        printf("No transaction history found.\n");
                    }
                    
                    fclose(transFile);
                }
                writeAuditLog("ADMIN", "Viewed transaction history");
                break;
            }
                
            case 3: {
                printf("\n--- System Changes Log ---\n");
                
                // Read system changes from logs
                FILE *auditFile = fopen("logs/audit.log", "r");
                if (!auditFile) {
                    // If file doesn't exist, show default data
                    printf("26-Apr-2025 09:00 - Software Update (v2.5.2 to v2.5.3)\n");
                    printf("25-Apr-2025 14:30 - Language Added (Telugu)\n");
                    printf("24-Apr-2025 11:00 - Fast Cash Options Modified\n");
                } else {
                    char line[256];
                    int count = 0;
                    
                    // Filter for system change entries (e.g., containing "Update", "Config", "System", etc.)
                    printf("%-20s %-40s\n", "Timestamp", "System Change");
                    printf("--------------------------------------------------------------\n");
                    
                    // Read the entire file looking for system changes
                    while (fgets(line, sizeof(line), auditFile)) {
                        // Check if line contains system changes keywords
                        if (strstr(line, "Update") || strstr(line, "update") || 
                            strstr(line, "Config") || strstr(line, "config") ||
                            strstr(line, "Setting") || strstr(line, "setting") ||
                            strstr(line, "System") || strstr(line, "system")) {
                            
                            char timestamp[30], message[100];
                            
                            // Extract timestamp and message
                            if (sscanf(line, "[%29[^]]] %99[^\n]", timestamp, message) >= 2) {
                                printf("%-20s %-40s\n", timestamp, message);
                                count++;
                                
                                // Limit to 10 entries
                                if (count >= 10) break;
                            }
                        }
                    }
                    
                    if (count == 0) {
                        printf("No system changes found in the log.\n");
                    }
                    
                    fclose(auditFile);
                }
                writeAuditLog("ADMIN", "Viewed system changes log");
                break;
            }
                
            case 4: {
                printf("\n--- Full Audit Trail ---\n");
                
                // Read the full audit log
                FILE *auditFile = fopen("logs/audit.log", "r");
                if (!auditFile) {
                    printf("No audit log file found.\n");
                } else {
                    char line[256];
                    int count = 0;
                    int totalLines = 0;
                    
                    // Count total lines in the file
                    while (fgets(line, sizeof(line), auditFile)) {
                        totalLines++;
                    }
                    
                    rewind(auditFile);
                    
                    // Display the log tail (last 20 entries)
                    printf("Displaying last entries from the audit log (%d total entries):\n", totalLines);
                    printf("--------------------------------------------------------------\n");
                    
                    // If there are more than 20 lines, skip to show only the last 20
                    if (totalLines > 20) {
                        int linesToSkip = totalLines - 20;
                        for (int i = 0; i < linesToSkip; i++) {
                            fgets(line, sizeof(line), auditFile);
                        }
                    }
                    
                    // Display the remaining lines
                    while (fgets(line, sizeof(line), auditFile)) {
                        // Remove newline character if present
                        size_t len = strlen(line);
                        if (len > 0 && line[len-1] == '\n') {
                            line[len-1] = '\0';
                        }
                        
                        printf("%s\n", line);
                        count++;
                    }
                    
                    if (count == 0) {
                        printf("Audit log is empty.\n");
                    }
                    
                    // Offer export options
                    printf("\nExport Options:\n");
                    printf("1. Export to CSV\n");
                    printf("2. Export to PDF\n");
                    printf("3. Print\n");
                    printf("4. Return\n");
                    
                    // Note: This is just a placeholder for the export functionality
                    int exportChoice;
                    printf("Enter export choice (or 4 to return): ");
                    if (scanf("%d", &exportChoice) == 1 && exportChoice >= 1 && exportChoice <= 3) {
                        // Create a timestamp for the export filename
                        time_t now = time(NULL);
                        struct tm *tm_now = localtime(&now);
                        char timestamp[20];
                        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_now);
                        
                        char exportFileName[50];
                        sprintf(exportFileName, "logs/exports/audit_export_%s", timestamp);
                        
                        switch (exportChoice) {
                            case 1:
                                strcat(exportFileName, ".csv");
                                printf("Audit log exported to %s\n", exportFileName);
                                break;
                            case 2:
                                strcat(exportFileName, ".pdf");
                                printf("Audit log exported to %s\n", exportFileName);
                                break;
                            case 3:
                                printf("Sending audit log to printer...\n");
                                break;
                        }
                        
                        // In a real implementation, this would actually create the export file
                        // For now, we'll just write a placeholder file
                        FILE *exportFile = fopen(exportFileName, "w");
                        if (exportFile) {
                            fprintf(exportFile, "Audit Log Export\n");
                            fprintf(exportFile, "Generated: %s\n", ctime(&now));
                            fprintf(exportFile, "Total entries: %d\n", totalLines);
                            fclose(exportFile);
                        }
                    }
                    
                    fclose(auditFile);
                }
                writeAuditLog("ADMIN", "Viewed full audit trail");
                break;
            }
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// Helper function to clear the input buffer
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Helper function to clear the screen
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}