/**
 * @file string_utils.c
 * @brief Implementation of string utility functions for the ATM Management System
 * @date May 2, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <common/utils/string_utils.h>

// Function to trim whitespace from both ends of a string
char* trim_string(char* str) {
    if (str == NULL) {
        return NULL;
    }
    
    // Trim leading spaces
    char* start = str;
    while (isspace((unsigned char)*start)) {
        start++;
    }
    
    // If the entire string is spaces
    if (*start == '\0') {
        *str = '\0';
        return str;
    }
    
    // Trim trailing spaces
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    
    // Null terminate the trimmed string
    *(end + 1) = '\0';
    
    // If there were leading spaces, move the trimmed string to the beginning
    if (start != str) {
        memmove(str, start, (end - start + 2) * sizeof(char));
    }
    
    return str;
}

// Function to convert a string to lowercase
char* str_to_lower(char* str) {
    if (str == NULL) {
        return NULL;
    }
    
    char* p = str;
    while (*p) {
        *p = tolower((unsigned char)*p);
        p++;
    }
    
    return str;
}

// Function to convert a string to uppercase
char* str_to_upper(char* str) {
    if (str == NULL) {
        return NULL;
    }
    
    char* p = str;
    while (*p) {
        *p = toupper((unsigned char)*p);
        p++;
    }
    
    return str;
}

// Function to check if a string contains only digits
int is_numeric(const char* str) {
    if (str == NULL || *str == '\0') {
        return 0; // NULL or empty string is not numeric
    }
    
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return 0; // Non-digit found
        }
        str++;
    }
    
    return 1; // All characters are digits
}

// Function to check if two strings are equal (case insensitive)
int str_equals_ignore_case(const char* str1, const char* str2) {
    if (str1 == NULL || str2 == NULL) {
        return str1 == str2; // Both NULL means equal, one NULL means not equal
    }
    
    while (*str1 && *str2) {
        if (tolower((unsigned char)*str1) != tolower((unsigned char)*str2)) {
            return 0; // Characters differ
        }
        str1++;
        str2++;
    }
    
    return *str1 == *str2; // Both should be at the end (equal length)
}

// Function to safely copy a string with bounds checking
char* safe_strcpy(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return NULL;
    }
    
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i]; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0'; // Null-terminate
    
    return dest;
}
