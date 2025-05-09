#include "../../../../../include/atm/transaction/transaction_processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int test_card_number = 123456789;
    float amount = 100.0f;
    
    printf("=======================================\n");
    printf(" ATM Transaction Processor Test\n");
    printf("=======================================\n\n");
    
    // Test 1: Balance Inquiry
    printf("Test 1: Balance Inquiry\n");
    TransactionResult balance_result = process_balance_inquiry(test_card_number);
    printf("Balance inquiry result: %d\n", balance_result.status);
    printf("Message: %s\n", balance_result.message);
    printf("Balance: %.2f\n\n", balance_result.balance_after);
    
    // Only continue if the first test passes
    if (balance_result.status == TRANSACTION_SUCCESS) {
        // Test 2: Withdrawal
        printf("Test 2: Withdrawal\n");
        TransactionResult withdrawal_result = process_withdrawal(test_card_number, amount);
        printf("Withdrawal result: %d\n", withdrawal_result.status);
        printf("Message: %s\n", withdrawal_result.message);
        printf("Amount processed: %.2f\n", withdrawal_result.amount_processed);
        printf("Current balance: %.2f\n\n", withdrawal_result.balance_after);
    }
    
    printf("Tests completed.\n");
    
    return 0;
}
