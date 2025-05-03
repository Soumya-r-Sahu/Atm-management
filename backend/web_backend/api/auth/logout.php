<?php
/**
 * Logout API endpoint for the ATM Management System web backend
 */

// Include configuration
require_once __DIR__ . '/../../config/config.php';

// Include necessary files
require_once BACKEND_PATH . '/includes/Auth.php';
require_once BACKEND_PATH . '/includes/Utilities.php';

// Start session if not already started
if (session_status() === PHP_SESSION_NONE) {
    session_start();
}

// Log out user
Auth::logout();

// Redirect to login page with message
Utilities::setFlashMessage('You have been successfully logged out.', 'success');
Utilities::redirect('index.php?page=login&message=logout');
