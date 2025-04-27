#include "input_validator.h"
#include "../common/error_handler.h"
#include "../utils/memory_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

// Validate numeric input within specified range
int validate_int_range(int value, int min, int max) {
    return (value >= min && value <= max);
}

// Validate a floating point value within specified range
int validate_float_range(float value, float min, float max) {
    return (value >= min && value <= max);
}

// Validate if a string contains only digits
int validate_digits(const char* str) {
    if (!str) return 0;
    
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    
    return 1;
}

// Validate if a string contains only alphanumeric characters
int validate_alnum(const char* str) {
    if (!str) return 0;
    
    while (*str) {
        if (!isalnum(*str)) {
            return 0;
        }
        str++;
    }
    
    return 1;
}

// Validate if a string matches the format for a card number (6 digits)
int validate_card_number(const char* str) {
    if (!str) return 0;
    
    // Check if string is exactly 6 digits
    if (strlen(str) != 6) {
        return 0;
    }
    
    return validate_digits(str);
}

// Validate if a string matches the format for a PIN (4-6 digits)
int validate_pin(const char* str) {
    if (!str) return 0;
    
    // Check if string length is between 4 and 6 digits
    size_t len = strlen(str);
    if (len < 4 || len > 6) {
        return 0;
    }
    
    return validate_digits(str);
}

// Validate if a string matches the format for a name
int validate_name(const char* str) {
    if (!str) return 0;
    
    while (*str) {
        if (!isalpha(*str) && *str != ' ' && *str != '-' && *str != '.') {
            return 0;
        }
        str++;
    }
    
    return 1;
}

// Validate if a string matches the format for a phone number
int validate_phone(const char* str) {
    if (!str) return 0;
    
    // Simple check: 10+ digits, possibly with spaces, hyphens, or parentheses
    size_t len = strlen(str);
    int digit_count = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (isdigit(str[i])) {
            digit_count++;
        } else if (str[i] != ' ' && str[i] != '-' && str[i] != '(' && str[i] != ')' && str[i] != '+') {
            return 0;  // Invalid character for phone number
        }
    }
    
    return (digit_count >= 10);
}

// Validate if a string matches the format for an email address
int validate_email(const char* str) {
    if (!str) return 0;
    
    // Use regex for email validation
    regex_t regex;
    int reti;
    
    // Simple email regex pattern
    reti = regcomp(&regex, "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$", REG_EXTENDED);
    if (reti) {
        SET_ERROR(ERR_SYSTEM, "Could not compile email regex");
        return 0;
    }
    
    reti = regexec(&regex, str, 0, NULL, 0);
    regfree(&regex);
    
    return (reti == 0);
}

// Validate if a string is within a specified length range
int validate_string_length(const char* str, size_t min_length, size_t max_length) {
    if (!str) return (min_length == 0);  // NULL string is valid only if min_length is 0
    
    size_t len = strlen(str);
    return (len >= min_length && len <= max_length);
}

// Sanitize a string by removing unsafe characters
char* sanitize_string(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    char* sanitized = (char*)MALLOC(len + 1, "Sanitized string");
    
    if (!sanitized) {
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for sanitized string");
        return NULL;
    }
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        // Keep alphanumeric characters and safe punctuation
        if (isalnum(str[i]) || strchr(" .,;:-_@", str[i]) != NULL) {
            sanitized[j++] = str[i];
        }
    }
    
    sanitized[j] = '\0';
    return sanitized;
}

// Read a line from stdin with proper bounds checking
int read_line(char* buffer, size_t size) {
    if (!buffer || size == 0) {
        SET_ERROR(ERR_INVALID_INPUT, "Invalid buffer for read_line");
        return 0;
    }
    
    // Clear buffer
    buffer[0] = '\0';
    
    // Read line
    if (!fgets(buffer, size, stdin)) {
        if (ferror(stdin)) {
            SET_ERROR(ERR_INVALID_INPUT, "Error reading input");
        }
        return 0;
    }
    
    // Remove trailing newline if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    // Check if input was too long (no newline found)
    if (len == size - 1 && buffer[len - 1] != '\n') {
        // Clear the remaining input
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        SET_ERROR(ERR_INVALID_INPUT, "Input too long");
        return 0;
    }
    
    return 1;
}

// Read an integer from stdin with validation
int read_int(int* value, int min, int max, const char* prompt, const char* error_msg) {
    if (!value) {
        SET_ERROR(ERR_INVALID_INPUT, "Invalid pointer for read_int");
        return 0;
    }
    
    char buffer[32];
    int attempts = 0;
    const int max_attempts = 3;
    
    while (attempts < max_attempts) {
        // Display prompt if provided
        if (prompt) {
            printf("%s", prompt);
        }
        
        if (!read_line(buffer, sizeof(buffer))) {
            attempts++;
            continue;
        }
        
        // Try to convert to integer
        char* endptr;
        long val = strtol(buffer, &endptr, 10);
        
        // Check conversion
        if (*endptr != '\0' || val < min || val > max || val != (int)val) {
            if (error_msg) {
                printf("%s\n", error_msg);
            } else {
                printf("Invalid input. Please enter a number between %d and %d.\n", min, max);
            }
            attempts++;
            continue;
        }
        
        *value = (int)val;
        return 1;
    }
    
    SET_ERROR(ERR_INVALID_INPUT, "Maximum input attempts exceeded");
    return 0;
}

// Read a float from stdin with validation
int read_float(float* value, float min, float max, const char* prompt, const char* error_msg) {
    if (!value) {
        SET_ERROR(ERR_INVALID_INPUT, "Invalid pointer for read_float");
        return 0;
    }
    
    char buffer[32];
    int attempts = 0;
    const int max_attempts = 3;
    
    while (attempts < max_attempts) {
        // Display prompt if provided
        if (prompt) {
            printf("%s", prompt);
        }
        
        if (!read_line(buffer, sizeof(buffer))) {
            attempts++;
            continue;
        }
        
        // Try to convert to float
        char* endptr;
        float val = strtof(buffer, &endptr);
        
        // Check conversion
        if (*endptr != '\0' || val < min || val > max) {
            if (error_msg) {
                printf("%s\n", error_msg);
            } else {
                printf("Invalid input. Please enter a number between %.2f and %.2f.\n", min, max);
            }
            attempts++;
            continue;
        }
        
        *value = val;
        return 1;
    }
    
    SET_ERROR(ERR_INVALID_INPUT, "Maximum input attempts exceeded");
    return 0;
}