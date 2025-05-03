<?php
/**
 * Register API endpoint for the ATM Management System web backend
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
    // Redirect to register page with error
    Utilities::setFlashMessage('Invalid request method.', 'danger');
    Utilities::redirect('index.php?page=register');
    exit;
}

// Get form data
$name = $_POST['name'] ?? '';
$email = $_POST['email'] ?? '';
$phone = $_POST['phone'] ?? '';
$password = $_POST['password'] ?? '';
$confirmPassword = $_POST['confirm_password'] ?? '';

// Validate form data
$errors = [];

if (empty($name)) {
    $errors[] = 'Name is required.';
}

if (empty($email)) {
    $errors[] = 'Email is required.';
} elseif (!Utilities::validateEmail($email)) {
    $errors[] = 'Invalid email format.';
}

if (empty($phone)) {
    $errors[] = 'Phone number is required.';
} elseif (!Utilities::validatePhone($phone)) {
    $errors[] = 'Invalid phone number format.';
}

if (empty($password)) {
    $errors[] = 'Password is required.';
} elseif (strlen($password) < 8) {
    $errors[] = 'Password must be at least 8 characters long.';
}

if ($password !== $confirmPassword) {
    $errors[] = 'Passwords do not match.';
}

// Check if there are any errors
if (!empty($errors)) {
    // Redirect to register page with errors
    $errorMessage = implode(' ', $errors);
    Utilities::setFlashMessage($errorMessage, 'danger');
    Utilities::redirect('index.php?page=register&error=validation');
    exit;
}

// Prepare user data
$userData = [
    'name' => $name,
    'email' => $email,
    'phone' => $phone,
    'password' => $password,
    'confirm_password' => $confirmPassword
];

// Attempt to register user
$userId = Auth::register($userData);

if ($userId) {
    // Automatically log in the user
    Auth::login($email, $password);
    
    // Redirect to dashboard with success message
    Utilities::setFlashMessage('Registration successful. Welcome to ATM Management System!', 'success');
    Utilities::redirect('index.php?page=dashboard');
} else {
    // Redirect to register page with error
    Utilities::setFlashMessage('Registration failed. Please try again.', 'danger');
    Utilities::redirect('index.php?page=register&error=failed');
}
