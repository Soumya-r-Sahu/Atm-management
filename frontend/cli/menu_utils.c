/**
 * @file menu_utils.c
 * @brief Implementation of utility functions for CLI menus
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include "../../include/frontend/cli/menu_utils.h"

#define SCREEN_WIDTH 80

/**
 * @brief Clear the screen
 */
void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief Print a header with a border
 * @param title Header title
 */
void printHeader(const char *title) {
    int titleLength = strlen(title);
    int padding = (SCREEN_WIDTH - titleLength - 4) / 2;
    
    printLine('=', SCREEN_WIDTH);
    printf("%*s", padding, "");
    printf("[ %s ]", title);
    printf("%*s\n", padding, "");
    printLine('=', SCREEN_WIDTH);
}

/**
 * @brief Print text centered on the screen
 * @param text Text to print
 */
void printCentered(const char *text) {
    int textLength = strlen(text);
    int padding = (SCREEN_WIDTH - textLength) / 2;
    printf("%*s%s%*s\n", padding, "", text, padding, "");
}

/**
 * @brief Get password input without echoing to the screen
 * @param password Buffer to store password
 * @param size Size of buffer
 */
void getPassword(char *password, size_t size) {
    struct termios old, new;
    
    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &old);
    new = old;
    
    // Disable echo
    new.c_lflag &= ~ECHO;
    
    // Set new terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
    
    // Get password
    fgets(password, size, stdin);
    
    // Remove newline
    password[strcspn(password, "\n")] = 0;
    
    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    
    // Print newline
    printf("\n");
}

/**
 * @brief Print a line of characters
 * @param character Character to print
 * @param length Length of line
 */
void printLine(char character, int length) {
    for (int i = 0; i < length; i++) {
        putchar(character);
    }
    putchar('\n');
}

/**
 * @brief Print a formatted currency amount
 * @param amount Amount to print
 * @param currency Currency symbol
 */
void printCurrency(double amount, const char *currency) {
    printf("%s %.2f", currency, amount);
}

/**
 * @brief Print a formatted date
 * @param date Date string in YYYY-MM-DD format
 */
void printDate(const char *date) {
    // Parse date
    int year, month, day;
    sscanf(date, "%d-%d-%d", &year, &month, &day);
    
    // Print formatted date
    printf("%02d/%02d/%04d", day, month, year);
}

/**
 * @brief Print a formatted datetime
 * @param datetime Datetime string in YYYY-MM-DD HH:MM:SS format
 */
void printDateTime(const char *datetime) {
    // Parse datetime
    int year, month, day, hour, minute, second;
    sscanf(datetime, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    
    // Print formatted datetime
    printf("%02d/%02d/%04d %02d:%02d:%02d", day, month, year, hour, minute, second);
}

/**
 * @brief Get confirmation from user (Y/N)
 * @param prompt Prompt to display
 * @return bool True if confirmed, false otherwise
 */
bool getConfirmation(const char *prompt) {
    char response;
    
    printf("%s (Y/N): ", prompt);
    scanf(" %c", &response);
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    return (response == 'Y' || response == 'y');
}

/**
 * @brief Pause execution until user presses Enter
 */
void pauseExecution(void) {
    printf("\nPress Enter to continue...");
    getchar();
}

/**
 * @brief Get a string input from user
 * @param prompt Prompt to display
 * @param buffer Buffer to store input
 * @param size Size of buffer
 */
void getString(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline
}

/**
 * @brief Get an integer input from user
 * @param prompt Prompt to display
 * @param min Minimum value
 * @param max Maximum value
 * @return int User input
 */
int getInteger(const char *prompt, int min, int max) {
    int value;
    char buffer[100];
    
    while (1) {
        printf("%s (%d-%d): ", prompt, min, max);
        fgets(buffer, sizeof(buffer), stdin);
        
        // Check if input is empty
        if (buffer[0] == '\n') {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        // Remove newline
        buffer[strcspn(buffer, "\n")] = 0;
        
        // Check if input is a number
        bool isNumber = true;
        for (int i = 0; buffer[i] != '\0'; i++) {
            if (!isdigit(buffer[i])) {
                isNumber = false;
                break;
            }
        }
        
        if (!isNumber) {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        // Convert to integer
        value = atoi(buffer);
        
        // Check range
        if (value < min || value > max) {
            printf("Invalid input. Please enter a number between %d and %d.\n", min, max);
            continue;
        }
        
        break;
    }
    
    return value;
}

/**
 * @brief Get a double input from user
 * @param prompt Prompt to display
 * @param min Minimum value
 * @param max Maximum value
 * @return double User input
 */
double getDouble(const char *prompt, double min, double max) {
    double value;
    char buffer[100];
    
    while (1) {
        printf("%s (%.2f-%.2f): ", prompt, min, max);
        fgets(buffer, sizeof(buffer), stdin);
        
        // Check if input is empty
        if (buffer[0] == '\n') {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        // Remove newline
        buffer[strcspn(buffer, "\n")] = 0;
        
        // Check if input is a number
        bool isNumber = true;
        bool hasDecimal = false;
        for (int i = 0; buffer[i] != '\0'; i++) {
            if (buffer[i] == '.') {
                if (hasDecimal) {
                    isNumber = false;
                    break;
                }
                hasDecimal = true;
            } else if (!isdigit(buffer[i])) {
                isNumber = false;
                break;
            }
        }
        
        if (!isNumber) {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        // Convert to double
        value = atof(buffer);
        
        // Check range
        if (value < min || value > max) {
            printf("Invalid input. Please enter a number between %.2f and %.2f.\n", min, max);
            continue;
        }
        
        break;
    }
    
    return value;
}

/**
 * @brief Print an error message
 * @param message Error message
 */
void printError(const char *message) {
    printf("\033[1;31mERROR: %s\033[0m\n", message);
}

/**
 * @brief Print a success message
 * @param message Success message
 */
void printSuccess(const char *message) {
    printf("\033[1;32mSUCCESS: %s\033[0m\n", message);
}

/**
 * @brief Print a warning message
 * @param message Warning message
 */
void printWarning(const char *message) {
    printf("\033[1;33mWARNING: %s\033[0m\n", message);
}

/**
 * @brief Print an info message
 * @param message Info message
 */
void printInfo(const char *message) {
    printf("\033[1;34mINFO: %s\033[0m\n", message);
}
