/**
 * @file atm_operations_menu.c
 * @brief Implementation of ATM operations menu for the Core Banking System
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
#include "../../../include/frontend/menus/atm_operations_menu.h"

/**
 * @brief Display the ATM operations menu
 * @param operatorId ATM operator ID
 */
static void displayAtmOperationsMenu(const char *operatorId) {
    char title[100];
    sprintf(title, "ATM OPERATIONS MENU - %s", operatorId);
    printHeader(title);
    
    printf("\n");
    printf("1. Cash Management\n");
    printf("2. Maintenance Operations\n");
    printf("3. Device Status\n");
    printf("4. Transaction Report\n");
    printf("5. Logout\n");
}

/**
 * @brief Run the ATM operations menu
 * @param cardNumber ATM card number
 */
void runATMOperationsMenu(int cardNumber) {
    // Convert card number to operator ID for compatibility with existing functions
    char operatorId[20];
    sprintf(operatorId, "OP%d", cardNumber);
    bool running = true;
    
    while (running) {
        clearScreen();
        displayAtmOperationsMenu(operatorId);
        
        int choice;
        printf("\nEnter your choice (1-5): ");
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
            case 1: // Cash Management
                runCashManagementMenu(operatorId);
                break;
                
            case 2: // Maintenance Operations
                runMaintenanceMenu(operatorId);
                break;
                
            case 3: // Device Status
                runDeviceStatusMenu(operatorId);
                break;
                
            case 4: // Transaction Report
                generateTransactionReport(operatorId);
                break;
                
            case 5: // Logout
                running = false;
                LOG_INFO("ATM Operator %s logged out", operatorId);
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                sleep(2);
                break;
        }
    }
}

/**
 * @brief Run the cash management menu
 * @param operatorId ATM operator ID
 */
void runCashManagementMenu(const char *operatorId) {
    bool running = true;
    
    while (running) {
        clearScreen();
        printHeader("CASH MANAGEMENT");
        
        printf("\n");
        printf("1. Check Cash Levels\n");
        printf("2. Cash Refill\n");
        printf("3. Cash Reconciliation\n");
        printf("4. Return to ATM Operations Menu\n");
        
        int choice;
        printf("\nEnter your choice (1-4): ");
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
            case 1: // Check Cash Levels
                checkCashLevels(operatorId);
                break;
                
            case 2: // Cash Refill
                cashRefill(operatorId);
                break;
                
            case 3: // Cash Reconciliation
                cashReconciliation(operatorId);
                break;
                
            case 4: // Return to ATM Operations Menu
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
 * @brief Run the maintenance menu
 * @param operatorId ATM operator ID
 */
void runMaintenanceMenu(const char *operatorId) {
    bool running = true;
    
    while (running) {
        clearScreen();
        printHeader("MAINTENANCE OPERATIONS");
        
        printf("\n");
        printf("1. Printer Maintenance\n");
        printf("2. Card Reader Maintenance\n");
        printf("3. Cash Dispenser Maintenance\n");
        printf("4. Software Update\n");
        printf("5. Return to ATM Operations Menu\n");
        
        int choice;
        printf("\nEnter your choice (1-5): ");
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
            case 1: // Printer Maintenance
                performMaintenance(operatorId, "Printer");
                break;
                
            case 2: // Card Reader Maintenance
                performMaintenance(operatorId, "Card Reader");
                break;
                
            case 3: // Cash Dispenser Maintenance
                performMaintenance(operatorId, "Cash Dispenser");
                break;
                
            case 4: // Software Update
                performSoftwareUpdate(operatorId);
                break;
                
            case 5: // Return to ATM Operations Menu
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
 * @brief Run the device status menu
 * @param operatorId ATM operator ID
 */
void runDeviceStatusMenu(const char *operatorId) {
    bool running = true;
    
    while (running) {
        clearScreen();
        printHeader("DEVICE STATUS");
        
        printf("\n");
        printf("1. Hardware Status\n");
        printf("2. Software Status\n");
        printf("3. Network Status\n");
        printf("4. Error Log\n");
        printf("5. Return to ATM Operations Menu\n");
        
        int choice;
        printf("\nEnter your choice (1-5): ");
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
            case 1: // Hardware Status
                showDeviceStatus(operatorId, "Hardware");
                break;
                
            case 2: // Software Status
                showDeviceStatus(operatorId, "Software");
                break;
                
            case 3: // Network Status
                showDeviceStatus(operatorId, "Network");
                break;
                
            case 4: // Error Log
                showErrorLog(operatorId);
                break;
                
            case 5: // Return to ATM Operations Menu
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
 * @brief Check cash levels
 * @param operatorId ATM operator ID
 */
void checkCashLevels(const char *operatorId) {
    clearScreen();
    printHeader("CASH LEVELS");
    
    // In a real application, this would fetch data from the database
    // For demo purposes, we'll use mock data
    
    printf("\nATM ID: ATM001\n");
    printf("Location: Main Branch, Delhi\n");
    printf("Last Refill: 2023-05-01 09:30:15\n\n");
    
    printLine('-', SCREEN_WIDTH);
    printf("%-20s %-20s %-15s %-15s\n", "Denomination", "Count", "Amount", "Status");
    printLine('-', SCREEN_WIDTH);
    
    printf("%-20s %-20d ", "₹2000 Notes", 150);
    printCurrency(300000.00, "₹");
    printf("%15s\n", "OK");
    
    printf("%-20s %-20d ", "₹500 Notes", 400);
    printCurrency(200000.00, "₹");
    printf("%15s\n", "OK");
    
    printf("%-20s %-20d ", "₹200 Notes", 250);
    printCurrency(50000.00, "₹");
    printf("%15s\n", "Low");
    
    printf("%-20s %-20d ", "₹100 Notes", 100);
    printCurrency(10000.00, "₹");
    printf("%15s\n", "Critical");
    
    printLine('-', SCREEN_WIDTH);
    
    printf("\nTotal Cash: ");
    printCurrency(560000.00, "₹");
    printf("\n\n");
    
    printf("Status Report:\n");
    printf("- ₹200 Notes: Low level, refill recommended\n");
    printf("- ₹100 Notes: Critical level, refill required\n");
    
    LOG_INFO("Cash levels checked by operator %s", operatorId);
    
    pauseExecution();
}

/**
 * @brief Cash refill
 * @param operatorId ATM operator ID
 */
void cashRefill(const char *operatorId) {
    clearScreen();
    printHeader("CASH REFILL");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    printf("\nATM ID: ATM001\n");
    printf("Location: Main Branch, Delhi\n\n");
    
    printf("Current Cash Levels:\n");
    printf("- ₹2000 Notes: 150 (₹300,000)\n");
    printf("- ₹500 Notes: 400 (₹200,000)\n");
    printf("- ₹200 Notes: 250 (₹50,000)\n");
    printf("- ₹100 Notes: 100 (₹10,000)\n");
    printf("Total: ₹560,000\n\n");
    
    printf("Enter Refill Amounts:\n");
    
    int notes2000 = getInteger("₹2000 Notes to add", 0, 500);
    int notes500 = getInteger("₹500 Notes to add", 0, 1000);
    int notes200 = getInteger("₹200 Notes to add", 0, 1000);
    int notes100 = getInteger("₹100 Notes to add", 0, 1000);
    
    double totalRefill = 2000 * notes2000 + 500 * notes500 + 200 * notes200 + 100 * notes100;
    
    printf("\nRefill Summary:\n");
    printf("- ₹2000 Notes: +%d (₹%d)\n", notes2000, 2000 * notes2000);
    printf("- ₹500 Notes: +%d (₹%d)\n", notes500, 500 * notes500);
    printf("- ₹200 Notes: +%d (₹%d)\n", notes200, 200 * notes200);
    printf("- ₹100 Notes: +%d (₹%d)\n", notes100, 100 * notes100);
    printf("Total Refill: ");
    printCurrency(totalRefill, "₹");
    printf("\n\n");
    
    printf("New Cash Levels:\n");
    printf("- ₹2000 Notes: %d (₹%d)\n", 150 + notes2000, 2000 * (150 + notes2000));
    printf("- ₹500 Notes: %d (₹%d)\n", 400 + notes500, 500 * (400 + notes500));
    printf("- ₹200 Notes: %d (₹%d)\n", 250 + notes200, 200 * (250 + notes200));
    printf("- ₹100 Notes: %d (₹%d)\n", 100 + notes100, 100 * (100 + notes100));
    printf("Total: ");
    printCurrency(560000.00 + totalRefill, "₹");
    printf("\n");
    
    char remarks[200];
    getString("\nEnter Remarks/Notes (optional): ", remarks, sizeof(remarks));
    
    if (getConfirmation("\nConfirm Cash Refill")) {
        // In a real application, this would update the database
        printSuccess("Cash refill completed successfully!");
        printf("Reference Number: REF%d\n", rand() % 1000000000);
        
        LOG_INFO("Cash refill completed by operator %s: Added ₹%.2f", operatorId, totalRefill);
    } else {
        printInfo("Cash refill cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief Cash reconciliation
 * @param operatorId ATM operator ID
 */
void cashReconciliation(const char *operatorId) {
    clearScreen();
    printHeader("CASH RECONCILIATION");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    printf("\nATM ID: ATM001\n");
    printf("Location: Main Branch, Delhi\n");
    printf("Reconciliation Date: 2023-05-10\n\n");
    
    printf("System Cash Balance:\n");
    printf("- ₹2000 Notes: 150 (₹300,000)\n");
    printf("- ₹500 Notes: 400 (₹200,000)\n");
    printf("- ₹200 Notes: 250 (₹50,000)\n");
    printf("- ₹100 Notes: 100 (₹10,000)\n");
    printf("Total System Balance: ₹560,000\n\n");
    
    printf("Enter Physical Cash Count:\n");
    
    int notes2000 = getInteger("₹2000 Notes counted", 0, 1000);
    int notes500 = getInteger("₹500 Notes counted", 0, 2000);
    int notes200 = getInteger("₹200 Notes counted", 0, 2000);
    int notes100 = getInteger("₹100 Notes counted", 0, 2000);
    
    double physicalTotal = 2000 * notes2000 + 500 * notes500 + 200 * notes200 + 100 * notes100;
    double difference = physicalTotal - 560000.00;
    
    printf("\nReconciliation Summary:\n");
    printLine('-', SCREEN_WIDTH);
    printf("%-20s %-15s %-15s %-15s\n", "Denomination", "System", "Physical", "Difference");
    printLine('-', SCREEN_WIDTH);
    
    printf("%-20s %-15d %-15d %-15d\n", "₹2000 Notes", 150, notes2000, notes2000 - 150);
    printf("%-20s %-15d %-15d %-15d\n", "₹500 Notes", 400, notes500, notes500 - 400);
    printf("%-20s %-15d %-15d %-15d\n", "₹200 Notes", 250, notes200, notes200 - 250);
    printf("%-20s %-15d %-15d %-15d\n", "₹100 Notes", 100, notes100, notes100 - 100);
    
    printLine('-', SCREEN_WIDTH);
    
    printf("Total System: ");
    printCurrency(560000.00, "₹");
    printf("\n");
    
    printf("Total Physical: ");
    printCurrency(physicalTotal, "₹");
    printf("\n");
    
    printf("Difference: ");
    printCurrency(difference, "₹");
    printf(" (%s)\n", difference > 0 ? "Surplus" : (difference < 0 ? "Shortage" : "Balanced"));
    
    char remarks[200];
    getString("\nEnter Reconciliation Remarks: ", remarks, sizeof(remarks));
    
    if (getConfirmation("\nConfirm Reconciliation")) {
        // In a real application, this would update the database
        printSuccess("Cash reconciliation recorded successfully!");
        printf("Reference Number: REC%d\n", rand() % 1000000000);
        
        LOG_INFO("Cash reconciliation by operator %s: Difference ₹%.2f", operatorId, difference);
    } else {
        printInfo("Cash reconciliation cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief Perform maintenance
 * @param operatorId ATM operator ID
 * @param deviceType Type of device
 */
void performMaintenance(const char *operatorId, const char *deviceType) {
    clearScreen();
    char title[100];
    sprintf(title, "%s MAINTENANCE", deviceType);
    printHeader(title);
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    printf("\nATM ID: ATM001\n");
    printf("Location: Main Branch, Delhi\n");
    printf("Device: %s\n\n", deviceType);
    
    printf("Maintenance Options:\n");
    printf("1. Run Diagnostics\n");
    printf("2. Clean Device\n");
    printf("3. Calibrate Device\n");
    printf("4. Replace Parts\n");
    
    int maintenanceOption = getInteger("\nSelect maintenance option", 1, 4);
    
    const char *actionDesc;
    switch (maintenanceOption) {
        case 1: actionDesc = "Diagnostics"; break;
        case 2: actionDesc = "Cleaning"; break;
        case 3: actionDesc = "Calibration"; break;
        case 4: actionDesc = "Parts Replacement"; break;
        default: actionDesc = "Maintenance"; break;
    }
    
    if (maintenanceOption == 1) {
        printf("\nRunning %s diagnostics...\n", deviceType);
        sleep(3);
        
        // Simulate diagnostic results with random success/failure
        if (rand() % 10 > 2) { // 80% chance of success
            printSuccess("Diagnostics completed successfully!");
            printf("All tests passed. Device is operating normally.\n");
        } else {
            printError("Diagnostics found issues!");
            printf("Error Code: E%d\n", 1000 + rand() % 9000);
            printf("Further maintenance required.\n");
        }
    } else if (maintenanceOption == 4) {
        printf("\nAvailable Parts for Replacement:\n");
        printf("1. Main Board\n");
        printf("2. Motor Assembly\n");
        printf("3. Sensors\n");
        printf("4. Cables\n");
        
        int partOption = getInteger("Select part to replace", 1, 4);
        
        const char *partName;
        switch (partOption) {
            case 1: partName = "Main Board"; break;
            case 2: partName = "Motor Assembly"; break;
            case 3: partName = "Sensors"; break;
            case 4: partName = "Cables"; break;
            default: partName = "Part"; break;
        }
        
        char serialNumber[20];
        getString("Enter new part serial number: ", serialNumber, sizeof(serialNumber));
        
        if (getConfirmation("\nConfirm part replacement")) {
            printSuccess("Part replacement recorded successfully!");
            printf("%s replaced with serial number %s\n", partName, serialNumber);
            
            LOG_INFO("Parts replaced by operator %s: %s for %s", operatorId, partName, deviceType);
        } else {
            printInfo("Part replacement cancelled.");
        }
    } else {
        if (getConfirmation("\nConfirm maintenance action")) {
            printSuccess("Maintenance action completed successfully!");
            printf("%s %s performed on %s\n", deviceType, actionDesc, getDateTime());
            
            LOG_INFO("%s %s performed by operator %s", deviceType, actionDesc, operatorId);
        } else {
            printInfo("Maintenance action cancelled.");
        }
    }
    
    pauseExecution();
}

/**
 * @brief Perform software update
 * @param operatorId ATM operator ID
 */
void performSoftwareUpdate(const char *operatorId) {
    clearScreen();
    printHeader("SOFTWARE UPDATE");
    
    // In a real application, this would interact with the database
    // For demo purposes, we'll simulate the process
    
    printf("\nATM ID: ATM001\n");
    printf("Location: Main Branch, Delhi\n\n");
    
    printf("Current Software Version: v2.5.3\n");
    printf("Latest Available Version: v2.6.1\n\n");
    
    printf("Update Notes:\n");
    printf("- Improved transaction processing speed\n");
    printf("- Enhanced security features\n");
    printf("- Fixed card reader issues\n");
    printf("- Added support for new banknotes\n\n");
    
    printf("Estimated Update Time: 15 minutes\n");
    printf("WARNING: ATM will be unavailable during the update!\n");
    
    if (getConfirmation("\nProceed with software update")) {
        printf("\nPreparing for software update...\n");
        sleep(2);
        printf("Downloading update package...\n");
        
        // Simulate progress bar
        for (int i = 0; i <= 20; i++) {
            printf("\r[");
            for (int j = 0; j < i; j++) printf("=");
            for (int j = i; j < 20; j++) printf(" ");
            printf("] %d%%", i * 5);
            fflush(stdout);
            usleep(300000); // Sleep for 300ms
        }
        
        printf("\n\nInstalling update...\n");
        sleep(3);
        printf("Configuring system...\n");
        sleep(2);
        printf("Restarting services...\n");
        sleep(2);
        
        printSuccess("\nSoftware update completed successfully!");
        printf("New Software Version: v2.6.1\n");
        
        LOG_INFO("Software updated by operator %s: v2.5.3 to v2.6.1", operatorId);
    } else {
        printInfo("Software update cancelled.");
    }
    
    pauseExecution();
}

/**
 * @brief Show device status
 * @param operatorId ATM operator ID
 * @param statusType Type of status to show
 */
void showDeviceStatus(const char *operatorId, const char *statusType) {
    clearScreen();
    char title[100];
    sprintf(title, "%s STATUS", statusType);
    printHeader(title);
    
    // In a real application, this would fetch data from the database
    // For demo purposes, we'll use mock data
    
    printf("\nATM ID: ATM001\n");
    printf("Location: Main Branch, Delhi\n");
    printf("Status as of: %s\n\n", "2023-05-10 14:30:45");
    
    if (strcmp(statusType, "Hardware") == 0) {
        printLine('-', SCREEN_WIDTH);
        printf("%-25s %-15s %-30s\n", "Component", "Status", "Details");
        printLine('-', SCREEN_WIDTH);
        
        printf("%-25s %-15s %-30s\n", "Card Reader", "OK", "Last maintenance: 2023-04-15");
        printf("%-25s %-15s %-30s\n", "Cash Dispenser", "OK", "Operational");
        printf("%-25s %-15s %-30s\n", "Receipt Printer", "Warning", "Low on paper");
        printf("%-25s %-15s %-30s\n", "Keypad", "OK", "Fully functional");
        printf("%-25s %-15s %-30s\n", "Display", "OK", "Calibrated");
        printf("%-25s %-15s %-30s\n", "Security Camera", "OK", "Recording active");
        printf("%-25s %-15s %-30s\n", "Safe Door", "OK", "Secured");
    } else if (strcmp(statusType, "Software") == 0) {
        printLine('-', SCREEN_WIDTH);
        printf("%-25s %-15s %-30s\n", "Component", "Status", "Details");
        printLine('-', SCREEN_WIDTH);
        
        printf("%-25s %-15s %-30s\n", "Core System", "OK", "v2.5.3 running");
        printf("%-25s %-15s %-30s\n", "Transaction Module", "OK", "Last update: 2023-05-01");
        printf("%-25s %-15s %-30s\n", "Security Module", "OK", "Signature verification active");
        printf("%-25s %-15s %-30s\n", "Card Services", "OK", "All card types supported");
        printf("%-25s %-15s %-30s\n", "Journal", "OK", "98% space available");
        printf("%-25s %-15s %-30s\n", "Host Communication", "OK", "Connected");
        printf("%-25s %-15s %-30s\n", "Monitoring Agent", "OK", "Real-time monitoring active");
    } else if (strcmp(statusType, "Network") == 0) {
        printLine('-', SCREEN_WIDTH);
        printf("%-25s %-15s %-30s\n", "Component", "Status", "Details");
        printLine('-', SCREEN_WIDTH);
        
        printf("%-25s %-15s %-30s\n", "Primary Connection", "OK", "10 Mbps, Latency: 35ms");
        printf("%-25s %-15s %-30s\n", "Backup Connection", "OK", "Ready (Standby)");
        printf("%-25s %-15s %-30s\n", "Host Link", "OK", "Secure tunnel established");
        printf("%-25s %-15s %-30s\n", "SSL Certificate", "OK", "Valid until 2024-05-10");
        printf("%-25s %-15s %-30s\n", "Firewall", "OK", "Rules updated");
        printf("%-25s %-15s %-30s\n", "Last Outage", "Info", "None in last 30 days");
    }
    
    printLine('-', SCREEN_WIDTH);
    
    LOG_INFO("%s status checked by operator %s", statusType, operatorId);
    
    pauseExecution();
}

/**
 * @brief Show error log
 * @param operatorId ATM operator ID
 */
void showErrorLog(const char *operatorId) {
    clearScreen();
    printHeader("ERROR LOG");
    
    // In a real application, this would fetch data from the database
    // For demo purposes, we'll use mock data
    
    printf("\nATM ID: ATM001\n");
    printf("Location: Main Branch, Delhi\n\n");
    
    printLine('-', SCREEN_WIDTH);
    printf("%-20s %-15s %-35s\n", "Date/Time", "Error Code", "Description");
    printLine('-', SCREEN_WIDTH);
    
    printf("%-20s %-15s %-35s\n", "2023-05-09 15:23:45", "E4011", "Card reader temporary failure");
    printf("%-20s %-15s %-35s\n", "2023-05-08 09:12:30", "E2063", "Network timeout - reconnected");
    printf("%-20s %-15s %-35s\n", "2023-05-07 18:45:22", "E3001", "Cash dispenser sensor error");
    printf("%-20s %-15s %-35s\n", "2023-05-05 11:30:10", "E1022", "Paper low warning");
    printf("%-20s %-15s %-35s\n", "2023-05-03 14:15:55", "E4011", "Card reader error - cleared");
    
    printLine('-', SCREEN_WIDTH);
    
    printf("\nFilter Options:\n");
    printf("1. Show All Errors\n");
    printf("2. Show Critical Errors Only\n");
    printf("3. Filter by Date Range\n");
    printf("4. Filter by Error Code\n");
    printf("5. Export Log\n");
    printf("6. Return to Device Status Menu\n");
    
    int filterOption = getInteger("\nSelect option", 1, 6);
    
    if (filterOption == 6) {
        return;
    } else if (filterOption == 5) {
        printf("\nExporting error log...\n");
        sleep(2);
        printSuccess("Error log exported successfully!");
        printf("File saved as: ErrorLog_ATM001_2023-05-10.csv\n");
        
        LOG_INFO("Error log exported by operator %s", operatorId);
    } else {
        printInfo("Feature not implemented in this demo.");
    }
    
    pauseExecution();
}

/**
 * @brief Generate transaction report
 * @param operatorId ATM operator ID
 */
void generateTransactionReport(const char *operatorId) {
    clearScreen();
    printHeader("TRANSACTION REPORT");
    
    // In a real application, this would fetch data from the database
    // For demo purposes, we'll use mock data
    
    printf("\nATM ID: ATM001\n");
    printf("Location: Main Branch, Delhi\n\n");
    
    printf("Select Report Period:\n");
    printf("1. Today\n");
    printf("2. Yesterday\n");
    printf("3. Last 7 days\n");
    printf("4. Last 30 days\n");
    printf("5. Custom Period\n");
    
    int periodOption = getInteger("\nSelect period", 1, 5);
    
    char startDate[20] = {0};
    char endDate[20] = {0};
    const char *periodDesc;
    
    switch (periodOption) {
        case 1: periodDesc = "Today (2023-05-10)"; break;
        case 2: periodDesc = "Yesterday (2023-05-09)"; break;
        case 3: periodDesc = "Last 7 days (2023-05-03 to 2023-05-10)"; break;
        case 4: periodDesc = "Last 30 days (2023-04-10 to 2023-05-10)"; break;
        case 5:
            getString("Enter Start Date (YYYY-MM-DD): ", startDate, sizeof(startDate));
            getString("Enter End Date (YYYY-MM-DD): ", endDate, sizeof(endDate));
            periodDesc = "Custom Period";
            break;
        default: periodDesc = ""; break;
    }
    
    clearScreen();
    printHeader("TRANSACTION REPORT");
    
    printf("\nATM ID: ATM001\n");
    printf("Location: Main Branch, Delhi\n");
    printf("Period: %s", periodDesc);
    if (periodOption == 5) {
        printf(" (%s to %s)", startDate, endDate);
    }
    printf("\n\n");
    
    printLine('-', SCREEN_WIDTH);
    printf("%-25s %-15s %-15s\n", "Transaction Type", "Count", "Amount");
    printLine('-', SCREEN_WIDTH);
    
    printf("%-25s %-15d ", "Cash Withdrawals", 234);
    printCurrency(1245670.00, "₹");
    printf("\n");
    
    printf("%-25s %-15d ", "Balance Inquiries", 156);
    printf("%-15s\n", "N/A");
    
    printf("%-25s %-15d ", "Mini Statements", 78);
    printf("%-15s\n", "N/A");
    
    printf("%-25s %-15d ", "Funds Transfers", 42);
    printCurrency(356750.00, "₹");
    printf("\n");
    
    printf("%-25s %-15d ", "PIN Changes", 18);
    printf("%-15s\n", "N/A");
    
    printLine('-', SCREEN_WIDTH);
    
    printf("%-25s %-15d ", "Total Transactions", 528);
    printCurrency(1602420.00, "₹");
    printf("\n");
    
    printLine('-', SCREEN_WIDTH);
    
    printf("\nTransaction Success Rate: 98.2%%\n");
    printf("Average Transaction Time: 37 seconds\n");
    printf("Peak Usage Time: 12:00 PM - 2:00 PM\n\n");
    
    printf("Report Options:\n");
    printf("1. Print Report\n");
    printf("2. Export as CSV\n");
    printf("3. Export as PDF\n");
    printf("4. Return to ATM Operations Menu\n");
    
    int reportOption = getInteger("\nSelect option", 1, 4);
    
    if (reportOption != 4) {
        printf("\nProcessing report...\n");
        sleep(2);
        printSuccess("Report processed successfully!");
        
        const char *formatName;
        switch (reportOption) {
            case 1: formatName = "printed"; break;
            case 2: formatName = "exported as CSV"; break;
            case 3: formatName = "exported as PDF"; break;
            default: formatName = "processed"; break;
        }
        
        printf("Report %s: ATM001_TransactionReport_%s.%s\n", 
               formatName, 
               "2023-05-10",
               (reportOption == 2) ? "csv" : ((reportOption == 3) ? "pdf" : "txt"));
        
        LOG_INFO("Transaction report %s by operator %s", formatName, operatorId);
    }
    
    pauseExecution();
}

/**
 * @brief Get current date and time as string
 * @return Current date and time string
 */
const char* getDateTime(void) {
    static char dateTime[30];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);
    return dateTime;
}
