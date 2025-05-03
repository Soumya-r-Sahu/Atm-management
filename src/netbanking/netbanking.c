/**
 * @file netbanking.c
 * @brief Implementation of NetBanking functionality
 * @date May 1, 2025
 */

#include <netbanking/netbanking.h>
#include <common/utils/logger.h>
#include <common/database/database.h>
#include <common/utils/hash_utils.h>
#include <common/config/config_manager.h>
#include <common/paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/common/config/config_manager.h" // For dynamic configuration

#define NETBANKING_DATA_FILE "data/netbanking_users.txt"
#define NETBANKING_TRANSACTIONS_FILE "data/netbanking_transactions.txt"
#define BENEFICIARY_DATA_FILE "data/beneficiary_data.txt"
#define OTP_DATA_FILE "data/netbanking_otp.txt"
#define MAX_LOGIN_ATTEMPTS 5
#undef SESSION_TIMEOUT_SECONDS
#define SESSION_TIMEOUT_SECONDS getConfigValueInt(CONFIG_SESSION_TIMEOUT_SECONDS) // Use dynamic value

// Database of active sessions
static NetBankingSession active_sessions[100] = {0};
static int active_session_count = 0;

// Helper functions
static bool validate_session(const NetBankingSession* session);
static NetBankingStatus save_transaction_record(const NetBankingTransaction* transaction);
static NetBankingStatus update_account_balance(const char* account_number, double amount);
static bool load_user_data(const char* username, char* stored_password, char* account_number, int* user_id, int* failed_attempts);
static bool update_failed_attempts(const char* username, int attempts);
static bool generate_session_token(char* token, int length);

bool netbanking_init(void) {
    writeInfoLog("Initializing NetBanking subsystem");
    
    // Check if required files exist, create them if not
    FILE* file = fopen(NETBANKING_DATA_FILE, "a+");
    if (!file) {
        writeErrorLog("Failed to initialize NetBanking: could not access user data file");
        return false;
    }
    fclose(file);
    
    file = fopen(NETBANKING_TRANSACTIONS_FILE, "a+");
    if (!file) {
        writeErrorLog("Failed to initialize NetBanking: could not access transactions file");
        return false;
    }
    fclose(file);
    
    file = fopen(BENEFICIARY_DATA_FILE, "a+");
    if (!file) {
        writeErrorLog("Failed to initialize NetBanking: could not access beneficiary data file");
        return false;
    }
    fclose(file);
    
    file = fopen(OTP_DATA_FILE, "a+");
    if (!file) {
        writeErrorLog("Failed to initialize NetBanking: could not access OTP data file");
        return false;
    }
    fclose(file);
    
    // Clear any expired sessions
    for (int i = 0; i < active_session_count; i++) {
        if (difftime(time(NULL), active_sessions[i].last_activity) > SESSION_TIMEOUT_SECONDS) {
            active_sessions[i].is_active = false;
        }
    }
    
    writeInfoLog("NetBanking subsystem initialized successfully");
    return true;
}

NetBankingStatus netbanking_authenticate(const char* username, const char* password, NetBankingSession* session) {
    if (!username || !password || !session) {
        return NB_INVALID_CREDENTIALS;
    }
    
    char stored_password[100] = {0};
    char account_number[20] = {0};
    int user_id = 0;
    int failed_attempts = 0;
    
    if (!load_user_data(username, stored_password, account_number, &user_id, &failed_attempts)) {
        writeInfoLog("NetBanking authentication failed: User not found");
        return NB_INVALID_CREDENTIALS;
    }
    
    if (failed_attempts >= MAX_LOGIN_ATTEMPTS) {
        writeWarningLog("NetBanking authentication failed: Account locked");
        return NB_ACCOUNT_LOCKED;
    }
    
    char hashed_password[100] = {0};
    hash_password(password, hashed_password, sizeof(hashed_password));
    
    if (strcmp(hashed_password, stored_password) != 0) {
        // Increment failed attempts and save
        update_failed_attempts(username, failed_attempts + 1);
        writeInfoLog("NetBanking authentication failed: Invalid password");
        return NB_INVALID_CREDENTIALS;
    }
    
    // Reset failed attempts on successful login
    if (failed_attempts > 0) {
        update_failed_attempts(username, 0);
    }
    
    // Create a new session
    session->user_id = user_id;
    strncpy(session->username, username, sizeof(session->username)-1);
    strncpy(session->account_number, account_number, sizeof(session->account_number)-1);
    generate_session_token(session->session_token, sizeof(session->session_token)-1);
    session->login_time = time(NULL);
    session->last_activity = time(NULL);
    session->is_active = true;
    
    // Add to active sessions
    if (active_session_count < 100) {
        active_sessions[active_session_count++] = *session;
    }
    
    writeInfoLog("NetBanking authentication successful for user: %s", username);
    return NB_SUCCESS;
}

NetBankingStatus netbanking_check_balance(const NetBankingSession* session, double* balance) {
    if (!validate_session(session) || !balance) {
        return NB_INVALID_CREDENTIALS;
    }
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/customer.txt", getDataPath());
    
    FILE* file = fopen(filepath, "r");
    if (!file) {
        writeErrorLog("Failed to open customer data file for balance check");
        return NB_SYSTEM_ERROR;
    }
    
    char line[256];
    bool found = false;
    
    while (fgets(line, sizeof(line), file)) {
        char acc_num[20] = {0};
        double acc_balance = 0.0;
        
        // Format: AccountNumber|Name|Balance|...
        if (sscanf(line, "%[^|]|%*[^|]|%lf", acc_num, &acc_balance) == 2) {
            if (strcmp(acc_num, session->account_number) == 0) {
                *balance = acc_balance;
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    
    if (!found) {
        writeErrorLog("Account not found in customer data: %s", session->account_number);
        return NB_SYSTEM_ERROR;
    }
    
    // Update last activity time
    for (int i = 0; i < active_session_count; i++) {
        if (strcmp(active_sessions[i].session_token, session->session_token) == 0) {
            active_sessions[i].last_activity = time(NULL);
            break;
        }
    }
    
    return NB_SUCCESS;
}

NetBankingStatus netbanking_fund_transfer(
    const NetBankingSession* session,
    const char* beneficiary_account,
    double amount,
    const char* remarks,
    int* transaction_id
) {
    if (!validate_session(session)) {
        return NB_INVALID_CREDENTIALS;
    }
    
    if (!beneficiary_account || amount <= 0 || !transaction_id) {
        return NB_INVALID_BENEFICIARY;
    }
    
    // Check if beneficiary is registered
    bool beneficiary_valid = false;
    FILE* ben_file = fopen(BENEFICIARY_DATA_FILE, "r");
    if (ben_file) {
        char line[256];
        while (fgets(line, sizeof(line), ben_file)) {
            char acc_num[20] = {0};
            char owner_acc[20] = {0};
            
            if (sscanf(line, "%[^|]|%[^|]", owner_acc, acc_num) == 2) {
                if (strcmp(owner_acc, session->account_number) == 0 && 
                    strcmp(acc_num, beneficiary_account) == 0) {
                    beneficiary_valid = true;
                    break;
                }
            }
        }
        fclose(ben_file);
    }
    
    if (!beneficiary_valid) {
        writeInfoLog("Invalid beneficiary account in transfer: %s", beneficiary_account);
        return NB_INVALID_BENEFICIARY;
    }
    
    // Check available balance
    double balance = 0.0;
    NetBankingStatus status = netbanking_check_balance(session, &balance);
    
    if (status != NB_SUCCESS) {
        return status;
    }
    
    if (balance < amount) {
        writeInfoLog("Insufficient balance for transfer: %.2f < %.2f", balance, amount);
        return NB_INSUFFICIENT_BALANCE;
    }
    
    // Generate transaction ID
    *transaction_id = (int)time(NULL) + rand() % 10000;
    
    // Create transaction record
    NetBankingTransaction transaction = {0};
    transaction.transaction_id = *transaction_id;
    strncpy(transaction.account_number, session->account_number, sizeof(transaction.account_number)-1);
    strncpy(transaction.beneficiary_account, beneficiary_account, sizeof(transaction.beneficiary_account)-1);
    strncpy(transaction.transaction_type, "TRANSFER", sizeof(transaction.transaction_type)-1);
    transaction.amount = amount;
    transaction.timestamp = time(NULL);
    strncpy(transaction.status, "SUCCESS", sizeof(transaction.status)-1);
    
    if (remarks) {
        strncpy(transaction.remarks, remarks, sizeof(transaction.remarks)-1);
    } else {
        strncpy(transaction.remarks, "Fund Transfer", sizeof(transaction.remarks)-1);
    }
    
    // Generate reference ID
    snprintf(transaction.reference_id, sizeof(transaction.reference_id), "NB%d", *transaction_id);
    
    // Update sender's balance
    status = update_account_balance(session->account_number, -amount);
    if (status != NB_SUCCESS) {
        return status;
    }
    
    // Update receiver's balance
    status = update_account_balance(beneficiary_account, amount);
    if (status != NB_SUCCESS) {
        // Rollback sender's balance
        update_account_balance(session->account_number, amount);
        return status;
    }
    
    // Save transaction record
    status = save_transaction_record(&transaction);
    if (status != NB_SUCCESS) {
        // This is critical - transaction went through but record failed
        writeErrorLog("Failed to save transaction record for ID: %d", *transaction_id);
    }
    
    // Update last activity time
    for (int i = 0; i < active_session_count; i++) {
        if (strcmp(active_sessions[i].session_token, session->session_token) == 0) {
            active_sessions[i].last_activity = time(NULL);
            break;
        }
    }
    
    writeInfoLog("Fund transfer successful: %s -> %s, Amount: %.2f, ID: %d", 
               session->account_number, beneficiary_account, amount, *transaction_id);
    
    return NB_SUCCESS;
}

NetBankingStatus netbanking_get_transaction_history(
    const NetBankingSession* session,
    NetBankingTransaction* transactions,
    int max_count,
    int* actual_count
) {
    if (!validate_session(session) || !transactions || max_count <= 0 || !actual_count) {
        return NB_INVALID_CREDENTIALS;
    }
    
    *actual_count = 0;
    
    FILE* file = fopen(NETBANKING_TRANSACTIONS_FILE, "r");
    if (!file) {
        writeErrorLog("Failed to open transactions file for history retrieval");
        return NB_SYSTEM_ERROR;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file) && *actual_count < max_count) {
        NetBankingTransaction trans = {0};
        char time_str[20] = {0};
        
        if (sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%lf|%[^|]|%[^|]|%[^|]|%s",
                  &trans.transaction_id, trans.account_number, trans.beneficiary_account,
                  trans.transaction_type, &trans.amount, time_str, trans.status,
                  trans.remarks, trans.reference_id) >= 8) {
            
            // Only include transactions for this account (sent or received)
            if (strcmp(trans.account_number, session->account_number) == 0 ||
                strcmp(trans.beneficiary_account, session->account_number) == 0) {
                
                trans.timestamp = (time_t)atol(time_str);
                transactions[*actual_count] = trans;
                (*actual_count)++;
            }
        }
    }
    
    fclose(file);
    
    // Update last activity time
    for (int i = 0; i < active_session_count; i++) {
        if (strcmp(active_sessions[i].session_token, session->session_token) == 0) {
            active_sessions[i].last_activity = time(NULL);
            break;
        }
    }
    
    return NB_SUCCESS;
}

NetBankingStatus netbanking_logout(NetBankingSession* session) {
    if (!session || !session->is_active) {
        return NB_INVALID_CREDENTIALS;
    }
    
    // Find and remove session from active sessions
    for (int i = 0; i < active_session_count; i++) {
        if (strcmp(active_sessions[i].session_token, session->session_token) == 0) {
            // Mark as inactive
            active_sessions[i].is_active = false;
            
            // Set the current session as inactive
            session->is_active = false;
            
            writeInfoLog("User logged out: %s", session->username);
            return NB_SUCCESS;
        }
    }
    
    return NB_SYSTEM_ERROR;
}

NetBankingStatus netbanking_change_password(
    const NetBankingSession* session,
    const char* old_password,
    const char* new_password
) {
    if (!validate_session(session) || !old_password || !new_password) {
        return NB_INVALID_CREDENTIALS;
    }
    
    // Verify old password first
    char stored_password[100] = {0};
    char account_number[20] = {0};
    int user_id = 0;
    int failed_attempts = 0;
    
    if (!load_user_data(session->username, stored_password, account_number, &user_id, &failed_attempts)) {
        return NB_SYSTEM_ERROR;
    }
    
    char hashed_old_password[100] = {0};
    hash_password(old_password, hashed_old_password, sizeof(hashed_old_password));
    
    if (strcmp(hashed_old_password, stored_password) != 0) {
        writeInfoLog("Password change failed: Invalid old password for user %s", session->username);
        return NB_INVALID_CREDENTIALS;
    }
    
    // Hash and save new password
    char hashed_new_password[100] = {0};
    hash_password(new_password, hashed_new_password, sizeof(hashed_new_password));
    
    // Read all users into memory
    FILE* file = fopen(NETBANKING_DATA_FILE, "r");
    if (!file) {
        writeErrorLog("Failed to open netbanking users file for password change");
        return NB_SYSTEM_ERROR;
    }
    
    char** lines = NULL;
    int line_count = 0;
    char line[512];
    
    while (fgets(line, sizeof(line), file)) {
        lines = realloc(lines, (line_count + 1) * sizeof(char*));
        if (!lines) {
            fclose(file);
            writeErrorLog("Memory allocation failed during password change");
            return NB_SYSTEM_ERROR;
        }
        
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        lines[line_count] = strdup(line);
        line_count++;
    }
    
    fclose(file);
    
    // Modify the line containing this user's data
    bool found = false;
    for (int i = 0; i < line_count; i++) {
        char username_in_file[50] = {0};
        char rest_of_line[512] = {0};
        
        if (sscanf(lines[i], "%[^|]|%[^\n]", username_in_file, rest_of_line) == 2) {
            if (strcmp(username_in_file, session->username) == 0) {
                // Split the rest of line at first | to separate password from the rest
                char* first_pipe = strchr(rest_of_line, '|');
                if (first_pipe) {
                    // Construct new line with updated password
                    char new_line[512];
                    snprintf(new_line, sizeof(new_line), "%s|%s%s", 
                             session->username, hashed_new_password, first_pipe);
                    
                    free(lines[i]);
                    lines[i] = strdup(new_line);
                    found = true;
                    break;
                }
            }
        }
    }
    
    if (!found) {
        // Free allocated memory
        for (int i = 0; i < line_count; i++) {
            free(lines[i]);
        }
        free(lines);
        
        writeErrorLog("Failed to find user record for password change: %s", session->username);
        return NB_SYSTEM_ERROR;
    }
    
    // Write updated data back to file
    file = fopen(NETBANKING_DATA_FILE, "w");
    if (!file) {
        // Free allocated memory
        for (int i = 0; i < line_count; i++) {
            free(lines[i]);
        }
        free(lines);
        
        writeErrorLog("Failed to open netbanking users file for writing updated password");
        return NB_SYSTEM_ERROR;
    }
    
    for (int i = 0; i < line_count; i++) {
        fprintf(file, "%s\n", lines[i]);
        free(lines[i]);
    }
    
    free(lines);
    fclose(file);
    
    // Update last activity time
    for (int i = 0; i < active_session_count; i++) {
        if (strcmp(active_sessions[i].session_token, session->session_token) == 0) {
            active_sessions[i].last_activity = time(NULL);
            break;
        }
    }
    
    writeInfoLog("Password changed successfully for user: %s", session->username);
    return NB_SUCCESS;
}

NetBankingStatus netbanking_add_beneficiary(
    const NetBankingSession* session,
    const char* beneficiary_account,
    const char* beneficiary_name,
    const char* bank_code
) {
    if (!validate_session(session) || !beneficiary_account || !beneficiary_name || !bank_code) {
        return NB_INVALID_CREDENTIALS;
    }
    
    // Validate beneficiary account exists in the system
    // For this example, we'll just check that it's not the same as the current account
    if (strcmp(session->account_number, beneficiary_account) == 0) {
        writeInfoLog("Cannot add own account as beneficiary: %s", beneficiary_account);
        return NB_INVALID_BENEFICIARY;
    }
    
    // Check if beneficiary already exists
    FILE* file = fopen(BENEFICIARY_DATA_FILE, "r");
    if (file) {
        char line[512];
        while (fgets(line, sizeof(line), file)) {
            char owner_acc[20] = {0};
            char ben_acc[20] = {0};
            
            if (sscanf(line, "%[^|]|%[^|]", owner_acc, ben_acc) == 2) {
                if (strcmp(owner_acc, session->account_number) == 0 && 
                    strcmp(ben_acc, beneficiary_account) == 0) {
                    fclose(file);
                    writeInfoLog("Beneficiary already exists: %s", beneficiary_account);
                    return NB_INVALID_BENEFICIARY;
                }
            }
        }
        fclose(file);
    }
    
    // Add new beneficiary
    file = fopen(BENEFICIARY_DATA_FILE, "a");
    if (!file) {
        writeErrorLog("Failed to open beneficiary data file for adding");
        return NB_SYSTEM_ERROR;
    }
    
    fprintf(file, "%s|%s|%s|%s|%ld\n", 
           session->account_number, beneficiary_account, beneficiary_name, 
           bank_code, time(NULL));
    
    fclose(file);
    
    // Update last activity time
    for (int i = 0; i < active_session_count; i++) {
        if (strcmp(active_sessions[i].session_token, session->session_token) == 0) {
            active_sessions[i].last_activity = time(NULL);
            break;
        }
    }
    
    writeInfoLog("Added beneficiary: %s for account: %s", beneficiary_name, session->account_number);
    return NB_SUCCESS;
}

NetBankingStatus netbanking_generate_otp(
    const NetBankingSession* session,
    char* otp,
    int* validity_mins
) {
    if (!validate_session(session) || !otp || !validity_mins) {
        return NB_INVALID_CREDENTIALS;
    }
    
    // Default validity period
    *validity_mins = 10;
    
    // Generate a 6-digit OTP
    srand((unsigned int)time(NULL));
    int otp_value = 100000 + rand() % 900000; // 6-digit number between 100000 and 999999
    snprintf(otp, 7, "%06d", otp_value);
    
    // Store OTP in the database
    FILE* file = fopen(OTP_DATA_FILE, "a");
    if (!file) {
        writeErrorLog("Failed to open OTP data file for writing");
        return NB_SYSTEM_ERROR;
    }
    
    time_t expires_at = time(NULL) + (*validity_mins * 60);
    fprintf(file, "%s|%s|%ld|%d\n", session->username, otp, expires_at, 0); // 0 = not used yet
    
    fclose(file);
    
    // Update last activity time
    for (int i = 0; i < active_session_count; i++) {
        if (strcmp(active_sessions[i].session_token, session->session_token) == 0) {
            active_sessions[i].last_activity = time(NULL);
            break;
        }
    }
    
    writeInfoLog("Generated OTP for user: %s", session->username);
    return NB_SUCCESS;
}

// Helper function implementations
static bool validate_session(const NetBankingSession* session) {
    if (!session || !session->is_active) {
        return false;
    }
    
    // Find session in active sessions
    for (int i = 0; i < active_session_count; i++) {
        if (strcmp(active_sessions[i].session_token, session->session_token) == 0) {
            if (!active_sessions[i].is_active) {
                return false;
            }
            
            // Check session timeout
            if (difftime(time(NULL), active_sessions[i].last_activity) > SESSION_TIMEOUT_SECONDS) {
                active_sessions[i].is_active = false;
                writeInfoLog("Session expired for user: %s", session->username);
                return false;
            }
            
            return true;
        }
    }
    
    return false;
}

static NetBankingStatus save_transaction_record(const NetBankingTransaction* transaction) {
    if (!transaction) {
        return NB_SYSTEM_ERROR;
    }
    
    FILE* file = fopen(NETBANKING_TRANSACTIONS_FILE, "a");
    if (!file) {
        writeErrorLog("Failed to open transactions file for writing");
        return NB_SYSTEM_ERROR;
    }
    
    fprintf(file, "%d|%s|%s|%s|%.2f|%ld|%s|%s|%s\n",
           transaction->transaction_id, transaction->account_number, 
           transaction->beneficiary_account, transaction->transaction_type, 
           transaction->amount, transaction->timestamp, transaction->status, 
           transaction->remarks, transaction->reference_id);
    
    fclose(file);
    return NB_SUCCESS;
}

static NetBankingStatus update_account_balance(const char* account_number, double amount) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/customer.txt", getDataPath());
    
    FILE* file = fopen(filepath, "r");
    if (!file) {
        writeErrorLog("Failed to open customer data file for balance update");
        return NB_SYSTEM_ERROR;
    }
    
    // Read all lines into memory
    char** lines = NULL;
    int line_count = 0;
    char line[512];
    
    while (fgets(line, sizeof(line), file)) {
        lines = realloc(lines, (line_count + 1) * sizeof(char*));
        if (!lines) {
            fclose(file);
            writeErrorLog("Memory allocation failed during balance update");
            return NB_SYSTEM_ERROR;
        }
        
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        lines[line_count] = strdup(line);
        line_count++;
    }
    
    fclose(file);
    
    // Find and update the account's balance
    bool found = false;
    for (int i = 0; i < line_count; i++) {
        char acc_num[20] = {0};
        char name[100] = {0};
        double balance = 0.0;
        char rest_of_line[512] = {0};
        
        // Format: AccountNumber|Name|Balance|...
        if (sscanf(lines[i], "%[^|]|%[^|]|%lf|%[^\n]", 
                  acc_num, name, &balance, rest_of_line) >= 3) {
            
            if (strcmp(acc_num, account_number) == 0) {
                double new_balance = balance + amount;
                
                if (new_balance < 0) {
                    // Insufficient funds
                    for (int j = 0; j < line_count; j++) {
                        free(lines[j]);
                    }
                    free(lines);
                    
                    writeInfoLog("Insufficient balance for account: %s", account_number);
                    return NB_INSUFFICIENT_BALANCE;
                }
                
                // Construct new line with updated balance
                char new_line[512];
                snprintf(new_line, sizeof(new_line), "%s|%s|%.2f", acc_num, name, new_balance);
                
                // Add back the rest of the line if it exists
                if (strlen(rest_of_line) > 0) {
                    strcat(new_line, "|");
                    strcat(new_line, rest_of_line);
                }
                
                free(lines[i]);
                lines[i] = strdup(new_line);
                found = true;
                break;
            }
        }
    }
    
    if (!found) {
        // Free allocated memory
        for (int i = 0; i < line_count; i++) {
            free(lines[i]);
        }
        free(lines);
        
        writeErrorLog("Account not found for balance update: %s", account_number);
        return NB_SYSTEM_ERROR;
    }
    
    // Write updated data back to file
    file = fopen(filepath, "w");
    if (!file) {
        // Free allocated memory
        for (int i = 0; i < line_count; i++) {
            free(lines[i]);
        }
        free(lines);
        
        writeErrorLog("Failed to open customer data file for writing updated balance");
        return NB_SYSTEM_ERROR;
    }
    
    for (int i = 0; i < line_count; i++) {
        fprintf(file, "%s\n", lines[i]);
        free(lines[i]);
    }
    
    free(lines);
    fclose(file);
    
    return NB_SUCCESS;
}

static bool load_user_data(const char* username, char* stored_password, char* account_number, int* user_id, int* failed_attempts) {
    FILE* file = fopen(NETBANKING_DATA_FILE, "r");
    if (!file) {
        writeErrorLog("Failed to open netbanking users file");
        return false;
    }
    
    char line[512];
    bool found = false;
    
    while (fgets(line, sizeof(line), file)) {
        char username_in_file[50] = {0};
        char password_in_file[100] = {0};
        char account_in_file[20] = {0};
        int id_in_file = 0;
        int attempts_in_file = 0;
        
        // Format: Username|Password|AccountNumber|UserID|FailedAttempts
        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%d", 
                  username_in_file, password_in_file, account_in_file,
                  &id_in_file, &attempts_in_file) >= 4) {
            
            if (strcmp(username_in_file, username) == 0) {
                strncpy(stored_password, password_in_file, 99);
                strncpy(account_number, account_in_file, 19);
                *user_id = id_in_file;
                *failed_attempts = attempts_in_file;
                found = true;
                break;
            }
        }
    }
    
    fclose(file);
    return found;
}

static bool update_failed_attempts(const char* username, int attempts) {
    FILE* file = fopen(NETBANKING_DATA_FILE, "r");
    if (!file) {
        writeErrorLog("Failed to open netbanking users file for updating failed attempts");
        return false;
    }
    
    // Read all lines into memory
    char** lines = NULL;
    int line_count = 0;
    char line[512];
    
    while (fgets(line, sizeof(line), file)) {
        lines = realloc(lines, (line_count + 1) * sizeof(char*));
        if (!lines) {
            fclose(file);
            writeErrorLog("Memory allocation failed during update of failed attempts");
            return false;
        }
        
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        lines[line_count] = strdup(line);
        line_count++;
    }
    
    fclose(file);
    
    // Find and update the user's failed attempts
    bool found = false;
    for (int i = 0; i < line_count; i++) {
        char username_in_file[50] = {0};
        char password[100] = {0};
        char account[20] = {0};
        int user_id = 0;
        int old_attempts = 0;
        
        // Format: Username|Password|AccountNumber|UserID|FailedAttempts
        if (sscanf(lines[i], "%[^|]|%[^|]|%[^|]|%d|%d", 
                  username_in_file, password, account, &user_id, &old_attempts) >= 4) {
            
            if (strcmp(username_in_file, username) == 0) {
                // Construct new line with updated attempts
                char new_line[512];
                snprintf(new_line, sizeof(new_line), "%s|%s|%s|%d|%d", 
                        username, password, account, user_id, attempts);
                
                free(lines[i]);
                lines[i] = strdup(new_line);
                found = true;
                break;
            }
        }
    }
    
    if (!found) {
        // Free allocated memory
        for (int i = 0; i < line_count; i++) {
            free(lines[i]);
        }
        free(lines);
        
        writeErrorLog("User not found for updating failed attempts: %s", username);
        return false;
    }
    
    // Write updated data back to file
    file = fopen(NETBANKING_DATA_FILE, "w");
    if (!file) {
        // Free allocated memory
        for (int i = 0; i < line_count; i++) {
            free(lines[i]);
        }
        free(lines);
        
        writeErrorLog("Failed to open netbanking users file for writing updated attempts");
        return false;
    }
    
    for (int i = 0; i < line_count; i++) {
        fprintf(file, "%s\n", lines[i]);
        free(lines[i]);
    }
    
    free(lines);
    fclose(file);
    
    return true;
}

static bool generate_session_token(char* token, int length) {
    if (!token || length <= 0) {
        return false;
    }
    
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const size_t charset_size = sizeof(charset) - 1;
    
    srand((unsigned int)time(NULL));
    
    for (int i = 0; i < length; i++) {
        token[i] = charset[rand() % charset_size];
    }
    
    token[length] = '\0';
    return true;
}