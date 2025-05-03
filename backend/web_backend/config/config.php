<?php
/**
 * Configuration file for the ATM Management System web backend
 */

// Database configuration
define('DB_HOST', 'localhost');
define('DB_USER', 'atm_user');
define('DB_PASS', 'secure_password_here');
define('DB_NAME', 'atm_management');
define('DB_PORT', 3306);

// Application settings
define('APP_NAME', 'ATM Management System');
define('APP_VERSION', '1.0.0');
define('APP_URL', 'http://localhost/atm-management');
define('APP_DEBUG', true);

// Security settings
define('SESSION_LIFETIME', 1800); // 30 minutes
define('PASSWORD_HASH_ALGO', PASSWORD_BCRYPT);
define('PASSWORD_HASH_COST', 12);
define('CSRF_TOKEN_LIFETIME', 3600); // 1 hour

// API settings
define('API_VERSION', 'v1');
define('API_RATE_LIMIT', 100); // requests per minute
define('API_RESPONSE_FORMAT', 'json');

// Logging settings
define('LOG_ENABLED', true);
define('LOG_LEVEL', 'debug'); // debug, info, warning, error, critical
define('LOG_FILE', __DIR__ . '/../logs/app.log');

// Path settings
define('ROOT_PATH', realpath(__DIR__ . '/../../..'));
define('BACKEND_PATH', ROOT_PATH . '/backend/web_backend');
define('FRONTEND_PATH', ROOT_PATH . '/frontend/web');
define('UPLOAD_PATH', FRONTEND_PATH . '/uploads');
define('TEMPLATE_PATH', FRONTEND_PATH . '/templates');

// Time zone
date_default_timezone_set('Asia/Kolkata');

// Error reporting
if (APP_DEBUG) {
    error_reporting(E_ALL);
    ini_set('display_errors', 1);
} else {
    error_reporting(0);
    ini_set('display_errors', 0);
}

// Session configuration
ini_set('session.cookie_httponly', 1);
ini_set('session.use_only_cookies', 1);
ini_set('session.cookie_secure', 0); // Set to 1 for HTTPS
ini_set('session.cookie_samesite', 'Strict');
ini_set('session.gc_maxlifetime', SESSION_LIFETIME);
session_set_cookie_params(SESSION_LIFETIME);

// Load environment-specific configuration if exists
$env_config = __DIR__ . '/config_' . (getenv('APP_ENV') ?: 'development') . '.php';
if (file_exists($env_config)) {
    require_once $env_config;
}
