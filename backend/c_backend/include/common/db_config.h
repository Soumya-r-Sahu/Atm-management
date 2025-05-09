/**
 * @file db_config.h
 * @brief Database configuration for the ATM Management System
 */

#ifndef DB_CONFIG_H
#define DB_CONFIG_H

// Include the unified database configuration
#include "../../../include/common/database/db_unified_config.h"

// This file now serves as a compatibility layer and can be removed in future versions
// All actual configuration has been moved to db_unified_config.h

/* Database table names - centralized for consistency */
#define TABLE_CUSTOMERS "customers"
#define TABLE_ACCOUNTS "accounts"
#define TABLE_CARDS "cards" 
#define TABLE_TRANSACTIONS "transactions"
#define TABLE_DAILY_WITHDRAWALS "daily_withdrawals"
#define TABLE_ADMIN_USERS "admin_users"
#define TABLE_UPI_ACCOUNTS "upi_accounts"
#define TABLE_UPI_PIN "upi_pin"
#define TABLE_UPI_TRANSACTIONS "upi_transactions"
#define TABLE_VIRTUAL_WALLETS "virtual_wallets"
#define TABLE_ATM_CONFIG "atm_config"
#define TABLE_LANGUAGES "languages"
#define TABLE_SYSTEM_LOGS "system_logs"
#define TABLE_AUDIT_LOGS "audit_logs"

/* Database table column names - for consistent SQL queries */
#define COL_CUSTOMER_ID "customer_id"
#define COL_ACCOUNT_NUMBER "account_number"
#define COL_CARD_ID "card_id"
#define COL_CARD_NUMBER "card_number"
#define COL_TRANSACTION_ID "transaction_id"
#define COL_ADMIN_ID "admin_id"
#define COL_VPA_ID "vpa_id"

/**
 * Database connection timeout in seconds
 */
#define DB_CONNECT_TIMEOUT 5

/**
 * Maximum number of database connection retries
 */
#define DB_MAX_RETRIES 3

/**
 * Delay between connection retries in seconds
 */
#define DB_RETRY_DELAY 2

#endif /* DB_CONFIG_H */
