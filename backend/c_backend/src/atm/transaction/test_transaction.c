#include "../../../../../include/atm/transaction/transaction_processor.h"
#include "../../../../../include/common/database/core_banking_interface.h"
#include "../../../../../include/common/database/account_management.h"
#include "../../../../../include/common/database/card_account_management.h"
#include "../../../../../include/common/transaction/bill_payment.h"
#include "../../../../../include/common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Helper function to print transaction result
void print_transaction_result(const char* test_name, TransactionResult result) {
    printf("\n--- %s Result ---\n", test_name);
    printf("Status: %s\n", 
        result.status == TRANSACTION_SUCCESS ? "SUCCESS" : 
        result.status == TRANSACTION_FAILED ? "FAILED" :
        result.status == TRANSACTION_PENDING ? "PENDING" : "CANCELED");
    printf("Message: %s\n", result.message);
    printf("Amount processed: %.2f\n", result.amount_processed);
    printf("Balance before: %.2f\n", result.balance_before);
    printf("Balance after: %.2f\n", result.balance_after);
    printf("---------------------\n\n");
}

int main() {
    printf("=====================================\n");
    printf("Testing ATM Transaction Processor\n");
    printf("Using MySQL Stub Implementation\n");
    printf("=====================================\n\n");
      int test_card = 123456789;
    int destination_card = 987654321;
    
    // Test 1: Balance Inquiry
    printf("1. Testing Balance Inquiry...\n");
    TransactionResult balance_result = process_balance_inquiry(test_card);
    print_transaction_result("Balance Inquiry", balance_result);
    
    // Test 2: Withdrawal
    printf("2. Testing Withdrawal...\n");
    TransactionResult withdrawal_result = process_withdrawal(test_card, 500.0);
    print_transaction_result("Withdrawal", withdrawal_result);
    
    // Test 3: Deposit
    printf("3. Testing Deposit...\n");
    TransactionResult deposit_result = process_deposit(test_card, 1000.0);
    print_transaction_result("Deposit", deposit_result);
    
    // Test 4: Transfer
    printf("4. Testing Transfer...\n");
    TransactionResult transfer_result = process_transfer(test_card, destination_card, 300.0);
    print_transaction_result("Transfer", transfer_result);
    
    // Test 5: PIN Change
    printf("5. Testing PIN Change...\n");
    TransactionResult pin_change_result = process_pin_change(test_card, 1234, 5678);
    print_transaction_result("PIN Change", pin_change_result);
    
    // Test 6: Mini Statement
    printf("6. Testing Mini Statement...\n");
    TransactionResult mini_statement_result = process_mini_statement(test_card);
    print_transaction_result("Mini Statement", mini_statement_result);
    
    // Test 7: Bill Payment
    printf("7. Testing Bill Payment...\n");
    TransactionResult bill_payment_result = process_bill_payment(
        test_card, "ELECTRICITY", "BILL123456", 750.0);
    print_transaction_result("Bill Payment", bill_payment_result);
    
    // Test 8: Recent Transactions Query
    printf("8. Testing Recent Transactions Query...\n");
    QueryResult recent_transactions = get_recent_transactions(test_card, 5);
    printf("\n--- Query Result ---\n");
    printf("Success: %s\n", recent_transactions.success ? "Yes" : "No");
    printf("Count: %d\n", recent_transactions.count);
    if (recent_transactions.data != NULL) {
        for (int i = 0; i < recent_transactions.count; i++) {
            printf("  Transaction %d: Type=%s, Amount=%.2f\n", 
                i+1, 
                recent_transactions.data[i].transaction_type,
                recent_transactions.data[i].amount);
        }
        free(recent_transactions.data); // Free the allocated memory
    }
    printf("---------------------\n\n");
    
    printf("All tests completed!\n");
    
    return 0;
}
