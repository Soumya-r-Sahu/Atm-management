-- ATM Management System Database Schema
-- This schema includes all tables needed to replace text-file storage

-- Drop existing database if needed (comment out in production)
-- DROP DATABASE IF EXISTS atm_management;

-- Create database
CREATE DATABASE IF NOT EXISTS atm_management;
USE atm_management;

-- Customers table
CREATE TABLE IF NOT EXISTS customers (
    customerId VARCHAR(10) PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    dob DATE,
    address VARCHAR(200),
    email VARCHAR(100),
    phone VARCHAR(20),
    status ENUM('ACTIVE', 'INACTIVE', 'SUSPENDED') DEFAULT 'ACTIVE',
    kycStatus ENUM('PENDING', 'COMPLETED') DEFAULT 'PENDING',
    registrationDate DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Accounts table
CREATE TABLE IF NOT EXISTS accounts (
    accountNumber VARCHAR(20) PRIMARY KEY,
    customerId VARCHAR(10) NOT NULL,
    accountType ENUM('SAVINGS', 'CURRENT', 'FD') DEFAULT 'SAVINGS',
    branchCode VARCHAR(10),
    ifscCode VARCHAR(15),
    balance DECIMAL(15,2) DEFAULT 0.0,
    status ENUM('ACTIVE', 'INACTIVE', 'CLOSED') DEFAULT 'ACTIVE',
    openDate DATETIME DEFAULT CURRENT_TIMESTAMP,
    lastTransaction DATETIME DEFAULT CURRENT_TIMESTAMP,
    minBalance DECIMAL(10,2) DEFAULT 0.0,
    FOREIGN KEY (customerId) REFERENCES customers(customerId)
);

-- Cards table
CREATE TABLE IF NOT EXISTS cards (
    cardId VARCHAR(20) PRIMARY KEY,
    customerId VARCHAR(10) NOT NULL,
    accountNumber VARCHAR(20) NOT NULL,
    cardNumber BIGINT NOT NULL UNIQUE,
    cardType ENUM('DEBIT', 'CREDIT') DEFAULT 'DEBIT',
    expiryDate DATE NOT NULL,
    cvv INT NOT NULL,
    pinHash VARCHAR(64) NOT NULL,
    isActive BOOLEAN DEFAULT TRUE,
    isBlocked BOOLEAN DEFAULT FALSE,
    issueDate DATETIME DEFAULT CURRENT_TIMESTAMP,
    dailyLimit DECIMAL(10,2) DEFAULT 10000.00,
    FOREIGN KEY (customerId) REFERENCES customers(customerId),
    FOREIGN KEY (accountNumber) REFERENCES accounts(accountNumber)
);

-- Transactions table
CREATE TABLE IF NOT EXISTS transactions (
    transactionId VARCHAR(20) PRIMARY KEY,
    cardNumber BIGINT NOT NULL,
    accountNumber VARCHAR(20) NOT NULL,
    amount DECIMAL(15,2) NOT NULL,
    balance DECIMAL(15,2),
    type VARCHAR(20) NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(10) DEFAULT 'Success',
    remarks VARCHAR(100),
    FOREIGN KEY (accountNumber) REFERENCES accounts(accountNumber)
);

-- Daily withdrawals tracking
CREATE TABLE IF NOT EXISTS dailyWithdrawals (
    id INT PRIMARY KEY AUTO_INCREMENT,
    cardNumber BIGINT NOT NULL,
    amount DECIMAL(15,2) NOT NULL,
    withdrawalDate DATE NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (cardNumber) REFERENCES cards(cardNumber),
    INDEX (cardNumber, withdrawalDate)
);

-- Admin users table
CREATE TABLE IF NOT EXISTS adminUsers (
    adminId VARCHAR(50) PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    passwordHash VARCHAR(65) NOT NULL,
    role VARCHAR(20) NOT NULL,
    lastLogin DATETIME,
    status ENUM('ACTIVE', 'INACTIVE', 'SUSPENDED') DEFAULT 'ACTIVE'
);

-- UPI accounts table
CREATE TABLE IF NOT EXISTS upiAccounts (
    upiId VARCHAR(50) PRIMARY KEY,
    accountNumber VARCHAR(20) NOT NULL,
    cardNumber BIGINT,
    pinHash VARCHAR(64),
    isActive BOOLEAN DEFAULT TRUE,
    creationDate DATETIME DEFAULT CURRENT_TIMESTAMP,
    lastTransaction DATETIME,
    FOREIGN KEY (accountNumber) REFERENCES accounts(accountNumber),
    FOREIGN KEY (cardNumber) REFERENCES cards(cardNumber)
);

-- UPI transactions table
CREATE TABLE IF NOT EXISTS upiTransactions (
    transactionId VARCHAR(36) PRIMARY KEY,
    fromUpiId VARCHAR(50) NOT NULL,
    toUpiId VARCHAR(50) NOT NULL,
    amount DECIMAL(15,2) NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(10) DEFAULT 'Success',
    remarks VARCHAR(100),
    FOREIGN KEY (fromUpiId) REFERENCES upiAccounts(upiId),
    FOREIGN KEY (toUpiId) REFERENCES upiAccounts(upiId)
);

-- NetBanking users table
CREATE TABLE IF NOT EXISTS netBankingUsers (
    userId INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    passwordHash VARCHAR(64) NOT NULL,
    accountNumber VARCHAR(20) NOT NULL,
    loginAttempts INT DEFAULT 0,
    lastLogin DATETIME,
    isActive BOOLEAN DEFAULT TRUE,
    FOREIGN KEY (accountNumber) REFERENCES accounts(accountNumber)
);

-- NetBanking sessions
CREATE TABLE IF NOT EXISTS netBankingSessions (
    sessionId VARCHAR(64) PRIMARY KEY,
    userId INT NOT NULL,
    loginTime DATETIME NOT NULL,
    lastActivity DATETIME NOT NULL,
    isActive BOOLEAN DEFAULT TRUE,
    ipAddress VARCHAR(50),
    userAgent VARCHAR(200),
    FOREIGN KEY (userId) REFERENCES netBankingUsers(userId)
);

-- Beneficiaries for fund transfers
CREATE TABLE IF NOT EXISTS beneficiaries (
    id INT AUTO_INCREMENT PRIMARY KEY,
    customerId VARCHAR(10) NOT NULL,
    beneficiaryName VARCHAR(100) NOT NULL,
    accountNumber VARCHAR(20) NOT NULL,
    ifscCode VARCHAR(15) NOT NULL,
    bankName VARCHAR(50) NOT NULL,
    nickname VARCHAR(50),
    isActive BOOLEAN DEFAULT TRUE,
    FOREIGN KEY (customerId) REFERENCES customers(customerId)
);

-- Bill payments
CREATE TABLE IF NOT EXISTS billPayments (
    id INT AUTO_INCREMENT PRIMARY KEY,
    accountNumber VARCHAR(20) NOT NULL,
    billerId VARCHAR(50) NOT NULL,
    billerName VARCHAR(100) NOT NULL,
    amount DECIMAL(15,2) NOT NULL,
    billDate DATE NOT NULL,
    paymentDate DATETIME DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(10) DEFAULT 'Pending',
    referenceNumber VARCHAR(30),
    FOREIGN KEY (accountNumber) REFERENCES accounts(accountNumber)
);

-- ATM Configuration table
CREATE TABLE IF NOT EXISTS atmConfig (
    configKey VARCHAR(50) PRIMARY KEY,
    configValue VARCHAR(200) NOT NULL,
    description VARCHAR(200),
    lastModified DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- System logs table
CREATE TABLE IF NOT EXISTS systemLogs (
    id INT AUTO_INCREMENT PRIMARY KEY,
    logLevel VARCHAR(10) NOT NULL,
    module VARCHAR(50) NOT NULL,
    message TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    username VARCHAR(50),
    ipAddress VARCHAR(50)
);

-- Insert default ATM configuration
INSERT INTO atmConfig (configKey, configValue, description) VALUES
('ATM_SERVICE_STATUS', 'ACTIVE', 'Indicates if ATM is in service'),
('DAILY_WITHDRAWAL_LIMIT', '10000.00', 'Maximum daily withdrawal limit'),
('SESSION_TIMEOUT_SECONDS', '300', 'Session timeout in seconds'),
('MINIMUM_BALANCE', '500.00', 'Minimum balance required in account');

-- Insert default admin user (username: admin, password: admin123)
INSERT INTO adminUsers (adminId, username, passwordHash, role, status)
VALUES ('A001', 'admin', '240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9', 'ADMIN', 'ACTIVE');

-- Add indexes for performance
CREATE INDEX idx_transactions_card ON transactions(cardNumber);
CREATE INDEX idx_transactions_account ON transactions(accountNumber);
CREATE INDEX idx_transactions_timestamp ON transactions(timestamp);
CREATE INDEX idx_cards_customer ON cards(customerId);
CREATE INDEX idx_accounts_customer ON accounts(customerId);