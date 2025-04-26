#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

// Function to trim whitespace from both ends of a string
char* trim_string(char* str);

// Function to convert a string to lowercase
char* str_to_lower(char* str);

// Function to convert a string to uppercase
char* str_to_upper(char* str);

// Function to check if a string contains only digits
int is_numeric(const char* str);

// Function to check if two strings are equal (case insensitive)
int str_equals_ignore_case(const char* str1, const char* str2);

// Function to safely copy a string with bounds checking
char* safe_strcpy(char* dest, size_t dest_size, const char* src);

#endif // STRING_UTILS_H