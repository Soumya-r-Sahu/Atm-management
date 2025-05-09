/**
 * @file transaction_reports.c
 * @brief Implementation of transaction reporting functions for Core Banking System
 */

#include "../../../include/common/database/core_banking_interface.h"
#include "../../../include/common/database/db_config.h"
#include "../../../include/common/database/database_conn.h"
#include "../../../include/common/utils/logger.h"
#include "../../../include/common/reporting/transaction_reports.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/**
 * @brief Generate a daily transaction report
 * @param report_date Date for the report (format: YYYY-MM-DD)
 * @param report_path Path to save the report
 * @return true if successful, false otherwise
 */
bool cbs_generate_daily_transaction_report(const char* report_date, const char* report_path) {
    if (!report_date || !report_path) {
        writeErrorLog("Invalid parameters in cbs_generate_daily_transaction_report");
        return false;
    }
    
    // Get MySQL connection
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_generate_daily_transaction_report");
        return false;
    }
    
    char query[500];
    sprintf(query,
            "SELECT t.transaction_id, t.account_number, t.transaction_type, "
            "t.amount, t.balance_before, t.balance_after, "
            "t.transaction_date, t.status, t.remarks "
            "FROM cbs_transactions t "
            "WHERE DATE(t.transaction_date) = '%s' "
            "ORDER BY t.transaction_date",
            report_date);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query transactions: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store result: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    // Calculate transaction totals
    int total_transactions = mysql_num_rows(result);
    if (total_transactions <= 0) {
        writeErrorLog("No transactions found for date: %s", report_date);
        mysql_free_result(result);
        db_release_connection(conn);
        return false;
    }
    
    // Create report file
    FILE *report_file = fopen(report_path, "w");
    if (!report_file) {
        writeErrorLog("Failed to create report file: %s", report_path);
        mysql_free_result(result);
        db_release_connection(conn);
        return false;
    }
    
    // Write report header
    fprintf(report_file, "====================================================\n");
    fprintf(report_file, "        CORE BANKING SYSTEM - DAILY TRANSACTION REPORT\n");
    fprintf(report_file, "====================================================\n\n");
    fprintf(report_file, "Date: %s\n", report_date);
    fprintf(report_file, "Report Generated: %s\n\n", __DATE__);
    fprintf(report_file, "Total Transactions: %d\n\n", total_transactions);
    
    // Write transaction summary section
    fprintf(report_file, "TRANSACTION SUMMARY\n");
    fprintf(report_file, "----------------------------------------------------\n");
    
    // Get summary data with another query
    sprintf(query,
            "SELECT transaction_type, COUNT(*) as count, SUM(amount) as total_amount "
            "FROM cbs_transactions "
            "WHERE DATE(transaction_date) = '%s' "
            "GROUP BY transaction_type",
            report_date);
    
    mysql_free_result(result);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query transaction summary: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store summary result: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    // Write summary by transaction type
    fprintf(report_file, "%-20s %-15s %-15s\n", "Transaction Type", "Count", "Total Amount");
    fprintf(report_file, "----------------------------------------------------\n");
    
    double total_amount = 0.0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        if (row[0] && row[1] && row[2]) {
            fprintf(report_file, "%-20s %-15s $%-14s\n", 
                   row[0], row[1], row[2]);
            total_amount += atof(row[2]);
        }
    }
    
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "%-20s %-15d $%-14.2f\n", "TOTAL", total_transactions, total_amount);
    fprintf(report_file, "\n\n");
    
    mysql_free_result(result);
    
    // Get detailed transaction list
    sprintf(query,
            "SELECT t.transaction_id, t.account_number, t.transaction_type, "
            "t.amount, t.balance_before, t.balance_after, "
            "t.transaction_date, t.status, t.remarks "
            "FROM cbs_transactions t "
            "WHERE DATE(t.transaction_date) = '%s' "
            "ORDER BY t.transaction_date",
            report_date);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query detailed transactions: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store detailed result: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    // Write transaction details
    fprintf(report_file, "TRANSACTION DETAILS\n");
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "%-15s %-20s %-15s %-10s %-15s %-10s\n", 
           "Time", "Transaction ID", "Type", "Amount", "Account", "Status");
    fprintf(report_file, "----------------------------------------------------\n");
    
    while ((row = mysql_fetch_row(result))) {
        if (row[0] && row[1] && row[2] && row[3] && row[6] && row[7]) {
            // Extract only the time portion from the timestamp
            char time_only[10] = {0};
            strncpy(time_only, row[6] + 11, 8);  // Assumes format is "YYYY-MM-DD HH:MM:SS"
            
            fprintf(report_file, "%-15s %-20s %-15s $%-9s %-15s %-10s\n", 
                   time_only, row[0], row[2], row[3], row[1], row[7]);
        }
    }
    
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "\nEnd of Report\n");
    
    // Clean up
    mysql_free_result(result);
    fclose(report_file);
    db_release_connection(conn);
    
    writeInfoLog("Daily transaction report generated for %s at %s", report_date, report_path);
    return true;
}

/**
 * @brief Generate a card usage report
 * @param start_date Start date for the report (format: YYYY-MM-DD)
 * @param end_date End date for the report (format: YYYY-MM-DD)
 * @param report_path Path to save the report
 * @return true if successful, false otherwise
 */
bool cbs_generate_card_usage_report(const char* start_date, const char* end_date, const char* report_path) {
    if (!start_date || !end_date || !report_path) {
        writeErrorLog("Invalid parameters in cbs_generate_card_usage_report");
        return false;
    }
    
    // Get MySQL connection
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_generate_card_usage_report");
        return false;
    }
    
    // Create report file
    FILE *report_file = fopen(report_path, "w");
    if (!report_file) {
        writeErrorLog("Failed to create report file: %s", report_path);
        db_release_connection(conn);
        return false;
    }
    
    // Write report header
    fprintf(report_file, "====================================================\n");
    fprintf(report_file, "           CORE BANKING SYSTEM - CARD USAGE REPORT\n");
    fprintf(report_file, "====================================================\n\n");
    fprintf(report_file, "Period: %s to %s\n", start_date, end_date);
    fprintf(report_file, "Report Generated: %s\n\n", __DATE__);
    
    // Query for card usage statistics
    char query[1000];
    sprintf(query,
            "SELECT c.card_number, c.card_type, c.status, "
            "COUNT(DISTINCT t.transaction_id) as transaction_count, "
            "COALESCE(SUM(t.amount), 0) as total_amount "
            "FROM cbs_cards c "
            "LEFT JOIN cbs_accounts a ON c.account_number = a.account_number "
            "LEFT JOIN cbs_transactions t ON a.account_number = t.account_number "
            "AND DATE(t.transaction_date) BETWEEN '%s' AND '%s' "
            "GROUP BY c.card_number "
            "ORDER BY transaction_count DESC",
            start_date, end_date);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query card usage: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store result: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    int total_cards = mysql_num_rows(result);
    fprintf(report_file, "Total Cards: %d\n\n", total_cards);
    
    // Write card usage details
    fprintf(report_file, "CARD USAGE SUMMARY\n");
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "%-16s %-10s %-10s %-15s %-15s\n", 
           "Card Number", "Type", "Status", "Transactions", "Total Amount");
    fprintf(report_file, "----------------------------------------------------\n");
    
    int total_transactions = 0;
    double total_amount = 0.0;
    int active_cards = 0;
    int inactive_cards = 0;
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        if (row[0] && row[1] && row[2] && row[3] && row[4]) {
            fprintf(report_file, "%-16s %-10s %-10s %-15s $%-14s\n", 
                   row[0], row[1], row[2], row[3], row[4]);
            
            int trans_count = atoi(row[3]);
            double amount = atof(row[4]);
            
            total_transactions += trans_count;
            total_amount += amount;
            
            if (strcmp(row[2], "ACTIVE") == 0) {
                active_cards++;
            } else {
                inactive_cards++;
            }
        }
    }
    
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "%-16s %-21s %-15d $%-14.2f\n", 
           "TOTAL", "", total_transactions, total_amount);
    fprintf(report_file, "\n\n");
    
    // Card status summary
    fprintf(report_file, "CARD STATUS SUMMARY\n");
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "Active Cards: %d (%.1f%%)\n", 
           active_cards, (active_cards * 100.0) / total_cards);
    fprintf(report_file, "Inactive/Blocked Cards: %d (%.1f%%)\n", 
           inactive_cards, (inactive_cards * 100.0) / total_cards);
    fprintf(report_file, "Total Cards: %d\n", total_cards);
    fprintf(report_file, "\n\n");
    
    mysql_free_result(result);
    
    // Get transaction type distribution for cards
    sprintf(query,
            "SELECT t.transaction_type, COUNT(*) as count "
            "FROM cbs_transactions t "
            "JOIN cbs_accounts a ON t.account_number = a.account_number "
            "JOIN cbs_cards c ON a.account_number = c.account_number "
            "WHERE DATE(t.transaction_date) BETWEEN '%s' AND '%s' "
            "GROUP BY t.transaction_type "
            "ORDER BY count DESC",
            start_date, end_date);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query transaction types: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store transaction types result: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    // Write transaction type distribution
    fprintf(report_file, "TRANSACTION TYPE DISTRIBUTION\n");
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "%-20s %-15s %-15s\n", 
           "Transaction Type", "Count", "Percentage");
    fprintf(report_file, "----------------------------------------------------\n");
    
    while ((row = mysql_fetch_row(result))) {
        if (row[0] && row[1]) {
            int type_count = atoi(row[1]);
            double percentage = (type_count * 100.0) / total_transactions;
            
            fprintf(report_file, "%-20s %-15s %.1f%%\n", 
                   row[0], row[1], percentage);
        }
    }
    
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "\nEnd of Report\n");
    
    // Clean up
    mysql_free_result(result);
    fclose(report_file);
    db_release_connection(conn);
    
    writeInfoLog("Card usage report generated for %s to %s at %s", start_date, end_date, report_path);
    return true;
}

/**
 * @brief Generate an account status report
 * @param report_path Path to save the report
 * @return true if successful, false otherwise
 */
bool cbs_generate_account_status_report(const char* report_path) {
    if (!report_path) {
        writeErrorLog("Invalid parameters in cbs_generate_account_status_report");
        return false;
    }
    
    // Get MySQL connection
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_generate_account_status_report");
        return false;
    }
    
    // Create report file
    FILE *report_file = fopen(report_path, "w");
    if (!report_file) {
        writeErrorLog("Failed to create report file: %s", report_path);
        db_release_connection(conn);
        return false;
    }
    
    // Write report header
    fprintf(report_file, "====================================================\n");
    fprintf(report_file, "        CORE BANKING SYSTEM - ACCOUNT STATUS REPORT\n");
    fprintf(report_file, "====================================================\n\n");
    fprintf(report_file, "Report Generated: %s\n\n", __DATE__);
    
    // Query for account summary statistics
    char query[500];
    sprintf(query,
            "SELECT COUNT(*) as total_accounts, "
            "SUM(CASE WHEN status = 'ACTIVE' THEN 1 ELSE 0 END) as active_accounts, "
            "SUM(CASE WHEN status = 'INACTIVE' THEN 1 ELSE 0 END) as inactive_accounts, "
            "SUM(CASE WHEN status = 'CLOSED' THEN 1 ELSE 0 END) as closed_accounts, "
            "SUM(balance) as total_balance "
            "FROM cbs_accounts");
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query account summary: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store result: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row || !row[0] || !row[1] || !row[2] || !row[3] || !row[4]) {
        writeErrorLog("Failed to fetch account summary data");
        mysql_free_result(result);
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    int total_accounts = atoi(row[0]);
    int active_accounts = atoi(row[1]);
    int inactive_accounts = atoi(row[2]);
    int closed_accounts = atoi(row[3]);
    double total_balance = atof(row[4]);
    
    // Write account summary
    fprintf(report_file, "ACCOUNT SUMMARY\n");
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "Total Accounts: %d\n", total_accounts);
    fprintf(report_file, "Active Accounts: %d (%.1f%%)\n", 
           active_accounts, (active_accounts * 100.0) / total_accounts);
    fprintf(report_file, "Inactive Accounts: %d (%.1f%%)\n", 
           inactive_accounts, (inactive_accounts * 100.0) / total_accounts);
    fprintf(report_file, "Closed Accounts: %d (%.1f%%)\n", 
           closed_accounts, (closed_accounts * 100.0) / total_accounts);
    fprintf(report_file, "Total Balance: $%.2f\n", total_balance);
    fprintf(report_file, "Average Balance: $%.2f\n", total_balance / total_accounts);
    fprintf(report_file, "\n\n");
    
    mysql_free_result(result);
    
    // Query for account type distribution
    sprintf(query,
            "SELECT account_type, COUNT(*) as count, SUM(balance) as total_balance "
            "FROM cbs_accounts "
            "GROUP BY account_type "
            "ORDER BY count DESC");
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query account types: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store account types result: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    // Write account type distribution
    fprintf(report_file, "ACCOUNT TYPE DISTRIBUTION\n");
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "%-15s %-10s %-15s %-15s\n", 
           "Account Type", "Count", "Percentage", "Total Balance");
    fprintf(report_file, "----------------------------------------------------\n");
    
    while ((row = mysql_fetch_row(result))) {
        if (row[0] && row[1] && row[2]) {
            int type_count = atoi(row[1]);
            double percentage = (type_count * 100.0) / total_accounts;
            double type_balance = atof(row[2]);
            
            fprintf(report_file, "%-15s %-10s %.1f%%         $%-14.2f\n", 
                   row[0], row[1], percentage, type_balance);
        }
    }
    
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "\n\n");
    
    mysql_free_result(result);
    
    // Query for accounts by balance range
    sprintf(query,
            "SELECT "
            "SUM(CASE WHEN balance < 100 THEN 1 ELSE 0 END) as low_balance, "
            "SUM(CASE WHEN balance >= 100 AND balance < 1000 THEN 1 ELSE 0 END) as medium_balance, "
            "SUM(CASE WHEN balance >= 1000 AND balance < 10000 THEN 1 ELSE 0 END) as high_balance, "
            "SUM(CASE WHEN balance >= 10000 THEN 1 ELSE 0 END) as very_high_balance "
            "FROM cbs_accounts "
            "WHERE status = 'ACTIVE'");
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query balance ranges: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store balance ranges result: %s", mysql_error(conn));
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    row = mysql_fetch_row(result);
    if (!row || !row[0] || !row[1] || !row[2] || !row[3]) {
        writeErrorLog("Failed to fetch balance range data");
        mysql_free_result(result);
        fclose(report_file);
        db_release_connection(conn);
        return false;
    }
    
    int low_balance = atoi(row[0]);
    int medium_balance = atoi(row[1]);
    int high_balance = atoi(row[2]);
    int very_high_balance = atoi(row[3]);
    
    // Write balance distribution
    fprintf(report_file, "BALANCE DISTRIBUTION (ACTIVE ACCOUNTS ONLY)\n");
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "Low Balance (< $100): %d (%.1f%%)\n", 
           low_balance, (low_balance * 100.0) / active_accounts);
    fprintf(report_file, "Medium Balance ($100 - $999): %d (%.1f%%)\n", 
           medium_balance, (medium_balance * 100.0) / active_accounts);
    fprintf(report_file, "High Balance ($1,000 - $9,999): %d (%.1f%%)\n", 
           high_balance, (high_balance * 100.0) / active_accounts);
    fprintf(report_file, "Very High Balance (>= $10,000): %d (%.1f%%)\n", 
           very_high_balance, (very_high_balance * 100.0) / active_accounts);
    fprintf(report_file, "----------------------------------------------------\n");
    fprintf(report_file, "\nEnd of Report\n");
    
    // Clean up
    mysql_free_result(result);
    fclose(report_file);
    db_release_connection(conn);
    
    writeInfoLog("Account status report generated at %s", report_path);
    return true;
}
