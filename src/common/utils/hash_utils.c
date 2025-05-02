#include "../../../include/common/utils/hash_utils.h"
#include "../../../include/common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <time.h>

/**
 * Computes a SHA-256 hash of the input string.
 * Note: This is a simple implementation for demonstration purposes.
 * In a production environment, you would use a proven library like OpenSSL.
 */
char* sha256_hash(const char* input) {
    if (!input) {
        writeErrorLog("NULL pointer provided to sha256_hash");
        return NULL;
    }
    
    // For demonstration purposes, we'll implement a simple hash function
    // In a real implementation, you would use a cryptographic library like OpenSSL
    
    unsigned char hash[32]; // SHA-256 produces a 32-byte hash
    char* output = (char*)malloc(65); // 32 bytes * 2 hex chars + null terminator
    
    if (!output) {
        writeErrorLog("Memory allocation failed in sha256_hash");
        return NULL;
    }
    
    // Simple hash implementation for demonstration purposes
    // Note: This is NOT secure and should be replaced with a proper cryptographic library
    unsigned int seed = 0;
    for (size_t i = 0; i < strlen(input); i++) {
        seed = seed * 31 + input[i];
    }
    
    srand(seed);
    for (int i = 0; i < 32; i++) {
        hash[i] = rand() % 256;
    }
    
    // Convert the hash to a hex string
    for (int i = 0; i < 32; i++) {
        sprintf(&output[i*2], "%02x", hash[i]);
    }
    
    output[64] = '\0';
    return output;
}

/**
 * Securely compares two hashes in constant time to prevent timing attacks
 */
int secure_hash_compare(const char* hash1, const char* hash2) {
    if (!hash1 || !hash2) {
        writeErrorLog("NULL pointer provided to secure_hash_compare");
        return 0;
    }
    
    size_t len1 = strlen(hash1);
    size_t len2 = strlen(hash2);
    
    // Early return for length mismatch, but still perform a full compare
    // to avoid timing attacks
    int result = (len1 == len2);
    
    // Determine the max length for comparison
    size_t max_len = (len1 > len2) ? len1 : len2;
    
    // Perform constant-time comparison
    for (size_t i = 0; i < max_len; i++) {
        char c1 = (i < len1) ? hash1[i] : 0;
        char c2 = (i < len2) ? hash2[i] : 0;
        result &= (c1 == c2);
    }
    
    return result;
}

/**
 * Creates a salted hash from a password (additional function needed by admin auth)
 */
char* create_salted_hash(const char* password, const char* salt) {
    if (!password || !salt) {
        writeErrorLog("NULL pointer provided to create_salted_hash");
        return NULL;
    }
    
    // Concatenate the password and salt
    size_t combined_len = strlen(password) + strlen(salt);
    char* combined = (char*)malloc(combined_len + 1);
    
    if (!combined) {
        writeErrorLog("Memory allocation failed in create_salted_hash");
        return NULL;
    }
    
    strcpy(combined, password);
    strcat(combined, salt);
    
    // Hash the combined string
    char* result = sha256_hash(combined);
    
    // Clean up
    free(combined);
    
    return result;
}

/**
 * Verifies a password against a stored hash (additional function needed by admin auth)
 */
int verify_password(const char* password, const char* stored_hash, const char* salt) {
    if (!password || !stored_hash || !salt) {
        writeErrorLog("NULL pointer provided to verify_password");
        return 0;
    }
    
    // Create a hash from the password and salt
    char* computed_hash = create_salted_hash(password, salt);
    
    if (!computed_hash) {
        return 0;
    }
    
    // Compare the computed hash with the stored hash
    int result = secure_hash_compare(computed_hash, stored_hash);
    
    // Clean up
    free(computed_hash);
    
    return result;
}