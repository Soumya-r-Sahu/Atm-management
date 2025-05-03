#ifndef DATA_ACCESS_H
#define DATA_ACCESS_H

// Handles centralized access to data files
struct DataAccessResult {
    int success;
    char error_message[256];
    void* data;
};

// Customer data access functions
struct DataAccessResult get_customer_data(int customer_id);
struct DataAccessResult update_customer_data(int customer_id, void* data);

// Card data access functions
struct DataAccessResult get_card_data(int card_number);
struct DataAccessResult update_card_data(int card_number, void* data);

// Transaction data access functions
struct DataAccessResult get_transaction_history(int card_number, int limit);
struct DataAccessResult record_transaction(void* transaction_data);

// Admin data access functions
struct DataAccessResult get_admin_credentials(const char* admin_id);
struct DataAccessResult update_admin_credentials(const char* admin_id, void* data);

#endif // DATA_ACCESS_H