#include "common/utils/input_validator.h"
#include "common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <regex.h>

// Card number validation (typically 6 digits in our system)
bool is_valid_card_number(int card_number) {
    // Card number must be a 6-digit number between 100000 and 999999
    return (card_number >= 100000 && card_number <= 999999);
}

// PIN validation (4 digits)
bool is_valid_pin(int pin) {
    // PIN must be a 4-digit number between 1000 and 9999
    return (pin >= 1000 && pin <= 9999);
}

// Amount validation for withdrawals and deposits
bool is_valid_amount(double amount) {
    // Amount must be positive and have at most 2 decimal places
    if (amount <= 0) {
        return false;
    }
    
    // Check if it has more than 2 decimal places
    double integer_part;
    double fractional_part = modf(amount, &integer_part);
    
    // Multiply by 100 and check if it's an integer
    double scaled = fractional_part * 100.0;
    double scaled_integer_part;
    double scaled_fractional_part = modf(scaled, &scaled_integer_part);
    
    // Allow a small epsilon for floating point imprecision
    return fabs(scaled_fractional_part) < 0.00001;
}

// Money format validation (e.g., "$123.45")
bool is_valid_money_format(const char* money_string) {
    if (!money_string) {
        return false;
    }
    
    // Skip optional currency symbol
    const char* ptr = money_string;
    if (*ptr == '$' || *ptr == '₹' || *ptr == '£' || *ptr == '€') {
        ptr++;
    }
    
    // Skip leading spaces
    while (isspace(*ptr)) {
        ptr++;
    }
    
    // Must have at least one digit before a decimal point
    if (!isdigit(*ptr)) {
        return false;
    }
    
    // Parse digits before decimal point
    while (isdigit(*ptr)) {
        ptr++;
    }
    
    // If there is a decimal point
    if (*ptr == '.') {
        ptr++;
        
        // Must have digits after decimal point
        if (!isdigit(*ptr)) {
            return false;
        }
        
        // Parse up to 2 digits after decimal point
        int decimal_places = 0;
        while (isdigit(*ptr)) {
            ptr++;
            decimal_places++;
            if (decimal_places > 2) {
                return false;
            }
        }
    }
    
    // Skip trailing spaces
    while (isspace(*ptr)) {
        ptr++;
    }
    
    // Must reach the end of string
    return *ptr == '\0';
}

// Email format validation
bool is_valid_email(const char* email) {
    if (!email) {
        return false;
    }
    
    // Basic pattern matching for email
    regex_t regex;
    int result = regcomp(&regex, "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$", REG_EXTENDED);
    
    if (result != 0) {
        write_error_log("Failed to compile email regex pattern");
        return false;
    }
    
    result = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0;
}

// Phone number format validation
bool is_valid_phone(const char* phone) {
    if (!phone) {
        return false;
    }
    
    // Allow variations like:
    // 1234567890, 123-456-7890, (123) 456-7890, +1 123-456-7890
    regex_t regex;
    int result = regcomp(&regex, "^\\+?[0-9]{0,3}[- ]?(\\([0-9]{3}\\)|[0-9]{3})[- ]?[0-9]{3}[- ]?[0-9]{4}$", REG_EXTENDED);
    
    if (result != 0) {
        write_error_log("Failed to compile phone regex pattern");
        return false;
    }
    
    result = regexec(&regex, phone, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0;
}

// Date format validation (YYYY-MM-DD)
bool is_valid_date(const char* date) {
    if (!date) {
        return false;
    }
    
    // Check format YYYY-MM-DD
    regex_t regex;
    int result = regcomp(&regex, "^[0-9]{4}-[0-9]{2}-[0-9]{2}$", REG_EXTENDED);
    
    if (result != 0) {
        write_error_log("Failed to compile date regex pattern");
        return false;
    }
    
    result = regexec(&regex, date, 0, NULL, 0);
    regfree(&regex);
    
    if (result != 0) {
        return false;
    }
    
    // Extract year, month, day
    int year, month, day;
    if (sscanf(date, "%d-%d-%d", &year, &month, &day) != 3) {
        return false;
    }
    
    // Validate month and day ranges
    if (month < 1 || month > 12) {
        return false;
    }
    
    // Days per month (ignoring leap years for simplicity)
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Adjust February for leap years
    if (month == 2) {
        if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
            days_in_month[2] = 29;
        }
    }
    
    return (day >= 1 && day <= days_in_month[month]);
}

// Clear input buffer (when reading from console)
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Get a validated integer input from user
int get_validated_int(const char* prompt, int min, int max) {
    int input;
    int valid = 0;
    
    do {
        printf("%s [%d-%d]: ", prompt, min, max);
        
        if (scanf("%d", &input) != 1) {
            printf("Invalid input. Please enter a number.\n");
            clear_input_buffer();
            continue;
        }
        
        clear_input_buffer();
        
        if (input < min || input > max) {
            printf("Input must be between %d and %d.\n", min, max);
            continue;
        }
        
        valid = 1;
    } while (!valid);
    
    return input;
}

// Get a validated double input from user
double get_validated_double(const char* prompt, double min, double max) {
    double input;
    int valid = 0;
    
    do {
        printf("%s [%.2f-%.2f]: ", prompt, min, max);
        
        if (scanf("%lf", &input) != 1) {
            printf("Invalid input. Please enter a number.\n");
            clear_input_buffer();
            continue;
        }
        
        clear_input_buffer();
        
        if (input < min || input > max) {
            printf("Input must be between %.2f and %.2f.\n", min, max);
            continue;
        }
        
        // Validate decimal places
        if (!is_valid_amount(input)) {
            printf("Input must have at most 2 decimal places.\n");
            continue;
        }
        
        valid = 1;
    } while (!valid);
    
    return input;
}

// Get a validated string input from user
void get_validated_string(const char* prompt, char* buffer, int max_length) {
    int valid = 0;
    
    do {
        printf("%s: ", prompt);
        
        if (fgets(buffer, max_length, stdin) == NULL) {
            printf("Error reading input.\n");
            continue;
        }
        
        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
            len--;
        }
        
        // Check if input is empty
        if (len == 0) {
            printf("Input cannot be empty.\n");
            continue;
        }
        
        valid = 1;
    } while (!valid);
}

// Get a secured (non-echoed) input like PIN or password
void get_secured_input(const char* prompt, char* buffer, int max_length) {
    printf("%s: ", prompt);
    
    // In a real implementation, you would use platform-specific code
    // to disable echo (e.g., termios.h on Unix or conio.h on Windows)
    
    // For this simplified version, we're just reading normally
    if (fgets(buffer, max_length, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    
    // Remove trailing newline
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }
    
    // Print a newline because the input was not echoed
    printf("\n");
}