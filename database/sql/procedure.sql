-- Core Banking System Database Procedures
-- Last Updated: May 12, 2025

-- ===============================================================
-- Customer Management Procedures
-- ===============================================================

-- Create a new customer
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_create_customer(
    IN p_first_name VARCHAR(50),
    IN p_last_name VARCHAR(50),
    IN p_date_of_birth DATE,
    IN p_address VARCHAR(255),
    IN p_city VARCHAR(50),
    IN p_state VARCHAR(50),
    IN p_postal_code VARCHAR(20),
    IN p_country VARCHAR(50),
    IN p_phone VARCHAR(20),
    IN p_email VARCHAR(100),
    IN p_customer_type TINYINT,
    OUT p_customer_id INT
)
BEGIN
    INSERT INTO customers (
        first_name, last_name, date_of_birth, 
        address, city, state, postal_code, country, 
        phone, email, customer_type, status
    ) VALUES (
        p_first_name, p_last_name, p_date_of_birth, 
        p_address, p_city, p_state, p_postal_code, p_country, 
        p_phone, p_email, p_customer_type, 1
    );
    
    SET p_customer_id = LAST_INSERT_ID();
END //
DELIMITER ;

-- Update customer information
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_update_customer(
    IN p_customer_id INT,
    IN p_first_name VARCHAR(50),
    IN p_last_name VARCHAR(50),
    IN p_address VARCHAR(255),
    IN p_city VARCHAR(50),
    IN p_state VARCHAR(50),
    IN p_postal_code VARCHAR(20),
    IN p_country VARCHAR(50),
    IN p_phone VARCHAR(20),
    IN p_email VARCHAR(100),
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE affected_rows INT;
    
    UPDATE customers SET
        first_name = p_first_name,
        last_name = p_last_name,
        address = p_address,
        city = p_city,
        state = p_state,
        postal_code = p_postal_code,
        country = p_country,
        phone = p_phone,
        email = p_email
    WHERE customer_id = p_customer_id;
    
    SET affected_rows = ROW_COUNT();
    SET p_success = IF(affected_rows > 0, TRUE, FALSE);
END //
DELIMITER ;

-- Get customer details
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_get_customer_details(
    IN p_customer_id INT
)
BEGIN
    SELECT * FROM customers WHERE customer_id = p_customer_id;
END //
DELIMITER ;

-- Change customer status
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_change_customer_status(
    IN p_customer_id INT,
    IN p_status TINYINT,
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE affected_rows INT;
    
    -- Status: 1 = Active, 2 = Inactive, 3 = Blocked
    UPDATE customers SET
        status = p_status
    WHERE customer_id = p_customer_id;
    
    SET affected_rows = ROW_COUNT();
    SET p_success = IF(affected_rows > 0, TRUE, FALSE);
END //
DELIMITER ;

-- Search customers by name
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_search_customers(
    IN p_name VARCHAR(100)
)
BEGIN
    SELECT * FROM customers 
    WHERE first_name LIKE CONCAT('%', p_name, '%')
    OR last_name LIKE CONCAT('%', p_name, '%')
    ORDER BY last_name, first_name;
END //
DELIMITER ;

-- ===============================================================
-- Account Management Procedures
-- ===============================================================

-- Create a new account
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_create_account(
    IN p_account_id VARCHAR(20),
    IN p_customer_id INT,
    IN p_account_type TINYINT,
    IN p_currency VARCHAR(3),
    IN p_initial_deposit DECIMAL(15,2),
    IN p_interest_rate DECIMAL(5,2),
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_success = FALSE;
        ROLLBACK;
    END;
    
    START TRANSACTION;
    
    -- Create account
    INSERT INTO accounts (
        account_id, customer_id, account_type,
        balance, currency, interest_rate, status
    ) VALUES (
        p_account_id, p_customer_id, p_account_type,
        p_initial_deposit, p_currency, p_interest_rate, 1
    );
    
    -- Create initial deposit transaction if amount > 0
    IF p_initial_deposit > 0 THEN
        -- Generate transaction ID
        SET @transaction_id = CONCAT('TRX', DATE_FORMAT(NOW(), '%Y%m%d%H%i%s'), FLOOR(RAND() * 1000));
        
        -- Insert transaction
        INSERT INTO transactions (
            transaction_id, account_id, type, 
            amount, description, status
        ) VALUES (
            @transaction_id, p_account_id, 1, -- Type 1 = Deposit
            p_initial_deposit, 'Initial deposit', 2 -- Status 2 = Complete
        );
    END IF;
    
    COMMIT;
    SET p_success = TRUE;
END //
DELIMITER ;

-- Get account details
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_get_account_details(
    IN p_account_id VARCHAR(20)
)
BEGIN
    SELECT a.*, c.first_name, c.last_name 
    FROM accounts a
    JOIN customers c ON a.customer_id = c.customer_id
    WHERE a.account_id = p_account_id;
END //
DELIMITER ;

-- Get all accounts for a customer
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_get_customer_accounts(
    IN p_customer_id INT
)
BEGIN
    SELECT * FROM accounts WHERE customer_id = p_customer_id;
END //
DELIMITER ;

-- Change account status
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_change_account_status(
    IN p_account_id VARCHAR(20),
    IN p_status TINYINT,
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE affected_rows INT;
    -- Status: 1 = Active, 2 = Dormant, 3 = Closed, 4 = Frozen
    
    UPDATE accounts SET
        status = p_status,
        closed_date = IF(p_status = 3, CURRENT_TIMESTAMP, closed_date)
    WHERE account_id = p_account_id;
    
    SET affected_rows = ROW_COUNT();
    SET p_success = IF(affected_rows > 0, TRUE, FALSE);
END //
DELIMITER ;

-- ===============================================================
-- Transaction Procedures
-- ===============================================================

-- Process a deposit
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_process_deposit(
    IN p_account_id VARCHAR(20),
    IN p_amount DECIMAL(15,2),
    IN p_description VARCHAR(255),
    IN p_reference_id VARCHAR(50),
    OUT p_transaction_id VARCHAR(20),
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE current_balance DECIMAL(15,2);
    DECLARE account_status TINYINT;
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        ROLLBACK;
    END;
    
    -- Check account exists and is active
    SELECT balance, status INTO current_balance, account_status 
    FROM accounts WHERE account_id = p_account_id;
    
    IF account_status IS NULL THEN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Account not found';
    ELSEIF account_status != 1 THEN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Account is not active';
    END IF;
    
    START TRANSACTION;
    
    -- Generate transaction ID
    SET p_transaction_id = CONCAT('TRX', DATE_FORMAT(NOW(), '%Y%m%d%H%i%s'), FLOOR(RAND() * 1000));
    
    -- Insert transaction
    INSERT INTO transactions (
        transaction_id, account_id, type, 
        amount, description, reference_id, status
    ) VALUES (
        p_transaction_id, p_account_id, 1, -- Type 1 = Deposit
        p_amount, p_description, p_reference_id, 2 -- Status 2 = Complete
    );
    
    -- Update account balance
    UPDATE accounts SET
        balance = balance + p_amount
    WHERE account_id = p_account_id;
    
    -- Log in general ledger
    -- This is simplified - in a real system, you'd have proper GL account references
    INSERT INTO general_ledger (
        entry_id, debit_account, credit_account, 
        amount, description, transaction_id
    ) VALUES (
        CONCAT('GL', DATE_FORMAT(NOW(), '%Y%m%d%H%i%s'), FLOOR(RAND() * 1000)),
        'CASH', -- Debit Cash
        p_account_id, -- Credit Customer Account
        p_amount, 
        CONCAT('Deposit: ', p_description),
        p_transaction_id
    );
    
    COMMIT;
    SET p_success = TRUE;
END //
DELIMITER ;

-- Process a withdrawal
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_process_withdrawal(
    IN p_account_id VARCHAR(20),
    IN p_amount DECIMAL(15,2),
    IN p_description VARCHAR(255),
    IN p_reference_id VARCHAR(50),
    OUT p_transaction_id VARCHAR(20),
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE current_balance DECIMAL(15,2);
    DECLARE account_status TINYINT;
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        ROLLBACK;
    END;
    
    -- Check account exists and is active
    SELECT balance, status INTO current_balance, account_status 
    FROM accounts WHERE account_id = p_account_id;
    
    IF account_status IS NULL THEN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Account not found';
    ELSEIF account_status != 1 THEN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Account is not active';
    ELSEIF current_balance < p_amount THEN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Insufficient funds';
    END IF;
    
    START TRANSACTION;
    
    -- Generate transaction ID
    SET p_transaction_id = CONCAT('TRX', DATE_FORMAT(NOW(), '%Y%m%d%H%i%s'), FLOOR(RAND() * 1000));
    
    -- Insert transaction
    INSERT INTO transactions (
        transaction_id, account_id, type, 
        amount, description, reference_id, status
    ) VALUES (
        p_transaction_id, p_account_id, 2, -- Type 2 = Withdrawal
        p_amount, p_description, p_reference_id, 2 -- Status 2 = Complete
    );
    
    -- Update account balance
    UPDATE accounts SET
        balance = balance - p_amount
    WHERE account_id = p_account_id;
    
    -- Log in general ledger
    INSERT INTO general_ledger (
        entry_id, debit_account, credit_account, 
        amount, description, transaction_id
    ) VALUES (
        CONCAT('GL', DATE_FORMAT(NOW(), '%Y%m%d%H%i%s'), FLOOR(RAND() * 1000)),
        p_account_id, -- Debit Customer Account
        'CASH', -- Credit Cash
        p_amount, 
        CONCAT('Withdrawal: ', p_description),
        p_transaction_id
    );
    
    COMMIT;
    SET p_success = TRUE;
END //
DELIMITER ;

-- Process a transfer between accounts
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_process_transfer(
    IN p_from_account VARCHAR(20),
    IN p_to_account VARCHAR(20),
    IN p_amount DECIMAL(15,2),
    IN p_description VARCHAR(255),
    OUT p_transaction_id VARCHAR(20),
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE from_balance DECIMAL(15,2);
    DECLARE from_status TINYINT;
    DECLARE to_status TINYINT;
    DECLARE from_currency VARCHAR(3);
    DECLARE to_currency VARCHAR(3);
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        ROLLBACK;
    END;
    
    -- Check source account
    SELECT balance, status, currency INTO from_balance, from_status, from_currency 
    FROM accounts WHERE account_id = p_from_account;
    
    IF from_status IS NULL THEN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Source account not found';
    ELSEIF from_status != 1 THEN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Source account is not active';
    ELSEIF from_balance < p_amount THEN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Insufficient funds';
    END IF;
    
    -- Check destination account
    SELECT status, currency INTO to_status, to_currency 
    FROM accounts WHERE account_id = p_to_account;
    
    IF to_status IS NULL THEN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Destination account not found';
    ELSEIF to_status != 1 THEN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Destination account is not active';
    END IF;
    
    -- Check if currencies match (simplification - real system would handle currency conversion)
    IF from_currency != to_currency THEN
        SET p_success = FALSE;
        SET p_transaction_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Currency mismatch between accounts';
    END IF;
    
    START TRANSACTION;
    
    -- Generate transaction ID
    SET p_transaction_id = CONCAT('TRX', DATE_FORMAT(NOW(), '%Y%m%d%H%i%s'), FLOOR(RAND() * 1000));
    
    -- Insert transaction for source account (withdrawal)
    INSERT INTO transactions (
        transaction_id, account_id, type, 
        amount, description, status
    ) VALUES (
        p_transaction_id, p_from_account, 3, -- Type 3 = Transfer
        p_amount, CONCAT('Transfer to ', p_to_account, ': ', p_description), 2 -- Status 2 = Complete
    );
    
    -- Insert transaction for destination account (deposit)
    INSERT INTO transactions (
        transaction_id, account_id, type, 
        amount, description, reference_id, status
    ) VALUES (
        CONCAT(p_transaction_id, '-TO'), p_to_account, 3, -- Type 3 = Transfer
        p_amount, CONCAT('Transfer from ', p_from_account, ': ', p_description),
        p_transaction_id, 2 -- Status 2 = Complete
    );
    
    -- Update account balances
    UPDATE accounts SET balance = balance - p_amount WHERE account_id = p_from_account;
    UPDATE accounts SET balance = balance + p_amount WHERE account_id = p_to_account;
    
    -- Log in general ledger
    INSERT INTO general_ledger (
        entry_id, debit_account, credit_account, 
        amount, description, transaction_id
    ) VALUES (
        CONCAT('GL', DATE_FORMAT(NOW(), '%Y%m%d%H%i%s'), FLOOR(RAND() * 1000)),
        p_from_account, -- Debit Source Account
        p_to_account, -- Credit Destination Account
        p_amount, 
        CONCAT('Transfer: ', p_description),
        p_transaction_id
    );
    
    COMMIT;
    SET p_success = TRUE;
END //
DELIMITER ;

-- Get transaction history for an account
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_get_transaction_history(
    IN p_account_id VARCHAR(20),
    IN p_start_date TIMESTAMP,
    IN p_end_date TIMESTAMP,
    IN p_limit INT
)
BEGIN
    SELECT * FROM transactions 
    WHERE account_id = p_account_id
    AND transaction_date BETWEEN COALESCE(p_start_date, '1900-01-01') AND COALESCE(p_end_date, NOW())
    ORDER BY transaction_date DESC
    LIMIT p_limit;
END //
DELIMITER ;

-- Reverse a transaction
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_reverse_transaction(
    IN p_transaction_id VARCHAR(20),
    OUT p_reversed_id VARCHAR(20),
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE trx_account VARCHAR(20);
    DECLARE trx_type TINYINT;
    DECLARE trx_amount DECIMAL(15,2);
    DECLARE trx_status TINYINT;
    DECLARE account_status TINYINT;
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_success = FALSE;
        SET p_reversed_id = NULL;
        ROLLBACK;
    END;
    
    -- Get transaction details
    SELECT account_id, type, amount, status
    INTO trx_account, trx_type, trx_amount, trx_status
    FROM transactions
    WHERE transaction_id = p_transaction_id;
    
    -- Check if transaction exists and can be reversed
    IF trx_account IS NULL THEN
        SET p_success = FALSE;
        SET p_reversed_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Transaction not found';
    ELSEIF trx_status = 3 OR trx_status = 4 THEN
        SET p_success = FALSE;
        SET p_reversed_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Transaction already failed or reversed';
    END IF;
    
    -- Check account status
    SELECT status INTO account_status 
    FROM accounts WHERE account_id = trx_account;
    
    IF account_status IS NULL OR account_status != 1 THEN
        SET p_success = FALSE;
        SET p_reversed_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Account inactive or not found';
    END IF;
    
    START TRANSACTION;
    
    -- Generate reversal transaction ID
    SET p_reversed_id = CONCAT('REV', DATE_FORMAT(NOW(), '%Y%m%d%H%i%s'), FLOOR(RAND() * 1000));
    
    -- Mark original transaction as reversed
    UPDATE transactions SET
        status = 4 -- Status 4 = Reversed
    WHERE transaction_id = p_transaction_id;
    
    -- Create reversal transaction
    INSERT INTO transactions (
        transaction_id, account_id, type, 
        amount, description, reference_id, status
    ) VALUES (
        p_reversed_id, trx_account, 
        CASE 
            WHEN trx_type = 1 THEN 2 -- Deposit becomes Withdrawal
            WHEN trx_type = 2 THEN 1 -- Withdrawal becomes Deposit
            ELSE trx_type            -- Others remain same
        END,
        trx_amount, 
        CONCAT('Reversal of transaction: ', p_transaction_id),
        p_transaction_id, 
        2 -- Status 2 = Complete
    );
    
    -- Update account balance accordingly
    IF trx_type = 1 THEN
        -- Reverse deposit (subtract money)
        UPDATE accounts SET balance = balance - trx_amount WHERE account_id = trx_account;
    ELSEIF trx_type = 2 THEN
        -- Reverse withdrawal (add money back)
        UPDATE accounts SET balance = balance + trx_amount WHERE account_id = trx_account;
    END IF;
    
    -- Special handling for transfers would be needed in a full implementation
    
    -- Log in general ledger
    INSERT INTO general_ledger (
        entry_id, debit_account, credit_account, 
        amount, description, transaction_id
    ) VALUES (
        CONCAT('GL', DATE_FORMAT(NOW(), '%Y%m%d%H%i%s'), FLOOR(RAND() * 1000)),
        IF(trx_type = 1, trx_account, 'CASH'), -- If original was deposit, debit account
        IF(trx_type = 1, 'CASH', trx_account), -- If original was deposit, credit cash
        trx_amount, 
        CONCAT('Reversal of transaction: ', p_transaction_id),
        p_reversed_id
    );
    
    COMMIT;
    SET p_success = TRUE;
END //
DELIMITER ;

-- ===============================================================
-- Card Management Procedures
-- ===============================================================

-- Create new card
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_create_card(
    IN p_account_id VARCHAR(20),
    IN p_card_number VARCHAR(19),
    IN p_expiry_date DATE,
    IN p_cvv VARCHAR(3),
    IN p_card_type TINYINT,
    OUT p_card_id VARCHAR(20),
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE account_exists INT;
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_success = FALSE;
        SET p_card_id = NULL;
        ROLLBACK;
    END;
    
    -- Check if account exists
    SELECT COUNT(*) INTO account_exists FROM accounts WHERE account_id = p_account_id AND status = 1;
    
    IF account_exists = 0 THEN
        SET p_success = FALSE;
        SET p_card_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Account not found or inactive';
    END IF;
    
    -- Generate card ID
    SET p_card_id = CONCAT('CRD', DATE_FORMAT(NOW(), '%Y%m%d%H%i%s'), FLOOR(RAND() * 1000));
    
    -- Create card record
    INSERT INTO cards (
        card_id, account_id, card_number, 
        expiry_date, cvv, card_type, status
    ) VALUES (
        p_card_id, p_account_id, p_card_number, 
        p_expiry_date, p_cvv, p_card_type, 1 -- Status 1 = Active
    );
    
    SET p_success = TRUE;
END //
DELIMITER ;

-- Change card status
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_change_card_status(
    IN p_card_id VARCHAR(20),
    IN p_status TINYINT,
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE affected_rows INT;
    
    UPDATE cards SET
        status = p_status
    WHERE card_id = p_card_id;
    
    SET affected_rows = ROW_COUNT();
    SET p_success = IF(affected_rows > 0, TRUE, FALSE);
END //
DELIMITER ;

-- Get cards for account
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_get_account_cards(
    IN p_account_id VARCHAR(20)
)
BEGIN
    SELECT * FROM cards 
    WHERE account_id = p_account_id
    ORDER BY issue_date DESC;
END //
DELIMITER ;

-- ===============================================================
-- Beneficiary Management Procedures
-- ===============================================================

-- Add beneficiary
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_add_beneficiary(
    IN p_customer_id INT,
    IN p_account_id VARCHAR(20),
    IN p_beneficiary_name VARCHAR(100),
    IN p_beneficiary_account VARCHAR(20),
    IN p_beneficiary_bank VARCHAR(100),
    IN p_beneficiary_bank_code VARCHAR(20),
    OUT p_beneficiary_id INT,
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE account_exists INT;
    
    -- Check if account exists and belongs to customer
    SELECT COUNT(*) INTO account_exists 
    FROM accounts 
    WHERE account_id = p_account_id AND customer_id = p_customer_id;
    
    IF account_exists = 0 THEN
        SET p_success = FALSE;
        SET p_beneficiary_id = NULL;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Account not found or does not belong to customer';
    END IF;
    
    -- Add beneficiary
    INSERT INTO beneficiaries (
        customer_id, account_id, beneficiary_name,
        beneficiary_account, beneficiary_bank, beneficiary_bank_code
    ) VALUES (
        p_customer_id, p_account_id, p_beneficiary_name,
        p_beneficiary_account, p_beneficiary_bank, p_beneficiary_bank_code
    );
    
    SET p_beneficiary_id = LAST_INSERT_ID();
    SET p_success = TRUE;
END //
DELIMITER ;

-- Delete beneficiary
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_delete_beneficiary(
    IN p_beneficiary_id INT,
    IN p_customer_id INT,
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE affected_rows INT;
    
    -- Delete only if beneficiary belongs to the customer
    DELETE FROM beneficiaries 
    WHERE beneficiary_id = p_beneficiary_id 
    AND customer_id = p_customer_id;
    
    SET affected_rows = ROW_COUNT();
    SET p_success = IF(affected_rows > 0, TRUE, FALSE);
END //
DELIMITER ;

-- Get beneficiaries for customer
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_get_customer_beneficiaries(
    IN p_customer_id INT
)
BEGIN
    SELECT * FROM beneficiaries 
    WHERE customer_id = p_customer_id
    ORDER BY beneficiary_name;
END //
DELIMITER ;

-- ===============================================================
-- User Management Procedures
-- ===============================================================

-- Create system user
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_create_user(
    IN p_user_id VARCHAR(50),
    IN p_password VARCHAR(255),
    IN p_role VARCHAR(50),
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE user_exists INT;
    
    -- Check if user already exists
    SELECT COUNT(*) INTO user_exists FROM users WHERE user_id = p_user_id;
    
    IF user_exists > 0 THEN
        SET p_success = FALSE;
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'User ID already exists';
    END IF;
    
    -- Create user
    INSERT INTO users (
        user_id, password, role, status
    ) VALUES (
        p_user_id, p_password, p_role, 1 -- Status 1 = Active
    );
    
    SET p_success = TRUE;
END //
DELIMITER ;

-- Change user password
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_change_user_password(
    IN p_user_id VARCHAR(50),
    IN p_new_password VARCHAR(255),
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE affected_rows INT;
    
    UPDATE users SET
        password = p_new_password
    WHERE user_id = p_user_id;
    
    SET affected_rows = ROW_COUNT();
    SET p_success = IF(affected_rows > 0, TRUE, FALSE);
END //
DELIMITER ;

-- Change user status
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_change_user_status(
    IN p_user_id VARCHAR(50),
    IN p_status TINYINT,
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE affected_rows INT;
    
    -- Status: 1 = Active, 2 = Locked, 3 = Disabled
    UPDATE users SET
        status = p_status
    WHERE user_id = p_user_id;
    
    SET affected_rows = ROW_COUNT();
    SET p_success = IF(affected_rows > 0, TRUE, FALSE);
END //
DELIMITER ;

-- ===============================================================
-- Reporting Procedures
-- ===============================================================

-- Get account activity summary
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_account_activity_summary(
    IN p_account_id VARCHAR(20),
    IN p_start_date TIMESTAMP,
    IN p_end_date TIMESTAMP
)
BEGIN
    SELECT 
        COUNT(*) AS total_transactions,
        SUM(CASE WHEN type = 1 THEN amount ELSE 0 END) AS total_deposits,
        SUM(CASE WHEN type = 2 THEN amount ELSE 0 END) AS total_withdrawals,
        SUM(CASE WHEN type = 3 THEN amount ELSE 0 END) AS total_transfers
    FROM transactions 
    WHERE account_id = p_account_id
    AND transaction_date BETWEEN COALESCE(p_start_date, '1900-01-01') AND COALESCE(p_end_date, NOW());
END //
DELIMITER ;

-- Daily transaction summary
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_daily_transaction_summary(
    IN p_date DATE
)
BEGIN
    SELECT 
        DATE(transaction_date) AS transaction_day,
        COUNT(*) AS transaction_count,
        SUM(CASE WHEN type = 1 THEN amount ELSE 0 END) AS total_deposits,
        SUM(CASE WHEN type = 2 THEN amount ELSE 0 END) AS total_withdrawals,
        SUM(CASE WHEN type = 3 THEN amount ELSE 0 END) AS total_transfers
    FROM transactions 
    WHERE DATE(transaction_date) = p_date
    GROUP BY DATE(transaction_date);
END //
DELIMITER ;

-- Get dormant accounts report
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_get_dormant_accounts(
    IN p_days_inactive INT
)
BEGIN
    -- Find accounts with no transactions in the specified period
    SELECT a.*, c.first_name, c.last_name
    FROM accounts a
    JOIN customers c ON a.customer_id = c.customer_id
    WHERE a.status = 1 -- Only active accounts
    AND a.account_id NOT IN (
        SELECT DISTINCT account_id 
        FROM transactions 
        WHERE transaction_date > DATE_SUB(NOW(), INTERVAL p_days_inactive DAY)
    );
END //
DELIMITER ;

-- Generate custom statement
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_generate_account_statement(
    IN p_account_id VARCHAR(20),
    IN p_start_date TIMESTAMP,
    IN p_end_date TIMESTAMP
)
BEGIN
    DECLARE opening_balance DECIMAL(15,2);
    DECLARE closing_balance DECIMAL(15,2);
    DECLARE account_currency VARCHAR(3);
    DECLARE customer_name VARCHAR(101);
    
    -- Get customer information
    SELECT CONCAT(c.first_name, ' ', c.last_name), a.currency
    INTO customer_name, account_currency
    FROM accounts a
    JOIN customers c ON a.customer_id = c.customer_id
    WHERE a.account_id = p_account_id;
    
    -- Calculate opening balance
    -- In a real system, this would need to account for historical transactions
    SELECT balance - COALESCE(
        (SELECT SUM(
            CASE
                WHEN type = 1 THEN amount
                WHEN type = 2 THEN -amount
                WHEN type = 3 AND t.account_id = p_account_id AND t.transaction_id LIKE 'TRX%' THEN -amount
                WHEN type = 3 AND t.account_id = p_account_id AND t.transaction_id LIKE '%TO' THEN amount
                ELSE 0
            END
        )
        FROM transactions t
        WHERE t.account_id = p_account_id
        AND t.transaction_date BETWEEN p_start_date AND p_end_date
        AND t.status = 2), 0)
    INTO opening_balance
    FROM accounts
    WHERE account_id = p_account_id;
    
    -- Get current balance as closing balance
    SELECT balance INTO closing_balance
    FROM accounts
    WHERE account_id = p_account_id;
    
    -- Return statement header information
    SELECT 
        p_account_id AS account_id,
        customer_name AS customer_name,
        account_currency AS currency,
        p_start_date AS start_date,
        p_end_date AS end_date,
        opening_balance AS opening_balance,
        closing_balance AS closing_balance;
        
    -- Return transaction details
    SELECT 
        transaction_id,
        transaction_date,
        CASE
            WHEN type = 1 THEN 'Deposit'
            WHEN type = 2 THEN 'Withdrawal'
            WHEN type = 3 AND transaction_id LIKE 'TRX%' THEN 'Transfer Out'
            WHEN type = 3 AND transaction_id LIKE '%TO' THEN 'Transfer In'
            WHEN type = 4 THEN 'Payment'
            WHEN type = 5 THEN 'Fee'
            WHEN type = 6 THEN 'Interest'
            ELSE 'Other'
        END AS transaction_type,
        description,
        CASE
            WHEN type = 1 OR (type = 3 AND transaction_id LIKE '%TO') THEN amount
            ELSE NULL
        END AS credit,
        CASE
            WHEN type = 2 OR (type = 3 AND transaction_id LIKE 'TRX%') OR type = 4 OR type = 5 THEN amount
            ELSE NULL
        END AS debit,
        (SELECT SUM(
            CASE
                WHEN t2.type = 1 OR (t2.type = 3 AND t2.transaction_id LIKE '%TO') THEN t2.amount
                WHEN t2.type = 2 OR (t2.type = 3 AND t2.transaction_id LIKE 'TRX%') OR t2.type = 4 OR t2.type = 5 THEN -t2.amount
                ELSE 0
            END)
         FROM transactions t2
         WHERE t2.account_id = p_account_id
         AND t2.transaction_date <= t1.transaction_date
         AND t2.status = 2
        ) + opening_balance AS running_balance
    FROM transactions t1
    WHERE t1.account_id = p_account_id
    AND t1.transaction_date BETWEEN p_start_date AND p_end_date
    AND t1.status = 2
    ORDER BY t1.transaction_date ASC;
END //
DELIMITER ;

-- ===============================================================
-- Audit Logging Procedures
-- ===============================================================

-- Log audit event
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_log_audit_event(
    IN p_event_type TINYINT,
    IN p_user_id VARCHAR(50),
    IN p_description VARCHAR(255),
    IN p_ip_address VARCHAR(45),
    IN p_system_info VARCHAR(255),
    OUT p_event_id VARCHAR(20)
)
BEGIN
    -- Generate event ID
    SET p_event_id = CONCAT('EVT', DATE_FORMAT(NOW(), '%Y%m%d%H%i%s'), FLOOR(RAND() * 1000));
    
    -- Insert audit log
    INSERT INTO audit_trail (
        event_id, event_type, user_id, 
        description, ip_address, system_info
    ) VALUES (
        p_event_id, p_event_type, p_user_id, 
        p_description, p_ip_address, p_system_info
    );
END //
DELIMITER ;

-- Get audit events by user
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_get_user_audit_events(
    IN p_user_id VARCHAR(50),
    IN p_start_date TIMESTAMP,
    IN p_end_date TIMESTAMP,
    IN p_limit INT
)
BEGIN
    SELECT * FROM audit_trail 
    WHERE user_id = p_user_id
    AND event_time BETWEEN COALESCE(p_start_date, '1900-01-01') AND COALESCE(p_end_date, NOW())
    ORDER BY event_time DESC
    LIMIT p_limit;
END //
DELIMITER ;

-- ===============================================================
-- Transaction Limit Procedures
-- ===============================================================

-- Set transaction limits
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_set_transaction_limits(
    IN p_account_id VARCHAR(20),
    IN p_transaction_type TINYINT,
    IN p_daily_limit DECIMAL(15,2),
    IN p_single_limit DECIMAL(15,2),
    OUT p_success BOOLEAN
)
BEGIN
    DECLARE limit_exists INT;
    
    -- Check if limit already exists for this account and transaction type
    SELECT COUNT(*) INTO limit_exists 
    FROM transaction_limits 
    WHERE account_id = p_account_id AND transaction_type = p_transaction_type;
    
    IF limit_exists > 0 THEN
        -- Update existing limit
        UPDATE transaction_limits SET
            daily_limit = p_daily_limit,
            single_limit = p_single_limit
        WHERE account_id = p_account_id AND transaction_type = p_transaction_type;
    ELSE
        -- Create new limit
        INSERT INTO transaction_limits (
            account_id, transaction_type, daily_limit, single_limit
        ) VALUES (
            p_account_id, p_transaction_type, p_daily_limit, p_single_limit
        );
    END IF;
    
    SET p_success = TRUE;
END //
DELIMITER ;

-- Check transaction limit
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_check_transaction_limit(
    IN p_account_id VARCHAR(20),
    IN p_transaction_type TINYINT,
    IN p_amount DECIMAL(15,2),
    OUT p_within_limit BOOLEAN
)
BEGIN
    DECLARE single_limit DECIMAL(15,2);
    DECLARE daily_limit DECIMAL(15,2);
    DECLARE today_total DECIMAL(15,2);
    
    -- Get limits for this account and transaction type
    SELECT daily_limit, single_limit
    INTO daily_limit, single_limit
    FROM transaction_limits
    WHERE account_id = p_account_id AND transaction_type = p_transaction_type;
    
    IF single_limit IS NULL THEN
        -- No limits set, assume unlimited
        SET p_within_limit = TRUE;
    ELSE
        -- Check single transaction limit
        IF p_amount > single_limit THEN
            SET p_within_limit = FALSE;
        ELSE
            -- Check daily limit
            SELECT COALESCE(SUM(amount), 0)
            INTO today_total
            FROM transactions
            WHERE account_id = p_account_id
            AND type = p_transaction_type
            AND DATE(transaction_date) = CURDATE()
            AND status = 2; -- Completed transactions
            
            SET p_within_limit = (today_total + p_amount <= daily_limit);
        END IF;
    END IF;
END //
DELIMITER ;

-- ===============================================================
-- Utility Procedures
-- ===============================================================

-- Generate a new unique account ID
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_generate_account_id(
    IN p_account_type TINYINT,
    OUT p_account_id VARCHAR(20)
)
BEGIN
    DECLARE prefix VARCHAR(3);
    DECLARE new_id VARCHAR(20);
    DECLARE id_exists INT;
    
    -- Set prefix based on account type
    CASE p_account_type
        WHEN 1 THEN SET prefix = 'SAV';  -- Savings
        WHEN 2 THEN SET prefix = 'CHK';  -- Checking
        WHEN 3 THEN SET prefix = 'FIX';  -- Fixed Deposit
        WHEN 4 THEN SET prefix = 'LON';  -- Loan
        ELSE SET prefix = 'ACC';          -- Generic Account
    END CASE;
    
    -- Generate unique ID with retry logic
    SET id_exists = 1;
    WHILE id_exists > 0 DO
        SET new_id = CONCAT(prefix, DATE_FORMAT(NOW(), '%Y%m%d'), FLOOR(RAND() * 10000));
        SELECT COUNT(*) INTO id_exists FROM accounts WHERE account_id = new_id;
    END WHILE;
    
    SET p_account_id = new_id;
END //
DELIMITER ;

-- Calculate account interest
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS sp_calculate_account_interest(
    IN p_account_id VARCHAR(20),
    OUT p_interest_amount DECIMAL(15,2)
)
BEGIN
    DECLARE account_type TINYINT;
    DECLARE account_balance DECIMAL(15,2);
    DECLARE interest_rate DECIMAL(5,2);
    
    -- Get account details
    SELECT account_type, balance, interest_rate 
    INTO account_type, account_balance, interest_rate
    FROM accounts
    WHERE account_id = p_account_id;
    
    -- Only calculate interest for certain account types
    IF account_type = 1 OR account_type = 3 THEN -- Savings or Fixed Deposit
        -- Simple interest calculation (would be more complex in real system)
        SET p_interest_amount = (account_balance * interest_rate / 100) / 12; -- Monthly interest
    ELSE
        SET p_interest_amount = 0.00;
    END IF;
END //
DELIMITER ;

-- ===============================================================
-- End of Stored Procedures
-- ===============================================================