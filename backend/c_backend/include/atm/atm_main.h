/**
 * @file atm_main.h
 * @brief Main header file for the ATM system
 */

#ifndef ATM_MAIN_H
#define ATM_MAIN_H

/**
 * @brief ATM system status codes
 */
typedef enum {
    ATM_SUCCESS = 0,           /**< Operation successful */
    ATM_ERROR_GENERAL = -1,    /**< General error */
    ATM_ERROR_DATABASE = -2,   /**< Database error */
    ATM_ERROR_AUTH = -3,       /**< Authentication error */
    ATM_ERROR_CARD = -4,       /**< Card error */
    ATM_ERROR_PIN = -5,        /**< PIN error */
    ATM_ERROR_ACCOUNT = -6,    /**< Account error */
    ATM_ERROR_FUNDS = -7,      /**< Insufficient funds */
    ATM_ERROR_LIMIT = -8,      /**< Limit exceeded */
    ATM_ERROR_NETWORK = -9,    /**< Network error */
    ATM_ERROR_HARDWARE = -10,  /**< Hardware error */
    ATM_ERROR_TIMEOUT = -11,   /**< Operation timeout */
    ATM_ERROR_CANCELLED = -12  /**< Operation cancelled by user */
} AtmStatusCode;

/**
 * @brief ATM transaction types
 */
typedef enum {
    TRANSACTION_WITHDRAWAL,    /**< Cash withdrawal */
    TRANSACTION_DEPOSIT,       /**< Cash deposit */
    TRANSACTION_TRANSFER,      /**< Fund transfer */
    TRANSACTION_BALANCE,       /**< Balance inquiry */
    TRANSACTION_MINI_STATEMENT,/**< Mini statement */
    TRANSACTION_PIN_CHANGE,    /**< PIN change */
    TRANSACTION_BILL_PAYMENT   /**< Bill payment */
} TransactionType;

/**
 * @brief ATM system configuration
 */
typedef struct {
    char atm_id[21];           /**< ATM ID */
    char location[256];        /**< ATM location */
    int max_withdrawal;        /**< Maximum withdrawal amount */
    int min_withdrawal;        /**< Minimum withdrawal amount */
    int withdrawal_step;       /**< Withdrawal amount step */
    int max_deposit;           /**< Maximum deposit amount */
    int min_deposit;           /**< Minimum deposit amount */
    int session_timeout;       /**< Session timeout in seconds */
    int max_pin_attempts;      /**< Maximum PIN attempts */
} AtmConfig;

/**
 * @brief Initialize the ATM system
 * @param config_file Path to the configuration file
 * @return AtmStatusCode Status code
 */
AtmStatusCode atm_init(const char *config_file);

/**
 * @brief Run the ATM system
 * @return AtmStatusCode Status code
 */
AtmStatusCode atm_run(void);

/**
 * @brief Shutdown the ATM system
 * @return AtmStatusCode Status code
 */
AtmStatusCode atm_shutdown(void);

/**
 * @brief Get the ATM configuration
 * @return const AtmConfig* Pointer to the ATM configuration
 */
const AtmConfig* atm_get_config(void);

#endif /* ATM_MAIN_H */
