/**
 * @file bill_payment_menu.c
 * @brief Implementation of bill payment menu for the Core Banking System
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "../../../include/common/utils/logger.h"
#include "../../../include/common/database/database.h"
#include "../../../include/frontend/cli/menu_utils.h"
#include "../../../include/frontend/menus/menu_system.h"
#include "../../../include/frontend/menus/bill_payment_menu.h"

// Bill types
#define BILL_ELECTRICITY       1
#define BILL_WATER             2
#define BILL_MOBILE            3
#define BILL_DTH               4
#define BILL_GAS               5

/**
 * @brief Display the bill payment menu
 * @param username Customer username
 */
static void displayBillPaymentMenu(const char *username) {
    char title[100];
    sprintf(title, "BILL PAYMENTS - %s", username);
    printHeader(title);
    
    printf("\n");
    printf("1. Electricity Bill\n");
    printf("2. Water Bill\n");
    printf("3. Mobile Recharge\n");
    printf("4. DTH Recharge\n");
    printf("5. Gas Bill\n");
    printf("6. Manage Scheduled Payments\n");
    printf("7. Return to Main Menu\n");
}

/**
 * @brief Run the bill payment menu
 * @param username Customer username
 */
void runBillPaymentMenu(const char *username) {
    bool running = true;
    
    while (running) {
        clearScreen();
        displayBillPaymentMenu(username);
        
        int choice;
        printf("\nEnter your choice (1-7): ");
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
            case 1: // Electricity Bill
                payUtilityBill(username, BILL_ELECTRICITY);
                break;
                
            case 2: // Water Bill
                payUtilityBill(username, BILL_WATER);
                break;
                
            case 3: // Mobile Recharge
                payUtilityBill(username, BILL_MOBILE);
                break;
                
            case 4: // DTH Recharge
                payUtilityBill(username, BILL_DTH);
                break;
                
            case 5: // Gas Bill
                payUtilityBill(username, BILL_GAS);
                break;
                
            case 6: // Manage Scheduled Payments
                manageBillSchedules(username);
                break;
                
            case 7: // Return to Main Menu
                running = false;
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}

/**
 * @brief Pay utility bill
 * @param username Customer username
 * @param billType Type of bill to pay
 */
void payUtilityBill(const char *username, int billType) {
    clearScreen();
    
    const char *billTypeName;
    switch (billType) {
        case BILL_ELECTRICITY: billTypeName = "ELECTRICITY BILL PAYMENT"; break;
        case BILL_WATER: billTypeName = "WATER BILL PAYMENT"; break;
        case BILL_MOBILE: billTypeName = "MOBILE RECHARGE"; break;
        case BILL_DTH: billTypeName = "DTH RECHARGE"; break;
        case BILL_GAS: billTypeName = "GAS BILL PAYMENT"; break;
        default: billTypeName = "BILL PAYMENT"; break;
    }
    
    printHeader(billTypeName);
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    printf("\nFrom Account: 1234567890 (Savings)\n");
    printf("Available Balance: ");
    printCurrency(25000.75, "₹");
    printf("\n\n");
    
    char billerName[50] = {0};
    
    if (billType == BILL_MOBILE || billType == BILL_DTH) {
        printf("Service Provider:\n");
        
        if (billType == BILL_MOBILE) {
            printf("1. Airtel\n");
            printf("2. Jio\n");
            printf("3. Vodafone Idea\n");
            printf("4. BSNL\n");
        } else { // DTH
            printf("1. Tata Sky\n");
            printf("2. Dish TV\n");
            printf("3. Airtel DTH\n");
            printf("4. Sun Direct\n");
        }
        
        int providerChoice = getInteger("Select Provider", 1, 4);
        
        if (billType == BILL_MOBILE) {
            switch (providerChoice) {
                case 1: strcpy(billerName, "Airtel"); break;
                case 2: strcpy(billerName, "Jio"); break;
                case 3: strcpy(billerName, "Vodafone Idea"); break;
                case 4: strcpy(billerName, "BSNL"); break;
            }
        } else { // DTH
            switch (providerChoice) {
                case 1: strcpy(billerName, "Tata Sky"); break;
                case 2: strcpy(billerName, "Dish TV"); break;
                case 3: strcpy(billerName, "Airtel DTH"); break;
                case 4: strcpy(billerName, "Sun Direct"); break;
            }
        }
    } else {
        if (billType == BILL_ELECTRICITY) {
            strcpy(billerName, "Tata Power");
        } else if (billType == BILL_WATER) {
            strcpy(billerName, "Delhi Jal Board");
        } else if (billType == BILL_GAS) {
            strcpy(billerName, "Indraprastha Gas Limited");
        }
    }
    
    char consumerNumber[20];
    getString("Enter Consumer Number/Mobile Number: ", consumerNumber, sizeof(consumerNumber));
    
    char billNumber[20] = {0};
    if (billType != BILL_MOBILE && billType != BILL_DTH) {
        getString("Enter Bill Number (optional): ", billNumber, sizeof(billNumber));
    }
    
    double amount;
    if (billType == BILL_MOBILE || billType == BILL_DTH) {
        printf("\nSelect Plan:\n");
        printf("1. ₹199 - 1GB/day, 28 days\n");
        printf("2. ₹399 - 2GB/day, 56 days\n");
        printf("3. ₹699 - 2GB/day, 84 days\n");
        printf("4. Other Amount\n");
        
        int planChoice = getInteger("Select Plan", 1, 4);
        
        if (planChoice == 1) {
            amount = 199.0;
        } else if (planChoice == 2) {
            amount = 399.0;
        } else if (planChoice == 3) {
            amount = 699.0;
        } else {
            amount = getDouble("Enter Amount", 1.0, 25000.75);
        }
    } else {
        amount = getDouble("Enter Amount", 1.0, 25000.75);
    }
    
    printf("\nBill Payment Details:\n");
    printf("From Account: 1234567890 (Savings)\n");
    printf("Biller: %s\n", billerName);
    printf("Consumer Number: %s\n", consumerNumber);
    if (strlen(billNumber) > 0) {
        printf("Bill Number: %s\n", billNumber);
    }
    printf("Amount: ");
    printCurrency(amount, "₹");
    printf("\n");
    
    printf("\nAutomatic Payment Options:\n");
    printf("1. Pay Once\n");
    printf("2. Set Up Recurring Payment\n");
    
    int paymentOption = getInteger("Select Option", 1, 2);
    
    if (paymentOption == 2) {
        printf("\nRecurrence:\n");
        printf("1. Monthly\n");
        printf("2. Quarterly\n");
        
        int recurrenceOption = getInteger("Select Recurrence", 1, 2);
        
        printf("Recurring payment will be set up for %s on the %dth of each %s\n",
               billerName,
               1 + (rand() % 28), // Random date between 1-28
               (recurrenceOption == 1) ? "month" : "quarter");
    }
    
    if (getConfirmation("\nConfirm Payment")) {
        // In a real application, this would update the database
        printSuccess("Bill payment completed successfully!");
        printf("Transaction Reference: TXN%d\n", rand() % 1000000000);
        
        LOG_INFO("Bill payment completed: %s paid %.2f to %s (Consumer: %s)", 
                 username, amount, billerName, consumerNumber);
    } else {
        printInfo("Payment cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief Manage bill payment schedules
 * @param username Customer username
 */
void manageBillSchedules(const char *username) {
    clearScreen();
    printHeader("MANAGE SCHEDULED PAYMENTS");
    
    // In a real application, this would fetch data from the database
    // For demo purposes, we'll use mock data
    
    printLine('-', SCREEN_WIDTH);
    printf("%-5s %-20s %-15s %-15s %-15s %-15s\n", 
           "ID", "Biller", "Amount", "Frequency", "Next Date", "Status");
    printLine('-', SCREEN_WIDTH);
    
    printf("%-5d %-20s ", 1, "Tata Power");
    printCurrency(1200.00, "₹");
    printf("  %-15s %-15s %-15s\n", "Monthly", "2023-06-15", "Active");
    
    printf("%-5d %-20s ", 2, "Airtel");
    printCurrency(399.00, "₹");
    printf("  %-15s %-15s %-15s\n", "Quarterly", "2023-07-01", "Active");
    
    printf("%-5d %-20s ", 3, "Dish TV");
    printCurrency(499.00, "₹");
    printf("  %-15s %-15s %-15s\n", "Monthly", "2023-06-05", "Active");
    
    printLine('-', SCREEN_WIDTH);
    
    printf("\n1. Modify Schedule\n");
    printf("2. Disable Schedule\n");
    printf("3. Enable Schedule\n");
    printf("4. Return to Bill Payment Menu\n");
    
    int choice = getInteger("\nEnter your choice", 1, 4);
    
    if (choice == 4) {
        return;
    }
    
    int scheduleId = getInteger("Enter Schedule ID", 1, 3);
    
    // Get the biller name based on the ID (in a real app, would fetch from database)
    const char *billerName = "";
    switch (scheduleId) {
        case 1: billerName = "Tata Power"; break;
        case 2: billerName = "Airtel"; break;
        case 3: billerName = "Dish TV"; break;
    }
    
    switch (choice) {
        case 1: // Modify
            {
                printf("\nModify Schedule for %s:\n", billerName);
                printf("1. Amount\n");
                printf("2. Frequency\n");
                printf("3. Next Date\n");
                
                int modifyChoice = getInteger("Select field to modify", 1, 3);
                
                switch (modifyChoice) {
                    case 1: // Amount
                        {
                            double newAmount = getDouble("Enter new amount", 1.0, 10000.0);
                            printSuccess("Amount updated successfully!");
                            LOG_INFO("Schedule amount modified by %s for %s: %.2f", 
                                     username, billerName, newAmount);
                        }
                        break;
                    case 2: // Frequency
                        {
                            printf("\nNew Frequency:\n");
                            printf("1. Monthly\n");
                            printf("2. Quarterly\n");
                            printf("3. Half-yearly\n");
                            printf("4. Yearly\n");
                            
                            int freqChoice = getInteger("Select frequency", 1, 4);
                            printSuccess("Frequency updated successfully!");
                            LOG_INFO("Schedule frequency modified by %s for %s", 
                                     username, billerName);
                        }
                        break;
                    case 3: // Next Date
                        {
                            char newDate[20];
                            getString("Enter new date (YYYY-MM-DD): ", newDate, sizeof(newDate));
                            printSuccess("Next date updated successfully!");
                            LOG_INFO("Schedule date modified by %s for %s: %s", 
                                     username, billerName, newDate);
                        }
                        break;
                }
            }
            break;
            
        case 2: // Disable
            if (getConfirmation("\nConfirm disable scheduled payment for " + billerName)) {
                printSuccess("Scheduled payment disabled successfully!");
                LOG_INFO("Schedule disabled by %s for %s", username, billerName);
            } else {
                printInfo("Operation cancelled.");
            }
            break;
            
        case 3: // Enable
            if (getConfirmation("\nConfirm enable scheduled payment for " + billerName)) {
                printSuccess("Scheduled payment enabled successfully!");
                LOG_INFO("Schedule enabled by %s for %s", username, billerName);
            } else {
                printInfo("Operation cancelled.");
            }
            break;
    }
    
    pauseExecution();
}
