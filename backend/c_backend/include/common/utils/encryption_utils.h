#ifndef ENCRYPTION_UTILS_H
#define ENCRYPTION_UTILS_H

/**
 * Initialize encryption system with the master key
 * 
 * @param masterKeyPath Path to the master key file (or NULL to use default)
 * @return 1 on success, 0 on failure
 */
int encryption_init(const char* masterKeyPath);

/**
 * Encrypt sensitive data with authenticated encryption
 * 
 * @param plaintext The plaintext data to encrypt
 * @param plaintext_len Length of plaintext data
 * @param associated_data Additional data to authenticate (can be NULL)
 * @param associated_data_len Length of additional data (0 if NULL)
 * @param ciphertext Output buffer for encrypted data
 * @param ciphertext_len Pointer to store the length of encrypted data
 * @param tag Output buffer for authentication tag
 * @return 1 on success, 0 on failure
 */
int encrypt_data(const unsigned char* plaintext, size_t plaintext_len,
                const unsigned char* associated_data, size_t associated_data_len,
                unsigned char* ciphertext, size_t* ciphertext_len,
                unsigned char* tag);

/**
 * Decrypt encrypted data with authentication
 * 
 * @param ciphertext The encrypted data to decrypt
 * @param ciphertext_len Length of encrypted data
 * @param associated_data Additional authenticated data (can be NULL)
 * @param associated_data_len Length of additional data (0 if NULL)
 * @param tag Authentication tag to verify
 * @param plaintext Output buffer for decrypted data
 * @param plaintext_len Pointer to store the length of decrypted data
 * @return 1 on success, 0 on failure (including authentication failure)
 */
int decrypt_data(const unsigned char* ciphertext, size_t ciphertext_len,
                const unsigned char* associated_data, size_t associated_data_len,
                const unsigned char* tag,
                unsigned char* plaintext, size_t* plaintext_len);

/**
 * Generate a secure encryption key
 * 
 * @param key Output buffer for the key (must be at least 32 bytes)
 * @return 1 on success, 0 on failure
 */
int generate_key(unsigned char* key);

/**
 * Encrypt a string and return base64-encoded result
 * 
 * @param plaintext The string to encrypt
 * @return Base64 encoded encrypted string (must be freed by caller) or NULL on error
 */
char* encrypt_string(const char* plaintext);

/**
 * Decrypt a base64-encoded encrypted string
 * 
 * @param encrypted_b64 Base64 encoded encrypted string
 * @return Decrypted string (must be freed by caller) or NULL on error
 */
char* decrypt_string(const char* encrypted_b64);

/**
 * Hash a password for storage (e.g., PIN or admin password)
 * 
 * @param password The password to hash
 * @param salt Optional salt (can be NULL for auto-generation)
 * @return Hashed password string (must be freed by caller) or NULL on error
 */
char* hash_password(const char* password, const char* salt);

/**
 * Verify a password against a stored hash
 * 
 * @param password The password to verify
 * @param stored_hash The stored hash to compare against
 * @return 1 if matched, 0 if not matched or error
 */
int verify_password(const char* password, const char* stored_hash);

/**
 * Generate a secure random token (e.g., for session IDs)
 * 
 * @param length The desired length of the token
 * @return Random token string (must be freed by caller) or NULL on error
 */
char* generate_secure_token(size_t length);

/**
 * Clean up encryption resources
 */
void encryption_cleanup(void);

#endif // ENCRYPTION_UTILS_H