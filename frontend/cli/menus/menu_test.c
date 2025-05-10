/**
 * @file menu_test.c
 * @brief Test script for the Core Banking System menu structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../include/frontend/menus/menu_types.h"
#include "../../../include/frontend/menus/menu_system.h"
#include "../../../include/frontend/cli/menu_utils.h"

/**
 * @brief Test the main menu
 */
void testMainMenu() {
    printf("Testing Main Menu...\n");
    printf("This will launch the main menu. Navigate through the options to test.\n");
    printf("Press Enter to start...");
    getchar();
    
    runMainMenu();
    
    // Note: runMainMenu has an infinite loop, so we won't reach this point
    printf("Main Menu Test Complete.\n\n");
}

/**
 * @brief Test the customer menu
 */
void testCustomerMenu() {
    printf("Testing Customer Menu...\n");
    printf("This will launch the customer menu with a test user.\n");
    printf("Press Enter to start...");
    getchar();
    
    runCustomerMenu("testcustomer", USER_CUSTOMER);
    
    printf("Customer Menu Test Complete.\n\n");
}

/**
 * @brief Test the account menu
 */
void testAccountMenu() {
    printf("Testing Account Menu...\n");
    printf("This will launch the account menu with a test user.\n");
    printf("Press Enter to start...");
    getchar();
    
    runAccountMenu("testcustomer");
    
    printf("Account Menu Test Complete.\n\n");
}

/**
 * @brief Test the transaction menu
 */
void testTransactionMenu() {
    printf("Testing Transaction Menu...\n");
    printf("This will launch the transaction menu with a test user.\n");
    printf("Press Enter to start...");
    getchar();
    
    runTransactionMenu("testcustomer");
    
    printf("Transaction Menu Test Complete.\n\n");
}

/**
 * @brief Test the beneficiary menu
 */
void testBeneficiaryMenu() {
    printf("Testing Beneficiary Menu...\n");
    printf("This will launch the beneficiary menu with a test user.\n");
    printf("Press Enter to start...");
    getchar();
    
    runBeneficiaryMenu("testcustomer");
    
    printf("Beneficiary Menu Test Complete.\n\n");
}

/**
 * @brief Test the bill payment menu
 */
void testBillPaymentMenu() {
    printf("Testing Bill Payment Menu...\n");
    printf("This will launch the bill payment menu with a test user.\n");
    printf("Press Enter to start...");
    getchar();
    
    runBillPaymentMenu("testcustomer");
    
    printf("Bill Payment Menu Test Complete.\n\n");
}

/**
 * @brief Test the ATM operations menu
 */
void testAtmOperationsMenu() {
    printf("Testing ATM Operations Menu...\n");
    printf("This will launch the ATM operations menu with a test card number.\n");
    printf("Press Enter to start...");
    getchar();
    
    runATMOperationsMenu(1234567890);
    
    printf("ATM Operations Menu Test Complete.\n\n");
}

/**
 * @brief Test the admin menu
 */
void testAdminMenu() {
    printf("Testing Admin Menu...\n");
    printf("This will launch the admin menu with a test admin user.\n");
    printf("Press Enter to start...");
    getchar();
    
    runAdminMenu("testadmin", USER_ADMIN);
    
    printf("Admin Menu Test Complete.\n\n");
    
    printf("Testing Super Admin Menu...\n");
    printf("This will launch the admin menu with a test super admin user.\n");
    printf("Press Enter to start...");
    getchar();
    
    runAdminMenu("testsuperadmin", USER_ADMIN_SUPER);
    
    printf("Super Admin Menu Test Complete.\n\n");
}

/**
 * @brief Test the CBS admin menu
 */
void testCbsAdminMenu() {
    printf("Testing CBS Admin Menu...\n");
    printf("This will launch the CBS admin menu.\n");
    printf("Press Enter to start...");
    getchar();
    
    runCBSAdminMenu();
    
    printf("CBS Admin Menu Test Complete.\n\n");
}

/**
 * @brief Test menu utility functions
 */
void testMenuUtils() {
    printf("Testing Menu Utilities...\n");
    
    // Test clearScreen
    printf("Testing clearScreen()...\n");
    printf("Press Enter to clear the screen...");
    getchar();
    clearScreen();
    printf("Screen cleared.\n");
    
    // Test printHeader
    printf("Testing printHeader()...\n");
    printHeader("TEST HEADER");
    
    // Test printLine
    printf("Testing printLine()...\n");
    printLine('-', 50);
    
    // Test message functions
    printf("Testing message functions...\n");
    printSuccess("This is a success message.");
    printError("This is an error message.");
    printInfo("This is an info message.");
    printWarning("This is a warning message.");
    
    // Test getString
    char stringBuffer[50];
    printf("Testing getString()...\n");
    getString("Enter a test string: ", stringBuffer, sizeof(stringBuffer));
    printf("You entered: %s\n", stringBuffer);
    
    // Test getInteger
    printf("Testing getInteger()...\n");
    int intValue = getInteger("Enter a number", 1, 100);
    printf("You entered: %d\n", intValue);
    
    // Test getFloat
    printf("Testing getFloat()...\n");
    float floatValue = getFloat("Enter a decimal number", 0.0, 100.0);
    printf("You entered: %.2f\n", floatValue);
    
    // Test getConfirmation
    printf("Testing getConfirmation()...\n");
    bool confirmed = getConfirmation("Do you confirm");
    printf("You %s\n", confirmed ? "confirmed" : "denied");
    
    // Test formatCurrency
    char currencyBuffer[20];
    printf("Testing formatCurrency()...\n");
    formatCurrency(1234.56, currencyBuffer, sizeof(currencyBuffer), "$");
    printf("Formatted currency: %s\n", currencyBuffer);
    
    // Test formatDate
    char dateBuffer[20];
    printf("Testing formatDate()...\n");
    formatDate(2023, 5, 15, dateBuffer, sizeof(dateBuffer));
    printf("Formatted date: %s\n", dateBuffer);
    
    // Test displayMenu
    const char *options[] = {
        "Option 1",
        "Option 2",
        "Option 3",
        "Option 4",
        "Exit"
    };
    
    printf("Testing displayMenu()...\n");
    int menuChoice = displayMenu("TEST MENU", options, 5);
    printf("You selected option %d\n", menuChoice);
    
    printf("Menu Utilities Test Complete.\n\n");
}

/**
 * @brief Display the test menu
 */
void displayTestMenu() {
    clearScreen();
    printHeader("MENU STRUCTURE TEST");
    
    printf("\n");
    printf("1. Test Main Menu\n");
    printf("2. Test Customer Menu\n");
    printf("3. Test Account Menu\n");
    printf("4. Test Transaction Menu\n");
    printf("5. Test Beneficiary Menu\n");
    printf("6. Test Bill Payment Menu\n");
    printf("7. Test ATM Operations Menu\n");
    printf("8. Test Admin Menu\n");
    printf("9. Test CBS Admin Menu\n");
    printf("10. Test Menu Utilities\n");
    printf("11. Exit Test\n");
    
    printf("\nSelect a menu to test.\n");
}

/**
 * @brief Main function
 */
int main() {
    bool running = true;
    
    while (running) {
        displayTestMenu();
        
        int choice;
        printf("\nEnter your choice (1-11): ");
        if (scanf("%d", &choice) != 1) {
            // Clear input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            sleep(2);
            continue;
        }
        
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        switch (choice) {
            case 1:
                testMainMenu();
                break;
                
            case 2:
                testCustomerMenu();
                break;
                
            case 3:
                testAccountMenu();
                break;
                
            case 4:
                testTransactionMenu();
                break;
                
            case 5:
                testBeneficiaryMenu();
                break;
                
            case 6:
                testBillPaymentMenu();
                break;
                
            case 7:
                testAtmOperationsMenu();
                break;
                
            case 8:
                testAdminMenu();
                break;
                
            case 9:
                testCbsAdminMenu();
                break;
                
            case 10:
                testMenuUtils();
                break;
                
            case 11:
                running = false;
                printf("\nExiting test program. Goodbye!\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
    
    return 0;
}
