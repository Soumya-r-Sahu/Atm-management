#include "utils.h"
#include <stdio.h>
#include <stdlib.h>  // For rand() function
#include <string.h>
#include <termios.h>  // For terminal control on Linux
#include <unistd.h>   // For POSIX read

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

// Linux implementation of getch() to read single character without echo
static char linux_getch() {
    struct termios oldattr, newattr;
    char ch;
    
    // Save current terminal settings
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    
    // Disable canonical mode and echo
    newattr.c_lflag &= ~(ICANON | ECHO);
    
    // Apply new settings
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    
    // Read a single character
    read(STDIN_FILENO, &ch, 1);
    
    // Restore old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    
    return ch;
}

// Securely read a PIN without displaying it on screen
void secure_pin_entry(char *pin, int max_length) {
    int i = 0;
    char ch;
    
    while ((ch = linux_getch()) != '\n' && ch != '\r' && i < max_length - 1) {
        if (ch == 127 || ch == '\b') {  // Handle backspace (127 is DEL on Linux)
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