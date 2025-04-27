#ifndef INPUT_VALIDATOR_H
#define INPUT_VALIDATOR_H

/**
 * Validate numeric input within specified range
 * 
 * @param value Value to validate
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @return 1 if valid, 0 if invalid
 */
int validate_int_range(int value, int min, int max);

/**
 * Validate a floating point value within specified range
 * 
 * @param value Value to validate
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @return 1 if valid, 0 if invalid
 */
int validate_float_range(float value, float min, float max);

/**
 * Validate if a string contains only digits
 * 
 * @param str String to validate
 * @return 1 if valid, 0 if invalid
 */
int validate_digits(const char* str);

/**
 * Validate if a string contains only alphanumeric characters
 * 
 * @param str String to validate
 * @return 1 if valid, 0 if invalid
 */
int validate_alnum(const char* str);

/**
 * Validate if a string matches the format for a card number
 * 
 * @param str String to validate
 * @return 1 if valid, 0 if invalid
 */
int validate_card_number(const char* str);

/**
 * Validate if a string matches the format for a PIN
 * 
 * @param str String to validate
 * @return 1 if valid, 0 if invalid
 */
int validate_pin(const char* str);

/**
 * Validate if a string matches the format for a name (allows letters, spaces, and hyphens)
 * 
 * @param str String to validate
 * @return 1 if valid, 0 if invalid
 */
int validate_name(const char* str);

/**
 * Validate if a string matches the format for a phone number
 * 
 * @param str String to validate
 * @return 1 if valid, 0 if invalid
 */
int validate_phone(const char* str);

/**
 * Validate if a string matches the format for an email address
 * 
 * @param str String to validate
 * @return 1 if valid, 0 if invalid
 */
int validate_email(const char* str);

/**
 * Validate if a string is within a specified length range
 * 
 * @param str String to validate
 * @param min_length Minimum allowed length
 * @param max_length Maximum allowed length
 * @return 1 if valid, 0 if invalid
 */
int validate_string_length(const char* str, size_t min_length, size_t max_length);

/**
 * Sanitize a string by removing unsafe characters
 * 
 * @param str String to sanitize
 * @return New sanitized string (must be freed by caller) or NULL on error
 */
char* sanitize_string(const char* str);

/**
 * Read a line from stdin with proper bounds checking
 * 
 * @param buffer Buffer to store input
 * @param size Size of buffer
 * @return 1 on success, 0 on failure
 */
int read_line(char* buffer, size_t size);

/**
 * Read an integer from stdin with validation
 * 
 * @param value Pointer to store the integer
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @param prompt Prompt message (optional, can be NULL)
 * @param error_msg Error message (optional, can be NULL)
 * @return 1 on success, 0 on failure
 */
int read_int(int* value, int min, int max, const char* prompt, const char* error_msg);

/**
 * Read a float from stdin with validation
 * 
 * @param value Pointer to store the float
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @param prompt Prompt message (optional, can be NULL)
 * @param error_msg Error message (optional, can be NULL)
 * @return 1 on success, 0 on failure
 */
int read_float(float* value, float min, float max, const char* prompt, const char* error_msg);

#endif // INPUT_VALIDATOR_H