<?php
/**
 * Authentication class for the ATM Management System web backend
 */

class Auth {
    /**
     * Check if user is logged in
     * @return bool True if user is logged in, false otherwise
     */
    public static function isLoggedIn() {
        return isset($_SESSION['user_id']);
    }
    
    /**
     * Get current user data
     * @return array|null User data or null if not logged in
     */
    public static function getUser() {
        if (!self::isLoggedIn()) {
            return null;
        }
        
        // For demo purposes, return a mock user
        // In a real application, this would fetch the user from the database
        return [
            'id' => $_SESSION['user_id'] ?? 1,
            'name' => $_SESSION['user_name'] ?? 'John Doe',
            'email' => $_SESSION['user_email'] ?? 'john.doe@example.com',
            'role' => $_SESSION['user_role'] ?? 'Customer'
        ];
    }
    
    /**
     * Check if user is an admin
     * @return bool True if user is an admin, false otherwise
     */
    public static function isAdmin() {
        if (!self::isLoggedIn()) {
            return false;
        }
        
        $user = self::getUser();
        return $user['role'] === 'Admin' || $user['role'] === 'SuperAdmin';
    }
    
    /**
     * Authenticate user
     * @param string $username Username or email
     * @param string $password Password
     * @param bool $remember Remember me
     * @return bool True if authentication successful, false otherwise
     */
    public static function login($username, $password, $remember = false) {
        // For demo purposes, accept any login with demo credentials
        // In a real application, this would verify against the database
        if (($username === 'customer@example.com' && $password === 'password123') ||
            ($username === 'admin@example.com' && $password === 'admin123')) {
            
            // Set session variables
            $_SESSION['user_id'] = ($username === 'admin@example.com') ? 2 : 1;
            $_SESSION['user_name'] = ($username === 'admin@example.com') ? 'Admin User' : 'John Doe';
            $_SESSION['user_email'] = $username;
            $_SESSION['user_role'] = ($username === 'admin@example.com') ? 'Admin' : 'Customer';
            $_SESSION['last_activity'] = time();
            
            // Set remember me cookie if requested
            if ($remember) {
                $token = bin2hex(random_bytes(32));
                $expires = time() + (86400 * 30); // 30 days
                setcookie('remember_token', $token, $expires, '/', '', false, true);
                
                // In a real application, store the token in the database
                // associated with the user
            }
            
            // Log the login
            self::logActivity('Authentication', 'Login', 'Success');
            
            return true;
        }
        
        // Log the failed login
        self::logActivity('Authentication', 'Login', 'Failed', 'Invalid username or password');
        
        return false;
    }
    
    /**
     * Log out user
     * @return void
     */
    public static function logout() {
        // Log the logout
        if (self::isLoggedIn()) {
            self::logActivity('Authentication', 'Logout', 'Success');
        }
        
        // Unset all session variables
        $_SESSION = [];
        
        // Delete the session cookie
        if (ini_get('session.use_cookies')) {
            $params = session_get_cookie_params();
            setcookie(
                session_name(),
                '',
                time() - 42000,
                $params['path'],
                $params['domain'],
                $params['secure'],
                $params['httponly']
            );
        }
        
        // Delete the remember me cookie
        setcookie('remember_token', '', time() - 3600, '/', '', false, true);
        
        // Destroy the session
        session_destroy();
    }
    
    /**
     * Register a new user
     * @param array $userData User data
     * @return int|bool User ID if registration successful, false otherwise
     */
    public static function register($userData) {
        // For demo purposes, always return success
        // In a real application, this would insert the user into the database
        
        // Validate required fields
        $requiredFields = ['name', 'email', 'password', 'confirm_password'];
        foreach ($requiredFields as $field) {
            if (empty($userData[$field])) {
                return false;
            }
        }
        
        // Validate email
        if (!filter_var($userData['email'], FILTER_VALIDATE_EMAIL)) {
            return false;
        }
        
        // Validate password
        if (strlen($userData['password']) < 8) {
            return false;
        }
        
        // Validate password confirmation
        if ($userData['password'] !== $userData['confirm_password']) {
            return false;
        }
        
        // In a real application, check if email already exists
        
        // In a real application, hash the password
        $passwordHash = password_hash($userData['password'], PASSWORD_DEFAULT);
        
        // In a real application, insert the user into the database
        $userId = 3; // Mock user ID
        
        // Log the registration
        self::logActivity('Authentication', 'Registration', 'Success', '', $userId);
        
        return $userId;
    }
    
    /**
     * Check if session is expired
     * @return bool True if session is expired, false otherwise
     */
    public static function isSessionExpired() {
        if (!isset($_SESSION['last_activity'])) {
            return true;
        }
        
        $inactiveTime = time() - $_SESSION['last_activity'];
        return $inactiveTime > SESSION_LIFETIME;
    }
    
    /**
     * Update last activity time
     * @return void
     */
    public static function updateLastActivity() {
        $_SESSION['last_activity'] = time();
    }
    
    /**
     * Check if user has permission
     * @param string $permission Permission to check
     * @return bool True if user has permission, false otherwise
     */
    public static function hasPermission($permission) {
        if (!self::isLoggedIn()) {
            return false;
        }
        
        $user = self::getUser();
        
        // For demo purposes, define some basic permissions
        $permissions = [
            'Customer' => ['view_account', 'view_transactions', 'make_transfer', 'pay_bill'],
            'Admin' => ['view_account', 'view_transactions', 'make_transfer', 'pay_bill', 'view_admin', 'manage_users', 'view_reports'],
            'SuperAdmin' => ['view_account', 'view_transactions', 'make_transfer', 'pay_bill', 'view_admin', 'manage_users', 'view_reports', 'manage_system']
        ];
        
        // Check if user role has the requested permission
        return in_array($permission, $permissions[$user['role']] ?? []);
    }
    
    /**
     * Log activity
     * @param string $category Activity category
     * @param string $action Activity action
     * @param string $status Activity status
     * @param string $details Activity details
     * @param int|null $userId User ID (if different from current user)
     * @return bool True if logging successful, false otherwise
     */
    public static function logActivity($category, $action, $status, $details = '', $userId = null) {
        // Get current user ID
        $currentUserId = self::isLoggedIn() ? self::getUser()['id'] : null;
        $userId = $userId ?? $currentUserId;
        
        // Get IP address
        $ipAddress = $_SERVER['REMOTE_ADDR'] ?? '0.0.0.0';
        
        // Get user agent
        $userAgent = $_SERVER['HTTP_USER_AGENT'] ?? '';
        
        // In a real application, insert the log into the database
        // For demo purposes, just return true
        return true;
    }
    
    /**
     * Generate CSRF token
     * @return string CSRF token
     */
    public static function generateCsrfToken() {
        if (empty($_SESSION['csrf_token'])) {
            $_SESSION['csrf_token'] = bin2hex(random_bytes(32));
            $_SESSION['csrf_token_time'] = time();
        }
        
        return $_SESSION['csrf_token'];
    }
    
    /**
     * Verify CSRF token
     * @param string $token CSRF token to verify
     * @return bool True if token is valid, false otherwise
     */
    public static function verifyCsrfToken($token) {
        if (empty($_SESSION['csrf_token']) || empty($_SESSION['csrf_token_time'])) {
            return false;
        }
        
        // Check if token matches
        if (hash_equals($_SESSION['csrf_token'], $token)) {
            // Check if token is expired
            $tokenAge = time() - $_SESSION['csrf_token_time'];
            if ($tokenAge <= CSRF_TOKEN_LIFETIME) {
                return true;
            }
        }
        
        return false;
    }
    
    /**
     * Reset CSRF token
     * @return void
     */
    public static function resetCsrfToken() {
        unset($_SESSION['csrf_token']);
        unset($_SESSION['csrf_token_time']);
    }
}
