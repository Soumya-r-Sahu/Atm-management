#ifndef DB_CONSTANTS_H
#define DB_CONSTANTS_H

/**
 * @file db_constants.h
 * @brief Database constants for the ATM Management System
 * @date May 3, 2025
 */

// Database table names
#define TABLE_TRANSACTIONS "Transactions"
#define TABLE_CARDS "Cards"
#define TABLE_ACCOUNTS "Accounts"
#define TABLE_CUSTOMERS "Customers"
#define TABLE_ADMINS "AdminUsers"
#define TABLE_BRANCHES "BranchInformation"
#define TABLE_ATM_MACHINES "ATMMachines"
#define TABLE_BILL_PAYMENTS "BillPayments"
#define TABLE_SECURITY_LOGS "SecurityLogs"
#define TABLE_CARD_REQUESTS "CardRequests"
#define TABLE_CARD_BLOCKS "CardBlocks"

// Client-side file names (for file-based operations)
#define ATM_CLIENT_TRANSACTIONS "data/atm_transactions.txt"
#define CLIENT_CARDS "data/card.txt"
#define CLIENT_CUSTOMERS "data/customer.txt"
#define CLIENT_ACCOUNTING "data/accounting.txt"

#endif /* DB_CONSTANTS_H */