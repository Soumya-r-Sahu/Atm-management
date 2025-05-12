#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mysql/mysql.h>
#include "../../include/common/database/db_config.h"
#include "../../include/common/utils/logger.h"

// Structure to hold bill payment information
typedef struct {
    int id;
    int cardNumber;
    char timestamp[30];
    float amount;
    char billerName[50];
    char accountId[50];
    char status[20]; // "Success", "Failed", "Pending", "Refunded"
    char customerName[100];
    char remarks[200];
    int transactionId;
} BillPayment;

// Save bill payment record to the database
int saveBillPayment(int cardNumber, float amount, const char* billerName, 
                   const char* accountId, const char* customerName, const char* remarks) {
    MYSQL* conn = (MYSQL*)db_get_connection();
    if (conn == NULL) {
        writeErrorLog("Failed to get database connection");
        return 0;
    }

    // Create a transaction record first
    char query[600];
    char accountNumber[21] = {0};
    
    // Get account number for the card
    sprintf(query, "SELECT a.accountNumber FROM %s a JOIN %s c ON a.customerId = c.customerId JOIN %s cd ON c.customerId = cd.customerId "
            "WHERE cd.cardNumber = %d LIMIT 1", 
            TABLE_ACCOUNTS, TABLE_CUSTOMERS, TABLE_CARDS, cardNumber);
    
    if (mysql_query(conn, query)) {
        writeErrorLog("MySQL query error getting account: %s", mysql_error(conn));
        db_release_connection(conn);
        return 0;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("MySQL result error getting account: %s", mysql_error(conn));
        db_release_connection(conn);
        return 0;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row && row[0]) {
        strncpy(accountNumber, row[0], sizeof(accountNumber) - 1);
    } else {
        writeErrorLog("Account not found for card %d", cardNumber);
        mysql_free_result(result);
        db_release_connection(conn);
        return 0;
    }
    
    mysql_free_result(result);
    
    // Create transaction record
    sprintf(query, "INSERT INTO %s (cardNumber, accountNumber, amount, timestamp, type, status, remarks) "
            "VALUES (%d, '%s', %.2f, NOW(), 'Bill_Payment', 'Success', 'Bill payment to %s')",
            TABLE_TRANSACTIONS, cardNumber, accountNumber, amount, billerName);
    
    if (mysql_query(conn, query)) {
        writeErrorLog("MySQL query error creating transaction: %s", mysql_error(conn));
        db_release_connection(conn);
        return 0;
    }
    
    int transactionId = mysql_insert_id(conn);
    
    // Create bill payment record
    sprintf(query, "INSERT INTO %s (cardNumber, amount, billerName, accountId, status, customerName, transactionId, timestamp, remarks) "
            "VALUES (%d, %.2f, '%s', '%s', 'Success', '%s', %d, NOW(), '%s')",
            TABLE_BILL_PAYMENTS, cardNumber, amount, billerName, accountId, customerName, transactionId, 
            remarks ? remarks : "Bill Payment");
    
    if (mysql_query(conn, query)) {
        writeErrorLog("MySQL query error creating bill payment: %s", mysql_error(conn));
        db_release_connection(conn);
        return 0;
    }

    int id = mysql_insert_id(conn);
    db_release_connection(conn);
    
    writeInfoLog("Bill payment recorded successfully: ID=%d, Transaction=%d", id, transactionId);
    return id;
}

// Get bill payment records for a specific card
BillPayment* getBillPaymentsByCard(int cardNumber, int* count) {
    MYSQL* conn = (MYSQL*)db_get_connection();
    if (conn == NULL) {
        *count = 0;
        return NULL;
    }

    char query[400];
    sprintf(query, "SELECT id, cardNumber, timestamp, amount, billerName, accountId, status, customerName, remarks, transactionId "
            "FROM %s WHERE cardNumber = %d ORDER BY timestamp DESC", TABLE_BILL_PAYMENTS, cardNumber);

    if (mysql_query(conn, query)) {
        writeErrorLog("MySQL query error fetching bill payments: %s", mysql_error(conn));
        db_release_connection(conn);
        *count = 0;
        return NULL;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result == NULL) {
        writeErrorLog("MySQL result error fetching bill payments: %s", mysql_error(conn));
        db_release_connection(conn);
        *count = 0;
        return NULL;
    }

    int numRows = mysql_num_rows(result);
    if (numRows == 0) {
        mysql_free_result(result);
        db_release_connection(conn);
        *count = 0;
        return NULL;
    }

    BillPayment* payments = (BillPayment*)malloc(numRows * sizeof(BillPayment));
    if (payments == NULL) {
        writeErrorLog("Memory allocation failed for bill payments");
        mysql_free_result(result);
        db_release_connection(conn);
        *count = 0;
        return NULL;
    }

    MYSQL_ROW row;
    int index = 0;
    while ((row = mysql_fetch_row(result))) {
        payments[index].id = atoi(row[0]);
        payments[index].cardNumber = atoi(row[1]);
        strncpy(payments[index].timestamp, row[2], sizeof(payments[index].timestamp) - 1);
        payments[index].timestamp[sizeof(payments[index].timestamp) - 1] = '\0';
        payments[index].amount = row[3] ? atof(row[3]) : 0.0;
        strncpy(payments[index].billerName, row[4] ? row[4] : "", sizeof(payments[index].billerName) - 1);
        payments[index].billerName[sizeof(payments[index].billerName) - 1] = '\0';
        strncpy(payments[index].accountId, row[5] ? row[5] : "", sizeof(payments[index].accountId) - 1);
        payments[index].accountId[sizeof(payments[index].accountId) - 1] = '\0';
        strncpy(payments[index].status, row[6] ? row[6] : "", sizeof(payments[index].status) - 1);
        payments[index].status[sizeof(payments[index].status) - 1] = '\0';
        strncpy(payments[index].customerName, row[7] ? row[7] : "", sizeof(payments[index].customerName) - 1);
        payments[index].customerName[sizeof(payments[index].customerName) - 1] = '\0';
        strncpy(payments[index].remarks, row[8] ? row[8] : "", sizeof(payments[index].remarks) - 1);
        payments[index].remarks[sizeof(payments[index].remarks) - 1] = '\0';
        payments[index].transactionId = row[9] ? atoi(row[9]) : 0;
        index++;
    }

    mysql_free_result(result);
    db_release_connection(conn);
    *count = index;
    return payments;
}

// Get bill payment by ID
BillPayment* getBillPaymentById(int paymentId) {
    MYSQL* conn = (MYSQL*)db_get_connection();
    if (conn == NULL) {
        return NULL;
    }

    char query[400];
    sprintf(query, "SELECT id, cardNumber, timestamp, amount, billerName, accountId, status, customerName, remarks, transactionId "
            "FROM %s WHERE id = %d", TABLE_BILL_PAYMENTS, paymentId);

    if (mysql_query(conn, query)) {
        writeErrorLog("MySQL query error fetching bill payment: %s", mysql_error(conn));
        db_release_connection(conn);
        return NULL;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result == NULL) {
        writeErrorLog("MySQL result error fetching bill payment: %s", mysql_error(conn));
        db_release_connection(conn);
        return NULL;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        db_release_connection(conn);
        return NULL;
    }

    BillPayment* payment = (BillPayment*)malloc(sizeof(BillPayment));
    if (payment == NULL) {
        writeErrorLog("Memory allocation failed for bill payment");
        mysql_free_result(result);
        db_release_connection(conn);
        return NULL;
    }

    payment->id = atoi(row[0]);
    payment->cardNumber = atoi(row[1]);
    strncpy(payment->timestamp, row[2], sizeof(payment->timestamp) - 1);
    payment->timestamp[sizeof(payment->timestamp) - 1] = '\0';
    payment->amount = row[3] ? atof(row[3]) : 0.0;
    strncpy(payment->billerName, row[4] ? row[4] : "", sizeof(payment->billerName) - 1);
    payment->billerName[sizeof(payment->billerName) - 1] = '\0';
    strncpy(payment->accountId, row[5] ? row[5] : "", sizeof(payment->accountId) - 1);
    payment->accountId[sizeof(payment->accountId) - 1] = '\0';
    strncpy(payment->status, row[6] ? row[6] : "", sizeof(payment->status) - 1);
    payment->status[sizeof(payment->status) - 1] = '\0';
    strncpy(payment->customerName, row[7] ? row[7] : "", sizeof(payment->customerName) - 1);
    payment->customerName[sizeof(payment->customerName) - 1] = '\0';
    strncpy(payment->remarks, row[8] ? row[8] : "", sizeof(payment->remarks) - 1);
    payment->remarks[sizeof(payment->remarks) - 1] = '\0';
    payment->transactionId = row[9] ? atoi(row[9]) : 0;

    mysql_free_result(result);
    db_release_connection(conn);
    return payment;
}

// Update bill payment status
bool updateBillPaymentStatus(int paymentId, const char* newStatus) {
    MYSQL* conn = (MYSQL*)db_get_connection();
    if (conn == NULL) {
        return false;
    }

    char query[200];
    sprintf(query, "UPDATE %s SET status = '%s' WHERE id = %d", TABLE_BILL_PAYMENTS, newStatus, paymentId);

    if (mysql_query(conn, query)) {
        writeErrorLog("MySQL query error updating bill payment: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }

    bool success = (mysql_affected_rows(conn) > 0);
    db_release_connection(conn);
    
    if (success) {
        writeInfoLog("Bill payment %d status updated to %s", paymentId, newStatus);
    }
    
    return success;
}

// Free bill payment records
void freeBillPayments(BillPayment* payments) {
    if (payments != NULL) {
        free(payments);
    }
}

// Free a single bill payment
void freeBillPayment(BillPayment* payment) {
    if (payment != NULL) {
        free(payment);
    }
}