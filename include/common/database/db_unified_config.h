/**
 * @file db_unified_config.h
 * @brief Unified Database Configuration for the ATM Management System
 * @version 1.0
 * @date May 10, 2025
 * 
 * This is the centralized configuration file for all database settings.
 * Any changes to database connection parameters should be made here.
 */

#ifndef DB_UNIFIED_CONFIG_H
#define DB_UNIFIED_CONFIG_H

/**
 * Database connection parameters
 * These should be changed according to your MySQL setup
 */
#define DB_HOST "localhost"
#define DB_USER "atm_app"  
#define DB_PASS "Admin@6074."
#define DB_NAME "core_banking_system"
#define DB_PORT 3306

/* Configuration switches */
#define USE_MYSQL 1         /* Set to 0 to use file-based storage as fallback */
#define USE_CONNECTION_POOL 1  /* Set to 0 to disable connection pooling */
#define MAX_DB_CONNECTIONS 10  /* Maximum number of connections in the pool */
#define CONNECTION_IDLE_TIMEOUT 300 /* Connection timeout in seconds (5 minutes) */

/* Database table names - centralized for consistency */
#define TABLE_CUSTOMERS "cbs_customers"
#define TABLE_ACCOUNTS "cbs_accounts"
#define TABLE_CARDS "cbs_cards" 
#define TABLE_TRANSACTIONS "cbs_transactions"
#define TABLE_DAILY_WITHDRAWALS "cbs_daily_withdrawals"
#define TABLE_ADMIN_USERS "cbs_admin_users"
#define TABLE_UPI_ACCOUNTS "cbs_upi_accounts"
#define TABLE_UPI_PIN "cbs_upi_pin"
#define TABLE_UPI_TRANSACTIONS "cbs_upi_transactions"
#define TABLE_VIRTUAL_WALLETS "cbs_virtual_wallets"
#define TABLE_ATM_CONFIG "cbs_atm_config"
#define TABLE_LANGUAGES "cbs_languages"
#define TABLE_SYSTEM_LOGS "cbs_system_logs"
#define TABLE_AUDIT_LOGS "cbs_audit_logs"

/* Database table column names - for consistent SQL queries */
#define COL_CUSTOMER_ID "customer_id"
#define COL_ACCOUNT_NUMBER "account_number"
#define COL_CARD_ID "card_id"
#define COL_CARD_NUMBER "card_number"
#define COL_TRANSACTION_ID "transaction_id"
#define COL_ADMIN_ID "admin_id"
#define COL_VPA_ID "vpa_id"
#define COL_STATUS "status"
#define COL_BALANCE "balance"
#define COL_PIN_HASH "pin_hash"
#define COL_EXPIRY_DATE "expiry_date"
#define COL_CREATE_DATE "created_at"
#define COL_LAST_UPDATED "last_updated"

/* Database connection settings */
#define DB_CONNECT_TIMEOUT 5         /* Connection timeout in seconds */
#define DB_MAX_RETRIES 3             /* Maximum connection retry attempts */
#define DB_RETRY_DELAY 2             /* Delay between retries in seconds */
#define MAX_LOG_QUERY_LENGTH 2048    /* Maximum length for logged queries */

#endif /* DB_UNIFIED_CONFIG_H */
