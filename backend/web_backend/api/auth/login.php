<?php
/**
 * Login API endpoint for the ATM Management System web backend
 */

// Include configuration
require_once __DIR__ . '/../../config/config.php';

// Include necessary files
require_once BACKEND_PATH . '/includes/Database.php';
require_once BACKEND_PATH . '/includes/Auth.php';
require_once BACKEND_PATH . '/includes/Utilities.php';

// Start session if not already started
if (session_status() === PHP_SESSION_NONE) {
    session_start();
}

// Check if request method is POST
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    // Redirect to login page with error
    Utilities::setFlashMessage('Invalid request method.', 'danger');
    Utilities::redirect('index.php?page=login');
    exit;
}

// Get form data
$username = $_POST['username'] ?? '';
$password = $_POST['password'] ?? '';
$remember = isset($_POST['remember']) && $_POST['remember'] === 'on';
$redirect = $_POST['redirect'] ?? 'dashboard';

// Validate form data
if (empty($username) || empty($password)) {
    // Redirect to login page with error
    Utilities::setFlashMessage('Please enter both username and password.', 'danger');
    Utilities::redirect('index.php?page=login&error=invalid');
    exit;
}

// Attempt to authenticate user
$authenticated = Auth::login($username, $password, $remember);

if ($authenticated) {
    // Redirect to requested page or dashboard
    Utilities::setFlashMessage('Login successful. Welcome back!', 'success');
    Utilities::redirect('index.php?page=' . urlencode($redirect));
} else {
    // Redirect to login page with error
    Utilities::setFlashMessage('Invalid username or password.', 'danger');
    Utilities::redirect('index.php?page=login&error=invalid');
}
