/**
 * @file menu_utils.h
 * @brief Utility functions for CLI menus
 */

#ifndef MENU_UTILS_H
#define MENU_UTILS_H

#include <stdbool.h>

/**
 * @brief Clear the screen
 */
void clearScreen(void);

/**
 * @brief Print a header with a border
 * @param title Header title
 */
void printHeader(const char *title);

/**
 * @brief Print text centered on the screen
 * @param text Text to print
 */
void printCentered(const char *text);

/**
 * @brief Get password input without echoing to the screen
 * @param password Buffer to store password
 * @param size Size of buffer
 */
void getPassword(char *password, size_t size);

/**
 * @brief Print a line of characters
 * @param character Character to print
 * @param length Length of line
 */
void printLine(char character, int length);

/**
 * @brief Print a formatted currency amount
 * @param amount Amount to print
 * @param currency Currency symbol
 */
void printCurrency(double amount, const char *currency);

/**
 * @brief Print a formatted date
 * @param date Date string in YYYY-MM-DD format
 */
void printDate(const char *date);

/**
 * @brief Print a formatted datetime
 * @param datetime Datetime string in YYYY-MM-DD HH:MM:SS format
 */
void printDateTime(const char *datetime);

/**
 * @brief Get confirmation from user (Y/N)
 * @param prompt Prompt to display
 * @return bool True if confirmed, false otherwise
 */
bool getConfirmation(const char *prompt);

/**
 * @brief Pause execution until user presses Enter
 */
void pauseExecution(void);

/**
 * @brief Get a string input from user
 * @param prompt Prompt to display
 * @param buffer Buffer to store input
 * @param size Size of buffer
 */
void getString(const char *prompt, char *buffer, size_t size);

/**
 * @brief Get an integer input from user
 * @param prompt Prompt to display
 * @param min Minimum value
 * @param max Maximum value
 * @return int User input
 */
int getInteger(const char *prompt, int min, int max);

/**
 * @brief Get a double input from user
 * @param prompt Prompt to display
 * @param min Minimum value
 * @param max Maximum value
 * @return double User input
 */
double getDouble(const char *prompt, double min, double max);

/**
 * @brief Print an error message
 * @param message Error message
 */
void printError(const char *message);

/**
 * @brief Print a success message
 * @param message Success message
 */
void printSuccess(const char *message);

/**
 * @brief Print a warning message
 * @param message Warning message
 */
void printWarning(const char *message);

/**
 * @brief Print an info message
 * @param message Info message
 */
void printInfo(const char *message);

#endif /* MENU_UTILS_H */
