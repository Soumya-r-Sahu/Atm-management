/**
 * @file account_management_dao.c
 * @brief Account Management functions using DAO pattern
 * @version 1.0
 * @date May 12, 2025
 */

#include "../../include/common/database/dao_interface.h"
#include "../../include/common/database/account_management.h"
#include "../../include/common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// External function to get the DAO instance
extern DatabaseAccessObject* getDAO(void);

/**
 * @brief Get the balance of an account using the card number with DAO
 * @param cardNumber The card number as a string
 * @param balance Output parameter for the account balance
 * @return true if the balance was retrieved successfully, false otherwise
 */
bool cbs_get_balance_by_card_dao(const char* cardNumber, double* balance) {
    if (!cardNumber || !balance) {
        writeErrorLog("Invalid parameters in cbs_get_balance_by_card_dao");
        return false;
    }
    
    // Get the DAO instance
    DatabaseAccessObject* dao = getDAO();
    if (!dao) {
        writeErrorLog("Failed to get DAO instance in cbs_get_balance_by_card_dao");
        return false;
    }
    
    // Convert string card number to integer
    int cardNum = atoi(cardNumber);
    if (cardNum <= 0) {
        writeErrorLog("Invalid card number format in cbs_get_balance_by_card_dao");
        return false;
    }
    
    // Check if card exists
    if (!dao->doesCardExist(cardNum)) {
        writeErrorLog("Card %s does not exist", cardNumber);
        return false;
    }
    
    // Fetch balance using DAO
    float bal = dao->fetchBalance(cardNum);
    if (bal < 0) {
        writeErrorLog("Failed to fetch balance for card %s", cardNumber);
        return false;
    }
    
    // Set the output parameter
    *balance = (double)bal;
    return true;
}

/**
 * @brief Update the balance of an account using card number with DAO
 * @param cardNumber The card number as a string
 * @param newBalance The new balance
 * @param transactionType The type of transaction (WITHDRAWAL, DEPOSIT, etc.)
 * @return true if the balance was updated successfully, false otherwise
 */
bool cbs_update_balance_by_card_dao(const char* cardNumber, double newBalance, const char* transactionType) {
    if (!cardNumber || !transactionType) {
        writeErrorLog("Invalid parameters in cbs_update_balance_by_card_dao");
        return false;
    }
    
    // Get the DAO instance
    DatabaseAccessObject* dao = getDAO();
    if (!dao) {
        writeErrorLog("Failed to get DAO instance in cbs_update_balance_by_card_dao");
        return false;
    }
    
    // Convert string card number to integer
    int cardNum = atoi(cardNumber);
    if (cardNum <= 0) {
        writeErrorLog("Invalid card number format in cbs_update_balance_by_card_dao");
        return false;
    }
    
    // Check if card exists
    if (!dao->doesCardExist(cardNum)) {
        writeErrorLog("Card %s does not exist", cardNumber);
        return false;
    }
    
    // Update balance using DAO
    bool success = dao->updateBalance(cardNum, (float)newBalance);
    if (!success) {
        writeErrorLog("Failed to update balance for card %s", cardNumber);
        return false;
    }
    
    // Log the transaction
    dao->logTransaction(cardNum, transactionType, (float)newBalance, true);
    
    return true;
}

/**
 * @brief Check if a card is active using DAO
 * @param cardNumber The card number as a string
 * @return true if the card is active, false otherwise
 */
bool cbs_is_card_active_dao(const char* cardNumber) {
    if (!cardNumber) {
        writeErrorLog("Invalid parameters in cbs_is_card_active_dao");
        return false;
    }
    
    // Get the DAO instance
    DatabaseAccessObject* dao = getDAO();
    if (!dao) {
        writeErrorLog("Failed to get DAO instance in cbs_is_card_active_dao");
        return false;
    }
    
    // Convert string card number to integer
    int cardNum = atoi(cardNumber);
    if (cardNum <= 0) {
        writeErrorLog("Invalid card number format in cbs_is_card_active_dao");
        return false;
    }
    
    // Check if card exists and is active
    return dao->doesCardExist(cardNum) && dao->isCardActive(cardNum);
}

/**
 * @brief Get card holder information using DAO
 * @param cardNumber The card number as a string
 * @param name Output buffer for name
 * @param nameSize Size of name buffer
 * @param phone Output buffer for phone
 * @param phoneSize Size of phone buffer
 * @return true if information was retrieved successfully, false otherwise
 */
bool cbs_get_cardholder_info_dao(const char* cardNumber, char* name, size_t nameSize, char* phone, size_t phoneSize) {
    if (!cardNumber || !name || !phone || nameSize == 0 || phoneSize == 0) {
        writeErrorLog("Invalid parameters in cbs_get_cardholder_info_dao");
        return false;
    }
    
    // Get the DAO instance
    DatabaseAccessObject* dao = getDAO();
    if (!dao) {
        writeErrorLog("Failed to get DAO instance in cbs_get_cardholder_info_dao");
        return false;
    }
    
    // Convert string card number to integer
    int cardNum = atoi(cardNumber);
    if (cardNum <= 0) {
        writeErrorLog("Invalid card number format in cbs_get_cardholder_info_dao");
        return false;
    }
    
    // Check if card exists
    if (!dao->doesCardExist(cardNum)) {
        writeErrorLog("Card %s does not exist", cardNumber);
        return false;
    }
    
    // Get cardholder information using DAO
    bool nameSuccess = dao->getCardHolderName(cardNum, name, nameSize);
    bool phoneSuccess = dao->getCardHolderPhone(cardNum, phone, phoneSize);
    
    return nameSuccess && phoneSuccess;
}
