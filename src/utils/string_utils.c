#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Function to trim whitespace from both ends of a string
char* trim_string(char* str) {
    if (str == NULL) return NULL;
    
    // Trim leading whitespace
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    
    if (*start == 0) { // All spaces
        *str = 0;
        return str;
    }
    
    // Trim trailing whitespace
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    // Null terminate the trimmed string
    *(end + 1) = 0;
    
    // If there was leading space, move the trimmed string
    if (start != str) memmove(str, start, strlen(start) + 1);
    
    return str;
}

// Function to convert a string to lowercase
char* str_to_lower(char* str) {
    if (str == NULL) return NULL;
    
    for (char* p = str; *p; p++) {
        *p = tolower((unsigned char)*p);
    }
    
    return str;
}

// Function to convert a string to uppercase
char* str_to_upper(char* str) {
    if (str == NULL) return NULL;
    
    for (char* p = str; *p; p++) {
        *p = toupper((unsigned char)*p);
    }
    
    return str;
}

// Function to check if a string contains only digits
int is_numeric(const char* str) {
    if (str == NULL || *str == '\0') return 0;
    
    while (*str) {
        if (!isdigit((unsigned char)*str)) return 0;
        str++;
    }
    
    return 1;
}

// Function to check if two strings are equal (case insensitive)
int str_equals_ignore_case(const char* str1, const char* str2) {
    if (str1 == NULL || str2 == NULL) return 0;
    
    while (*str1 && *str2) {
        if (tolower((unsigned char)*str1) != tolower((unsigned char)*str2)) {
            return 0;
        }
        str1++;
        str2++;
    }
    
    return *str1 == *str2; // Both strings end at the same position
}

// Function to safely copy a string with bounds checking
char* safe_strcpy(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || src == NULL || dest_size == 0) return NULL;
    
    size_t src_len = strlen(src);
    size_t copy_len = src_len < dest_size - 1 ? src_len : dest_size - 1;
    
    memcpy(dest, src, copy_len);
    dest[copy_len] = '\0';
    
    return dest;
}