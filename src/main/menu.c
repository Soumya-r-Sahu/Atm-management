#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../validation/card_num_validation.h"
#include "../validation/pin_validation.h"
#include "../database/database.h"
#include "../utils/logger.h"
#include "../utils/language_support.h"
#include "../transaction/transaction_manager.h"
#include "../database/customer_profile.h"

// Account Type enumeration
typedef enum {
    ACCOUNT_SAVINGS,
    ACCOUNT_CURRENT,
    ACCOUNT_CREDIT
} AccountType;

// Function to get account type string
const char* getAccountTypeString(AccountType type) {
    switch (type) {
        case ACCOUNT_SAVINGS: return getLocalizedText("SAVINGS");
        case ACCOUNT_CURRENT: return getLocalizedText("CURRENT");
        case ACCOUNT_CREDIT: return getLocalizedText("CREDIT");
        default: return getLocalizedText("UNKNOWN");
    }
}

// Function to select account type
AccountType selectAccountType() {
    int choice;
    printf("\n===== %s =====\n", getLocalizedText("SELECT_ACCOUNT_TYPE"));
    printf("1. %s\n", getLocalizedText("SAVINGS_ACCOUNT"));
    printf("2. %s\n", getLocalizedText("CURRENT_ACCOUNT"));
    printf("3. %s\n", getLocalizedText("CREDIT_ACCOUNT"));
    printf("%s: ", getLocalizedText("ENTER_CHOICE"));
    scanf("%d", &choice);
    
    switch (choice) {
        case 1: return ACCOUNT_SAVINGS;
        case 2: return ACCOUNT_CURRENT;
        case 3: return ACCOUNT_CREDIT;
        default: 
            printf("%s\n", getLocalizedText("INVALID_CHOICE_DEFAULT"));
            return ACCOUNT_SAVINGS;
    }
}

void displayMenu() {
    printf("\n===== %s =====\n", getLocalizedText("MAIN_MENU"));
    printf("1. %s\n", getLocalizedText("CASH_WITHDRAWAL"));
    printf("2. %s\n", getLocalizedText("BALANCE_INQUIRY"));
    printf("3. %s\n", getLocalizedText("MINI_STATEMENT"));
    printf("4. %s\n", getLocalizedText("FUND_TRANSFER"));
    printf("5. %s\n", getLocalizedText("CHANGE_PIN"));
    printf("6. %s\n", getLocalizedText("OTHERS"));
    printf("7. %s\n", getLocalizedText("EXIT"));
    printf("=====================\n");
}

void displayOthersMenu() {
    printf("\n===== %s =====\n", getLocalizedText("OTHERS_MENU"));
    printf("1. %s\n", getLocalizedText("FAST_CASH"));
    printf("2. %s\n", getLocalizedText("CARD_SERVICES"));
    printf("3. %s\n", getLocalizedText("CHEQUE_BOOK"));
    printf("4. %s\n", getLocalizedText("MOBILE_NUMBER_UPDATE"));
    printf("5. %s\n", getLocalizedText("AADHAAR_LINKING"));
    printf("6. %s\n", getLocalizedText("BACK"));
    printf("=====================\n");
}

void displayCardServicesMenu() {
    printf("\n===== %s =====\n", getLocalizedText("CARD_SERVICES"));
    printf("1. %s\n", getLocalizedText("BLOCK_CARD"));
    printf("2. %s\n", getLocalizedText("REQUEST_NEW_CARD"));
    printf("3. %s\n", getLocalizedText("CARD_ACTIVATION"));
    printf("4. %s\n", getLocalizedText("BACK"));
    printf("=====================\n");
}

void displayFastCashMenu() {
    printf("\n===== %s =====\n", getLocalizedText("FAST_CASH"));
    printf("1. %s ₹1000\n", getLocalizedText("AMOUNT"));
    printf("2. %s ₹2000\n", getLocalizedText("AMOUNT"));
    printf("3. %s ₹5000\n", getLocalizedText("AMOUNT"));
    printf("4. %s ₹10000\n", getLocalizedText("AMOUNT"));
    printf("5. %s\n", getLocalizedText("BACK"));
    printf("=====================\n");
}

void handleCardServices(int cardNumber, const char* username) {
    int choice;
    char inputBuffer[10];
    
    displayCardServicesMenu();
    printf("\n%s: ", getLocalizedText("ENTER_CHOICE"));
    
    if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
        if (sscanf(inputBuffer, "%d", &choice) != 1) {
            printf("%s\n", getLocalizedText("INVALID_INPUT_RETURNING"));
            return;
        }
    }
    
    switch (choice) {
        case 1: // Block Card
            printf("%s\n", getLocalizedText("CARD_BLOCK_WARNING"));
            printf("%s: ", getLocalizedText("CONFIRM_PROMPT"));
            char confirm;
            scanf(" %c", &confirm);
            if (confirm == 'Y' || confirm == 'y') {
                // Set card status to "Locked" in database
                if (updateCardStatus(cardNumber, "Locked")) {
                    printf("%s\n", getLocalizedText("CARD_BLOCKED_SUCCESS"));
                    printf("%s\n", getLocalizedText("CONTACT_CUSTOMER_SERVICE"));
                } else {
                    printf("%s\n", getLocalizedText("CARD_BLOCK_FAIL"));
                }
            }
            break;
            
        case 2: // Request New Card
            printf("%s\n", getLocalizedText("CARD_REQUEST_REGISTERED"));
            printf("%s\n", getLocalizedText("CARD_DELIVERY_TIME"));
            // Log the request
            logTransaction(cardNumber, TRANSACTION_CARD_REQUEST, 0.0, 1);
            break;
            
        case 3: // Card Activation
            printf("%s\n", getLocalizedText("CARD_ACTIVATION_CALL"));
            printf("%s\n", getLocalizedText("CARD_ACTIVATION_VISIT"));
            break;
            
        case 4: // Back
            return;
            
        default:
            printf("%s\n", getLocalizedText("INVALID_CHOICE"));
    }
}

void handleFastCash(int cardNumber, const char* username) {
    int choice;
    float withdrawAmount = 0;
    TransactionResult result;
    const char* phoneNumber;
    char wantReceipt;
    AccountType accountType;
    
    displayFastCashMenu();
    printf("\n%s: ", getLocalizedText("ENTER_CHOICE"));
    scanf("%d", &choice);
    
    switch (choice) {
        case 1: withdrawAmount = 1000; break;
        case 2: withdrawAmount = 2000; break;
        case 3: withdrawAmount = 5000; break;
        case 4: withdrawAmount = 10000; break;
        case 5: return; // Back to previous menu
        default:
            printf("%s\n", getLocalizedText("INVALID_CHOICE_RETURNING"));
            return;
    }
    
    // Select account type
    accountType = selectAccountType();
    printf("%s: %s\n", getLocalizedText("SELECTED_ACCOUNT"), getAccountTypeString(accountType));
    
    // Perform withdrawal
    result = performWithdrawal(cardNumber, withdrawAmount, username);
    if (result.success) {
        printf("%s\n", result.message);
        
        // Ask if receipt is needed
        printf("%s: ", getLocalizedText("RECEIPT_QUESTION"));
        scanf(" %c", &wantReceipt);
        
        if (wantReceipt == 'Y' || wantReceipt == 'y') {
            // Generate and print receipt
            phoneNumber = getCustomerPhoneNumber(cardNumber);
            if (phoneNumber != NULL && strlen(phoneNumber) > 0) {
                generateReceipt(cardNumber, TRANSACTION_WITHDRAWAL, withdrawAmount, 
                               result.newBalance, phoneNumber);
                printf("%s\n", getLocalizedText("RECEIPT_SENT"));
            } else {
                printf("%s\n", getLocalizedText("NO_MOBILE"));
            }
        }
    } else {
        printf("%s: %s\n", getLocalizedText("ERROR"), result.message);
    }
}

void handleFundTransfer(int cardNumber, const char* username) {
    int targetAccountNumber;
    float transferAmount;
    TransactionResult result;
    const char* phoneNumber;
    char wantReceipt;
    AccountType accountType;
    
    // Select account type
    accountType = selectAccountType();
    printf("%s: %s\n", getLocalizedText("SELECTED_ACCOUNT"), getAccountTypeString(accountType));
    
    // Get target account number
    printf("%s: ", getLocalizedText("ENTER_BENEFICIARY_ACCOUNT"));
    scanf("%d", &targetAccountNumber);
    
    // Get transfer amount
    printf("%s: ₹", getLocalizedText("ENTER_TRANSFER_AMOUNT"));
    scanf("%f", &transferAmount);
    
    // Confirm transfer details
    printf("\n===== %s =====\n", getLocalizedText("CONFIRM_TRANSFER_DETAILS"));
    printf("%s: %d\n", getLocalizedText("TO_ACCOUNT_NUMBER"), targetAccountNumber);
    printf("%s: ₹%.2f\n", getLocalizedText("AMOUNT"), transferAmount);
    printf("%s: %s\n", getLocalizedText("FROM_ACCOUNT"), getAccountTypeString(accountType));
    
    char confirm;
    printf("\n%s: ", getLocalizedText("CONFIRM_TRANSFER"));
    scanf(" %c", &confirm);
    
    if (confirm == 'Y' || confirm == 'y') {
        // Perform transfer
        result = performFundTransfer(cardNumber, targetAccountNumber, transferAmount, username);
        
        if (result.success) {
            printf("%s\n", result.message);
            
            // Ask if receipt is needed
            printf("%s: ", getLocalizedText("RECEIPT_QUESTION"));
            scanf(" %c", &wantReceipt);
            
            if (wantReceipt == 'Y' || wantReceipt == 'y') {
                // Generate and print receipt
                phoneNumber = getCustomerPhoneNumber(cardNumber);
                if (phoneNumber != NULL && strlen(phoneNumber) > 0) {
                    generateReceipt(cardNumber, TRANSACTION_FUND_TRANSFER, transferAmount, 
                                   result.newBalance, phoneNumber);
                    printf("%s\n", getLocalizedText("RECEIPT_SENT"));
                } else {
                    printf("%s\n", getLocalizedText("NO_MOBILE"));
                }
            }
        } else {
            printf("%s: %s\n", getLocalizedText("ERROR"), result.message);
        }
    } else {
        printf("%s\n", getLocalizedText("TRANSFER_CANCELLED"));
    }
}

void handleOthersMenu(int cardNumber, const char* username) {
    int choice;
    char inputBuffer[10];
    
    system("cls");
    printf("\n===== %s =====\n\n", getLocalizedText("OTHER_SERVICES"));
    
    printf("1. %s\n", getLocalizedText("LANGUAGE_CHANGE"));
    printf("2. %s\n", getLocalizedText("TRANSACTION_HISTORY"));
    printf("3. %s\n", getLocalizedText("ACCOUNT_DETAILS"));
    printf("4. %s\n", getLocalizedText("BACK_TO_MAIN_MENU"));
    printf("\n%s: ", getLocalizedText("ENTER_CHOICE"));
    
    if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
        if (sscanf(inputBuffer, "%d", &choice) != 1) {
            printf("%s\n", getLocalizedText("INVALID_INPUT_TRY_AGAIN"));
            sleep(2);
            handleOthersMenu(cardNumber, username);
            return;
        }
    }
    
    switch (choice) {
        case 1: // Language Change
            handleLanguageChange();
            handleOthersMenu(cardNumber, username);
            break;
            
        case 2: // Transaction History
            handleTransactionHistory(cardNumber);
            printf("\n%s ", getLocalizedText("PRESS_ENTER_TO_CONTINUE"));
            getchar();
            handleOthersMenu(cardNumber, username);
            break;
            
        case 3: // Account Details
            handleAccountDetails(cardNumber);
            printf("\n%s ", getLocalizedText("PRESS_ENTER_TO_CONTINUE"));
            getchar();
            handleOthersMenu(cardNumber, username);
            break;
            
        case 4: // Back to Main Menu
            handleUserChoice(cardNumber, username);
            break;
            
        default:
            printf("%s\n", getLocalizedText("INVALID_CHOICE"));
            sleep(2);
            handleOthersMenu(cardNumber, username);
    }
}

// Function to handle user choices from main menu
void handleUserChoice(int choice, int cardNumber, const char* username) {
    char inputBuffer[10];
    
    switch (choice) {
        case 1: // Balance Enquiry
            handleBalanceEnquiry(cardNumber);
            printf("\n%s ", getLocalizedText("PRESS_ENTER_TO_CONTINUE"));
            getchar();
            break;
            
        case 2: // Cash Withdrawal
            handleCashWithdrawal(cardNumber);
            printf("\n%s ", getLocalizedText("PRESS_ENTER_TO_CONTINUE"));
            getchar();
            break;
            
        case 3: // Cash Deposit
            handleCashDeposit(cardNumber);
            printf("\n%s ", getLocalizedText("PRESS_ENTER_TO_CONTINUE"));
            getchar();
            break;
            
        case 4: // Fund Transfer
            handleFundTransfer(cardNumber, username);
            printf("\n%s ", getLocalizedText("PRESS_ENTER_TO_CONTINUE"));
            getchar();
            break;
            
        case 5: // Change PIN
            handleChangePin(cardNumber);
            printf("\n%s ", getLocalizedText("PRESS_ENTER_TO_CONTINUE"));
            getchar();
            break;
            
        case 6: // Others
            handleOthersMenu(cardNumber, username);
            break;
            
        case 7: // Exit - handled in main.c
            break;
            
        default:
            printf("%s\n", getLocalizedText("INVALID_CHOICE"));
            sleep(2);
    }
}