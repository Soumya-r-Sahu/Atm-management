#include "common/security/hash_utils.h"
#include "common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

// Convert binary hash to hex string
static char* binary_to_hex(const unsigned char* binary, size_t length) {
    char* hex = (char*)malloc(length * 2 + 1);
    if (!hex) {
        writeErrorLog("Memory allocation failed in binary_to_hex");
        return NULL;
    }
    
    for (size_t i = 0; i < length; i++) {
        sprintf(hex + i * 2, "%02x", binary[i]);
    }
    hex[length * 2] = '\0';
    return hex;
}

// Generate SHA256 hash of input string
char* sha256_hash(const char* input) {
    if (!input) {
        writeErrorLog("NULL input to sha256_hash");
        return NULL;
    }
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, strlen(input));
    SHA256_Final(hash, &sha256);
    
    return binary_to_hex(hash, SHA256_DIGEST_LENGTH);
}

// Compare two hash strings securely (constant time)
bool secure_hash_compare(const char* hash1, const char* hash2) {
    if (!hash1 || !hash2) {
        writeErrorLog("NULL input to secure_hash_compare");
        return false;
    }
    
    size_t len1 = strlen(hash1);
    size_t len2 = strlen(hash2);
    
    // Different lengths means they can't be the same
    if (len1 != len2) {
        return false;
    }
    
    // Using constant-time comparison to prevent timing attacks
    int result = 0;
    for (size_t i = 0; i < len1; i++) {
        result |= hash1[i] ^ hash2[i];
    }
    
    return result == 0;
}

// Generate a salt for passwords
char* generate_salt(int length) {
    if (length <= 0) {
        writeErrorLog("Invalid length for salt");
        return NULL;
    }
    
    unsigned char* salt_bin = (unsigned char*)malloc(length);
    if (!salt_bin) {
        writeErrorLog("Memory allocation failed in generate_salt");
        return NULL;
    }
    
    // Use OpenSSL's RAND_bytes for cryptographically secure random bytes
    if (RAND_bytes(salt_bin, length) != 1) {
        writeErrorLog("Failed to generate random bytes for salt");
        free(salt_bin);
        return NULL;
    }
    
    char* salt_hex = binary_to_hex(salt_bin, length);
    free(salt_bin);
    
    return salt_hex;
}

// Create a salted and hashed password
char* create_salted_hash(const char* password, const char* salt) {
    if (!password) {
        writeErrorLog("NULL password in create_salted_hash");
        return NULL;
    }
    
    // Generate a salt if none is provided
    char* salt_to_use = NULL;
    bool salt_generated = false;
    
    if (!salt) {
        salt_to_use = generate_salt(16);
        salt_generated = true;
    } else {
        salt_to_use = strdup(salt);
    }
    
    if (!salt_to_use) {
        writeErrorLog("Failed to allocate memory for salt");
        return NULL;
    }
    
    // Concatenate salt and password
    size_t salted_len = strlen(salt_to_use) + strlen(password) + 1;
    char* salted_pwd = (char*)malloc(salted_len);
    if (!salted_pwd) {
        writeErrorLog("Memory allocation failed in create_salted_hash");
        if (salt_generated) {
            free(salt_to_use);
        }
        return NULL;
    }
    
    sprintf(salted_pwd, "%s%s", salt_to_use, password);
    
    // Hash the salted password
    char* hashed = sha256_hash(salted_pwd);
    free(salted_pwd);
    
    if (!hashed) {
        if (salt_generated) {
            free(salt_to_use);
        }
        return NULL;
    }
    
    // Format as salt:hash
    size_t result_len = strlen(salt_to_use) + 1 + strlen(hashed) + 1;
    char* result = (char*)malloc(result_len);
    if (!result) {
        writeErrorLog("Memory allocation failed for salted hash result");
        free(hashed);
        if (salt_generated) {
            free(salt_to_use);
        }
        return NULL;
    }
    
    sprintf(result, "%s:%s", salt_to_use, hashed);
    
    free(hashed);
    free(salt_to_use);
    
    return result;
}

// Verify a password against a salted hash
bool verify_password(const char* password, const char* salted_hash) {
    if (!password || !salted_hash) {
        writeErrorLog("NULL input to verify_password");
        return false;
    }
    
    // Extract salt from salted_hash
    char* colon = strchr(salted_hash, ':');
    if (!colon) {
        writeErrorLog("Invalid salted hash format in verify_password");
        return false;
    }
    
    size_t salt_len = colon - salted_hash;
    char* salt = (char*)malloc(salt_len + 1);
    if (!salt) {
        writeErrorLog("Memory allocation failed in verify_password");
        return false;
    }
    
    strncpy(salt, salted_hash, salt_len);
    salt[salt_len] = '\0';
    
    // Create salted hash with the same salt
    char* test_hash = create_salted_hash(password, salt);
    free(salt);
    
    if (!test_hash) {
        return false;
    }
    
    // Compare with the provided salted hash
    bool result = (strcmp(test_hash, salted_hash) == 0);
    free(test_hash);
    
    return result;
}