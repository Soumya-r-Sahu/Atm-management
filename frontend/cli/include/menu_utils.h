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
 * @brief Print header with formatting
 * @param title Header title text
 */
void printHeader(const char* title);

/**
 * @brief Print text centered on screen
 * @param text Text to center
 */
void printCentered(const char* text);

/**
 * @brief Get password input (masked)
 * @param password Buffer to store the password
 * @param size Size of the buffer
 */
void getPassword(char* password, size_t size);

#endif /* MENU_UTILS_H */
