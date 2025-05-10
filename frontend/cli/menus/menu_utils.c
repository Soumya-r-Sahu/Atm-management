/**
 * @file menu_utils.c
 * @brief Implementation of utility functions for the Core Banking System menu structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <termios.h>
#include "../../../include/frontend/cli/menu_utils.h"

/**
 * @brief Clear the console screen
 */
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief Print a header with centered text
 * @param text The text to center in the header
 */
void printHeader(const char *text) {
    const int width = SCREEN_WIDTH;
    int padding = (width - strlen(text)) / 2;
    padding = padding > 0 ? padding : 0;
    
    printLine('=', width);
    printf("%*s%s%*s\n", padding, "", text, padding, "");
    printLine('=', width);
}

/**
 * @brief Print a line of characters
 * @param ch The character to repeat
 * @param width The number of times to repeat the character
 */
void printLine(char ch, int width) {
    for (int i = 0; i < width; i++) {
        putchar(ch);
    }
    putchar('\n');
}

/**
 * @brief Print a success message
 * @param message The success message to display
 */
void printSuccess(const char *message) {
    printf("\n[SUCCESS] %s\n", message);
}

/**
 * @brief Print an error message
 * @param message The error message to display
 */
void printError(const char *message) {
    printf("\n[ERROR] %s\n", message);
}

/**
 * @brief Print an information message
 * @param message The information message to display
 */
void printInfo(const char *message) {
    printf("\n[INFO] %s\n", message);
}

/**
 * @brief Print a warning message
 * @param message The warning message to display
 */
void printWarning(const char *message) {
    printf("\n[WARNING] %s\n", message);
}

/**
 * @brief Pause program execution until the user presses Enter
 */
void pauseExecution() {
    printf("\nPress Enter to continue...");
    // Clear input buffer first
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    getchar(); // Wait for Enter key
}

/**
 * @brief Get a string input from the user
 * @param prompt The prompt to display
 * @param buffer The buffer to store the input
 * @param size The size of the buffer
 */
void getString(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    fgets(buffer, size, stdin);
    
    // Remove trailing newline
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
}

/**
 * @brief Get a password input from the user (with masked characters)
 * @param prompt The prompt to display
 * @param buffer The buffer to store the input
 * @param size The size of the buffer
 */
void getPassword(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    
#ifdef _WIN32
    // Windows implementation
    int i = 0;
    char ch;
    while (i < size - 1) {
        ch = _getch();
        if (ch == '\r' || ch == '\n') {
            break;
        } else if (ch == '\b' && i > 0) {
            printf("\b \b");
            i--;
        } else if (isprint(ch)) {
            buffer[i++] = ch;
            printf("*");
        }
    }
    buffer[i] = '\0';
    printf("\n");
#else
    // Unix/Linux implementation
    struct termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    
    fgets(buffer, size, stdin);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    
    // Remove trailing newline
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    printf("\n");
#endif
}

/**
 * @brief Get an integer input from the user within a specified range
 * @param prompt The prompt to display
 * @param min The minimum valid value
 * @param max The maximum valid value
 * @return The integer input from the user
 */
int getInteger(const char *prompt, int min, int max) {
    int value;
    char input[50];
    
    while (1) {
        printf("%s (%d-%d): ", prompt, min, max);
        fgets(input, sizeof(input), stdin);
        
        if (sscanf(input, "%d", &value) == 1 && value >= min && value <= max) {
            return value;
        }
        
        printf("Invalid input. Please enter a number between %d and %d.\n", min, max);
    }
}

/**
 * @brief Get a floating-point input from the user within a specified range
 * @param prompt The prompt to display
 * @param min The minimum valid value
 * @param max The maximum valid value
 * @return The floating-point input from the user
 */
float getFloat(const char *prompt, float min, float max) {
    float value;
    char input[50];
    
    while (1) {
        printf("%s (%.2f-%.2f): ", prompt, min, max);
        fgets(input, sizeof(input), stdin);
        
        if (sscanf(input, "%f", &value) == 1 && value >= min && value <= max) {
            return value;
        }
        
        printf("Invalid input. Please enter a number between %.2f and %.2f.\n", min, max);
    }
}

/**
 * @brief Get a confirmation from the user (Y/N)
 * @param prompt The prompt to display
 * @return true if the user confirms, false otherwise
 */
bool getConfirmation(const char *prompt) {
    char input[10];
    
    while (1) {
        printf("%s (y/n): ", prompt);
        fgets(input, sizeof(input), stdin);
        
        // Convert to lowercase and check first character
        input[0] = tolower(input[0]);
        
        if (input[0] == 'y') {
            return true;
        } else if (input[0] == 'n') {
            return false;
        }
        
        printf("Invalid input. Please enter 'y' or 'n'.\n");
    }
}

/**
 * @brief Format a currency value
 * @param amount The amount to format
 * @param buffer The buffer to store the formatted amount
 * @param size The size of the buffer
 * @param currencySymbol The currency symbol to use
 */
void formatCurrency(float amount, char *buffer, size_t size, const char *currencySymbol) {
    snprintf(buffer, size, "%s %.2f", currencySymbol, amount);
}

/**
 * @brief Format a date (YYYY-MM-DD)
 * @param year The year
 * @param month The month
 * @param day The day
 * @param buffer The buffer to store the formatted date
 * @param size The size of the buffer
 */
void formatDate(int year, int month, int day, char *buffer, size_t size) {
    snprintf(buffer, size, "%04d-%02d-%02d", year, month, day);
}

/**
 * @brief Format a time (HH:MM:SS)
 * @param hour The hour
 * @param minute The minute
 * @param second The second
 * @param buffer The buffer to store the formatted time
 * @param size The size of the buffer
 */
void formatTime(int hour, int minute, int second, char *buffer, size_t size) {
    snprintf(buffer, size, "%02d:%02d:%02d", hour, minute, second);
}

/**
 * @brief Get a date input from the user
 * @param prompt The prompt to display
 * @param year Pointer to store the year
 * @param month Pointer to store the month
 * @param day Pointer to store the day
 */
void getDate(const char *prompt, int *year, int *month, int *day) {
    char input[20];
    
    while (1) {
        printf("%s (YYYY-MM-DD): ", prompt);
        fgets(input, sizeof(input), stdin);
        
        // Remove trailing newline
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }
        
        if (sscanf(input, "%d-%d-%d", year, month, day) == 3) {
            // Validate date
            if (*year >= 1900 && *year <= 2100 && 
                *month >= 1 && *month <= 12 && 
                *day >= 1 && *day <= 31) {
                
                // Further validate days in month (simplified)
                int daysInMonth;
                switch (*month) {
                    case 2: // February
                        daysInMonth = (*year % 4 == 0 && (*year % 100 != 0 || *year % 400 == 0)) ? 29 : 28;
                        break;
                    case 4: case 6: case 9: case 11: // 30 days
                        daysInMonth = 30;
                        break;
                    default: // 31 days
                        daysInMonth = 31;
                        break;
                }
                
                if (*day <= daysInMonth) {
                    return;
                }
            }
        }
        
        printf("Invalid date format or value. Please use YYYY-MM-DD format with valid date.\n");
    }
}

/**
 * @brief Display a menu and get the user's choice
 * @param title The menu title
 * @param options Array of menu options
 * @param numOptions The number of menu options
 * @return The user's choice (1 to numOptions)
 */
int displayMenu(const char *title, const char *options[], int numOptions) {
    clearScreen();
    printHeader(title);
    
    printf("\n");
    for (int i = 0; i < numOptions; i++) {
        printf("%d. %s\n", i + 1, options[i]);
    }
    
    return getInteger("\nEnter your choice", 1, numOptions);
}
