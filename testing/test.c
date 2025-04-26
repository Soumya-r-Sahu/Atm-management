#include <stdio.h>
#include "../src/database/database.h"
#include "../src/validation/pin_validation.h"
#include "../src/utils/logger.h"

void test_fetchBalance() {
    float balance = fetchBalance(100041);
    if (balance == 500.00) {
        printf("fetchBalance test passed.\n");
    } else {
        printf("fetchBalance test failed.\n");
    }
}

void test_validatePIN() {
    if (validatePIN(1467, 1467)) {
        printf("validatePIN test passed.\n");
    } else {
        printf("validatePIN test failed.\n");
    }
}

int main() {
    test_fetchBalance();
    test_validatePIN();
    return 0;
}