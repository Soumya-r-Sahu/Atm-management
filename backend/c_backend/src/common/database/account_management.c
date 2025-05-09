#include "../../../include/common/database/account_management.h"
#include "../../../include/common/database/db_config.h"
#include "../../../include/common/database/database_conn.h"
#include "../../../include/common/database/core_banking_interface.h"
#include "../../../include/common/utils/logger.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/**
 * Create a new account in the core banking system
 * Time complexity: O(1) - multiple atomic SQL operations
 */
bool cbs_create_account(const char* name, const char* address, const char* phone, 
                       const char* email, const char* accountType, double initialDeposit,
                       char* newAccountNumber, char* newCardNumber) {
                       
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_create_account");
        return false;
    }
    
    // Start a transaction
    if (mysql_query(conn, "START TRANSACTION") != 0) {
        writeErrorLog("Failed to start transaction: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    bool success = false;
    char customer_id[21] = {0};
    char account_number[21] = {0};
    char card_id[21] = {0};
    char card_number[21] = {0};
    
    // Generate unique IDs
    sprintf(customer_id, "CUST%010ld", random() % 10000000000);
    sprintf(account_number, "10%010ld", random() % 10000000000);
    sprintf(card_id, "CARD%010ld", random() % 10000000000);
    sprintf(card_number, "%016ld", random() % 10000000000000000);
    
    // Create customer
    char customer_query[1000];
    sprintf(customer_query, 
            "INSERT INTO cbs_customers (customer_id, name, dob, address, email, phone, status, kyc_status) "
            "VALUES ('%s', '%s', CURDATE(), '%s', '%s', '%s', 'ACTIVE', 'COMPLETED')",
            customer_id, name, address, email, phone);
    
    if (mysql_query(conn, customer_query) != 0) {
        writeErrorLog("Failed to create customer: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // Determine account type enum value
    const char* accountTypeEnum = "SAVINGS"; // Default
    if (strcasecmp(accountType, "current") == 0) {
        accountTypeEnum = "CURRENT";
    } else if (strcasecmp(accountType, "fixed_deposit") == 0) {
        accountTypeEnum = "FIXED_DEPOSIT";
    } else if (strcasecmp(accountType, "salary") == 0) {
        accountTypeEnum = "SALARY";
    }
    
    // Create account
    char account_query[1000];
    sprintf(account_query, 
            "INSERT INTO cbs_accounts (account_number, customer_id, account_type, branch_code, "
            "ifsc_code, balance, status, opening_date) "
            "VALUES ('%s', '%s', '%s', 'BR001', 'IFSC00000001', %.2f, 'ACTIVE', CURDATE())",
            account_number, customer_id, accountTypeEnum, (double)initialDeposit);
    
    if (mysql_query(conn, account_query) != 0) {
        writeErrorLog("Failed to create account: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // Generate expiry date (3 years from now)
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    tm_info->tm_year += 3;
    char expiry_date[20];
    strftime(expiry_date, sizeof(expiry_date), "%Y-%m-%d", tm_info);
    
    // Create card
    char card_query[1000];
    sprintf(card_query, 
            "INSERT INTO cbs_cards (card_id, account_id, card_number, card_type, card_network, "
            "expiry_date, cvv, pin_hash, status, issue_date, primary_user_name) "
            "VALUES ('%s', '%s', '%s', 'DEBIT', 'VISA', '%s', '%03d', '%d', 'ACTIVE', CURDATE(), '%s')",
            card_id, account_number, card_number, expiry_date, rand() % 1000, 1234, name);
    
    if (mysql_query(conn, card_query) != 0) {
        writeErrorLog("Failed to create card: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // Log the initial deposit if there is one
    if (initialDeposit > 0) {
        char transaction_query[1000];
        sprintf(transaction_query, 
                "INSERT INTO cbs_transactions (transaction_id, account_number, transaction_type, "
                "channel, amount, balance_before, balance_after, value_date, status, remarks) "
                "VALUES (UUID(), '%s', 'DEPOSIT', 'BRANCH', %.2f, 0.00, %.2f, CURDATE(), 'SUCCESS', "
                "'Initial deposit for account opening')",
                account_number, (double)initialDeposit, (double)initialDeposit);
        
        if (mysql_query(conn, transaction_query) != 0) {
            writeErrorLog("Failed to create initial deposit transaction: %s", mysql_error(conn));
            mysql_query(conn, "ROLLBACK");
            db_release_connection(conn);
            return false;
        }
    }
    
    // Commit the transaction
    if (mysql_query(conn, "COMMIT") != 0) {
        writeErrorLog("Failed to commit transaction: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // Output parameters
    if (newAccountNumber) {
        strcpy(newAccountNumber, account_number);
    }
    
    if (newCardNumber) {
        strcpy(newCardNumber, card_number);
    }
    
    writeInfoLog("Successfully created account %s with card %s for customer %s", 
                account_number, card_number, customer_id);
    
    db_release_connection(conn);
    return true;
}

/**
 * Get account balance from the core banking system
 * Time complexity: O(1) - indexed query by account number
 */
bool cbs_get_balance(const char* accountNumber, double* balance) {
    if (!accountNumber || !balance) {
        writeErrorLog("Invalid parameters in cbs_get_balance");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_balance");
        return false;
    }

    char query[200];
    sprintf(query, "SELECT balance FROM cbs_accounts WHERE account_number = '%s'", accountNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                *balance = atof(row[0]);
                success = true;
            } else {
                writeErrorLog("Account %s not found", accountNumber);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to query account balance: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * Get account balance by card number from the core banking system
 * Time complexity: O(1) - indexed query by card number
 */
bool cbs_get_balance_by_card(const char* cardNumber, double* balance) {
    if (!cardNumber || !balance) {
        writeErrorLog("Invalid parameters in cbs_get_balance_by_card");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_balance_by_card");
        return false;
    }

    char query[300];
    sprintf(query, 
            "SELECT a.balance FROM cbs_accounts a "
            "JOIN cbs_cards c ON a.account_number = c.account_id "
            "WHERE c.card_number = '%s'", 
            cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                *balance = atof(row[0]);
                success = true;
            } else {
                writeErrorLog("Card %s not found or not linked to an account", cardNumber);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to query account balance by card: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * Update account balance in the core banking system
 * Time complexity: O(1) - indexed update by account number
 */
bool cbs_update_balance(const char* accountNumber, double newBalance, const char* transactionType) {
    if (!accountNumber || !transactionType) {
        writeErrorLog("Invalid parameters in cbs_update_balance");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_update_balance");
        return false;
    }
    
    // Start a transaction
    if (mysql_query(conn, "START TRANSACTION") != 0) {
        writeErrorLog("Failed to start transaction: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    // Get current balance
    char balance_query[200];
    sprintf(balance_query, "SELECT balance FROM cbs_accounts WHERE account_number = '%s' FOR UPDATE", 
            accountNumber);
    
    double currentBalance = 0.0;
    bool success = false;
    
    if (mysql_query(conn, balance_query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                currentBalance = atof(row[0]);
                
                // Update balance
                char update_query[300];
                sprintf(update_query, 
                        "UPDATE cbs_accounts SET balance = %.2f, last_transaction = NOW() "
                        "WHERE account_number = '%s'", 
                        newBalance, accountNumber);
                
                if (mysql_query(conn, update_query) == 0) {
                    // Log transaction
                    char transaction_query[600];
                    sprintf(transaction_query, 
                            "INSERT INTO cbs_transactions "
                            "(transaction_id, account_number, transaction_type, channel, "
                            "amount, balance_before, balance_after, value_date, status) "
                            "VALUES (UUID(), '%s', '%s', 'ATM', %.2f, %.2f, %.2f, CURDATE(), 'SUCCESS')",
                            accountNumber, transactionType, 
                            fabs(newBalance - currentBalance), currentBalance, newBalance);
                    
                    if (mysql_query(conn, transaction_query) == 0) {
                        success = true;
                    } else {
                        writeErrorLog("Failed to log transaction: %s", mysql_error(conn));
                    }
                } else {
                    writeErrorLog("Failed to update balance: %s", mysql_error(conn));
                }
            } else {
                writeErrorLog("Account %s not found", accountNumber);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to query current balance: %s", mysql_error(conn));
    }
    
    // Commit or rollback
    if (success) {
        if (mysql_query(conn, "COMMIT") != 0) {
            writeErrorLog("Failed to commit transaction: %s", mysql_error(conn));
            mysql_query(conn, "ROLLBACK");
            success = false;
        }
    } else {
        mysql_query(conn, "ROLLBACK");
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * Get account holder name from the core banking system
 * Time complexity: O(1) - indexed join query
 */
bool cbs_get_account_holder_name(const char* accountNumber, char* name, size_t nameSize) {
    if (!accountNumber || !name || nameSize == 0) {
        writeErrorLog("Invalid parameters in cbs_get_account_holder_name");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_account_holder_name");
        return false;
    }

    char query[300];
    sprintf(query, 
            "SELECT c.name FROM cbs_customers c "
            "JOIN cbs_accounts a ON c.customer_id = a.customer_id "
            "WHERE a.account_number = '%s'", 
            accountNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                strncpy(name, row[0], nameSize - 1);
                name[nameSize - 1] = '\0';
                success = true;
            } else {
                writeErrorLog("Account %s not found or has no customer", accountNumber);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to query account holder name: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * Get account holder name by card number from the core banking system
 * Time complexity: O(1) - indexed join query
 */
bool cbs_get_card_holder_name(const char* cardNumber, char* name, size_t nameSize) {
    if (!cardNumber || !name || nameSize == 0) {
        writeErrorLog("Invalid parameters in cbs_get_card_holder_name");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_card_holder_name");
        return false;
    }

    char query[400];
    sprintf(query, 
            "SELECT c.name FROM cbs_customers c "
            "JOIN cbs_accounts a ON c.customer_id = a.customer_id "
            "JOIN cbs_cards cd ON a.account_number = cd.account_id "
            "WHERE cd.card_number = '%s'", 
            cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                strncpy(name, row[0], nameSize - 1);
                name[nameSize - 1] = '\0';
                success = true;
            } else {
                writeErrorLog("Card %s not found or not linked to a customer", cardNumber);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to query card holder name: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * Generate an account receipt for the new account in the core banking system
 * Time complexity: O(1) - multiple indexed queries
 */
bool cbs_generate_account_receipt(const char* name, const char* accountNumber, const char* accountType,
                                 const char* ifscCode, const char* cardNumber, 
                                 const char* expiryDate, const char* cvv, char* receipt, size_t receiptSize) {
    if (!name || !accountNumber || !accountType || !ifscCode || !cardNumber || 
        !expiryDate || !cvv || !receipt || receiptSize == 0) {
        writeErrorLog("Invalid parameters in cbs_generate_account_receipt");
        return false;
    }
    
    // Get current date and time
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char date_str[30];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Format the receipt
    int written = snprintf(receipt, receiptSize,
        "===== ACCOUNT CREATION RECEIPT =====\n"
        "Date: %s\n"
        "Name: %s\n"
        "Account Number: %s\n"
        "Account Type: %s\n"
        "IFSC Code: %s\n"
        "\n"
        "Card Number: %s\n"
        "Expiry Date: %s\n"
        "CVV: %s\n"
        "\n"
        "Thank you for banking with us.\n"
        "==================================\n",
        date_str, name, accountNumber, accountType, ifscCode, 
        cardNumber, expiryDate, cvv);
    
    if (written >= (int)receiptSize) {
        writeErrorLog("Receipt buffer too small in cbs_generate_account_receipt");
        return false;
    }
    
    return true;
}

/**
 * Find the account number associated with a card number
 * Time complexity: O(1) - indexed query
 */
bool cbs_get_account_by_card(const char* cardNumber, char* accountNumber, size_t accountNumberSize) {
    if (!cardNumber || !accountNumber || accountNumberSize == 0) {
        writeErrorLog("Invalid parameters in cbs_get_account_by_card");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_account_by_card");
        return false;
    }

    char query[200];
    sprintf(query, "SELECT account_id FROM cbs_cards WHERE card_number = '%s'", cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                strncpy(accountNumber, row[0], accountNumberSize - 1);
                accountNumber[accountNumberSize - 1] = '\0';
                success = true;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to query account by card: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

/* Legacy function names for backward compatibility */

bool getCardHolderName(int cardNumber, char* name, size_t nameSize) {
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", cardNumber);
    return cbs_get_card_holder_name(card_number_str, name, nameSize);
}

bool fetchBalance(int cardNumber, float* balance) {
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", cardNumber);
    double dblBalance = 0.0;
    bool result = cbs_get_balance_by_card(card_number_str, &dblBalance);
    if (result && balance) {
        *balance = (float)dblBalance;
    }
    return result;
}

bool updateBalance(int cardNumber, float newBalance) {
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", cardNumber);
    
    // First get the account number for this card
    char account_number[21] = {0};
    if (!cbs_get_account_by_card(card_number_str, account_number, sizeof(account_number))) {
        return false;
    }
    
    // Determine transaction type based on balance change
    const char* transactionType = "DEPOSIT";
    MYSQL *conn = db_get_connection();
    if (conn) {
        char query[200];
        sprintf(query, 
                "SELECT balance FROM cbs_accounts a "
                "JOIN cbs_cards c ON a.account_number = c.account_id "
                "WHERE c.card_number = '%s'", 
                card_number_str);
        
        if (mysql_query(conn, query) == 0) {
            MYSQL_RES *result = mysql_store_result(conn);
            if (result) {
                MYSQL_ROW row = mysql_fetch_row(result);
                if (row && row[0]) {
                    double currentBalance = atof(row[0]);
                    if (newBalance < currentBalance) {
                        transactionType = "WITHDRAWAL";
                    }
                }
                mysql_free_result(result);
            }
        }
        db_release_connection(conn);
    }
    
    return cbs_update_balance(account_number, (double)newBalance, transactionType);
}

bool createNewAccount(const char* name, const char* address, const char* phone, 
                     const char* email, const char* accountType, float initialDeposit,
                     int* newAccountNumber, int* newCardNumber) {
    char accountNumberStr[21] = {0};
    char cardNumberStr[21] = {0};
    
    bool result = cbs_create_account(name, address, phone, email, accountType, 
                                    (double)initialDeposit, accountNumberStr, cardNumberStr);
    
    if (result) {
        if (newAccountNumber) {
            *newAccountNumber = atoi(accountNumberStr);
        }
        
        if (newCardNumber) {
            *newCardNumber = atoi(cardNumberStr);
        }
    }
    
    return result;
}

bool generateAccountReceipt(const char* name, long accountNumber, const char* accountType,
                           const char* ifscCode, const char* cardNumber, 
                           const char* expiryDate, int cvv) {
    // Convert CVV to string
    char cvvStr[10];
    snprintf(cvvStr, sizeof(cvvStr), "%03d", cvv);
    
    // Convert account number to string
    char accountNumberStr[21];
    snprintf(accountNumberStr, sizeof(accountNumberStr), "%ld", accountNumber);
    
    char receipt[1024];
    bool result = cbs_generate_account_receipt(name, accountNumberStr, accountType, 
                                             ifscCode, cardNumber, expiryDate, cvvStr,
                                             receipt, sizeof(receipt));
    
    if (result) {
        // Just log the receipt for demo purposes
        writeInfoLog("Generated account receipt:\n%s", receipt);
    }
    
    return result;
}
