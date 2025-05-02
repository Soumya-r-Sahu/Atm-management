/**
 * @file upi_transaction.c
 * @brief Implementation of UPI Transaction functionality for the ATM Management System
 * @date May 1, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>  /* Added for va_list, va_start, va_end */
#include <upi_transaction/upi_transaction.h>
#include <common/utils/file_utils.h>
#include <common/utils/string_utils.h>
#include <common/security/hash_utils.h>
#include <common/utils/logger.h>
#include <common/database/database.h>
#include <common/paths.h>
#include <common/constants.h>
#include <common/utils/hash_utils.h>

#define UPI_DATA_FILE "data/virtual_wallet.txt"
#define UPI_TRANSACTION_FILE "data/upi_transactions.txt"
#define MAX_LINE_LENGTH 512
#define UPI_PIN_LENGTH 6
#define UPI_VPA_MAX_LENGTH 50
#define UPI_DAILY_LIMIT 100000.0

/* Forward declarations of local functions */
bool get_account_holder_name(const char* account_number, char* name, size_t size);
bool save_upi_pin(const char* vpa, const char* pin_hash);
bool get_upi_pin_hash(const char* vpa, char* hash, size_t size);
int generate_unique_id(void);
bool get_account_balance(const char* account_number, double* balance);
bool debit_account(const char* account_number, double amount, double* new_balance);
bool credit_account(const char* account_number, double amount, double* new_balance);

// Custom logging function to handle formatted messages
static void log_formatted_error(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    writeErrorLog(buffer);
}

static void log_formatted_info(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    writeInfoLog(buffer);
}

// Replace the macros with these functions
#define log_error log_formatted_error
#define log_info log_formatted_info

// Map hash functions to the hash utilities
#define compute_hash(input, output, size) { \
    char* temp = sha256_hash(input); \
    if (temp) { \
        strncpy(output, temp, size-1); \
        output[size-1] = '\0'; \
        free(temp); \
    } else { \
        output[0] = '\0'; \
    } \
}

// Local function prototypes
static bool load_vpa_details(const char* vpa, UpiVirtualAddress* vpa_details);
static bool save_transaction_record(const UpiTransaction* transaction);
static bool update_account_balance(const char* account_number, double amount, bool is_debit);
static bool generate_reference_id(char* reference_id, size_t size);
static bool validate_upi_pin(const char* vpa, const char* pin);
static bool save_vpa_to_file(const UpiVirtualAddress* vpa_details);
static double get_daily_transaction_amount(const char* vpa);

bool upi_init(void) {
    log_info("Initializing UPI transaction system...");
    
    // Ensure data files exist
    FILE* wallet_file = fopen(UPI_DATA_FILE, "a+");
    if (!wallet_file) {
        log_error("Failed to open or create UPI wallet file");
        return false;
    }
    fclose(wallet_file);
    
    FILE* transaction_file = fopen(UPI_TRANSACTION_FILE, "a+");
    if (!transaction_file) {
        log_error("Failed to open or create UPI transaction file");
        return false;
    }
    fclose(transaction_file);
    
    log_info("UPI transaction system initialized successfully");
    return true;
}

UpiStatus upi_register_vpa(
    const char* account_number,
    const char* vpa_prefix,
    const char* bank_code,
    const char* mobile_number,
    const char* upi_pin,
    char* vpa_out
) {
    if (!account_number || !vpa_prefix || !bank_code || !mobile_number || !upi_pin || !vpa_out) {
        log_error("Invalid parameters provided for UPI VPA registration");
        return UPI_SYSTEM_ERROR;
    }
    
    // Validate inputs
    if (strlen(upi_pin) != UPI_PIN_LENGTH || !is_numeric(upi_pin)) {
        log_error("Invalid UPI PIN format");
        return UPI_INVALID_VPA;
    }
    
    // Generate VPA
    char vpa[UPI_VPA_MAX_LENGTH];
    snprintf(vpa, sizeof(vpa), "%s@%s", vpa_prefix, bank_code);
    
    // Check if VPA already exists
    bool exists;
    char account_holder[50];
    UpiStatus status = upi_validate_vpa(vpa, &exists, account_holder);
    if (status != UPI_SUCCESS) {
        log_error("Failed to validate VPA during registration");
        return status;
    }
    
    if (exists) {
        log_error("VPA already exists: %s", vpa);
        return UPI_DUPLICATE_TRANSACTION;
    }
    
    // Get account holder name
    char holder_name[50];
    if (!get_account_holder_name(account_number, holder_name, sizeof(holder_name))) {
        log_error("Could not find account holder for account number: %s", account_number);
        return UPI_SYSTEM_ERROR;
    }
    
    // Create new VPA record
    UpiVirtualAddress vpa_details;
    strncpy(vpa_details.vpa_id, vpa, sizeof(vpa_details.vpa_id));
    strncpy(vpa_details.account_number, account_number, sizeof(vpa_details.account_number));
    strncpy(vpa_details.account_holder, holder_name, sizeof(vpa_details.account_holder));
    strncpy(vpa_details.linked_mobile, mobile_number, sizeof(vpa_details.linked_mobile));
    vpa_details.is_active = true;
    vpa_details.creation_date = time(NULL);
    
    // Hash UPI PIN and save
    char pin_hash[100];
    compute_hash(upi_pin, pin_hash, sizeof(pin_hash));
    
    // Save VPA details and PIN hash
    if (!save_vpa_to_file(&vpa_details) || 
        !save_upi_pin(vpa, pin_hash)) {
        log_error("Failed to save VPA details or PIN hash");
        return UPI_SYSTEM_ERROR;
    }
    
    // Return created VPA
    strncpy(vpa_out, vpa, UPI_VPA_MAX_LENGTH);
    log_info("UPI VPA registered successfully: %s", vpa);
    
    return UPI_SUCCESS;
}

UpiStatus upi_authenticate(const char* vpa, const char* upi_pin) {
    if (!vpa || !upi_pin) {
        return UPI_SYSTEM_ERROR;
    }
    
    // Check if VPA exists
    UpiVirtualAddress vpa_details;
    if (!load_vpa_details(vpa, &vpa_details)) {
        log_error("VPA not found: %s", vpa);
        return UPI_INVALID_VPA;
    }
    
    // Check if VPA is active
    if (!vpa_details.is_active) {
        log_error("VPA is inactive: %s", vpa);
        return UPI_INVALID_VPA;
    }
    
    // Validate PIN
    if (!validate_upi_pin(vpa, upi_pin)) {
        log_error("UPI authentication failed for VPA: %s", vpa);
        return UPI_AUTHENTICATION_FAILED;
    }
    
    return UPI_SUCCESS;
}

UpiStatus upi_transfer(
    const char* sender_vpa,
    const char* receiver_vpa,
    double amount,
    const char* upi_pin,
    const char* remarks,
    int* transaction_id
) {
    if (!sender_vpa || !receiver_vpa || amount <= 0 || !upi_pin) {
        return UPI_SYSTEM_ERROR;
    }
    
    // Authenticate sender
    UpiStatus auth_status = upi_authenticate(sender_vpa, upi_pin);
    if (auth_status != UPI_SUCCESS) {
        return auth_status;
    }
    
    // Check if receiver VPA exists
    bool receiver_exists;
    char receiver_name[50];
    UpiStatus receiver_status = upi_validate_vpa(receiver_vpa, &receiver_exists, receiver_name);
    if (receiver_status != UPI_SUCCESS || !receiver_exists) {
        log_error("Receiver VPA not found or error: %s", receiver_vpa);
        return UPI_INVALID_VPA;
    }
    
    // Get sender details
    UpiVirtualAddress sender_details;
    if (!load_vpa_details(sender_vpa, &sender_details)) {
        log_error("Failed to load sender VPA details: %s", sender_vpa);
        return UPI_SYSTEM_ERROR;
    }
    
    // Get receiver details
    UpiVirtualAddress receiver_details;
    if (!load_vpa_details(receiver_vpa, &receiver_details)) {
        log_error("Failed to load receiver VPA details: %s", receiver_vpa);
        return UPI_SYSTEM_ERROR;
    }
    
    // Check daily limit
    double daily_amount = get_daily_transaction_amount(sender_vpa);
    if (daily_amount + amount > UPI_DAILY_LIMIT) {
        log_error("Daily UPI transaction limit exceeded for VPA: %s", sender_vpa);
        return UPI_LIMIT_EXCEEDED;
    }
    
    // Check balance
    double balance;
    if (!get_account_balance(sender_details.account_number, &balance)) {
        log_error("Failed to retrieve balance for account: %s", sender_details.account_number);
        return UPI_SYSTEM_ERROR;
    }
    
    if (balance < amount) {
        log_error("Insufficient balance for UPI transfer. Required: %.2f, Available: %.2f", amount, balance);
        return UPI_INSUFFICIENT_BALANCE;
    }
    
    // Generate transaction record
    UpiTransaction transaction;
    transaction.transaction_id = generate_unique_id();
    strncpy(transaction.sender_vpa, sender_vpa, sizeof(transaction.sender_vpa));
    strncpy(transaction.receiver_vpa, receiver_vpa, sizeof(transaction.receiver_vpa));
    strncpy(transaction.sender_account, sender_details.account_number, sizeof(transaction.sender_account));
    strncpy(transaction.receiver_account, receiver_details.account_number, sizeof(transaction.receiver_account));
    transaction.amount = amount;
    transaction.timestamp = time(NULL);
    strncpy(transaction.status, "PENDING", sizeof(transaction.status));
    
    if (remarks) {
        strncpy(transaction.remarks, remarks, sizeof(transaction.remarks));
    } else {
        strncpy(transaction.remarks, "UPI Transfer", sizeof(transaction.remarks));
    }
    
    generate_reference_id(transaction.reference_id, sizeof(transaction.reference_id));
    
    // Execute transaction
    if (!update_account_balance(sender_details.account_number, amount, true)) {
        log_error("Failed to debit sender account: %s", sender_details.account_number);
        return UPI_TRANSACTION_FAILED;
    }
    
    if (!update_account_balance(receiver_details.account_number, amount, false)) {
        // Rollback debit if credit fails
        update_account_balance(sender_details.account_number, amount, false);
        log_error("Failed to credit receiver account: %s", receiver_details.account_number);
        return UPI_TRANSACTION_FAILED;
    }
    
    // Update transaction status
    strncpy(transaction.status, "SUCCESS", sizeof(transaction.status));
    
    // Save transaction record
    if (!save_transaction_record(&transaction)) {
        log_error("Failed to save UPI transaction record");
        return UPI_SYSTEM_ERROR;
    }
    
    // Return transaction ID
    if (transaction_id) {
        *transaction_id = transaction.transaction_id;
    }
    
    log_info("UPI transfer successful: %s -> %s, Amount: %.2f, Ref: %s", 
             sender_vpa, receiver_vpa, amount, transaction.reference_id);
    
    return UPI_SUCCESS;
}

UpiStatus upi_check_balance(const char* vpa, const char* upi_pin, double* balance) {
    if (!vpa || !upi_pin || !balance) {
        return UPI_SYSTEM_ERROR;
    }
    
    // Authenticate user
    UpiStatus auth_status = upi_authenticate(vpa, upi_pin);
    if (auth_status != UPI_SUCCESS) {
        return auth_status;
    }
    
    // Get VPA details
    UpiVirtualAddress vpa_details;
    if (!load_vpa_details(vpa, &vpa_details)) {
        log_error("VPA not found: %s", vpa);
        return UPI_INVALID_VPA;
    }
    
    // Get account balance
    if (!get_account_balance(vpa_details.account_number, balance)) {
        log_error("Failed to retrieve balance for account: %s", vpa_details.account_number);
        return UPI_SYSTEM_ERROR;
    }
    
    log_info("Balance check successful for VPA: %s", vpa);
    return UPI_SUCCESS;
}

UpiStatus upi_validate_vpa(const char* vpa, bool* exists, char* account_holder) {
    if (!vpa || !exists) {
        return UPI_SYSTEM_ERROR;
    }
    
    // Initialize result
    *exists = false;
    if (account_holder) {
        account_holder[0] = '\0';
    }
    
    // Try to load VPA details
    UpiVirtualAddress vpa_details;
    if (!load_vpa_details(vpa, &vpa_details)) {
        // VPA not found is not an error in this function
        return UPI_SUCCESS;
    }
    
    // VPA found
    *exists = true;
    if (account_holder) {
        strncpy(account_holder, vpa_details.account_holder, 50);
    }
    
    return UPI_SUCCESS;
}

UpiStatus upi_get_transaction_history(
    const char* vpa,
    const char* upi_pin,
    UpiTransaction* transactions,
    int max_count,
    int* actual_count
) {
    if (!vpa || !upi_pin || !transactions || max_count <= 0 || !actual_count) {
        return UPI_SYSTEM_ERROR;
    }
    
    // Authenticate user
    UpiStatus auth_status = upi_authenticate(vpa, upi_pin);
    if (auth_status != UPI_SUCCESS) {
        return auth_status;
    }
    
    // Initialize count
    *actual_count = 0;
    
    // Open transaction file
    FILE* file = fopen(UPI_TRANSACTION_FILE, "r");
    if (!file) {
        log_error("Failed to open UPI transaction file");
        return UPI_SYSTEM_ERROR;
    }
    
    // Read transactions
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) && *actual_count < max_count) {
        UpiTransaction transaction;
        
        // Parse transaction details
        if (sscanf(line, "%d|%49[^|]|%49[^|]|%19[^|]|%19[^|]|%lf|%ld|%19[^|]|%99[^|]|%19[^|]",
                  &transaction.transaction_id, transaction.sender_vpa, transaction.receiver_vpa,
                  transaction.sender_account, transaction.receiver_account, &transaction.amount,
                  &transaction.timestamp, transaction.status, transaction.remarks,
                  transaction.reference_id) == 10) {
            
            // Add to result if this transaction involves the VPA
            if (strcmp(transaction.sender_vpa, vpa) == 0 || strcmp(transaction.receiver_vpa, vpa) == 0) {
                transactions[*actual_count] = transaction;
                (*actual_count)++;
            }
        }
    }
    
    fclose(file);
    log_info("Retrieved %d transactions for VPA: %s", *actual_count, vpa);
    
    return UPI_SUCCESS;
}

UpiStatus upi_change_pin(const char* vpa, const char* old_pin, const char* new_pin) {
    if (!vpa || !old_pin || !new_pin) {
        return UPI_SYSTEM_ERROR;
    }
    
    // Validate new PIN
    if (strlen(new_pin) != UPI_PIN_LENGTH || !is_numeric(new_pin)) {
        log_error("Invalid new UPI PIN format");
        return UPI_INVALID_VPA;
    }
    
    // Authenticate with old PIN
    UpiStatus auth_status = upi_authenticate(vpa, old_pin);
    if (auth_status != UPI_SUCCESS) {
        return auth_status;
    }
    
    // Hash new PIN
    char new_pin_hash[100];
    compute_hash(new_pin, new_pin_hash, sizeof(new_pin_hash));
    
    // Save new PIN hash
    if (!save_upi_pin(vpa, new_pin_hash)) {
        log_error("Failed to update UPI PIN for VPA: %s", vpa);
        return UPI_SYSTEM_ERROR;
    }
    
    log_info("UPI PIN changed successfully for VPA: %s", vpa);
    return UPI_SUCCESS;
}

UpiStatus upi_deactivate_vpa(const char* vpa, const char* upi_pin) {
    if (!vpa || !upi_pin) {
        return UPI_SYSTEM_ERROR;
    }
    
    // Authenticate user
    UpiStatus auth_status = upi_authenticate(vpa, upi_pin);
    if (auth_status != UPI_SUCCESS) {
        return auth_status;
    }
    
    // Get VPA details
    UpiVirtualAddress vpa_details;
    if (!load_vpa_details(vpa, &vpa_details)) {
        log_error("VPA not found: %s", vpa);
        return UPI_INVALID_VPA;
    }
    
    // Deactivate VPA
    vpa_details.is_active = false;
    
    // Save updated details
    if (!save_vpa_to_file(&vpa_details)) {
        log_error("Failed to deactivate VPA: %s", vpa);
        return UPI_SYSTEM_ERROR;
    }
    
    log_info("VPA deactivated successfully: %s", vpa);
    return UPI_SUCCESS;
}

/* Helper functions */

static bool load_vpa_details(const char* vpa, UpiVirtualAddress* vpa_details) {
    if (!vpa || !vpa_details) {
        return false;
    }
    
    FILE* file = fopen(UPI_DATA_FILE, "r");
    if (!file) {
        return false;
    }
    
    char line[MAX_LINE_LENGTH];
    bool found = false;
    
    while (fgets(line, sizeof(line), file)) {
        char vpa_id[50], account_number[20], account_holder[50], linked_mobile[15];
        int is_active;
        time_t creation_date;
        
        if (sscanf(line, "%49[^|]|%19[^|]|%49[^|]|%d|%ld|%14[^\n]",
                  vpa_id, account_number, account_holder, &is_active, 
                  &creation_date, linked_mobile) == 6) {
            
            if (strcmp(vpa_id, vpa) == 0) {
                strncpy(vpa_details->vpa_id, vpa_id, sizeof(vpa_details->vpa_id));
                strncpy(vpa_details->account_number, account_number, sizeof(vpa_details->account_number));
                strncpy(vpa_details->account_holder, account_holder, sizeof(vpa_details->account_holder));
                strncpy(vpa_details->linked_mobile, linked_mobile, sizeof(vpa_details->linked_mobile));
                vpa_details->is_active = (bool)is_active;
                vpa_details->creation_date = creation_date;
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

static bool save_transaction_record(const UpiTransaction* transaction) {
    if (!transaction) {
        return false;
    }
    
    FILE* file = fopen(UPI_TRANSACTION_FILE, "a");
    if (!file) {
        return false;
    }
    
    fprintf(file, "%d|%s|%s|%s|%s|%.2f|%ld|%s|%s|%s\n",
            transaction->transaction_id, transaction->sender_vpa, transaction->receiver_vpa,
            transaction->sender_account, transaction->receiver_account, transaction->amount,
            transaction->timestamp, transaction->status, transaction->remarks,
            transaction->reference_id);
    
    fclose(file);
    return true;
}

static bool update_account_balance(const char* account_number, double amount, bool is_debit) {
    // Implementation would interact with the database module to update account balance
    // For now, we'll assume the function exists in database module
    double new_balance;
    if (is_debit) {
        if (!debit_account(account_number, amount, &new_balance)) {
            return false;
        }
    } else {
        if (!credit_account(account_number, amount, &new_balance)) {
            return false;
        }
    }
    
    return true;
}

static bool generate_reference_id(char* reference_id, size_t size) {
    if (!reference_id || size < 12) {
        return false;
    }
    
    // Generate format: UPI-XXXXXXXX (X is alphanumeric)
    strncpy(reference_id, "UPI-", 5);
    
    // Generate 8 random alphanumeric characters
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 4; i < 12; i++) {
        reference_id[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    reference_id[12] = '\0';
    
    return true;
}

static bool validate_upi_pin(const char* vpa, const char* pin) {
    if (!vpa || !pin) {
        return false;
    }
    
    // Compute hash of provided PIN
    char pin_hash[100];
    compute_hash(pin, pin_hash, sizeof(pin_hash));
    
    // Get stored PIN hash
    char stored_hash[100];
    if (!get_upi_pin_hash(vpa, stored_hash, sizeof(stored_hash))) {
        log_error("Failed to retrieve UPI PIN hash for VPA: %s", vpa);
        return false;
    }
    
    // Compare hashes
    return (strcmp(pin_hash, stored_hash) == 0);
}

static bool save_vpa_to_file(const UpiVirtualAddress* vpa_details) {
    if (!vpa_details) {
        return false;
    }
    
    // Check if VPA already exists (for update)
    UpiVirtualAddress existing;
    bool exists = load_vpa_details(vpa_details->vpa_id, &existing);
    
    if (exists) {
        // VPA exists, update the record
        FILE* input_file = fopen(UPI_DATA_FILE, "r");
        FILE* temp_file = fopen(UPI_DATA_FILE ".tmp", "w");
        
        if (!input_file || !temp_file) {
            if (input_file) fclose(input_file);
            if (temp_file) fclose(temp_file);
            return false;
        }
        
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), input_file)) {
            char vpa_id[50];
            if (sscanf(line, "%49[^|]", vpa_id) == 1 && strcmp(vpa_id, vpa_details->vpa_id) == 0) {
                // Replace with updated record
                fprintf(temp_file, "%s|%s|%s|%d|%ld|%s\n",
                        vpa_details->vpa_id, vpa_details->account_number, vpa_details->account_holder,
                        vpa_details->is_active, vpa_details->creation_date, vpa_details->linked_mobile);
            } else {
                fputs(line, temp_file);
            }
        }
        
        fclose(input_file);
        fclose(temp_file);
        
        remove(UPI_DATA_FILE);
        rename(UPI_DATA_FILE ".tmp", UPI_DATA_FILE);
    } else {
        // New VPA, append to file
        FILE* file = fopen(UPI_DATA_FILE, "a");
        if (!file) {
            return false;
        }
        
        fprintf(file, "%s|%s|%s|%d|%ld|%s\n",
                vpa_details->vpa_id, vpa_details->account_number, vpa_details->account_holder,
                vpa_details->is_active, vpa_details->creation_date, vpa_details->linked_mobile);
        
        fclose(file);
    }
    
    return true;
}

static double get_daily_transaction_amount(const char* vpa) {
    if (!vpa) {
        return 0.0;
    }
    
    FILE* file = fopen(UPI_TRANSACTION_FILE, "r");
    if (!file) {
        return 0.0;
    }
    
    double daily_total = 0.0;
    time_t now = time(NULL);
    struct tm* now_tm = localtime(&now);
    int today_year = now_tm->tm_year;
    int today_month = now_tm->tm_mon;
    int today_day = now_tm->tm_mday;
    
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char sender_vpa[50];
        double amount;
        time_t timestamp;
        
        if (sscanf(line, "%*d|%49[^|]|%*[^|]|%*[^|]|%*[^|]|%lf|%ld", 
                   sender_vpa, &amount, &timestamp) == 3) {
            
            if (strcmp(sender_vpa, vpa) == 0) {
                // Check if this transaction was today
                struct tm* trans_tm = localtime(&timestamp);
                if (trans_tm->tm_year == today_year && 
                    trans_tm->tm_mon == today_month && 
                    trans_tm->tm_mday == today_day) {
                    daily_total += amount;
                }
            }
        }
    }
    
    fclose(file);
    return daily_total;
}

// These would be implemented in database module
// Here we provide stubs for compilation
bool get_account_holder_name(const char* account_number, char* name, size_t size) {
    if (!account_number || !name) {
        return false;
    }
    
    // In a real implementation, this would query the database
    strncpy(name, "Account Holder", size);
    return true;
}

bool save_upi_pin(const char* vpa, const char* pin_hash) {
    // In a real implementation, this would save to a secure file or database
    return true;
}

bool get_upi_pin_hash(const char* vpa, char* hash, size_t size) {
    // In a real implementation, this would retrieve from a secure file or database
    strncpy(hash, "dummy_hash", size);
    return true;
}

int generate_unique_id(void) {
    // Generate random ID between 100000 and 999999
    return (rand() % 900000) + 100000;
}

bool get_account_balance(const char* account_number, double* balance) {
    if (!account_number || !balance) {
        return false;
    }
    
    // In a real implementation, this would query the database
    *balance = 10000.0;  // Dummy value
    return true;
}

bool debit_account(const char* account_number, double amount, double* new_balance) {
    if (!account_number || amount <= 0 || !new_balance) {
        return false;
    }
    
    // In a real implementation, this would update the database
    *new_balance = 10000.0 - amount;  // Dummy implementation
    return true;
}

bool credit_account(const char* account_number, double amount, double* new_balance) {
    if (!account_number || amount <= 0 || !new_balance) {
        return false;
    }
    
    // In a real implementation, this would update the database
    *new_balance = 10000.0 + amount;  // Dummy implementation
    return true;
}

// Main function for the UPI system executable
int main(int argc, char* argv[]) {
    printf("UPI Transaction System Starting...\n");
    
    // Initialize the UPI system
    if (!upi_init()) {
        fprintf(stderr, "Failed to initialize UPI system\n");
        return 1;
    }
    
    printf("UPI Transaction System initialized successfully.\n");
    printf("Run with appropriate command-line arguments to use specific functions.\n");
    printf("Example: %s register <account_number> <vpa_prefix> <bank_code> <mobile> <pin>\n", argv[0]);
    
    // Simple command-line interface
    if (argc > 1) {
        if (strcmp(argv[1], "register") == 0 && argc >= 7) {
            char vpa_out[UPI_VPA_MAX_LENGTH];
            UpiStatus status = upi_register_vpa(
                argv[2], // account number
                argv[3], // vpa prefix
                argv[4], // bank code
                argv[5], // mobile
                argv[6], // pin
                vpa_out
            );
            
            printf("Registration result: %s\n", 
                  (status == UPI_SUCCESS) ? "SUCCESS" : "FAILED");
                  
            if (status == UPI_SUCCESS) {
                printf("VPA created: %s\n", vpa_out);
            }
        }
        else if (strcmp(argv[1], "balance") == 0 && argc >= 4) {
            double balance;
            UpiStatus status = upi_check_balance(
                argv[2], // vpa
                argv[3], // pin
                &balance
            );
            
            if (status == UPI_SUCCESS) {
                printf("Current balance: %.2f\n", balance);
            } else {
                printf("Balance check failed: %d\n", status);
            }
        }
        else if (strcmp(argv[1], "transfer") == 0 && argc >= 7) {
            int transaction_id;
            UpiStatus status = upi_transfer(
                argv[2], // sender vpa
                argv[3], // receiver vpa
                atof(argv[4]), // amount
                argv[5], // pin
                argv[6], // remarks
                &transaction_id
            );
            
            if (status == UPI_SUCCESS) {
                printf("Transfer successful, Transaction ID: %d\n", transaction_id);
            } else {
                printf("Transfer failed: %d\n", status);
            }
        }
        else {
            printf("Invalid command or insufficient arguments\n");
        }
    }
    
    return 0;
}