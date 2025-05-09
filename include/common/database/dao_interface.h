/**
 * @file dao_interface.h
 * @brief Database Access Object Interface for ATM Management System
 * @version 1.0
 * @date May 10, 2025
 * 
 * This provides a unified interface for database operations,
 * abstracting the underlying storage mechanism (MySQL or file-based).
 */

#ifndef DAO_INTERFACE_H
#define DAO_INTERFACE_H

#include <stdbool.h>
#include <stddef.h>  /* For size_t */
#include "db_unified_config.h"
#include "../../atm/transaction/transaction_types.h"

/**
 * @brief Database Access Object structure for transaction operations
 */
typedef struct {
    // Card operations
    bool (*doesCardExist)(int cardNumber);
    bool (*isCardActive)(int cardNumber);
    bool (*validateCard)(int cardNumber, int pin);
    bool (*validateCardWithHash)(int cardNumber, const char* pinHash);
    bool (*validateCardCVV)(int cardNumber, int cvv);
    bool (*blockCard)(int cardNumber);
    bool (*unblockCard)(int cardNumber);
    bool (*updateCardPIN)(int cardNumber, const char* newPINHash);
    
    // Account operations
    bool (*getCardHolderName)(int cardNumber, char* name, size_t nameSize);
    bool (*getCardHolderPhone)(int cardNumber, char* phone, size_t phoneSize);
    float (*fetchBalance)(int cardNumber);
    bool (*updateBalance)(int cardNumber, float newBalance);
    
    // Transaction operations
    float (*getDailyWithdrawals)(int cardNumber);
    void (*logWithdrawal)(int cardNumber, float amount);
    bool (*logTransaction)(int cardNumber, const char* transactionType, float amount, bool success);
    bool (*getMiniStatement)(int cardNumber, Transaction* transactions, int maxTransactions, int* count);
    
    // Session management
    void* (*getConnection)(void);
    void (*releaseConnection)(void* conn);
} DatabaseAccessObject;

// Factory functions to create DAOs
DatabaseAccessObject* createMySQLDAO(void);
DatabaseAccessObject* createFileBasedDAO(void);

// Function to get the appropriate DAO based on configuration
DatabaseAccessObject* getDAO(void);

#endif /* DAO_INTERFACE_H */
