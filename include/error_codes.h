/**
 * @file error_codes.h
 * @brief Error codes for the Core Banking System
 */

#ifndef CBS_ERROR_CODES_H
#define CBS_ERROR_CODES_H

// API error codes
#define API_SUCCESS 0
#define API_ERROR_GENERAL -1
#define API_ERROR_INVALID_PARAMS -2
#define API_ERROR_NOT_FOUND -3
#define API_ERROR_INSUFFICIENT_FUNDS -4
#define API_ERROR_UNAUTHORIZED -5
#define API_ERROR_ACCOUNT_BLOCKED -6
#define API_ERROR_DUPLICATE_ENTRY -7
#define API_ERROR_CONNECTION -8
#define API_ERROR_TIMEOUT -9
#define API_ERROR_DATABASE -10

// Database error codes
#define DB_SUCCESS 0
#define DB_ERROR_GENERAL -1
#define DB_ERROR_CONNECTION -2
#define DB_ERROR_QUERY -3
#define DB_ERROR_NOT_FOUND -4
#define DB_ERROR_DUPLICATE -5
#define DB_ERROR_CONSTRAINT -6
#define DB_ERROR_TRANSACTION -7

// System error codes
#define SYS_SUCCESS 0
#define SYS_ERROR_GENERAL -1
#define SYS_ERROR_MEMORY -2
#define SYS_ERROR_FILE_ACCESS -3
#define SYS_ERROR_CONFIG -4
#define SYS_ERROR_INITIALIZATION -5

// Menu error codes
#define MENU_SUCCESS 0
#define MENU_ERROR_INVALID_OPTION -1
#define MENU_ERROR_UNAUTHORIZED -2
#define MENU_ERROR_SESSION_EXPIRED -3
#define MENU_ERROR_NAVIGATION -4

#endif /* CBS_ERROR_CODES_H */
