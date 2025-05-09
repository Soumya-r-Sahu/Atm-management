#ifndef TRANSACTION_TYPES_H
#define TRANSACTION_TYPES_H

// Transaction type enumeration
typedef enum {
    TRANSACTION_BALANCE = 0,              // Balance inquiry
    TRANSACTION_WITHDRAWAL = 1,           // Cash withdrawal
    TRANSACTION_DEPOSIT = 2,              // Cash deposit
    TRANSACTION_TRANSFER = 3,             // Fund transfer
    TRANSACTION_PIN_CHANGE = 4,           // PIN change
    TRANSACTION_MINI_STATEMENT = 5,       // Mini statement request
    TRANSACTION_CARD_REQUEST = 6,         // New card request
    TRANSACTION_BILL_PAYMENT = 7          // Bill payment
} TransactionType;

// Transaction structure for storing transaction records
typedef struct {
    int id;                         // Transaction ID
    int card_number;                // Card number
    TransactionType type;           // Transaction type
    char timestamp[20];             // Timestamp
    float amount;                   // Transaction amount
    float balance;                  // Account balance after transaction
    char transaction_type[20];      // String representation of type
    char transaction_id[37];        // Transaction UUID
    int status;                     // 1 for Success, 0 for Failed
} Transaction;

// Query result structure for returning multiple transactions
typedef struct {
    int success;                    // Whether query was successful
    int count;                      // Number of transactions returned
    Transaction* data;              // Array of transactions
} QueryResult;

#endif // TRANSACTION_TYPES_H