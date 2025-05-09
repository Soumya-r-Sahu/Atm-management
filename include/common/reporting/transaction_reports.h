/**
 * @file transaction_reports.h
 * @brief Header file for transaction reporting functionality
 */

#ifndef TRANSACTION_REPORTS_H
#define TRANSACTION_REPORTS_H

#include <stdbool.h>

/**
 * @brief Generate a daily transaction report
 * @param report_date Date for the report (format: YYYY-MM-DD)
 * @param report_path Path to save the report
 * @return true if successful, false otherwise
 */
bool cbs_generate_daily_transaction_report(const char* report_date, const char* report_path);

/**
 * @brief Generate a card usage report
 * @param start_date Start date for the report (format: YYYY-MM-DD)
 * @param end_date End date for the report (format: YYYY-MM-DD)
 * @param report_path Path to save the report
 * @return true if successful, false otherwise
 */
bool cbs_generate_card_usage_report(const char* start_date, const char* end_date, const char* report_path);

/**
 * @brief Generate an account status report
 * @param report_path Path to save the report
 * @return true if successful, false otherwise
 */
bool cbs_generate_account_status_report(const char* report_path);

#endif /* TRANSACTION_REPORTS_H */
