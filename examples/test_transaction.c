/**
 * @file test_transaction.c
 * @brief Example: Simulate debit/credit transactions
 * 
 * This file provides examples of using the transaction processing functions
 * in the Core Banking System.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/global.h"
#include "../include/error_codes.h"
#include "../backend/c_backend/src/account.c"
#include "../backend/c_backend/src/transaction.c"

/**
 * @brief Example of processing a debit transaction
 */
void test_process_debit() {
    printf("===== Testing Debit Transaction Processing =====\n");
    
    const char account_id[] = "ACC123456789";
    double amount = 100.0;
    const char description[] = "ATM Withdrawal";
    char transaction_id[MAX_ACCOUNT_ID_LEN];
    
    // Check the balance before the transaction
    double initial_balance = 0.0;
    if (check_balance(account_id, &initial_balance) == SUCCESS) {
        printf("Initial balance for account %s: $%.2f\n", account_id, initial_balance);
        
        // Process the debit transaction
        int result = process_debit(account_id, amount, description, transaction_id);
        
        if (result == SUCCESS) {
            printf("Debit transaction processed successfully\n");
            printf("Transaction ID: %s\n", transaction_id);
            
            // Check the balance after the transaction
            double final_balance = 0.0;
            if (check_balance(account_id, &final_balance) == SUCCESS) {
                printf("Final balance for account %s: $%.2f\n", account_id, final_balance);
                printf("Balance difference: $%.2f\n", initial_balance - final_balance);
            }
        } else if (result == ERR_INSUFFICIENT_FUNDS) {
            printf("Debit transaction failed: Insufficient funds\n");
        } else {
            printf("Debit transaction failed: Error %d\n", result);
        }
    }
}

/**
 * @brief Example of processing a credit transaction
 */
void test_process_credit() {
    printf("\n===== Testing Credit Transaction Processing =====\n");
    
    const char account_id[] = "ACC123456789";
    double amount = 250.0;
    const char description[] = "Salary Deposit";
    char transaction_id[MAX_ACCOUNT_ID_LEN];
    
    // Check the balance before the transaction
    double initial_balance = 0.0;
    if (check_balance(account_id, &initial_balance) == SUCCESS) {
        printf("Initial balance for account %s: $%.2f\n", account_id, initial_balance);
        
        // Process the credit transaction
        int result = process_credit(account_id, amount, description, transaction_id);
        
        if (result == SUCCESS) {
            printf("Credit transaction processed successfully\n");
            printf("Transaction ID: %s\n", transaction_id);
            
            // Check the balance after the transaction
            double final_balance = 0.0;
            if (check_balance(account_id, &final_balance) == SUCCESS) {
                printf("Final balance for account %s: $%.2f\n", account_id, final_balance);
                printf("Balance difference: $%.2f\n", final_balance - initial_balance);
            }
        } else {
            printf("Credit transaction failed: Error %d\n", result);
        }
    }
}

/**
 * @brief Example of retrieving transaction history
 */
void test_transaction_history() {
    printf("\n===== Testing Transaction History Retrieval =====\n");
    
    const char account_id[] = "ACC123456789";
    const char start_date[] = "2025-05-01";
    const char end_date[] = "2025-05-10";
    
    // Placeholder for transaction data - in a real implementation,
    // this would be a structure to hold multiple transactions
    void* transactions = malloc(1024);
    int count = 0;
    
    if (!transactions) {
        printf("Memory allocation failed\n");
        return;
    }
    
    int result = get_transaction_history(account_id, start_date, end_date, transactions, &count);
    
    if (result == SUCCESS) {
        printf("Successfully retrieved transaction history\n");
        printf("Number of transactions found: %d\n", count);
        
        // In a real implementation, we would iterate through the transactions
        // and display details of each one
        printf("(This is a placeholder - in a real implementation, transaction details would be displayed here)\n");
    } else {
        printf("Failed to retrieve transaction history: Error %d\n", result);
    }
    
    free(transactions);
}

/**
 * @brief Example of account creation
 */
void test_create_account() {
    printf("\n===== Testing Account Creation =====\n");
    
    int customer_id = 12345;
    int account_type = ACCOUNT_TYPE_SAVINGS;
    double initial_balance = 1000.0;
    char account_id[MAX_ACCOUNT_ID_LEN];
    
    int result = create_account(customer_id, account_type, initial_balance, account_id);
    
    if (result == SUCCESS) {
        printf("Account created successfully\n");
        printf("New account ID: %s\n", account_id);
        
        // Check the initial balance
        double balance = 0.0;
        if (check_balance(account_id, &balance) == SUCCESS) {
            printf("Initial balance for new account: $%.2f\n", balance);
        }
    } else {
        printf("Account creation failed: Error %d\n", result);
    }
}

int main() {
    printf("Transaction Processing Test Application\n");
    printf("======================================\n\n");
    
    // Test account creation
    test_create_account();
    
    // Test debit transaction
    test_process_debit();
    
    // Test credit transaction
    test_process_credit();
    
    // Test transaction history
    test_transaction_history();
    
    return 0;
}
