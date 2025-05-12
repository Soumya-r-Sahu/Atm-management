#ifndef ENCRYPTION_UTILS_H
#define ENCRYPTION_UTILS_H

#include <stddef.h>

/**
 * Initialize the encryption subsystem, loading or creating a master key as needed
 * @param masterKeyPath Optional path to the master key file, or NULL to use default path
 * @return 1 on success, 0 on failure
 */
int encryption_init(const char* masterKeyPath);

/**
 * Encrypt a string and return a base64-encoded result
 * @param plaintext The string to encrypt
 * @return Encrypted, base64-encoded string that must be freed by the caller, or NULL on error
 */
char* encrypt_string(const char* plaintext);

/**
 * Decrypt a base64-encoded encrypted string
 * @param encrypted_b64 The base64-encoded encrypted string
 * @return Decrypted string that must be freed by the caller, or NULL on error
 */
char* decrypt_string(const char* encrypted_b64);

/**
 * Hash a password with an optional salt
 * @param password The password to hash
 * @param salt Optional salt, or NULL to generate a random salt
 * @return Hashed password string that must be freed by the caller, or NULL on error
 */
char* hash_password(const char* password, const char* salt);

/**
 * Verify a password against a stored hash
 * @param password The password to verify
 * @param stored_hash The stored hash to compare against
 * @return 1 if the password matches, 0 otherwise
 */
int verify_password(const char* password, const char* stored_hash);

/**
 * Generate a secure random token
 * @param length The length of the token in bytes (output will be twice this length as hex)
 * @return A hex string token that must be freed by the caller, or NULL on error
 */
char* generate_secure_token(size_t length);

/**
 * Clean up encryption resources
 */
void encryption_cleanup(void);

// Low-level encryption/decryption functions for advanced use
int encrypt_data(const unsigned char* plaintext, size_t plaintext_len,
                const unsigned char* associated_data, size_t associated_data_len,
                unsigned char* ciphertext, size_t* ciphertext_len,
                unsigned char* tag);

int decrypt_data(const unsigned char* ciphertext, size_t ciphertext_len,
                const unsigned char* associated_data, size_t associated_data_len,
                const unsigned char* tag,
                unsigned char* plaintext, size_t* plaintext_len);

#endif // ENCRYPTION_UTILS_H