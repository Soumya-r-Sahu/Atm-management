// Mock implementation for bill payment functions
#include "../../../../../include/common/transaction/bill_payment.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Process bill payment
bool cbs_process_bill_payment(const char* card_number, 
                              const char* bill_type,
                              const char* bill_reference,
                              double amount, 
                              char* transaction_id_out) {
    printf("MOCK: Processing bill payment for card %s, type: %s, ref: %s, amount: %.2f\n", 
           card_number, bill_type, bill_reference, amount);
    
    // Generate a simple transaction ID
    sprintf(transaction_id_out, "BILL-%s-%ld", bill_type, (long)time(NULL));
    return true;  // Always succeed for testing
}

// Get bill payment history
bool cbs_get_bill_payment_history(const char* card_number,
                                TransactionRecord* records,
                                int* count,
                                int max_records) {
    printf("MOCK: Getting bill payment history for card %s (max %d records)\n", 
           card_number, max_records);
    
    // Generate a few mock bill payment records
    *count = max_records > 2 ? 2 : max_records;
    
    if (*count > 0) {
        strcpy(records[0].transaction_id, "BILL-ELEC-20250501");
        strcpy(records[0].transaction_type, "BILL_PAYMENT");
        records[0].amount = 120.50;
        records[0].balance = 5000.0 - 120.50;
        strcpy(records[0].date, "2025-05-01 15:30:00");
        strcpy(records[0].status, "SUCCESS");
    }
    
    if (*count > 1) {
        strcpy(records[1].transaction_id, "BILL-WATER-20250503");
        strcpy(records[1].transaction_type, "BILL_PAYMENT");
        records[1].amount = 45.75;
        records[1].balance = 5000.0 - 120.50 - 45.75;
        strcpy(records[1].date, "2025-05-03 10:15:00");
        strcpy(records[1].status, "SUCCESS");
    }
    
    return true;
}

// Verify bill details
bool cbs_verify_bill(const char* bill_type, 
                    const char* bill_reference, 
                    double* amount_out, 
                    char* due_date_out, 
                    char* payee_name_out,
                    size_t string_buffer_size) {
    printf("MOCK: Verifying bill type: %s, reference: %s\n", bill_type, bill_reference);
    
    // Return mock data
    *amount_out = 125.75;
    strncpy(due_date_out, "2025-05-15", string_buffer_size - 1);
    due_date_out[string_buffer_size - 1] = '\0';
    
    strncpy(payee_name_out, "City Utility Company", string_buffer_size - 1);
    payee_name_out[string_buffer_size - 1] = '\0';
    
    return true;
}
