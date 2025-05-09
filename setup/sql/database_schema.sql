-- Core Banking System Database Schema
-- Generated: May 9, 2025
-- Modified for phpMyAdmin compatibility

-- Note: In phpMyAdmin, you may need to select the database after importing or run this script in parts
-- You can create the database through phpMyAdmin interface or uncomment these lines:
-- DROP DATABASE IF EXISTS core_banking_system;
-- CREATE DATABASE core_banking_system CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
-- USE core_banking_system;

-- For phpMyAdmin: If you've already created the database through the interface,
-- make sure it's selected before running this script

-- Create customers table
CREATE TABLE cbs_customers (
    customer_id VARCHAR(20) PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    dob DATE NOT NULL,
    address VARCHAR(255) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    phone VARCHAR(20) NOT NULL,
    status ENUM('ACTIVE', 'INACTIVE', 'SUSPENDED', 'CLOSED') NOT NULL DEFAULT 'ACTIVE',
    registration_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    kyc_status ENUM('PENDING', 'COMPLETED', 'REJECTED') NOT NULL DEFAULT 'PENDING',
    pan_number VARCHAR(10),
    aadhar_number VARCHAR(12),
    customer_segment ENUM('RETAIL', 'CORPORATE', 'PRIORITY', 'NRI') NOT NULL DEFAULT 'RETAIL',
    last_updated DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_customer_status (status),
    INDEX idx_customer_email (email),
    INDEX idx_customer_phone (phone),
    INDEX idx_customer_segment (customer_segment)
) ENGINE=InnoDB;

-- Create accounts table
CREATE TABLE cbs_accounts (
    account_number VARCHAR(20) PRIMARY KEY,
    customer_id VARCHAR(20) NOT NULL,
    account_type ENUM('SAVINGS', 'CURRENT', 'FIXED_DEPOSIT', 'RECURRING_DEPOSIT', 'LOAN', 'SALARY', 'NRI') NOT NULL,
    branch_code VARCHAR(20) NOT NULL,
    ifsc_code VARCHAR(20) NOT NULL,
    opening_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    balance DECIMAL(12,2) NOT NULL DEFAULT 0.00,
    interest_rate DECIMAL(5,2),
    status ENUM('ACTIVE', 'DORMANT', 'FROZEN', 'CLOSED') NOT NULL DEFAULT 'ACTIVE',
    last_transaction DATETIME DEFAULT CURRENT_TIMESTAMP,
    nominee_name VARCHAR(100),
    nominee_relation VARCHAR(50),
    service_charges_applicable BOOLEAN DEFAULT TRUE,
    FOREIGN KEY (customer_id) REFERENCES cbs_customers(customer_id),
    INDEX idx_account_customer (customer_id),
    INDEX idx_account_status (status),
    INDEX idx_account_type (account_type),
    INDEX idx_branch_code (branch_code)
) ENGINE=InnoDB;

-- Create cards table
CREATE TABLE cbs_cards (
    card_id VARCHAR(20) PRIMARY KEY,
    account_id VARCHAR(20) NOT NULL,
    card_number VARCHAR(20) NOT NULL UNIQUE,
    card_type ENUM('DEBIT', 'CREDIT', 'PREPAID', 'INTERNATIONAL', 'VIRTUAL', 'CORPORATE') NOT NULL,
    card_network ENUM('VISA', 'MASTERCARD', 'RUPAY', 'AMEX', 'DINERS') NOT NULL,
    expiry_date DATE NOT NULL,
    cvv VARCHAR(3) NOT NULL,
    pin_hash VARCHAR(128) NOT NULL,
    status ENUM('ACTIVE', 'INACTIVE', 'BLOCKED', 'EXPIRED', 'HOTLISTED') NOT NULL DEFAULT 'ACTIVE',
    issue_date DATE NOT NULL,
    daily_atm_limit DECIMAL(10,2) NOT NULL DEFAULT 10000.00,
    daily_pos_limit DECIMAL(10,2) NOT NULL DEFAULT 50000.00,
    daily_online_limit DECIMAL(10,2) NOT NULL DEFAULT 30000.00,
    primary_user_name VARCHAR(100) NOT NULL,
    international_usage_enabled BOOLEAN DEFAULT FALSE,
    contactless_enabled BOOLEAN DEFAULT TRUE,
    FOREIGN KEY (account_id) REFERENCES cbs_accounts(account_number),
    INDEX idx_card_number (card_number),
    INDEX idx_card_status (status),
    INDEX idx_card_account (account_id)
) ENGINE=InnoDB;

-- Create transactions table
CREATE TABLE cbs_transactions (
    transaction_id VARCHAR(36) PRIMARY KEY,
    card_number VARCHAR(20),
    account_number VARCHAR(20) NOT NULL,
    transaction_type ENUM('WITHDRAWAL', 'DEPOSIT', 'TRANSFER', 'PAYMENT', 'BALANCE_INQUIRY', 'MINI_STATEMENT', 'PIN_CHANGE', 'CHEQUE_DEPOSIT', 'INTEREST_CREDIT', 'FEE_DEBIT', 'REVERSAL') NOT NULL,
    channel ENUM('ATM', 'BRANCH', 'INTERNET', 'MOBILE', 'POS', 'UPI', 'IMPS', 'NEFT', 'RTGS') NOT NULL,
    amount DECIMAL(12,2) NOT NULL DEFAULT 0.00,
    balance_before DECIMAL(12,2) NOT NULL,
    balance_after DECIMAL(12,2) NOT NULL,
    transaction_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    value_date DATE NOT NULL,
    status ENUM('PENDING', 'SUCCESS', 'FAILED', 'REVERSED', 'DISPUTED') NOT NULL DEFAULT 'PENDING',
    reference_number VARCHAR(50),
    remarks VARCHAR(255),
    transaction_location VARCHAR(255),
    merchant_category_code VARCHAR(4),
    merchant_name VARCHAR(100),
    sender_details VARCHAR(255),
    receiver_details VARCHAR(255),
    FOREIGN KEY (card_number) REFERENCES cbs_cards(card_number) ON DELETE SET NULL,
    FOREIGN KEY (account_number) REFERENCES cbs_accounts(account_number),
    INDEX idx_transaction_date (transaction_date),
    INDEX idx_transaction_card (card_number),
    INDEX idx_transaction_account (account_number),
    INDEX idx_transaction_type (transaction_type),
    INDEX idx_transaction_channel (channel),
    INDEX idx_transaction_status (status),
    INDEX idx_transaction_reference (reference_number)
) ENGINE=InnoDB;

-- Create daily withdrawals tracking table
CREATE TABLE cbs_daily_withdrawals (
    id INT AUTO_INCREMENT PRIMARY KEY,
    card_number VARCHAR(20) NOT NULL,
    amount DECIMAL(10,2) NOT NULL,
    withdrawal_date DATE NOT NULL,
    withdrawal_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    atm_id VARCHAR(20),
    location VARCHAR(255),
    status ENUM('COMPLETED', 'FAILED', 'REVERSED') NOT NULL DEFAULT 'COMPLETED',
    FOREIGN KEY (card_number) REFERENCES cbs_cards(card_number),
    INDEX idx_withdrawal_card_date (card_number, withdrawal_date),
    INDEX idx_withdrawal_status (status)
) ENGINE=InnoDB;

-- Create bill payments table
CREATE TABLE cbs_bill_payments (
    payment_id VARCHAR(36) PRIMARY KEY,
    transaction_id VARCHAR(36) NOT NULL,
    customer_id VARCHAR(20) NOT NULL,
    biller_id VARCHAR(50) NOT NULL,
    biller_name VARCHAR(100) NOT NULL,
    biller_category ENUM('ELECTRICITY', 'WATER', 'GAS', 'TELEPHONE', 'MOBILE', 'INTERNET', 'INSURANCE', 'TAX', 'EDUCATION', 'OTHER') NOT NULL,
    consumer_id VARCHAR(50) NOT NULL,
    bill_amount DECIMAL(12,2) NOT NULL,
    due_date DATE,
    payment_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    payment_channel ENUM('ATM', 'NET_BANKING', 'MOBILE_APP', 'BRANCH', 'AUTO_DEBIT') NOT NULL,
    status ENUM('PENDING', 'SUCCESS', 'FAILED', 'REVERSED') NOT NULL DEFAULT 'PENDING',
    receipt_number VARCHAR(50),
    FOREIGN KEY (transaction_id) REFERENCES cbs_transactions(transaction_id),
    FOREIGN KEY (customer_id) REFERENCES cbs_customers(customer_id),
    INDEX idx_payment_biller (biller_id),
    INDEX idx_payment_consumer (consumer_id),
    INDEX idx_payment_date (payment_date),
    INDEX idx_payment_customer (customer_id)
) ENGINE=InnoDB;

-- Create transfer table for inter-account transfers
CREATE TABLE cbs_transfers (
    transfer_id VARCHAR(36) PRIMARY KEY,
    transaction_id VARCHAR(36) NOT NULL,
    source_account VARCHAR(20) NOT NULL,
    destination_account VARCHAR(20) NOT NULL,
    beneficiary_name VARCHAR(100),
    beneficiary_bank VARCHAR(100),
    beneficiary_ifsc VARCHAR(20),
    transfer_type ENUM('INTERNAL', 'NEFT', 'RTGS', 'IMPS', 'UPI', 'SWIFT') NOT NULL,
    amount DECIMAL(12,2) NOT NULL,
    transfer_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    processing_date DATETIME,
    status ENUM('INITIATED', 'PROCESSING', 'SUCCESS', 'FAILED', 'REVERSED', 'RETURNED') NOT NULL DEFAULT 'INITIATED',
    reference_number VARCHAR(50),
    purpose_code VARCHAR(4),
    remarks VARCHAR(255),
    charges DECIMAL(10,2) DEFAULT 0.00,
    scheduled_transfer BOOLEAN DEFAULT FALSE,
    recurring_transfer BOOLEAN DEFAULT FALSE,
    frequency ENUM('DAILY', 'WEEKLY', 'MONTHLY', 'QUARTERLY', 'YEARLY') DEFAULT NULL,
    FOREIGN KEY (transaction_id) REFERENCES cbs_transactions(transaction_id),
    FOREIGN KEY (source_account) REFERENCES cbs_accounts(account_number),
    INDEX idx_transfer_source (source_account),
    INDEX idx_transfer_dest (destination_account),
    INDEX idx_transfer_date (transfer_date),
    INDEX idx_transfer_type (transfer_type),
    INDEX idx_transfer_status (status)
) ENGINE=InnoDB;

-- Create audit logs table
-- Note: If your phpMyAdmin MySQL version doesn't support partitioning, you can remove the PARTITION clause
CREATE TABLE cbs_audit_logs (
    log_id BIGINT NOT NULL AUTO_INCREMENT,
    timestamp DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    user_id VARCHAR(50),
    action ENUM('LOGIN', 'LOGOUT', 'CREATE', 'UPDATE', 'DELETE', 'VIEW',
                'PIN_CHANGE', 'PASSWORD_CHANGE', 'CARD_BLOCK', 'CARD_UNBLOCK', 'CARD_ISSUE',
                'ACCOUNT_FREEZE', 'ACCOUNT_UNFREEZE', 'LIMIT_CHANGE', 'KYC_UPDATE',
                'BENEFICIARY_ADD', 'BENEFICIARY_DELETE', 'PERMISSION_CHANGE') NOT NULL,
    entity_type VARCHAR(50) NOT NULL,
    entity_id VARCHAR(50) NOT NULL,
    previous_value TEXT,
    new_value TEXT,
    details TEXT,
    ip_address VARCHAR(50),
    user_agent VARCHAR(255),
    channel VARCHAR(50),
    session_id VARCHAR(64),
    status ENUM('SUCCESS', 'FAILURE', 'WARNING') NOT NULL DEFAULT 'SUCCESS',
    PRIMARY KEY (log_id, timestamp),
    INDEX idx_audit_timestamp (timestamp),
    INDEX idx_audit_user (user_id),
    INDEX idx_audit_action (action),
    INDEX idx_audit_entity (entity_type, entity_id),
    INDEX idx_audit_status (status)
) ENGINE=InnoDB
/* 
If your phpMyAdmin's MySQL version supports partitioning, uncomment this section:
PARTITION BY RANGE (TO_DAYS(timestamp)) (
    PARTITION p_history VALUES LESS THAN (TO_DAYS('2025-01-01')),
    PARTITION p_current VALUES LESS THAN (TO_DAYS('2025-12-31')),
    PARTITION p_future VALUES LESS THAN MAXVALUE
)
*/;

-- Create system logs table
-- Note: If your phpMyAdmin MySQL version doesn't support partitioning, you can remove the PARTITION clause
CREATE TABLE cbs_system_logs (
    log_id BIGINT NOT NULL AUTO_INCREMENT,
    timestamp DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    log_level ENUM('DEBUG', 'INFO', 'NOTICE', 'WARNING', 'ERROR', 'CRITICAL', 'ALERT', 'EMERGENCY') NOT NULL,
    component VARCHAR(50) NOT NULL,
    module VARCHAR(50) NOT NULL,
    message TEXT NOT NULL,
    details TEXT,
    stack_trace TEXT,
    request_id VARCHAR(64),
    server_name VARCHAR(100),
    severity INT NOT NULL DEFAULT 1,
    PRIMARY KEY (log_id, timestamp),
    INDEX idx_log_timestamp (timestamp),
    INDEX idx_log_level (log_level),
    INDEX idx_log_component (component),
    INDEX idx_log_module (module),
    INDEX idx_log_severity (severity)
) ENGINE=InnoDB 
/* 
If your phpMyAdmin's MySQL version supports partitioning, uncomment this section:
PARTITION BY RANGE (TO_DAYS(timestamp)) (
    PARTITION p_history VALUES LESS THAN (TO_DAYS('2025-01-01')),
    PARTITION p_current VALUES LESS THAN (TO_DAYS('2025-12-31')),
    PARTITION p_future VALUES LESS THAN MAXVALUE
)
*/;

-- Create admin users table
CREATE TABLE cbs_admin_users (
    admin_id VARCHAR(20) PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(128) NOT NULL,
    full_name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL,
    mobile VARCHAR(20) NOT NULL,
    department VARCHAR(50) NOT NULL,
    branch_code VARCHAR(20) NOT NULL,
    employee_id VARCHAR(20) NOT NULL UNIQUE,
    role ENUM('TELLER', 'CLERK', 'OFFICER', 'MANAGER', 'REGIONAL_MANAGER', 'AUDITOR', 'IT_ADMIN', 'SUPER_ADMIN') NOT NULL,
    status ENUM('ACTIVE', 'INACTIVE', 'SUSPENDED', 'LOCKED', 'TERMINATED') NOT NULL DEFAULT 'ACTIVE',
    password_expiry_date DATE NOT NULL,
    account_locked BOOLEAN DEFAULT FALSE,
    failed_login_attempts INT DEFAULT 0,
    last_login DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    created_by VARCHAR(20),
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    updated_by VARCHAR(20),
    requires_2fa BOOLEAN DEFAULT TRUE,
    INDEX idx_admin_username (username),
    INDEX idx_admin_role (role),
    INDEX idx_admin_status (status),
    INDEX idx_admin_branch (branch_code),
    INDEX idx_admin_dept (department)
) ENGINE=InnoDB;

-- Create ATM machines table
CREATE TABLE cbs_atm_machines (
    atm_id VARCHAR(20) PRIMARY KEY,
    atm_type ENUM('ONSITE', 'OFFSITE', 'DRIVE_THROUGH', 'MOBILE', 'KIOSK') NOT NULL DEFAULT 'ONSITE',
    location VARCHAR(255) NOT NULL,
    address VARCHAR(255) NOT NULL,
    city VARCHAR(100) NOT NULL,
    state VARCHAR(100) NOT NULL,
    pin_code VARCHAR(10) NOT NULL,
    geo_coordinates VARCHAR(50),
    branch_code VARCHAR(20) NOT NULL,
    manufacturer VARCHAR(100),
    model VARCHAR(100),
    serial_number VARCHAR(100) NOT NULL UNIQUE,
    ip_address VARCHAR(15),
    status ENUM('ONLINE', 'OFFLINE', 'MAINTENANCE', 'OUT_OF_CASH', 'PARTIAL_SERVICE') NOT NULL DEFAULT 'ONLINE',
    cash_balance DECIMAL(12,2) NOT NULL DEFAULT 0.00,
    cash_limit DECIMAL(12,2) NOT NULL DEFAULT 1000000.00,
    last_maintenance DATETIME,
    next_maintenance_due DATETIME,
    last_cash_refill DATETIME,
    installation_date DATE NOT NULL,
    is_accessible BOOLEAN DEFAULT TRUE,
    supports_deposit BOOLEAN DEFAULT FALSE,
    supports_cardless BOOLEAN DEFAULT FALSE,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_atm_status (status),
    INDEX idx_atm_location (location),
    INDEX idx_atm_branch (branch_code),
    INDEX idx_atm_city (city),
    INDEX idx_atm_state (state),
    INDEX idx_atm_pincode (pin_code)
) ENGINE=InnoDB;

-- Create cash transactions for ATM loading/unloading
CREATE TABLE cbs_atm_cash_operations (
    operation_id VARCHAR(36) PRIMARY KEY,
    atm_id VARCHAR(20) NOT NULL,
    operation_type ENUM('LOAD', 'UNLOAD', 'AUDIT', 'MAINTENANCE') NOT NULL,
    denomination_2000 INT DEFAULT 0,
    denomination_500 INT DEFAULT 0,
    denomination_200 INT DEFAULT 0,
    denomination_100 INT DEFAULT 0,
    amount DECIMAL(12,2) NOT NULL,
    performed_by VARCHAR(20) NOT NULL,
    verified_by VARCHAR(20),
    custodian VARCHAR(100),
    balance_before DECIMAL(12,2) NOT NULL,
    balance_after DECIMAL(12,2) NOT NULL,
    operation_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    completion_date DATETIME,
    discrepancy_amount DECIMAL(10,2) DEFAULT 0.00,
    status ENUM('INITIATED', 'IN_PROGRESS', 'COMPLETED', 'FAILED', 'CANCELLED') NOT NULL DEFAULT 'INITIATED',
    notes TEXT,
    FOREIGN KEY (atm_id) REFERENCES cbs_atm_machines(atm_id),
    FOREIGN KEY (performed_by) REFERENCES cbs_admin_users(admin_id),
    FOREIGN KEY (verified_by) REFERENCES cbs_admin_users(admin_id),
    INDEX idx_operation_atm (atm_id),
    INDEX idx_operation_date (operation_date),
    INDEX idx_operation_status (status)
) ENGINE=InnoDB;

-- Create app sessions table
CREATE TABLE cbs_sessions (
    session_id VARCHAR(64) PRIMARY KEY,
    user_id VARCHAR(50) NOT NULL,
    user_type ENUM('CUSTOMER', 'ADMIN', 'API') NOT NULL,
    login_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    expiry_time DATETIME NOT NULL,
    last_activity_time DATETIME NOT NULL,
    ip_address VARCHAR(50),
    device_info VARCHAR(255),
    browser VARCHAR(100),
    operating_system VARCHAR(100),
    location VARCHAR(255),
    user_agent VARCHAR(512),
    auth_method ENUM('PASSWORD', 'OTP', 'BIOMETRIC', '2FA', 'SSO') NOT NULL,
    mfa_verified BOOLEAN DEFAULT FALSE,
    status ENUM('ACTIVE', 'EXPIRED', 'TERMINATED', 'SUSPICIOUS') NOT NULL DEFAULT 'ACTIVE',
    reason_for_termination VARCHAR(255),
    INDEX idx_session_user (user_id, user_type),
    INDEX idx_session_status (status),
    INDEX idx_session_expiry (expiry_time),
    INDEX idx_session_last_activity (last_activity_time)
) ENGINE=InnoDB;

-- Create database version control table
CREATE TABLE cbs_db_version (
    version_id INT AUTO_INCREMENT PRIMARY KEY,
    version_number VARCHAR(20) NOT NULL,
    applied_on DATETIME DEFAULT CURRENT_TIMESTAMP,
    approved_by VARCHAR(50),
    applied_by VARCHAR(50),
    description VARCHAR(255),
    script_name VARCHAR(255),
    checksum VARCHAR(64),
    execution_time INT,
    status ENUM('SUCCESS', 'FAILED', 'REVERTED') NOT NULL DEFAULT 'SUCCESS'
) ENGINE=InnoDB;

-- Insert initial version record
INSERT INTO cbs_db_version (version_number, description, script_name, applied_by, checksum)
VALUES ('1.0.0', 'Initial core banking system schema creation', 'fresh_schema.sql', 'System', MD5('Initial core banking system schema creation'));

-- phpMyAdmin note: If you're running this script in sections, make sure all tables are created before this point

-- Create stored procedure to check daily withdrawal limits
-- If you encounter issues with the DELIMITER in phpMyAdmin, you can create this procedure using the phpMyAdmin interface later
DELIMITER //
CREATE PROCEDURE check_daily_withdrawal_limit(
    IN p_card_number VARCHAR(20),
    IN p_amount DECIMAL(10,2),
    IN p_channel ENUM('ATM', 'POS', 'ONLINE') DEFAULT 'ATM',
    OUT p_can_withdraw BOOLEAN,
    OUT p_remaining_limit DECIMAL(10,2),
    OUT p_message VARCHAR(255)
)
BEGIN
    DECLARE v_total_today DECIMAL(10,2);
    DECLARE v_daily_limit DECIMAL(10,2);
    DECLARE v_today DATE;
    DECLARE v_card_status VARCHAR(20);
    DECLARE v_account_status VARCHAR(20);
    DECLARE v_account_number VARCHAR(20);
    
    SET v_today = CURDATE();
    
    -- Check card status
    SELECT c.status, c.account_id, a.status 
    INTO v_card_status, v_account_number, v_account_status
    FROM cbs_cards c
    JOIN cbs_accounts a ON c.account_id = a.account_number
    WHERE c.card_number = p_card_number;
    
    -- Validate card and account status
    IF v_card_status IS NULL THEN
        SET p_can_withdraw = FALSE;
        SET p_remaining_limit = 0;
        SET p_message = 'Invalid card number';
        RETURN;
    ELSEIF v_card_status != 'ACTIVE' THEN
        SET p_can_withdraw = FALSE;
        SET p_remaining_limit = 0;
        SET p_message = CONCAT('Card is ', v_card_status);
        RETURN;
    ELSEIF v_account_status != 'ACTIVE' THEN
        SET p_can_withdraw = FALSE;
        SET p_remaining_limit = 0;
        SET p_message = CONCAT('Account is ', v_account_status);
        RETURN;
    END IF;
    
    -- Get card's daily limit based on channel
    IF p_channel = 'ATM' THEN
        SELECT daily_atm_limit INTO v_daily_limit
        FROM cbs_cards
        WHERE card_number = p_card_number;
    ELSEIF p_channel = 'POS' THEN
        SELECT daily_pos_limit INTO v_daily_limit
        FROM cbs_cards
        WHERE card_number = p_card_number;
    ELSE
        SELECT daily_online_limit INTO v_daily_limit
        FROM cbs_cards
        WHERE card_number = p_card_number;
    END IF;
    
    -- Get total withdrawals today
    SELECT COALESCE(SUM(amount), 0) INTO v_total_today
    FROM cbs_daily_withdrawals
    WHERE card_number = p_card_number 
    AND withdrawal_date = v_today;
    
    -- Calculate remaining limit
    SET p_remaining_limit = v_daily_limit - v_total_today;
    
    -- Check if withdrawal is allowed
    IF (v_total_today + p_amount) <= v_daily_limit THEN
        SET p_can_withdraw = TRUE;
        SET p_message = 'Withdrawal allowed';
    ELSE
        SET p_can_withdraw = FALSE;
        SET p_message = CONCAT('Exceeds daily limit. Remaining limit: ', p_remaining_limit);
    END IF;
END//
DELIMITER ;

-- phpMyAdmin note: In phpMyAdmin you may need to execute each trigger separately
-- If you encounter issues with these triggers in phpMyAdmin, you can create them through the phpMyAdmin interface later

-- Create triggers for additional integrity and auditing
DELIMITER //

-- After update trigger on accounts to log balance changes
CREATE TRIGGER cbs_accounts_after_update
AFTER UPDATE ON cbs_accounts
FOR EACH ROW
BEGIN
    IF OLD.balance != NEW.balance THEN
        INSERT INTO cbs_system_logs (log_level, component, module, message, details)
        VALUES ('INFO', 'DATABASE', 'ACCOUNT_MANAGEMENT', 
                CONCAT('Balance updated for account ', NEW.account_number),
                CONCAT('Old balance: ', OLD.balance, ', New balance: ', NEW.balance));
                
        -- Also log to audit trail
        INSERT INTO cbs_audit_logs (user_id, action, entity_type, entity_id, 
                                previous_value, new_value, details)
        VALUES (CURRENT_USER(), 'UPDATE', 'ACCOUNT', NEW.account_number, 
                OLD.balance, NEW.balance, 'Account balance updated');
    END IF;
END//

-- Before update trigger on cards to validate status changes
CREATE TRIGGER cbs_cards_before_update
BEFORE UPDATE ON cbs_cards
FOR EACH ROW
BEGIN
    IF NEW.status = 'EXPIRED' AND NEW.expiry_date > CURDATE() THEN
        SIGNAL SQLSTATE '45000' 
        SET MESSAGE_TEXT = 'Cannot set card status to EXPIRED before the expiry date';
    END IF;
    
    -- Validate status transitions
    IF OLD.status = 'HOTLISTED' AND NEW.status != 'HOTLISTED' THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Cannot change status once card is hotlisted';
    END IF;
END//

-- After insert trigger on cbs_transactions to update account balance
CREATE TRIGGER cbs_transactions_after_insert
AFTER INSERT ON cbs_transactions
FOR EACH ROW
BEGIN
    -- Update last transaction timestamp in accounts table
    UPDATE cbs_accounts 
    SET last_transaction = NOW()
    WHERE account_number = NEW.account_number;
    
    -- Log transaction to system logs
    INSERT INTO cbs_system_logs (log_level, component, module, message, details)
    VALUES ('INFO', 'TRANSACTIONS', 'CORE_BANKING', 
            CONCAT('New transaction recorded: ', NEW.transaction_id),
            CONCAT('Account: ', NEW.account_number, ', Type: ', NEW.transaction_type, 
                  ', Amount: ', NEW.amount));
END//

-- Before insert trigger on cbs_transfers to validate account status
CREATE TRIGGER cbs_transfers_before_insert
BEFORE INSERT ON cbs_transfers
FOR EACH ROW
BEGIN
    DECLARE v_source_status VARCHAR(20);
    DECLARE v_dest_status VARCHAR(20);
    
    -- Get account statuses
    SELECT status INTO v_source_status 
    FROM cbs_accounts 
    WHERE account_number = NEW.source_account;
    
    -- Only check destination if it's an internal account
    IF NEW.transfer_type = 'INTERNAL' THEN
        SELECT status INTO v_dest_status 
        FROM cbs_accounts 
        WHERE account_number = NEW.destination_account;
        
        IF v_dest_status != 'ACTIVE' THEN
            SIGNAL SQLSTATE '45000'
            SET MESSAGE_TEXT = 'Cannot transfer to non-active account';
        END IF;
    END IF;
    
    -- Validate source account
    IF v_source_status != 'ACTIVE' THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'Cannot transfer from non-active account';
    END IF;
END//

DELIMITER ;

-- Grant privileges (modify as needed for production)
-- Note: In phpMyAdmin, you may need to create users through the 'User accounts' interface
-- The following statements may require adjustment based on your MySQL version

-- For MySQL 8.0+, use separate CREATE USER and GRANT statements:
-- CREATE USER 'atm_app'@'localhost' IDENTIFIED BY 'secure_password';
-- GRANT ALL PRIVILEGES ON core_banking_system.* TO 'atm_app'@'localhost';
-- CREATE USER 'cbs_app'@'localhost' IDENTIFIED BY 'secure_password';
-- GRANT ALL PRIVILEGES ON core_banking_system.* TO 'cbs_app'@'localhost';
-- CREATE USER 'cbs_readonly'@'localhost' IDENTIFIED BY 'readonly_pwd';
-- GRANT SELECT, UPDATE, INSERT ON core_banking_system.* TO 'cbs_readonly'@'localhost';
-- CREATE USER 'cbs_report'@'localhost' IDENTIFIED BY 'report_pwd';
-- GRANT SELECT ON core_banking_system.* TO 'cbs_report'@'localhost';

-- For MySQL 5.7 and earlier, you can use:
CREATE USER 'atm_app'@'localhost' IDENTIFIED BY 'secure_password';
GRANT ALL PRIVILEGES ON core_banking_system.* TO 'atm_app'@'localhost';

CREATE USER 'cbs_app'@'localhost' IDENTIFIED BY 'secure_password';
GRANT ALL PRIVILEGES ON core_banking_system.* TO 'cbs_app'@'localhost';

CREATE USER 'cbs_readonly'@'localhost' IDENTIFIED BY 'readonly_pwd';
GRANT SELECT, UPDATE, INSERT ON core_banking_system.* TO 'cbs_readonly'@'localhost';

CREATE USER 'cbs_report'@'localhost' IDENTIFIED BY 'report_pwd';
GRANT SELECT ON core_banking_system.* TO 'cbs_report'@'localhost';

FLUSH PRIVILEGES;

-- Additional tables for core banking systems

-- Branches table
CREATE TABLE cbs_branches (
    branch_code VARCHAR(20) PRIMARY KEY,
    branch_name VARCHAR(100) NOT NULL,
    branch_type ENUM('HEAD_OFFICE', 'REGIONAL', 'MAIN', 'EXTENSION_COUNTER', 'SATELLITE') NOT NULL,
    address VARCHAR(255) NOT NULL,
    city VARCHAR(100) NOT NULL,
    state VARCHAR(100) NOT NULL,
    country VARCHAR(100) NOT NULL DEFAULT 'India',
    pin_code VARCHAR(10) NOT NULL,
    phone VARCHAR(20) NOT NULL,
    email VARCHAR(100) NOT NULL,
    ifsc_code VARCHAR(11) NOT NULL UNIQUE,
    micr_code VARCHAR(9),
    manager_id VARCHAR(20),
    opening_date DATE NOT NULL,
    status ENUM('ACTIVE', 'INACTIVE', 'CLOSED') NOT NULL DEFAULT 'ACTIVE',
    working_hours VARCHAR(100),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_branch_name (branch_name),
    INDEX idx_branch_ifsc (ifsc_code),
    INDEX idx_branch_status (status)
) ENGINE=InnoDB;

-- Fixed Deposits table
CREATE TABLE cbs_fixed_deposits (
    fd_id VARCHAR(36) PRIMARY KEY,
    account_number VARCHAR(20) NOT NULL,
    customer_id VARCHAR(20) NOT NULL,
    principal_amount DECIMAL(12,2) NOT NULL,
    interest_rate DECIMAL(5,2) NOT NULL,
    tenure_months INT NOT NULL,
    start_date DATE NOT NULL,
    maturity_date DATE NOT NULL,
    maturity_amount DECIMAL(12,2) NOT NULL,
    interest_payout_frequency ENUM('MONTHLY', 'QUARTERLY', 'HALF_YEARLY', 'ANNUALLY', 'AT_MATURITY') NOT NULL,
    auto_renewal BOOLEAN DEFAULT FALSE,
    premature_withdrawal_allowed BOOLEAN DEFAULT TRUE,
    premature_withdrawal_penalty DECIMAL(5,2) DEFAULT 1.00,
    interest_payout_account VARCHAR(20),
    status ENUM('ACTIVE', 'MATURED', 'CLOSED', 'PREMATURE_WITHDRAWAL') NOT NULL DEFAULT 'ACTIVE',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (account_number) REFERENCES cbs_accounts(account_number),
    FOREIGN KEY (customer_id) REFERENCES cbs_customers(customer_id),
    INDEX idx_fd_customer (customer_id),
    INDEX idx_fd_account (account_number),
    INDEX idx_fd_status (status),
    INDEX idx_fd_maturity (maturity_date)
) ENGINE=InnoDB;

-- Loans table
CREATE TABLE cbs_loans (
    loan_id VARCHAR(36) PRIMARY KEY,
    account_number VARCHAR(20) NOT NULL,
    customer_id VARCHAR(20) NOT NULL,
    loan_type ENUM('HOME', 'VEHICLE', 'PERSONAL', 'EDUCATION', 'BUSINESS', 'AGRICULTURE', 'GOLD') NOT NULL,
    application_date DATE NOT NULL,
    approval_date DATE,
    principal_amount DECIMAL(12,2) NOT NULL,
    disbursed_amount DECIMAL(12,2),
    interest_rate DECIMAL(5,2) NOT NULL,
    interest_type ENUM('FIXED', 'FLOATING', 'HYBRID') NOT NULL,
    tenure_months INT NOT NULL,
    start_date DATE,
    end_date DATE,
    emi_amount DECIMAL(12,2),
    emi_date INT NOT NULL,
    total_emis INT NOT NULL,
    emis_paid INT DEFAULT 0,
    processing_fee DECIMAL(10,2) DEFAULT 0.00,
    prepayment_penalty DECIMAL(5,2) DEFAULT 0.00,
    collateral_type VARCHAR(50),
    collateral_value DECIMAL(14,2),
    collateral_details TEXT,
    guarantor_id VARCHAR(20),
    purpose VARCHAR(255),
    status ENUM('APPLIED', 'APPROVED', 'REJECTED', 'ACTIVE', 'CLOSED', 'DEFAULTED', 'SETTLED', 'WRITTEN_OFF') NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (account_number) REFERENCES cbs_accounts(account_number),
    FOREIGN KEY (customer_id) REFERENCES cbs_customers(customer_id),
    INDEX idx_loan_customer (customer_id),
    INDEX idx_loan_status (status),
    INDEX idx_loan_type (loan_type)
) ENGINE=InnoDB;

-- Loan Repayments table
CREATE TABLE cbs_loan_repayments (
    repayment_id VARCHAR(36) PRIMARY KEY,
    loan_id VARCHAR(36) NOT NULL,
    amount DECIMAL(12,2) NOT NULL,
    principal_component DECIMAL(12,2) NOT NULL,
    interest_component DECIMAL(12,2) NOT NULL,
    penalties DECIMAL(10,2) DEFAULT 0.00,
    repayment_date DATE NOT NULL,
    payment_mode ENUM('CASH', 'CHEQUE', 'AUTO_DEBIT', 'ONLINE', 'UPI') NOT NULL,
    payment_reference VARCHAR(50),
    status ENUM('PAID', 'MISSED', 'PARTIAL', 'FUTURE') NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (loan_id) REFERENCES cbs_loans(loan_id),
    INDEX idx_repayment_loan (loan_id),
    INDEX idx_repayment_date (repayment_date),
    INDEX idx_repayment_status (status)
) ENGINE=InnoDB;

-- Interest Rates table
CREATE TABLE cbs_interest_rates (
    rate_id INT AUTO_INCREMENT PRIMARY KEY,
    product_type ENUM('SAVINGS', 'CURRENT', 'FIXED_DEPOSIT', 'RECURRING_DEPOSIT', 'HOME_LOAN', 'PERSONAL_LOAN', 'VEHICLE_LOAN', 'EDUCATION_LOAN') NOT NULL,
    customer_type ENUM('REGULAR', 'SENIOR_CITIZEN', 'STAFF', 'MINOR', 'CORPORATE') NOT NULL DEFAULT 'REGULAR',
    minimum_balance DECIMAL(12,2),
    maximum_balance DECIMAL(14,2),
    tenure_days INT,
    interest_rate DECIMAL(5,2) NOT NULL,
    effective_from DATE NOT NULL,
    effective_to DATE,
    is_active BOOLEAN DEFAULT TRUE,
    created_by VARCHAR(20),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_interest_product (product_type),
    INDEX idx_interest_customer (customer_type),
    INDEX idx_interest_effective (effective_from, effective_to)
) ENGINE=InnoDB;

-- Standing Instructions table
CREATE TABLE cbs_standing_instructions (
    instruction_id VARCHAR(36) PRIMARY KEY,
    customer_id VARCHAR(20) NOT NULL,
    source_account VARCHAR(20) NOT NULL,
    destination_account VARCHAR(20) NOT NULL,
    destination_account_ifsc VARCHAR(11),
    beneficiary_name VARCHAR(100) NOT NULL,
    transfer_type ENUM('INTERNAL', 'NEFT', 'RTGS', 'IMPS') NOT NULL,
    amount DECIMAL(12,2) NOT NULL,
    frequency ENUM('DAILY', 'WEEKLY', 'MONTHLY', 'QUARTERLY', 'HALF_YEARLY', 'YEARLY') NOT NULL,
    execution_day INT NOT NULL,
    start_date DATE NOT NULL,
    end_date DATE,
    next_execution_date DATE NOT NULL,
    status ENUM('ACTIVE', 'PAUSED', 'EXPIRED', 'CANCELLED') NOT NULL DEFAULT 'ACTIVE',
    remarks VARCHAR(255),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (customer_id) REFERENCES cbs_customers(customer_id),
    FOREIGN KEY (source_account) REFERENCES cbs_accounts(account_number),
    INDEX idx_si_customer (customer_id),
    INDEX idx_si_source (source_account),
    INDEX idx_si_next_execution (next_execution_date),
    INDEX idx_si_status (status)
) ENGINE=InnoDB;

-- Beneficiaries table
CREATE TABLE cbs_beneficiaries (
    beneficiary_id VARCHAR(36) PRIMARY KEY,
    customer_id VARCHAR(20) NOT NULL,
    beneficiary_name VARCHAR(100) NOT NULL,
    account_number VARCHAR(20) NOT NULL,
    ifsc_code VARCHAR(11) NOT NULL,
    bank_name VARCHAR(100) NOT NULL,
    branch_name VARCHAR(100),
    account_type ENUM('SAVINGS', 'CURRENT', 'CASH_CREDIT', 'LOAN', 'NRE', 'NRO') NOT NULL,
    nickname VARCHAR(50),
    status ENUM('ACTIVE', 'INACTIVE', 'DELETED') NOT NULL DEFAULT 'ACTIVE',
    last_transfer_date DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (customer_id) REFERENCES cbs_customers(customer_id),
    INDEX idx_beneficiary_customer (customer_id),
    INDEX idx_beneficiary_account (account_number),
    INDEX idx_beneficiary_status (status)
) ENGINE=InnoDB;

-- Currency Exchange Rates table
CREATE TABLE cbs_currency_rates (
    rate_id INT AUTO_INCREMENT PRIMARY KEY,
    currency_code VARCHAR(3) NOT NULL,
    base_currency VARCHAR(3) NOT NULL DEFAULT 'INR',
    buy_rate DECIMAL(12,6) NOT NULL,
    sell_rate DECIMAL(12,6) NOT NULL,
    mid_rate DECIMAL(12,6) NOT NULL,
    effective_date DATE NOT NULL,
    expiry_date DATE,
    is_active BOOLEAN DEFAULT TRUE,
    updated_by VARCHAR(20) NOT NULL,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_currency_code (currency_code),
    INDEX idx_effective_date (effective_date)
) ENGINE=InnoDB;

-- Cheque Books table
CREATE TABLE cbs_cheque_books (
    cheque_book_id VARCHAR(36) PRIMARY KEY,
    account_number VARCHAR(20) NOT NULL,
    start_number VARCHAR(20) NOT NULL,
    end_number VARCHAR(20) NOT NULL,
    leaves_count INT NOT NULL,
    issue_date DATE NOT NULL,
    requested_by VARCHAR(50),
    status ENUM('REQUESTED', 'ISSUED', 'DELIVERED', 'ACTIVATED', 'CANCELLED') NOT NULL DEFAULT 'REQUESTED',
    delivery_method ENUM('BRANCH', 'COURIER', 'REGISTERED_POST') NOT NULL,
    tracking_number VARCHAR(50),
    delivery_date DATE,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (account_number) REFERENCES cbs_accounts(account_number),
    INDEX idx_cheque_account (account_number),
    INDEX idx_cheque_status (status)
) ENGINE=InnoDB;

-- Cheques table
CREATE TABLE cbs_cheques (
    cheque_id VARCHAR(36) PRIMARY KEY,
    account_number VARCHAR(20) NOT NULL,
    cheque_number VARCHAR(20) NOT NULL,
    amount DECIMAL(12,2) NOT NULL,
    payee_name VARCHAR(100) NOT NULL,
    presentation_date DATE,
    clearing_date DATE,
    status ENUM('ISSUED', 'PRESENTED', 'CLEARED', 'BOUNCED', 'STOPPED') NOT NULL DEFAULT 'ISSUED',
    transaction_id VARCHAR(36),
    bounce_reason VARCHAR(255),
    image_path VARCHAR(255),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (account_number) REFERENCES cbs_accounts(account_number),
    INDEX idx_cheque_account (account_number),
    INDEX idx_cheque_number (cheque_number),
    INDEX idx_cheque_status (status)
) ENGINE=InnoDB;

-- Scheduled Jobs table
CREATE TABLE cbs_scheduled_jobs (
    job_id VARCHAR(36) PRIMARY KEY,
    job_type ENUM('INTEREST_CALCULATION', 'ACCOUNT_STATEMENT', 'LOAN_EMI', 'STANDING_INSTRUCTION', 'FIXED_DEPOSIT_MATURITY', 'EOD_PROCESS') NOT NULL,
    entity_id VARCHAR(36),
    entity_type VARCHAR(50),
    scheduled_time DATETIME NOT NULL,
    status ENUM('SCHEDULED', 'RUNNING', 'COMPLETED', 'FAILED', 'CANCELLED') NOT NULL DEFAULT 'SCHEDULED',
    priority INT DEFAULT 5,
    retry_count INT DEFAULT 0,
    max_retries INT DEFAULT 3,
    last_run_time DATETIME,
    next_run_time DATETIME,
    error_message TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_job_type (job_type),
    INDEX idx_job_status (status),
    INDEX idx_job_scheduled (scheduled_time),
    INDEX idx_job_next_run (next_run_time)
) ENGINE=InnoDB;

-- -----------------------------------------------------------------------
-- IMPORTANT NOTES FOR PHPMYADMIN USERS
-- -----------------------------------------------------------------------
-- 
-- If you encounter issues importing this large SQL file in phpMyAdmin, consider:
--
-- 1. Increasing phpMyAdmin's upload/post limits in php.ini:
--    - post_max_size = 64M
--    - upload_max_filesize = 64M
--    - max_execution_time = 300
--
-- 2. Splitting this file into smaller parts:
--    - Tables creation
--    - Triggers creation
--    - Stored procedures creation
--    - User permissions
--
-- 3. For partitioning issues, you may need to:
--    - Uncomment the partitioning code if your MySQL version supports it
--    - Or remove partitioning entirely if not supported
--
-- 4. For stored procedures and triggers:
--    - You may need to create these individually using phpMyAdmin's interface
--    - Remember to set the proper delimiter when creating procedures/triggers
--
-- 5. For user creation:
--    - Use the "User accounts" section in phpMyAdmin instead of SQL commands
--    - The method of creating users varies between MySQL versions
--
-- -----------------------------------------------------------------------