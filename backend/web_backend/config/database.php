<?php
/**
 * Database connection configuration for the ATM Management System web backend
 */

// Database connection credentials
define('DB_HOST', 'localhost');
define('DB_NAME', 'atm_management');
define('DB_USER', 'atm_app');
define('DB_PASS', 'secure_password');
define('DB_PORT', 3306);
define('DB_CHARSET', 'utf8mb4');

// Session configuration
define('SESSION_LIFETIME', 1800); // 30 minutes
define('CSRF_TOKEN_LIFETIME', 3600); // 1 hour

// Database tables
define('TABLE_USERS', 'users');
define('TABLE_ADMIN_USERS', 'admin_users');
define('TABLE_CUSTOMERS', 'customers');
define('TABLE_ACCOUNTS', 'accounts');
define('TABLE_CARDS', 'cards');
define('TABLE_TRANSACTIONS', 'transactions');
define('TABLE_USER_TOKENS', 'user_tokens');
define('TABLE_SYSTEM_LOGS', 'system_logs');

// Connection timeout settings
define('DB_CONNECTION_TIMEOUT', 5); // seconds
