/**
 * @file secure_password.h
 * @brief Advanced password security implementation
 * @version 1.0
 * @date May 10, 2025
 */

#ifndef SECURE_PASSWORD_H
#define SECURE_PASSWORD_H

#include <stdbool.h>

/**
 * Algorithm options for password hashing
 */
typedef enum {
    HASH_ALGORITHM_BCRYPT,   /**< Bcrypt algorithm (preferred) */
    HASH_ALGORITHM_ARGON2,   /**< Argon2 algorithm (if available) */
    HASH_ALGORITHM_PBKDF2    /**< PBKDF2 algorithm (fallback) */
} HashAlgorithm;

/**
 * Initialize the secure password subsystem
 * @return True if initialization was successful
 */
bool secure_password_init(void);

/**
 * Hash a password using the specified algorithm
 * 
 * The output hash string includes algorithm identifier, cost parameters,
 * salt, and the hash itself in a format that can be stored directly in 
 * a database or file.
 * 
 * @param password The password to hash
 * @param algorithm The algorithm to use
 * @param work_factor The work factor (cost) for the algorithm, 0 for default
 * @return Hashed password string (must be freed by caller) or NULL on error
 */
char* secure_password_hash(const char* password, HashAlgorithm algorithm, int work_factor);

/**
 * Verify a password against a stored hash
 * 
 * This function automatically detects the algorithm used to create the hash
 * from the hash string format.
 * 
 * @param password The password to verify
 * @param hash The stored hash string
 * @return True if the password matches the hash
 */
bool secure_password_verify(const char* password, const char* hash);

/**
 * Check if a hash needs to be upgraded
 * 
 * This function checks if a hash was created with parameters that are now
 * considered too weak and should be upgraded.
 * 
 * @param hash The hash string to check
 * @return True if the hash should be upgraded
 */
bool secure_password_needs_rehash(const char* hash);

/**
 * Generate a secure random token
 * 
 * @param length The desired length of the token
 * @return Random token string (must be freed by caller) or NULL on error
 */
char* secure_generate_token(size_t length);

/**
 * Clean up resources used by the secure password subsystem
 */
void secure_password_cleanup(void);

#endif /* SECURE_PASSWORD_H */
