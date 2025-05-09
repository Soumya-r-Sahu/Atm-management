// This is a simple wrapper for process_balance_inquiry
#include "../../../../../include/atm/transaction/transaction_processor.h"
#include <stdio.h>

// Custom implementation of transaction functions
TransactionResult process_balance_inquiry_wrapper(int card_number) {
    TransactionResult result;
    result.status = TRANSACTION_SUCCESS;
    result.balance_before = 1000.0;
    result.balance_after = 1000.0;
    result.amount_processed = 0.0;
    sprintf(result.message, "Balance inquiry successful. Balance: $%.2f", result.balance_after);
    
    return result;
}

// Simple main function
int main(int argc, char* argv[]) {
    int card_number = 123456789;
    
    printf("Testing balance inquiry wrapper with card number: %d\n", card_number);
    
    TransactionResult result = process_balance_inquiry_wrapper(card_number);
    
    printf("Transaction status: %s\n", 
           result.status == TRANSACTION_SUCCESS ? "SUCCESS" : 
           result.status == TRANSACTION_FAILED ? "FAILED" :
           result.status == TRANSACTION_PENDING ? "PENDING" : "CANCELED");
    printf("Message: %s\n", result.message);
    printf("Balance: %.2f\n", result.balance_after);
    
    return 0;
}
