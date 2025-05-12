/**
 * @file global.h
 * @brief Global definitions for the Core Banking System
 */

#ifndef CBS_GLOBAL_H
#define CBS_GLOBAL_H

// Success and error codes
#define SUCCESS 0
#define ERR_INVALID_REQUEST -1
#define ERR_SERVER_ERROR -2
#define ERR_INSUFFICIENT_FUNDS -3
#define ERR_ACCOUNT_NOT_FOUND -4
#define ERR_UNAUTHORIZED -5

// Status codes
#define STATUS_OK 200
#define STATUS_CREATED 201
#define STATUS_BAD_REQUEST 400
#define STATUS_UNAUTHORIZED 401
#define STATUS_FORBIDDEN 403
#define STATUS_NOT_FOUND 404
#define STATUS_SERVER_ERROR 500

// Maximum length definitions
#define MAX_ACCOUNT_ID_LEN 20
#define MAX_CUSTOMER_ID_LEN 20
#define MAX_ACCOUNT_NAME_LEN 50
#define MAX_DESCRIPTION_LEN 100
#define MAX_TRANSACTION_ID_LEN 24
#define MAX_API_RESPONSE_LEN 2048
#define MAX_API_REQUEST_LEN 1024

// Account status definitions
#define ACCOUNT_STATUS_ACTIVE 1
#define ACCOUNT_STATUS_INACTIVE 0
#define ACCOUNT_STATUS_BLOCKED 2

// Transaction type definitions
#define TRANSACTION_TYPE_CREDIT 1
#define TRANSACTION_TYPE_DEBIT 2
#define TRANSACTION_TYPE_TRANSFER 3

// Menu type definitions
#define MENU_TYPE_MAIN 1
#define MENU_TYPE_CUSTOMER 2
#define MENU_TYPE_ADMIN 3
#define MENU_TYPE_TRANSACTION 4

// Basic type definitions for testing
typedef struct {
    char account_id[MAX_ACCOUNT_ID_LEN];
    char customer_id[MAX_CUSTOMER_ID_LEN];
    char account_name[MAX_ACCOUNT_NAME_LEN];
    double balance;
    int status;
    long creation_date;
    long last_updated;
} Account;

typedef struct {
    char transaction_id[MAX_TRANSACTION_ID_LEN];
    char account_id[MAX_ACCOUNT_ID_LEN];
    int type;
    double amount;
    char description[MAX_DESCRIPTION_LEN];
    long timestamp;
} Transaction;

#endif /* CBS_GLOBAL_H */
