#include "admin_menu.h"
#include "admin_operations.h"
#include "admin_db.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================
// Admin Menu Functions
// ============================

// Display admin menu
void displayAdminMenu() {
    printf("\n===== ATM Admin Panel =====\n");
    printf("1. Dashboard\n");
    printf("2. Cash Management\n");
    printf("3. Transaction Management\n");
    printf("4. User Management\n");
    printf("5. ATM Maintenance\n");
    printf("6. Security Management\n");
    printf("7. Settings\n");
    printf("8. Notifications and Alerts\n");
    printf("9. Audit Logs\n");
    printf("10. Exit\n");
    printf("=========================\n");
    printf("Enter your choice: ");
}

// Display the admin dashboard with ATM status and statistics
void displayDashboard() {
    printf("\n===== 📊 Dashboard =====\n");
    
    // Get ATM status
    int status = getServiceStatus();
    const char* statusStr = status ? "Offline" : "Online";
    printf("ATM Status: %s\n", statusStr);
    
    // Calculate total cash available (placeholder)
    printf("Total Cash Available: ₹50,000\n");
    
    // Count transactions today (placeholder)
    printf("Number of Transactions Today: 42\n");
    
    // Display average transaction time (placeholder)
    printf("Average Transaction Time: 45 seconds\n");
    
    // Display alerts
    printf("\n--- Alerts ---\n");
    printf("- Low ₹500 notes (Refill recommended)\n");
    printf("- Multiple card read errors detected\n");
    
    writeAuditLog("ADMIN", "Viewed dashboard");
}

// Manage cash levels in the ATM
void manageCash() {
    int choice;
    
    do {
        printf("\n===== 💰 Cash Management =====\n");
        printf("1. View Cash Levels\n");
        printf("2. Load/Refill Cash\n");
        printf("3. Cash Withdrawal Reports\n");
        printf("4. Daily Virtual Withdrawals Summary\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- Current Cash Levels ---\n");
                printf("₹100 notes: 200 (₹20,000)\n");
                printf("₹200 notes: 100 (₹20,000)\n");
                printf("₹500 notes: 20 (₹10,000)\n");
                printf("₹2000 notes: 0 (₹0)\n");
                printf("Total Cash: ₹50,000\n");
                writeAuditLog("ADMIN", "Viewed cash levels");
                break;
                
            case 2:
                printf("\n--- Load/Refill Cash ---\n");
                printf("Refill request submitted to Cash Management Team.\n");
                printf("Reference ID: REF123456\n");
                writeAuditLog("ADMIN", "Submitted cash refill request");
                break;
                
            case 3:
                printf("\n--- Cash Withdrawal Reports ---\n");
                printf("Today's Withdrawals: ₹120,000\n");
                printf("This Week's Withdrawals: ₹750,000\n");
                printf("This Month's Withdrawals: ₹3,250,000\n");
                writeAuditLog("ADMIN", "Viewed cash withdrawal reports");
                break;
                
            case 4:
                printf("\n--- Daily Virtual Withdrawals Summary ---\n");
                printf("Total Transactions: 42\n");
                printf("Total Amount: ₹210,000\n");
                printf("Average Withdrawal: ₹5,000\n");
                printf("Peak Hour: 17:00 - 18:00 (12 transactions)\n");
                writeAuditLog("ADMIN", "Viewed daily virtual withdrawals summary");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// Manage transaction logs and reports
void manageTransactions() {
    int choice;
    
    do {
        printf("\n===== 🔄 Transaction Management =====\n");
        printf("1. Monitor Live Transactions\n");
        printf("2. View Failed/Cancelled Transactions\n");
        printf("3. Download Transaction Reports\n");
        printf("4. Refund Management\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- Live Transactions ---\n");
                printf("Card: **** 1234 | Amount: ₹2,000 | Time: Just now | Status: Processing\n");
                printf("Card: **** 5678 | Amount: ₹10,000 | Time: 2 mins ago | Status: Completed\n");
                printf("Card: **** 9012 | Amount: ₹500 | Time: 5 mins ago | Status: Completed\n");
                writeAuditLog("ADMIN", "Monitored live transactions");
                break;
                
            case 2:
                printf("\n--- Failed/Cancelled Transactions ---\n");
                printf("Card: **** 3456 | Amount: ₹15,000 | Time: 10:15 AM | Status: Failed (Insufficient Funds)\n");
                printf("Card: **** 7890 | Amount: ₹5,000 | Time: 11:30 AM | Status: Cancelled by User\n");
                printf("Card: **** 1357 | Amount: ₹2,000 | Time: 12:45 PM | Status: Failed (Card Error)\n");
                writeAuditLog("ADMIN", "Viewed failed transactions");
                break;
                
            case 3:
                printf("\n--- Download Transaction Reports ---\n");
                printf("1. Daily Report (27-Apr-2025) - Generated\n");
                printf("2. Weekly Report (21-Apr to 27-Apr-2025) - Generated\n");
                printf("3. Monthly Report (Apr-2025) - Generated\n");
                printf("Reports saved to logs/transactions/ directory\n");
                writeAuditLog("ADMIN", "Downloaded transaction reports");
                break;
                
            case 4:
                printf("\n--- Refund Management ---\n");
                printf("Pending Refunds: 2\n");
                printf("Card: **** 2468 | Amount: ₹1,000 | Failed on: 26-Apr-2025 | Status: Refund Pending\n");
                printf("Card: **** 1357 | Amount: ₹2,000 | Failed on: 27-Apr-2025 | Status: Refund Pending\n");
                printf("Process refunds? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Viewed refund management");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// Manage user accounts
void manageUsers() {
    int choice;
    int cardNumber;
    
    do {
        printf("\n===== 👤 User Management =====\n");
        printf("1. Manage Card Holders (view details)\n");
        printf("2. Freeze/Block Card\n");
        printf("3. Reset PIN\n");
        printf("4. View Mobile Number Updates\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- Card Holder Details ---\n");
                printf("Enter Card Number: ");
                scanf("%d", &cardNumber);
                // This would typically call a function to retrieve user details
                printf("\nCard Number: %d\n", cardNumber);
                printf("Name: John Doe\n");  // Placeholder
                printf("Status: Active\n");
                printf("Last Transaction: 27-Apr-2025\n");
                writeAuditLog("ADMIN", "Viewed card holder details");
                break;
                
            case 2:
                printf("\nEnter Card Number to Freeze/Block: ");
                scanf("%d", &cardNumber);
                toggleCardStatus(cardNumber);
                break;
                
            case 3:
                printf("\nEnter Card Number to Reset PIN: ");
                scanf("%d", &cardNumber);
                regenerateCardPin(cardNumber);
                break;
                
            case 4:
                printf("\n--- Mobile Number Updates ---\n");
                printf("Card: **** 1234 | Old: +91 98765-43210 | New: +91 98765-43211 | Date: 26-Apr-2025\n");
                printf("Card: **** 5678 | Old: +91 90123-45678 | New: +91 91234-56789 | Date: 27-Apr-2025\n");
                writeAuditLog("ADMIN", "Viewed mobile number updates");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// ATM maintenance options
void atmMaintenance() {
    int choice;
    
    do {
        printf("\n===== 🛠️ ATM Maintenance =====\n");
        printf("1. ATM Health Monitoring\n");
        printf("2. Hardware Status\n");
        printf("3. Schedule Maintenance\n");
        printf("4. Error Logs\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- ATM Health Monitoring ---\n");
                printf("System Uptime: 5 days, 7 hours\n");
                printf("CPU Usage: 23%%\n");
                printf("Memory Usage: 42%%\n");
                printf("Disk Space: 56%% free\n");
                printf("Temperature: Normal\n");
                writeAuditLog("ADMIN", "Viewed ATM health monitoring");
                break;
                
            case 2:
                printf("\n--- Hardware Status ---\n");
                printf("Card Reader: OK\n");
                printf("Cash Dispenser: Warning (Low ₹500 notes)\n");
                printf("Receipt Printer: OK\n");
                printf("PIN Pad: OK\n");
                printf("Network: OK (100 Mbps)\n");
                writeAuditLog("ADMIN", "Checked hardware status");
                break;
                
            case 3:
                printf("\n--- Schedule Maintenance ---\n");
                printf("Next scheduled maintenance: 30-Apr-2025\n");
                printf("Maintenance tasks:\n");
                printf("- Clean card reader\n");
                printf("- Update software\n");
                printf("- Calibrate cash dispenser\n");
                writeAuditLog("ADMIN", "Viewed scheduled maintenance");
                break;
                
            case 4:
                printf("\n--- Error Logs ---\n");
                printf("27-Apr-2025 09:15 - Card read error (Card: **** 1234)\n");
                printf("27-Apr-2025 10:30 - Cash dispenser jam (Resolved)\n");
                printf("26-Apr-2025 15:45 - Network timeout (Resolved)\n");
                writeAuditLog("ADMIN", "Viewed error logs");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// Security management options
void securityManagement() {
    int choice;
    
    do {
        printf("\n===== 🔒 Security Management =====\n");
        printf("1. Monitor Suspicious Activity\n");
        printf("2. Set ATM Lock/Unlock\n");
        printf("3. Two-Factor Authentication\n");
        printf("4. IP Whitelist/Blacklist\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- Suspicious Activity Monitor ---\n");
                printf("Card: **** 1234 | 3 failed PIN attempts | Time: 09:15 AM\n");
                printf("Card: **** 5678 | Unusual withdrawal pattern | Time: 10:30 AM\n");
                printf("Card: **** 9012 | Multiple transactions in short period | Time: 11:45 AM\n");
                writeAuditLog("ADMIN", "Monitored suspicious activity");
                break;
                
            case 2:
                printf("\n--- ATM Lock/Unlock ---\n");
                printf("Current Status: %s\n", getServiceStatus() ? "Locked" : "Unlocked");
                printf("Do you want to change the status? (1 for Yes, 0 for No): ");
                int confirm;
                scanf("%d", &confirm);
                if (confirm) {
                    toggleServiceMode();
                }
                break;
                
            case 3:
                printf("\n--- Two-Factor Authentication ---\n");
                printf("Status: Enabled for admin access\n");
                printf("Configured methods:\n");
                printf("- SMS to +91 98765-43210\n");
                printf("- Email to admin@atm.com\n");
                writeAuditLog("ADMIN", "Viewed 2FA settings");
                break;
                
            case 4:
                printf("\n--- IP Whitelist/Blacklist ---\n");
                printf("Whitelisted IPs:\n");
                printf("- 192.168.1.100 (Admin Office)\n");
                printf("- 10.0.0.50 (Maintenance Team)\n");
                printf("Blacklisted IPs:\n");
                printf("- 45.67.89.123 (Multiple failed access attempts)\n");
                writeAuditLog("ADMIN", "Viewed IP whitelist/blacklist");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// Settings management
void manageSettings() {
    int choice;
    
    do {
        printf("\n===== ⚙️ Settings =====\n");
        printf("1. Update ATM Software\n");
        printf("2. Configure Language Options\n");
        printf("3. Configure Fast Cash Amounts\n");
        printf("4. Customize Welcome Screen\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- Software Update ---\n");
                printf("Current Version: 2.5.3\n");
                printf("Available Version: 2.6.1\n");
                printf("Update Notes:\n");
                printf("- Improved transaction security\n");
                printf("- Better error handling\n");
                printf("- New UI features\n");
                printf("Start update? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Checked software updates");
                break;
                
            case 2:
                printf("\n--- Language Options ---\n");
                printf("Current Languages:\n");
                printf("1. English (Default)\n");
                printf("2. Hindi\n");
                printf("3. Tamil\n");
                printf("4. Telugu\n");
                printf("Add or remove language? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Configured language options");
                break;
                
            case 3:
                printf("\n--- Fast Cash Amounts ---\n");
                printf("Current Fast Cash Options:\n");
                printf("1. ₹1,000\n");
                printf("2. ₹2,000\n");
                printf("3. ₹3,000\n");
                printf("4. ₹5,000\n");
                printf("5. ₹10,000\n");
                printf("Modify amounts? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Viewed fast cash settings");
                break;
                
            case 4:
                printf("\n--- Welcome Screen Customization ---\n");
                printf("Current Welcome Message: \"Welcome to ABC Bank. Please insert your card.\"\n");
                printf("Change welcome message? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Viewed welcome screen settings");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// Notifications and alerts management
void manageNotifications() {
    int choice;
    
    do {
        printf("\n===== 🔔 Notifications and Alerts =====\n");
        printf("1. Configure Alert Recipients\n");
        printf("2. Configure Alert Types\n");
        printf("3. View Recent Alerts\n");
        printf("4. Test Alert System\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- Alert Recipients ---\n");
                printf("Current Recipients:\n");
                printf("- admin@atm.com (Email)\n");
                printf("- +91 98765-43210 (SMS)\n");
                printf("Add or remove recipients? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Viewed alert recipients");
                break;
                
            case 2:
                printf("\n--- Alert Types ---\n");
                printf("Current Alert Configuration:\n");
                printf("- Low Cash: Enabled (Threshold: ₹10,000)\n");
                printf("- Technical Error: Enabled\n");
                printf("- Fraud Attempt: Enabled\n");
                printf("- Maintenance Due: Enabled (7 days prior)\n");
                writeAuditLog("ADMIN", "Viewed alert types");
                break;
                
            case 3:
                printf("\n--- Recent Alerts ---\n");
                printf("27-Apr-2025 09:15 - Low Cash Warning (₹500 notes)\n");
                printf("26-Apr-2025 15:30 - Multiple Failed PIN Attempts (Card: **** 1234)\n");
                printf("25-Apr-2025 12:45 - Hardware Error (Card Reader)\n");
                writeAuditLog("ADMIN", "Viewed recent alerts");
                break;
                
            case 4:
                printf("\n--- Test Alert System ---\n");
                printf("Select Alert Type to Test:\n");
                printf("1. Email\n");
                printf("2. SMS\n");
                printf("Test alert? (Feature not implemented)\n");
                writeAuditLog("ADMIN", "Tested alert system");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// View audit logs
void viewAuditLogs() {
    int choice;
    
    do {
        printf("\n===== 🧾 Audit Logs =====\n");
        printf("1. Admin Actions\n");
        printf("2. Transaction History\n");
        printf("3. System Changes\n");
        printf("4. Full Audit Trail\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch(choice) {
            case 1:
                printf("\n--- Admin Actions Log ---\n");
                printf("27-Apr-2025 09:00 - Admin Login (User: admin)\n");
                printf("27-Apr-2025 09:05 - Card Status Change (Card: **** 1234)\n");
                printf("27-Apr-2025 09:10 - PIN Reset (Card: **** 5678)\n");
                writeAuditLog("ADMIN", "Viewed admin actions log");
                break;
                
            case 2:
                printf("\n--- Transaction History ---\n");
                printf("27-Apr-2025 10:15 - Withdrawal: ₹5,000 (Card: **** 1234)\n");
                printf("27-Apr-2025 11:30 - Balance Check (Card: **** 5678)\n");
                printf("27-Apr-2025 12:45 - Failed Withdrawal: ₹20,000 (Card: **** 9012, Reason: Insufficient Funds)\n");
                writeAuditLog("ADMIN", "Viewed transaction history");
                break;
                
            case 3:
                printf("\n--- System Changes Log ---\n");
                printf("26-Apr-2025 09:00 - Software Update (v2.5.2 to v2.5.3)\n");
                printf("25-Apr-2025 14:30 - Language Added (Telugu)\n");
                printf("24-Apr-2025 11:00 - Fast Cash Options Modified\n");
                writeAuditLog("ADMIN", "Viewed system changes log");
                break;
                
            case 4:
                printf("\n--- Full Audit Trail ---\n");
                printf("This would display the full audit log (Feature not implemented)\n");
                printf("Note: Full audit trail can be exported to CSV/PDF\n");
                writeAuditLog("ADMIN", "Viewed full audit trail");
                break;
                
            case 5:
                printf("Returning to main menu...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 5) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
        
    } while (choice != 5);
}

// Helper function to clear the input buffer
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Helper function to clear the screen
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}