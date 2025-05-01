#include "common/utils/hash_utils.h"
#include "common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>  // Added for bool type

// Simple hash function for development purposes
// In a production environment, use a proper cryptographic library
char* sha256_hash(const char* input) {
    if (!input) {
        return NULL;
    }
    
    // Allocate memory for hash output (64 chars + null terminator)
    char* hash = (char*)malloc(65);
    if (!hash) {
        write_error_log("Memory allocation failed in sha256_hash");
        return NULL;
    }
    
    // Simple hash algorithm for demonstration (not secure)
    // In production, use a proper crypto library like OpenSSL
    unsigned int sum = 0;
    for (size_t i = 0; i < strlen(input); i++) {
        sum = (sum * 31 + input[i]) % 999999;
    }
    
    // Format as a hex string
    sprintf(hash, "%064x", sum);
    
    return hash;
}

// Securely compare two hashes to prevent timing attacks
int secure_hash_compare(const char* hash1, const char* hash2) {
    if (!hash1 || !hash2) {
        return 0;
    }
    
    size_t len1 = strlen(hash1);
    size_t len2 = strlen(hash2);
    
    // Different lengths mean different hashes
    if (len1 != len2) {
        return 0;
    }
    
    // Constant-time comparison to prevent timing attacks
    int result = 0;
    for (size_t i = 0; i < len1; i++) {
        result |= hash1[i] ^ hash2[i];
    }
    
    return result == 0 ? 1 : 0;
}

// Generate a random salt for password hashing
char* generate_salt(size_t length) {
    if (length == 0) {
        return NULL;
    }
    
    char* salt = (char*)malloc(length + 1);
    if (!salt) {
        write_error_log("Memory allocation failed in generate_salt");
        return NULL;
    }
    
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const size_t charset_size = sizeof(charset) - 1;
    
    for (size_t i = 0; i < length; i++) {
        salt[i] = charset[rand() % charset_size];
    }
    salt[length] = '\0';
    
    return salt;
}

// Hash a password with a salt
char* hash_password(const char* password, const char* salt) {
    if (!password || !salt) {
        return NULL;
    }
    
    // Concatenate password and salt
    size_t pass_len = strlen(password);
    size_t salt_len = strlen(salt);
    
    char* combined = (char*)malloc(pass_len + salt_len + 1);
    if (!combined) {
        write_error_log("Memory allocation failed in hash_password");
        return NULL;
    }
    
    strcpy(combined, password);
    strcat(combined, salt);
    
    // Hash the combined string
    char* hash = sha256_hash(combined);
    
    // Clean up
    free(combined);
    
    return hash;
}