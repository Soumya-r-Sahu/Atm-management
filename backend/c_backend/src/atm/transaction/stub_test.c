#include "../../../../../include/common/database/core_banking_interface.h"
#include "../../../../../include/common/database/account_management.h"
#include "../../../../../include/common/database/card_account_management.h"
#include <stdio.h>

int main() {
    double balance = 0.0;
    char account_number[20];
    
    printf("Testing CBS stubs\n");
    
    // Test a few core functions
    if (cbs_card_exists(123456789)) {
        printf("Card exists\n");
    }
    
    if (cbs_get_balance_by_card("123456789", &balance)) {
        printf("Balance: %.2f\n", balance);
    }
    
    if (cbs_get_account_by_card("123456789", account_number, sizeof(account_number))) {
        printf("Account number: %s\n", account_number);
    }
    
    return 0;
}
