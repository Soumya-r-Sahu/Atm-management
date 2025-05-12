-- Core Banking System Database Schema

-- Customers table
CREATE TABLE IF NOT EXISTS customers (
    customer_id INT AUTO_INCREMENT PRIMARY KEY,
    first_name VARCHAR(50) NOT NULL,
    last_name VARCHAR(50) NOT NULL,
    date_of_birth DATE NOT NULL,
    address VARCHAR(255) NOT NULL,
    city VARCHAR(50) NOT NULL,
    state VARCHAR(50) NOT NULL,
    postal_code VARCHAR(20) NOT NULL,
    country VARCHAR(50) NOT NULL,
    phone VARCHAR(20) NOT NULL,
    email VARCHAR(100),
    customer_type TINYINT DEFAULT 1, -- 1 = Individual, 2 = Corporate, 3 = Government
    status TINYINT DEFAULT 1, -- 1 = Active, 2 = Inactive, 3 = Blocked
    created_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    modified_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Accounts table
CREATE TABLE IF NOT EXISTS accounts (
    account_id VARCHAR(20) PRIMARY KEY,
    customer_id INT NOT NULL,
    account_type TINYINT NOT NULL, -- 1 = Savings, 2 = Checking, 3 = Fixed, 4 = Loan
    balance DECIMAL(15,2) NOT NULL DEFAULT 0.00,
    currency VARCHAR(3) NOT NULL DEFAULT 'USD',
    interest_rate DECIMAL(5,2) DEFAULT 0.00,
    created_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status TINYINT DEFAULT 1, -- 1 = Active, 2 = Dormant, 3 = Closed, 4 = Frozen
    closed_date TIMESTAMP NULL,
    FOREIGN KEY (customer_id) REFERENCES customers(customer_id)
) ENGINE=InnoDB;

-- Transactions table
CREATE TABLE IF NOT EXISTS transactions (
    transaction_id VARCHAR(20) PRIMARY KEY,
    account_id VARCHAR(20) NOT NULL,
    type TINYINT NOT NULL, -- 1 = Deposit, 2 = Withdrawal, 3 = Transfer, 4 = Payment, 5 = Fee, 6 = Interest
    amount DECIMAL(15,2) NOT NULL,
    description VARCHAR(255),
    reference_id VARCHAR(50),
    transaction_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status TINYINT DEFAULT 2, -- 1 = Pending, 2 = Complete, 3 = Failed, 4 = Reversed
    FOREIGN KEY (account_id) REFERENCES accounts(account_id)
) ENGINE=InnoDB;

-- General ledger table
CREATE TABLE IF NOT EXISTS general_ledger (
    entry_id VARCHAR(20) PRIMARY KEY,
    debit_account VARCHAR(20) NOT NULL,
    credit_account VARCHAR(20) NOT NULL,
    amount DECIMAL(15,2) NOT NULL,
    description VARCHAR(255),
    entry_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    transaction_id VARCHAR(20),
    FOREIGN KEY (transaction_id) REFERENCES transactions(transaction_id)
) ENGINE=InnoDB;

-- Audit trail table
CREATE TABLE IF NOT EXISTS audit_trail (
    event_id VARCHAR(20) PRIMARY KEY,
    event_type TINYINT NOT NULL, -- 1 = Login, 2 = Logout, 3 = Create, 4 = Update, 5 = Delete, 6 = View, etc.
    user_id VARCHAR(50) NOT NULL,
    event_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    description VARCHAR(255),
    ip_address VARCHAR(45),
    system_info VARCHAR(255)
) ENGINE=InnoDB;

-- Cards table
CREATE TABLE IF NOT EXISTS cards (
    card_id VARCHAR(20) PRIMARY KEY,
    account_id VARCHAR(20) NOT NULL,
    card_number VARCHAR(19) NOT NULL,
    expiry_date DATE NOT NULL,
    cvv VARCHAR(3) NOT NULL,
    card_type TINYINT NOT NULL, -- 1 = Debit, 2 = Credit
    status TINYINT DEFAULT 1, -- 1 = Active, 2 = Blocked, 3 = Expired
    issue_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (account_id) REFERENCES accounts(account_id)
) ENGINE=InnoDB;

-- Users (system users) table
CREATE TABLE IF NOT EXISTS users (
    user_id VARCHAR(50) PRIMARY KEY,
    password VARCHAR(255) NOT NULL,
    role VARCHAR(50) NOT NULL,
    last_login TIMESTAMP,
    created_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status TINYINT DEFAULT 1 -- 1 = Active, 2 = Locked, 3 = Disabled
) ENGINE=InnoDB;

-- Beneficiaries (for transfers) table
CREATE TABLE IF NOT EXISTS beneficiaries (
    beneficiary_id INT AUTO_INCREMENT PRIMARY KEY,
    customer_id INT NOT NULL,
    account_id VARCHAR(20) NOT NULL,
    beneficiary_name VARCHAR(100) NOT NULL,
    beneficiary_account VARCHAR(20) NOT NULL,
    beneficiary_bank VARCHAR(100),
    beneficiary_bank_code VARCHAR(20),
    created_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (customer_id) REFERENCES customers(customer_id),
    FOREIGN KEY (account_id) REFERENCES accounts(account_id)
) ENGINE=InnoDB;

-- Transaction limits table
CREATE TABLE IF NOT EXISTS transaction_limits (
    limit_id INT AUTO_INCREMENT PRIMARY KEY,
    account_id VARCHAR(20) NOT NULL,
    transaction_type TINYINT NOT NULL, -- 1 = Deposit, 2 = Withdrawal, etc.
    daily_limit DECIMAL(15,2) NOT NULL,
    single_limit DECIMAL(15,2) NOT NULL,
    FOREIGN KEY (account_id) REFERENCES accounts(account_id)
) ENGINE=InnoDB;

-- Create indexes for performance
CREATE INDEX idx_transactions_account_id ON transactions(account_id);
CREATE INDEX idx_transactions_date ON transactions(transaction_date);
CREATE INDEX idx_audit_trail_time ON audit_trail(event_time);
CREATE INDEX idx_audit_trail_user ON audit_trail(user_id);
CREATE INDEX idx_accounts_customer ON accounts(customer_id);
