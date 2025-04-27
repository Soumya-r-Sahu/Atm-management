#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <conio.h>  // For _getch() on Windows

// Validate if a string contains only allowed characters
int is_valid_string(const char *str) {
    if (!str) return 0;  // Null check
    
    // Check for empty string
    if (str[0] == '\0') return 0;
    
    // Validate string characters
    while (*str) {
        // Allow alphanumeric characters, spaces, and common punctuation
        if (!(*str >= 'a' && *str <= 'z') &&
            !(*str >= 'A' && *str <= 'Z') &&
            !(*str >= '0' && *str <= '9') &&
            !(*str == ' ' || *str == '.' || *str == ',' || 
              *str == '-' || *str == '_' || *str == '@')) {
            return 0;
        }
        str++;
    }
    
    return 1;
}

// Securely read a PIN without displaying it on screen
void secure_pin_entry(char *pin, int max_length) {
    int i = 0;
    char ch;
    
    while ((ch = _getch()) != '\r' && i < max_length - 1) {
        if (ch == '\b') {  // Handle backspace
            if (i > 0) {
                i--;
                printf("\b \b");  // Erase character on screen
            }
        } else if (ch >= '0' && ch <= '9') {  // Only accept digits
            pin[i++] = ch;
            printf("*");  // Show asterisk instead of the digit
        }
    }
    pin[i] = '\0';  // Null-terminate the string
    printf("\n");
}

// Clear input buffer
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Validate numeric input range
int validate_int_range(int value, int min, int max) {
    return (value >= min && value <= max);
}

// Generate a random number within a range
int generate_random_in_range(int min, int max) {
    return min + rand() % (max - min + 1);
}