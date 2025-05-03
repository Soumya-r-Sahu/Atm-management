-- ATM Management System Fresh Database Schema
-- Generated: May 3, 2025

-- Drop database if exists and create new one
DROP DATABASE IF EXISTS atm_management;
CREATE DATABASE atm_management CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE atm_management;

-- Create customers table
CREATE TABLE customers (
    customer_id VARCHAR(20) PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    dob DATE NOT NULL,
    address VARCHAR(255) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    phone VARCHAR(20) NOT NULL,
    status ENUM('ACTIVE', 'INACTIVE', 'SUSPENDED', 'CLOSED') NOT NULL DEFAULT 'ACTIVE',
    registration_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    kyc_status ENUM('PENDING', 'COMPLETED', 'REJECTED') NOT NULL DEFAULT 'PENDING',
    last_updated DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_customer_status (status),
    INDEX idx_customer_email (email),
    INDEX idx_customer_phone (phone)
) ENGINE=InnoDB;

-- Create accounts table
CREATE TABLE accounts (
    account_number VARCHAR(20) PRIMARY KEY,
    customer_id VARCHAR(20) NOT NULL,
    account_type ENUM('SAVINGS', 'CURRENT', 'FIXED_DEPOSIT', 'LOAN') NOT NULL,
    branch_code VARCHAR(20) NOT NULL,
    ifsc_code VARCHAR(20) NOT NULL,
    opening_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    balance DECIMAL(12,2) NOT NULL DEFAULT 0.00,
    status ENUM('ACTIVE', 'DORMANT', 'FROZEN', 'CLOSED') NOT NULL DEFAULT 'ACTIVE',
    last_transaction DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (customer_id) REFERENCES customers(customer_id),
    INDEX idx_account_customer (customer_id),
    INDEX idx_account_status (status),
    INDEX idx_account_type (account_type)
) ENGINE=InnoDB;

-- Create cards table
CREATE TABLE cards (
    card_id VARCHAR(20) PRIMARY KEY,
    account_id VARCHAR(20) NOT NULL,
    card_number VARCHAR(20) NOT NULL UNIQUE,
    card_type ENUM('DEBIT', 'CREDIT', 'PREPAID') NOT NULL,
    expiry_date DATE NOT NULL,
    cvv VARCHAR(3) NOT NULL,
    pin_hash VARCHAR(128) NOT NULL,
    status ENUM('ACTIVE', 'INACTIVE', 'BLOCKED', 'EXPIRED') NOT NULL DEFAULT 'ACTIVE',
    issue_date DATE NOT NULL,
    daily_limit DECIMAL(10,2) NOT NULL DEFAULT 10000.00,
    FOREIGN KEY (account_id) REFERENCES accounts(account_number),
    INDEX idx_card_number (card_number),
    INDEX idx_card_status (status),
    INDEX idx_card_account (account_id)
) ENGINE=InnoDB;

-- Create transactions table
CREATE TABLE transactions (
    transaction_id VARCHAR(36) PRIMARY KEY,
    card_number VARCHAR(20),
    account_number VARCHAR(20) NOT NULL,
    transaction_type ENUM('WITHDRAWAL', 'DEPOSIT', 'TRANSFER', 'PAYMENT', 'BALANCE_INQUIRY', 'MINI_STATEMENT', 'PIN_CHANGE') NOT NULL,
    amount DECIMAL(12,2) NOT NULL DEFAULT 0.00,
    balance_before DECIMAL(12,2) NOT NULL,
    balance_after DECIMAL(12,2) NOT NULL,
    transaction_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    status ENUM('PENDING', 'SUCCESS', 'FAILED', 'REVERSED') NOT NULL DEFAULT 'PENDING',
    remarks VARCHAR(255),
    FOREIGN KEY (card_number) REFERENCES cards(card_number) ON DELETE SET NULL,
    FOREIGN KEY (account_number) REFERENCES accounts(account_number),
    INDEX idx_transaction_date (transaction_date),
    INDEX idx_transaction_card (card_number),
    INDEX idx_transaction_account (account_number),
    INDEX idx_transaction_type (transaction_type),
    INDEX idx_transaction_status (status)
) ENGINE=InnoDB;

-- Create daily withdrawals tracking table
CREATE TABLE daily_withdrawals (
    id INT AUTO_INCREMENT PRIMARY KEY,
    card_number VARCHAR(20) NOT NULL,
    amount DECIMAL(10,2) NOT NULL,
    withdrawal_date DATE NOT NULL,
    withdrawal_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (card_number) REFERENCES cards(card_number),
    INDEX idx_withdrawal_card_date (card_number, withdrawal_date)
) ENGINE=InnoDB;

-- Create bill payments table
CREATE TABLE bill_payments (
    payment_id VARCHAR(36) PRIMARY KEY,
    transaction_id VARCHAR(36) NOT NULL,
    biller_id VARCHAR(50) NOT NULL,
    biller_name VARCHAR(100) NOT NULL,
    consumer_id VARCHAR(50) NOT NULL,
    bill_amount DECIMAL(12,2) NOT NULL,
    payment_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    status ENUM('PENDING', 'SUCCESS', 'FAILED') NOT NULL DEFAULT 'PENDING',
    FOREIGN KEY (transaction_id) REFERENCES transactions(transaction_id),
    INDEX idx_payment_biller (biller_id),
    INDEX idx_payment_consumer (consumer_id),
    INDEX idx_payment_date (payment_date)
) ENGINE=InnoDB;

-- Create transfer table for inter-account transfers
CREATE TABLE transfers (
    transfer_id VARCHAR(36) PRIMARY KEY,
    transaction_id VARCHAR(36) NOT NULL,
    source_account VARCHAR(20) NOT NULL,
    destination_account VARCHAR(20) NOT NULL,
    amount DECIMAL(12,2) NOT NULL,
    transfer_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    status ENUM('PENDING', 'SUCCESS', 'FAILED', 'REVERSED') NOT NULL DEFAULT 'PENDING',
    remarks VARCHAR(255),
    FOREIGN KEY (transaction_id) REFERENCES transactions(transaction_id),
    FOREIGN KEY (source_account) REFERENCES accounts(account_number),
    FOREIGN KEY (destination_account) REFERENCES accounts(account_number),
    INDEX idx_transfer_source (source_account),
    INDEX idx_transfer_dest (destination_account),
    INDEX idx_transfer_date (transfer_date)
) ENGINE=InnoDB;

-- Create audit logs table
CREATE TABLE audit_logs (
    log_id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    user_id VARCHAR(50),
    action ENUM('LOGIN', 'LOGOUT', 'CREATE', 'UPDATE', 'DELETE', 
                'PIN_CHANGE', 'CARD_BLOCK', 'CARD_UNBLOCK', 'CARD_ISSUE') NOT NULL,
    entity_type VARCHAR(50) NOT NULL,
    entity_id VARCHAR(50) NOT NULL,
    details TEXT,
    ip_address VARCHAR(50),
    user_agent VARCHAR(255),
    INDEX idx_audit_timestamp (timestamp),
    INDEX idx_audit_user (user_id),
    INDEX idx_audit_action (action),
    INDEX idx_audit_entity (entity_type, entity_id)
) ENGINE=InnoDB;

-- Create system logs table
CREATE TABLE system_logs (
    log_id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    log_level ENUM('INFO', 'WARNING', 'ERROR', 'CRITICAL') NOT NULL,
    component VARCHAR(50) NOT NULL,
    message TEXT NOT NULL,
    details TEXT,
    INDEX idx_log_timestamp (timestamp),
    INDEX idx_log_level (log_level),
    INDEX idx_log_component (component)
) ENGINE=InnoDB;

-- Create admin users table
CREATE TABLE admin_users (
    admin_id VARCHAR(20) PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(128) NOT NULL,
    full_name VARCHAR(100),
    email VARCHAR(100),
    role ENUM('CLERK', 'OFFICER', 'MANAGER', 'SUPER_ADMIN') NOT NULL,
    status ENUM('ACTIVE', 'INACTIVE', 'SUSPENDED') NOT NULL DEFAULT 'ACTIVE',
    last_login DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_admin_username (username),
    INDEX idx_admin_role (role),
    INDEX idx_admin_status (status)
) ENGINE=InnoDB;

-- Create ATM machines table
CREATE TABLE atm_machines (
    atm_id VARCHAR(20) PRIMARY KEY,
    location VARCHAR(255) NOT NULL,
    branch_code VARCHAR(20) NOT NULL,
    status ENUM('ONLINE', 'OFFLINE', 'MAINTENANCE') NOT NULL DEFAULT 'ONLINE',
    cash_balance DECIMAL(12,2) NOT NULL DEFAULT 0.00,
    last_maintenance DATETIME,
    last_cash_refill DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_atm_status (status),
    INDEX idx_atm_location (location),
    INDEX idx_atm_branch (branch_code)
) ENGINE=InnoDB;

-- Create cash transactions for ATM loading/unloading
CREATE TABLE atm_cash_operations (
    operation_id VARCHAR(36) PRIMARY KEY,
    atm_id VARCHAR(20) NOT NULL,
    operation_type ENUM('LOAD', 'UNLOAD') NOT NULL,
    amount DECIMAL(12,2) NOT NULL,
    performed_by VARCHAR(20) NOT NULL,
    verified_by VARCHAR(20),
    balance_before DECIMAL(12,2) NOT NULL,
    balance_after DECIMAL(12,2) NOT NULL,
    operation_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    notes TEXT,
    FOREIGN KEY (atm_id) REFERENCES atm_machines(atm_id),
    FOREIGN KEY (performed_by) REFERENCES admin_users(admin_id),
    FOREIGN KEY (verified_by) REFERENCES admin_users(admin_id),
    INDEX idx_operation_atm (atm_id),
    INDEX idx_operation_date (operation_date)
) ENGINE=InnoDB;

-- Create app sessions table
CREATE TABLE sessions (
    session_id VARCHAR(64) PRIMARY KEY,
    user_id VARCHAR(50) NOT NULL,
    user_type ENUM('CUSTOMER', 'ADMIN') NOT NULL,
    login_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    expiry_time DATETIME NOT NULL,
    ip_address VARCHAR(50),
    device_info VARCHAR(255),
    status ENUM('ACTIVE', 'EXPIRED', 'TERMINATED') NOT NULL DEFAULT 'ACTIVE',
    INDEX idx_session_user (user_id, user_type),
    INDEX idx_session_status (status),
    INDEX idx_session_expiry (expiry_time)
) ENGINE=InnoDB;

-- Create database version control table
CREATE TABLE db_version (
    version_id INT AUTO_INCREMENT PRIMARY KEY,
    version_number VARCHAR(20) NOT NULL,
    applied_on DATETIME DEFAULT CURRENT_TIMESTAMP,
    description VARCHAR(255),
    script_name VARCHAR(255),
    applied_by VARCHAR(50)
) ENGINE=InnoDB;

-- Insert initial version record
INSERT INTO db_version (version_number, description, script_name, applied_by)
VALUES ('1.0.0', 'Initial schema creation', 'fresh_schema.sql', 'System');

-- Create stored procedure to check daily withdrawal limits
DELIMITER //
CREATE PROCEDURE check_daily_withdrawal_limit(
    IN p_card_number VARCHAR(20),
    IN p_amount DECIMAL(10,2),
    OUT p_can_withdraw BOOLEAN,
    OUT p_remaining_limit DECIMAL(10,2)
)
BEGIN
    DECLARE v_total_today DECIMAL(10,2);
    DECLARE v_daily_limit DECIMAL(10,2);
    DECLARE v_today DATE;
    
    SET v_today = CURDATE();
    
    -- Get card's daily limit
    SELECT daily_limit INTO v_daily_limit
    FROM cards
    WHERE card_number = p_card_number;
    
    -- Get total withdrawals today
    SELECT COALESCE(SUM(amount), 0) INTO v_total_today
    FROM daily_withdrawals
    WHERE card_number = p_card_number 
    AND withdrawal_date = v_today;
    
    -- Calculate remaining limit
    SET p_remaining_limit = v_daily_limit - v_total_today;
    
    -- Check if withdrawal is allowed
    IF (v_total_today + p_amount) <= v_daily_limit THEN
        SET p_can_withdraw = TRUE;
    ELSE
        SET p_can_withdraw = FALSE;
    END IF;
END//
DELIMITER ;

-- Create triggers for additional integrity and auditing
DELIMITER //

-- After update trigger on accounts to log balance changes
CREATE TRIGGER accounts_after_update
AFTER UPDATE ON accounts
FOR EACH ROW
BEGIN
    IF OLD.balance != NEW.balance THEN
        INSERT INTO system_logs (log_level, component, message, details)
        VALUES ('INFO', 'DATABASE', 
                CONCAT('Balance updated for account ', NEW.account_number),
                CONCAT('Old balance: ', OLD.balance, ', New balance: ', NEW.balance));
    END IF;
END//

-- Before update trigger on cards to validate status changes
CREATE TRIGGER cards_before_update
BEFORE UPDATE ON cards
FOR EACH ROW
BEGIN
    IF NEW.status = 'EXPIRED' AND NEW.expiry_date > CURDATE() THEN
        SIGNAL SQLSTATE '45000' 
        SET MESSAGE_TEXT = 'Cannot set card status to EXPIRED before the expiry date';
    END IF;
END//

DELIMITER ;

-- Grant privileges (modify as needed for production)
GRANT ALL PRIVILEGES ON atm_management.* TO 'atm_app'@'localhost' IDENTIFIED BY 'secure_password';
FLUSH PRIVILEGES;