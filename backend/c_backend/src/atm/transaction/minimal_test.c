#include "../../../../../include/atm/transaction/transaction_processor.h"
#include <stdio.h>

// A simple test function for the balance inquiry feature
int main() {
    int card_number = 123456789;
    
    printf("Testing balance inquiry with card number: %d\n", card_number);
    
    TransactionResult result = process_balance_inquiry(card_number);
    
    printf("Transaction status: %s\n", 
           result.status == TRANSACTION_SUCCESS ? "SUCCESS" : 
           result.status == TRANSACTION_FAILED ? "FAILED" :
           result.status == TRANSACTION_PENDING ? "PENDING" : "CANCELED");
    printf("Message: %s\n", result.message);
    printf("Balance: %.2f\n", result.balance_after);
    
    return 0;
}
