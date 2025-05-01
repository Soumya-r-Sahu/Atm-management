/**
 * Test utility to initialize all necessary data files for the ATM system
 * This includes setting up card data, customer profiles, and system configurations
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Define data file paths (should match paths.h definitions)
#define CARD_FILE "../data/card.txt"
#define CUSTOMER_FILE "../data/customer.txt" 
#define ACCOUNTING_FILE "../data/accounting.txt"
#define ATM_DATA_FILE "../data/atm_data.txt"
#define VIRTUAL_WALLET_FILE "../data/virtual_wallet.txt"
#define SYSTEM_CONFIG_FILE "../data/system_config.txt"

// Updated paths for direct access from project root
#define ROOT_CARD_FILE "data/card.txt"
#define ROOT_CUSTOMER_FILE "data/customer.txt" 
#define ROOT_ACCOUNTING_FILE "data/accounting.txt"
#define ROOT_ATM_DATA_FILE "data/atm_data.txt"
#define ROOT_VIRTUAL_WALLET_FILE "data/virtual_wallet.txt"
#define ROOT_SYSTEM_CONFIG_FILE "data/system_config.txt"

// Utility function to generate a random account number
long generateAccountNumber() {
    // Generate a random 11-digit account number
    long base = 10000000000;
    return base + (rand() % 90000000000);
}

// Utility function to generate a random card number (16 digits)
void generateCardNumber(char *cardNumber) {
    // Format: XXXX-XXXX-XXXX-XXXX
    sprintf(cardNumber, "%04d-%04d-%04d-%04d", 
            4000 + (rand() % 999),  // Start with 4 for "Visa-like" numbers
            1000 + (rand() % 9000),
            1000 + (rand() % 9000),
            1000 + (rand() % 9000));
}

// Utility function to generate random CVV (3 digits)
int generateCVV() {
    return 100 + (rand() % 900); // 3-digit number between 100 and 999
}

// Utility function to generate expiry date (MM/YY format, 2-5 years from now)
void generateExpiryDate(char *expiryDate) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    int year = tm_info->tm_year + 1900;
    int month = tm_info->tm_mon + 1;
    
    // Add 2-5 years to current date
    year += 2 + (rand() % 4);
    
    // Format as MM/YY
    sprintf(expiryDate, "%02d/%02d", month, year % 100);
}

// Initialize card data file with improved table formatting
void initCardFile() {
    FILE *file = fopen(ROOT_CARD_FILE, "w");
    if (!file) {
        printf("Failed to create card data file!\n");
        return;
    }
    
    // Table header with proper formatting
    fprintf(file, "+----------+----------------------+-------+------------+------+-----------+-------------+\n");
    fprintf(file, "| CARD_ID  | CARD_NUMBER         | CVV   | EXPIRY     | PIN  | IS_BLOCKED| CUSTOMER_ID |\n");
    fprintf(file, "+----------+----------------------+-------+------------+------+-----------+-------------+\n");
    
    char cardNumber[20];
    char expiryDate[6];
    int numCards = 10; // Create 10 sample cards
    
    for (int i = 1; i <= numCards; i++) {
        generateCardNumber(cardNumber);
        generateExpiryDate(expiryDate);
        int cvv = generateCVV();
        int pin = 1000 + (rand() % 9000);  // 4-digit PIN
        int isBlocked = (i == 8 ? 1 : 0);  // Make one card blocked for testing
        int customerId = i;
        
        fprintf(file, "| %-8d | %-20s | %-5d | %-10s | %4d | %-9d | %-11d |\n", 
                i, cardNumber, cvv, expiryDate, pin, isBlocked, customerId);
    }
    
    // Table footer
    fprintf(file, "+----------+----------------------+-------+------------+------+-----------+-------------+\n");
    
    fclose(file);
    printf("Card data file created successfully with %d sample cards.\n", numCards);
}

// Initialize customer data file with improved table formatting
void initCustomerFile() {
    FILE *file = fopen(ROOT_CUSTOMER_FILE, "w");
    if (!file) {
        printf("Failed to create customer data file!\n");
        return;
    }
    
    // Table header with proper formatting
    fprintf(file, "+------------+----------------+---------------------------+------------------+----------------------+--------------------+--------------+-------------+\n");
    fprintf(file, "| CUSTOMER_ID| NAME           | ADDRESS                   | PHONE           | EMAIL                | ACCOUNT_NUMBER     | IFSC_CODE    | ACCOUNT_TYPE|\n");
    fprintf(file, "+------------+----------------+---------------------------+------------------+----------------------+--------------------+--------------+-------------+\n");
    
    const char *names[] = {"John Doe", "Jane Smith", "Robert Johnson", "Emily Davis", 
                          "Michael Wilson", "Sarah Brown", "David Lee", "Lisa Wang", 
                          "James Miller", "Emma Taylor"};
    
    const char *addresses[] = {"123 Main St, City", "456 Oak Ave, Town", "789 Pine Rd, Village",
                              "101 Elm Blvd, Metro", "202 Cedar Ln, County", "303 Birch Dr, District",
                              "404 Maple Way, Borough", "505 Willow St, Municipality",
                              "606 Cherry Ave, Hamlet", "707 Spruce Ct, Township"};
    
    const char *ifsc_codes[] = {"SBIN0012345", "SBIN0023456", "SBIN0034567", "SBIN0045678",
                               "SBIN0056789", "SBIN0067890", "SBIN0078901", "SBIN0089012",
                               "SBIN0090123", "SBIN0001234"};
    
    const char *account_types[] = {"Savings", "Current", "Savings", "Savings", "Current",
                                  "Savings", "Current", "Savings", "Savings", "Current"};
    
    int numCustomers = 10;
    
    for (int i = 1; i <= numCustomers; i++) {
        long accountNumber = generateAccountNumber();
        fprintf(file, "| %-10d | %-14s | %-25s | +91-98765-%-4d | %-20s | %-18ld | %-12s | %-11s |\n",
                i, names[i-1], addresses[i-1], 10000+i*1111, 
                i < 10 ? "user0@example.com" : "user10@example.com", accountNumber, 
                ifsc_codes[i-1], account_types[i-1]);
    }
    
    // Table footer
    fprintf(file, "+------------+----------------+---------------------------+------------------+----------------------+--------------------+--------------+-------------+\n");
    
    fclose(file);
    printf("Customer data file created successfully with %d sample customers.\n", numCustomers);
}

// Initialize accounting data file with improved table formatting
void initAccountingFile() {
    FILE *file = fopen(ROOT_ACCOUNTING_FILE, "w");
    if (!file) {
        printf("Failed to create accounting data file!\n");
        return;
    }
    
    // Table header with proper formatting
    fprintf(file, "+-------------+----------+---------------+-------------------------+\n");
    fprintf(file, "| CUSTOMER_ID | CARD_ID  | BALANCE       | LAST_TRANSACTION       |\n");
    fprintf(file, "+-------------+----------+---------------+-------------------------+\n");
    
    int numAccounts = 10;
    
    for (int i = 1; i <= numAccounts; i++) {
        float balance = 10000.0f + (rand() % 90000);
        time_t now = time(NULL);
        char lastTransaction[30];
        strftime(lastTransaction, sizeof(lastTransaction), "%Y-%m-%d %H:%M:%S", localtime(&now));
        
        fprintf(file, "| %-11d | %-8d | ₹ %-11.2f | %-23s |\n", 
                i, i, balance, lastTransaction);
    }
    
    // Table footer
    fprintf(file, "+-------------+----------+---------------+-------------------------+\n");
    
    fclose(file);
    printf("Accounting data file created successfully with %d accounts.\n", numAccounts);
}

// Initialize virtual wallet data file with improved table formatting
void initVirtualWalletFile() {
    FILE *file = fopen(ROOT_VIRTUAL_WALLET_FILE, "w");
    if (!file) {
        printf("Failed to create virtual wallet data file!\n");
        return;
    }
    
    // Table header with proper formatting
    fprintf(file, "+------------+-------------+----------------+---------------+-----------+\n");
    fprintf(file, "| WALLET_ID  | CUSTOMER_ID | VIRTUAL_CARD_ID| BALANCE       | IS_ACTIVE |\n");
    fprintf(file, "+------------+-------------+----------------+---------------+-----------+\n");
    
    int numWallets = 5; // Only some customers have virtual wallets
    
    for (int i = 1; i <= numWallets; i++) {
        int customerId = i * 2; // Every other customer has a virtual wallet
        int virtualCardId = i + 100; // Virtual card IDs start from 101
        float balance = 5000.0f + (rand() % 5000);
        int isActive = 1;
        
        fprintf(file, "| %-10d | %-11d | %-14d | ₹ %-11.2f | %-9d |\n", 
                i, customerId, virtualCardId, balance, isActive);
    }
    
    // Table footer
    fprintf(file, "+------------+-------------+----------------+---------------+-----------+\n");
    
    fclose(file);
    printf("Virtual wallet data file created successfully with %d wallets.\n", numWallets);
}

// Initialize ATM data file (including virtual ATMs) with improved table formatting
void initATMDataFile() {
    FILE *file = fopen(ROOT_ATM_DATA_FILE, "w");
    if (!file) {
        printf("Failed to create ATM data file!\n");
        return;
    }
    
    // Table header with proper formatting
    fprintf(file, "+----------+--------------------+----------+----------+----------+----------+----------+----------+\n");
    fprintf(file, "| ATM_ID   | LOCATION           | TYPE     | STATUS   | CASH_100 | CASH_200 | CASH_500 | CASH_2000|\n");
    fprintf(file, "+----------+--------------------+----------+----------+----------+----------+----------+----------+\n");
    
    // Physical ATMs
    fprintf(file, "| %-8s | %-18s | %-8s | %-8s | %-8d | %-8d | %-8d | %-8d |\n", 
            "ATM001", "Main Branch", "physical", "online", 500, 300, 200, 100);
    fprintf(file, "| %-8s | %-18s | %-8s | %-8s | %-8d | %-8d | %-8d | %-8d |\n", 
            "ATM002", "City Center", "physical", "online", 400, 300, 150, 75);
    fprintf(file, "| %-8s | %-18s | %-8s | %-8s | %-8d | %-8d | %-8d | %-8d |\n", 
            "ATM003", "West Mall", "physical", "online", 300, 200, 100, 50);
    fprintf(file, "| %-8s | %-18s | %-8s | %-8s | %-8d | %-8d | %-8d | %-8d |\n", 
            "ATM004", "North Station", "physical", "online", 450, 250, 180, 90);
    fprintf(file, "| %-8s | %-18s | %-8s | %-8s | %-8d | %-8d | %-8d | %-8d |\n", 
            "ATM005", "South Campus", "physical", "online", 350, 220, 120, 60);
    fprintf(file, "| %-8s | %-18s | %-8s | %-8s | %-8d | %-8d | %-8d | %-8d |\n", 
            "ATM006", "Airport Terminal", "physical", "offline", 0, 0, 0, 0);
    
    // Virtual ATMs
    fprintf(file, "| %-8s | %-18s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s |\n", 
            "VATM001", "Mobile App", "virtual", "online", "N/A", "N/A", "N/A", "N/A");
    fprintf(file, "| %-8s | %-18s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s |\n", 
            "VATM002", "Web Banking", "virtual", "online", "N/A", "N/A", "N/A", "N/A");
    fprintf(file, "| %-8s | %-18s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s |\n", 
            "VATM003", "UPI", "virtual", "online", "N/A", "N/A", "N/A", "N/A");
    
    // Table footer
    fprintf(file, "+----------+--------------------+----------+----------+----------+----------+----------+----------+\n");
    
    fclose(file);
    printf("ATM data file created successfully with both physical and virtual ATMs.\n");
}

// Initialize system configuration file with improved table formatting
void initSystemConfigFile() {
    FILE *file = fopen(ROOT_SYSTEM_CONFIG_FILE, "w");
    if (!file) {
        printf("Failed to create system configuration file!\n");
        return;
    }
    
    // Table header with proper formatting
    fprintf(file, "+---------------------------+---------------------+--------------------------------------+\n");
    fprintf(file, "| CONFIGURATION PARAMETER   | VALUE               | DESCRIPTION                          |\n");
    fprintf(file, "+---------------------------+---------------------+--------------------------------------+\n");
    
    // System configurations
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "max_withdrawal_limit", "₹25000", "Maximum amount per withdrawal");
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "max_transfer_limit", "₹50000", "Maximum amount per transfer");
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "daily_transaction_limit", "₹100000", "Maximum daily transaction limit");
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "enable_virtual_atm", "true", "Enable virtual ATM services");
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "virtual_withdrawal_limit", "₹10000", "Virtual ATM withdrawal limit");
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "transaction_fee", "₹0.00", "Fee per transaction");
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "force_pin_change_days", "90 days", "Force PIN change interval");
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "receipt_enabled", "true", "Enable receipt printing");
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "service_start_time", "00:00", "Service start time");
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "service_end_time", "24:00", "Service end time");
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "maintenance_day", "Sunday", "Weekly maintenance day");
    fprintf(file, "| %-25s | %-19s | %-36s |\n", "language_support", "en,hi,mr,gu", "Supported languages");
    
    // Table footer
    fprintf(file, "+---------------------------+---------------------+--------------------------------------+\n");
    
    fclose(file);
    printf("System configuration file created successfully.\n");
}

// Generate a receipt for account creation
void generateAccountReceipt() {
    // Create directory if it doesn't exist
    system("mkdir -p tests/sample_receipts");
    
    FILE *file = fopen("tests/sample_receipts/account_creation.txt", "w");
    if (!file) {
        printf("Failed to create sample receipt!\n");
        return;
    }
    
    // Generate sample data for receipt
    char cardNumber[20];
    char expiryDate[6];
    generateCardNumber(cardNumber);
    generateExpiryDate(expiryDate);
    int cvv = generateCVV();
    long accountNumber = generateAccountNumber();
    
    // Print receipt format
    fprintf(file, "****************************************\n");
    fprintf(file, "*           ACCOUNT CREATION           *\n");
    fprintf(file, "****************************************\n");
    fprintf(file, "Date: 2025-05-01    Time: 10:15:22\n");
    fprintf(file, "Branch: Main Branch\n");
    fprintf(file, "----------------------------------------\n");
    fprintf(file, "CUSTOMER DETAILS:\n");
    fprintf(file, "Customer ID: CUS12345\n");
    fprintf(file, "Name: John Doe\n");
    fprintf(file, "----------------------------------------\n");
    fprintf(file, "ACCOUNT DETAILS:\n");
    fprintf(file, "Account Number: %ld\n", accountNumber);
    fprintf(file, "Account Type: Savings\n");
    fprintf(file, "IFSC Code: SBIN0012345\n");
    fprintf(file, "----------------------------------------\n");
    fprintf(file, "CARD DETAILS:\n");
    fprintf(file, "Card Number: %s\n", cardNumber);
    fprintf(file, "Valid Thru: %s\n", expiryDate);
    fprintf(file, "CVV: %03d (KEEP CONFIDENTIAL)\n", cvv);
    fprintf(file, "----------------------------------------\n");
    fprintf(file, "Your PIN has been sent to your registered\n");
    fprintf(file, "mobile number. Please change it on first use.\n");
    fprintf(file, "\n");
    fprintf(file, "Thank you for banking with us!\n");
    fprintf(file, "****************************************\n");
    
    fclose(file);
    printf("Sample account creation receipt generated successfully.\n");
}

int main() {
    // Seed the random number generator
    srand((unsigned int)time(NULL));
    
    printf("Initializing ATM system data files...\n");
    
    // Create all necessary data files
    initCardFile();
    initCustomerFile();
    initAccountingFile();
    initVirtualWalletFile();
    initATMDataFile();
    initSystemConfigFile();
    
    // Generate sample account creation receipt
    generateAccountReceipt();
    
    printf("\nAll data files initialized successfully!\n");
    printf("The ATM system is ready for testing.\n");
    
    return 0;
}