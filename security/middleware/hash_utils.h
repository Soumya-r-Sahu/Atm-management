#ifndef HASH_UTILS_H
#define HASH_UTILS_H

#include <stdbool.h>

/**
 * Generate SHA256 hash of input string
 * @param input String to hash
 * @return Pointer to newly allocated string containing the hash in hex format, 
 *         NULL if failed. Caller must free the returned string.
 */
char* sha256_hash(const char* input);

/**
 * Compare two hash strings securely (constant time)
 * @param hash1 First hash to compare
 * @param hash2 Second hash to compare
 * @return true if hashes match, false otherwise
 */
bool secure_hash_compare(const char* hash1, const char* hash2);

/**
 * Generate a salt for passwords
 * @param length Length of the salt to generate
 * @return Pointer to newly allocated string containing the salt,
 *         NULL if failed. Caller must free the returned string.
 */
char* generate_salt(int length);

/**
 * Create a salted and hashed password
 * @param password Password to hash
 * @param salt Salt to use (if NULL, a new salt will be generated)
 * @return Pointer to newly allocated string containing the salted hash,
 *         NULL if failed. Caller must free the returned string.
 */
char* create_salted_hash(const char* password, const char* salt);

/**
 * Verify a password against a salted hash
 * @param password Password to verify
 * @param salted_hash Previously created salted hash
 * @return true if password is correct, false otherwise
 */
bool verify_password(const char* password, const char* salted_hash);

#endif // HASH_UTILS_H