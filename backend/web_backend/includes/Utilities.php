<?php
/**
 * Utilities class for the ATM Management System web backend
 */

class Utilities {
    /**
     * Format currency
     * @param float $amount Amount to format
     * @param string $currency Currency symbol
     * @return string Formatted currency
     */
    public static function formatCurrency($amount, $currency = '₹') {
        return $currency . number_format($amount, 2);
    }
    
    /**
     * Format date
     * @param string $date Date to format
     * @param string $format Format string
     * @return string Formatted date
     */
    public static function formatDate($date, $format = 'd M Y') {
        return date($format, strtotime($date));
    }
    
    /**
     * Format datetime
     * @param string $datetime Datetime to format
     * @param string $format Format string
     * @return string Formatted datetime
     */
    public static function formatDateTime($datetime, $format = 'd M Y, h:i A') {
        return date($format, strtotime($datetime));
    }
    
    /**
     * Generate random string
     * @param int $length Length of the string
     * @param string $characters Characters to use
     * @return string Random string
     */
    public static function generateRandomString($length = 10, $characters = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ') {
        $randomString = '';
        $charactersLength = strlen($characters);
        
        for ($i = 0; $i < $length; $i++) {
            $randomString .= $characters[rand(0, $charactersLength - 1)];
        }
        
        return $randomString;
    }
    
    /**
     * Generate reference number
     * @param string $prefix Prefix for the reference number
     * @return string Reference number
     */
    public static function generateReferenceNumber($prefix = 'TXN') {
        return $prefix . time() . self::generateRandomString(6, '0123456789');
    }
    
    /**
     * Sanitize input
     * @param mixed $input Input to sanitize
     * @return mixed Sanitized input
     */
    public static function sanitizeInput($input) {
        if (is_array($input)) {
            foreach ($input as $key => $value) {
                $input[$key] = self::sanitizeInput($value);
            }
            return $input;
        }
        
        if (is_string($input)) {
            return htmlspecialchars(trim($input), ENT_QUOTES, 'UTF-8');
        }
        
        return $input;
    }
    
    /**
     * Validate email
     * @param string $email Email to validate
     * @return bool True if email is valid, false otherwise
     */
    public static function validateEmail($email) {
        return filter_var($email, FILTER_VALIDATE_EMAIL) !== false;
    }
    
    /**
     * Validate phone number
     * @param string $phone Phone number to validate
     * @return bool True if phone number is valid, false otherwise
     */
    public static function validatePhone($phone) {
        // Basic validation for Indian phone numbers
        return preg_match('/^[6-9]\d{9}$/', $phone) === 1;
    }
    
    /**
     * Validate account number
     * @param string $accountNumber Account number to validate
     * @return bool True if account number is valid, false otherwise
     */
    public static function validateAccountNumber($accountNumber) {
        // Basic validation for account numbers
        return preg_match('/^\d{10,16}$/', $accountNumber) === 1;
    }
    
    /**
     * Validate IFSC code
     * @param string $ifscCode IFSC code to validate
     * @return bool True if IFSC code is valid, false otherwise
     */
    public static function validateIfscCode($ifscCode) {
        // Basic validation for IFSC codes
        return preg_match('/^[A-Z]{4}0[A-Z0-9]{6}$/', $ifscCode) === 1;
    }
    
    /**
     * Validate amount
     * @param float $amount Amount to validate
     * @param float $min Minimum amount
     * @param float $max Maximum amount
     * @return bool True if amount is valid, false otherwise
     */
    public static function validateAmount($amount, $min = 1, $max = PHP_FLOAT_MAX) {
        return is_numeric($amount) && $amount >= $min && $amount <= $max;
    }
    
    /**
     * Set flash message
     * @param string $message Message to set
     * @param string $type Message type (success, danger, warning, info)
     * @return void
     */
    public static function setFlashMessage($message, $type = 'success') {
        $_SESSION['flash_message'] = $message;
        $_SESSION['flash_type'] = $type;
    }
    
    /**
     * Get flash message
     * @return array|null Flash message or null if not set
     */
    public static function getFlashMessage() {
        if (isset($_SESSION['flash_message']) && isset($_SESSION['flash_type'])) {
            $message = [
                'message' => $_SESSION['flash_message'],
                'type' => $_SESSION['flash_type']
            ];
            
            unset($_SESSION['flash_message']);
            unset($_SESSION['flash_type']);
            
            return $message;
        }
        
        return null;
    }
    
    /**
     * Redirect to URL
     * @param string $url URL to redirect to
     * @return void
     */
    public static function redirect($url) {
        header("Location: $url");
        exit;
    }
    
    /**
     * Check if request is AJAX
     * @return bool True if request is AJAX, false otherwise
     */
    public static function isAjaxRequest() {
        return !empty($_SERVER['HTTP_X_REQUESTED_WITH']) && strtolower($_SERVER['HTTP_X_REQUESTED_WITH']) === 'xmlhttprequest';
    }
    
    /**
     * Send JSON response
     * @param mixed $data Data to send
     * @param int $statusCode HTTP status code
     * @return void
     */
    public static function sendJsonResponse($data, $statusCode = 200) {
        http_response_code($statusCode);
        header('Content-Type: application/json');
        echo json_encode($data);
        exit;
    }
    
    /**
     * Log error
     * @param string $message Error message
     * @param string $file File where error occurred
     * @param int $line Line where error occurred
     * @param array $context Additional context
     * @return void
     */
    public static function logError($message, $file = '', $line = 0, $context = []) {
        $logFile = LOG_FILE;
        $logDir = dirname($logFile);
        
        // Create log directory if it doesn't exist
        if (!is_dir($logDir)) {
            mkdir($logDir, 0755, true);
        }
        
        // Format log message
        $timestamp = date('Y-m-d H:i:s');
        $logMessage = "[$timestamp] [ERROR] $message";
        
        if ($file) {
            $logMessage .= " in $file";
            
            if ($line) {
                $logMessage .= " on line $line";
            }
        }
        
        if (!empty($context)) {
            $logMessage .= " | Context: " . json_encode($context);
        }
        
        $logMessage .= PHP_EOL;
        
        // Write to log file
        file_put_contents($logFile, $logMessage, FILE_APPEND);
    }
    
    /**
     * Get client IP address
     * @return string Client IP address
     */
    public static function getClientIp() {
        $ipAddress = '';
        
        if (isset($_SERVER['HTTP_CLIENT_IP'])) {
            $ipAddress = $_SERVER['HTTP_CLIENT_IP'];
        } elseif (isset($_SERVER['HTTP_X_FORWARDED_FOR'])) {
            $ipAddress = $_SERVER['HTTP_X_FORWARDED_FOR'];
        } elseif (isset($_SERVER['HTTP_X_FORWARDED'])) {
            $ipAddress = $_SERVER['HTTP_X_FORWARDED'];
        } elseif (isset($_SERVER['HTTP_FORWARDED_FOR'])) {
            $ipAddress = $_SERVER['HTTP_FORWARDED_FOR'];
        } elseif (isset($_SERVER['HTTP_FORWARDED'])) {
            $ipAddress = $_SERVER['HTTP_FORWARDED'];
        } elseif (isset($_SERVER['REMOTE_ADDR'])) {
            $ipAddress = $_SERVER['REMOTE_ADDR'];
        } else {
            $ipAddress = 'UNKNOWN';
        }
        
        return $ipAddress;
    }
    
    /**
     * Mask account number
     * @param string $accountNumber Account number to mask
     * @return string Masked account number
     */
    public static function maskAccountNumber($accountNumber) {
        $length = strlen($accountNumber);
        
        if ($length <= 4) {
            return $accountNumber;
        }
        
        $visibleChars = 4;
        $maskedPart = str_repeat('*', $length - $visibleChars);
        $visiblePart = substr($accountNumber, -$visibleChars);
        
        return $maskedPart . $visiblePart;
    }
    
    /**
     * Mask card number
     * @param string $cardNumber Card number to mask
     * @return string Masked card number
     */
    public static function maskCardNumber($cardNumber) {
        $length = strlen($cardNumber);
        
        if ($length <= 4) {
            return $cardNumber;
        }
        
        $firstFour = substr($cardNumber, 0, 4);
        $lastFour = substr($cardNumber, -4);
        $maskedPart = str_repeat('*', $length - 8);
        
        return $firstFour . ' ' . $maskedPart . ' ' . $lastFour;
    }
}
