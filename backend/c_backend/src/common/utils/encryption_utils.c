#include "common/utils/encryption_utils.h"
#include "common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#define KEY_SIZE 32    // 256 bits
#define IV_SIZE 16     // 128 bits
#define TAG_SIZE 16    // 128 bits
#define SALT_SIZE 16   // 128 bits

// Master encryption key
static unsigned char master_key[KEY_SIZE];
static int is_initialized = 0;

// Base64 encoding and decoding functions
static char* base64_encode(const unsigned char *input, int length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    char *result = (char *)malloc(bufferPtr->length + 1);
    memcpy(result, bufferPtr->data, bufferPtr->length);
    result[bufferPtr->length] = 0;
    
    BIO_free_all(bio);
    
    return result;
}

static unsigned char* base64_decode(const char *input, int *outlen) {
    BIO *bio, *b64;
    int inlen = strlen(input);
    unsigned char *buffer = (unsigned char*)malloc(inlen);
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(input, inlen);
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    *outlen = BIO_read(bio, buffer, inlen);
    
    BIO_free_all(bio);
    
    return buffer;
}

// Initialize OpenSSL and the encryption system
int encryption_init(const char* masterKeyPath) {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    
    // If a master key file is provided, try to load it
    if (masterKeyPath) {
        FILE* keyFile = fopen(masterKeyPath, "rb");
        if (keyFile) {
            if (fread(master_key, 1, KEY_SIZE, keyFile) != KEY_SIZE) {
                fclose(keyFile);
                return 0; // Failed to read the key
            }
            fclose(keyFile);
            is_initialized = 1;
            return 1;
        }
    }
    
    // No key file provided or couldn't open it, generate a new key
    if (!RAND_bytes(master_key, KEY_SIZE)) {
        return 0; // Failed to generate random key
    }
    
    // If a path was provided, try to save the key for future use
    if (masterKeyPath) {
        FILE* keyFile = fopen(masterKeyPath, "wb");
        if (keyFile) {
            if (fwrite(master_key, 1, KEY_SIZE, keyFile) != KEY_SIZE) {
                fclose(keyFile);
                return 0; // Failed to write the key
            }
            fclose(keyFile);
        }
    }
    
    is_initialized = 1;
    return 1;
}

// Encrypt data using AES-GCM with authentication
int encrypt_data(const unsigned char* plaintext, size_t plaintext_len,
                const unsigned char* associated_data, size_t associated_data_len,
                unsigned char* ciphertext, size_t* ciphertext_len,
                unsigned char* tag) {
                
    if (!is_initialized) {
        if (!encryption_init(NULL)) {
            return 0;
        }
    }
    
    // Generate a random IV for this encryption
    unsigned char iv[IV_SIZE];
    if (!RAND_bytes(iv, IV_SIZE)) {
        return 0;
    }
    
    // Copy IV to the beginning of the ciphertext buffer
    memcpy(ciphertext, iv, IV_SIZE);
    
    // Initialize cipher context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return 0;
    }
    
    // Initialize encryption operation
    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, master_key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    // Add associated data if present
    if (associated_data && associated_data_len > 0) {
        int outlen;
        if (!EVP_EncryptUpdate(ctx, NULL, &outlen, associated_data, associated_data_len)) {
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }
    }
    
    // Encrypt the plaintext
    int outlen;
    if (!EVP_EncryptUpdate(ctx, ciphertext + IV_SIZE, &outlen, plaintext, plaintext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    int tmplen;
    // Finalize encryption
    if (!EVP_EncryptFinal_ex(ctx, ciphertext + IV_SIZE + outlen, &tmplen)) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    // Get the authentication tag
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag)) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    // Set the actual ciphertext length (IV + ciphertext)
    *ciphertext_len = IV_SIZE + outlen + tmplen;
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    return 1;
}

// Decrypt data using AES-GCM with authentication
int decrypt_data(const unsigned char* ciphertext, size_t ciphertext_len,
                const unsigned char* associated_data, size_t associated_data_len,
                const unsigned char* tag,
                unsigned char* plaintext, size_t* plaintext_len) {
                
    if (!is_initialized) {
        if (!encryption_init(NULL)) {
            return 0;
        }
    }
    
    // The IV is at the beginning of the ciphertext
    const unsigned char* iv = ciphertext;
    
    // The actual ciphertext starts after the IV
    const unsigned char* actual_ciphertext = ciphertext + IV_SIZE;
    size_t actual_ciphertext_len = ciphertext_len - IV_SIZE;
    
    // Initialize cipher context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return 0;
    }
    
    // Initialize decryption operation
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, master_key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    // Add associated data if present
    if (associated_data && associated_data_len > 0) {
        int outlen;
        if (!EVP_DecryptUpdate(ctx, NULL, &outlen, associated_data, associated_data_len)) {
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }
    }
    
    // Decrypt the ciphertext
    int outlen;
    if (!EVP_DecryptUpdate(ctx, plaintext, &outlen, actual_ciphertext, actual_ciphertext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    // Set the authentication tag
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, (void*)tag)) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    // Finalize decryption and verify the tag
    int tmplen;
    int ret = EVP_DecryptFinal_ex(ctx, plaintext + outlen, &tmplen);
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    if (ret <= 0) {
        // Authentication failed
        return 0;
    }
    
    // Set the actual plaintext length
    *plaintext_len = outlen + tmplen;
    
    return 1;
}

// Generate a secure encryption key
int generate_key(unsigned char* key) {
    return RAND_bytes(key, KEY_SIZE);
}

// Encrypt a string and return base64-encoded result
char* encrypt_string(const char* plaintext) {
    if (!plaintext) {
        return NULL;
    }
    
    size_t plaintext_len = strlen(plaintext);
    size_t ciphertext_buffer_len = IV_SIZE + plaintext_len + EVP_MAX_BLOCK_LENGTH;
    unsigned char* ciphertext = (unsigned char*)malloc(ciphertext_buffer_len);
    unsigned char tag[TAG_SIZE];
    size_t ciphertext_len;
    
    if (!encrypt_data((unsigned char*)plaintext, plaintext_len, 
                     NULL, 0, ciphertext, &ciphertext_len, tag)) {
        free(ciphertext);
        return NULL;
    }
    
    // Allocate a buffer that can hold both ciphertext and tag
    unsigned char* combined = (unsigned char*)malloc(ciphertext_len + TAG_SIZE);
    memcpy(combined, ciphertext, ciphertext_len);
    memcpy(combined + ciphertext_len, tag, TAG_SIZE);
    
    free(ciphertext);
    
    // Base64 encode the combined buffer
    char* result = base64_encode(combined, ciphertext_len + TAG_SIZE);
    free(combined);
    
    return result;
}

// Decrypt a base64-encoded encrypted string
char* decrypt_string(const char* encrypted_b64) {
    if (!encrypted_b64) {
        return NULL;
    }
    
    int combined_len;
    unsigned char* combined = base64_decode(encrypted_b64, &combined_len);
    
    if (combined_len <= IV_SIZE + TAG_SIZE) {
        free(combined);
        return NULL; // Invalid length
    }
    
    size_t ciphertext_len = combined_len - TAG_SIZE;
    unsigned char* tag = combined + ciphertext_len;
    
    // Allocate buffer for plaintext (will be smaller than ciphertext)
    size_t max_plaintext_len = ciphertext_len;
    unsigned char* plaintext = (unsigned char*)malloc(max_plaintext_len);
    size_t plaintext_len;
    
    if (!decrypt_data(combined, ciphertext_len, NULL, 0, tag, plaintext, &plaintext_len)) {
        free(combined);
        free(plaintext);
        return NULL;
    }
    
    free(combined);
    
    // Ensure result is null-terminated
    char* result = (char*)malloc(plaintext_len + 1);
    memcpy(result, plaintext, plaintext_len);
    result[plaintext_len] = '\0';
    
    free(plaintext);
    
    return result;
}

// Hash a password using PBKDF2
char* hash_password(const char* password, const char* salt_str) {
    if (!password) {
        return NULL;
    }
    
    // Generate a salt if not provided
    unsigned char salt[SALT_SIZE];
    if (!salt_str) {
        if (!RAND_bytes(salt, SALT_SIZE)) {
            return NULL;
        }
    } else {
        // Use the provided salt
        memcpy(salt, salt_str, SALT_SIZE < strlen(salt_str) ? SALT_SIZE : strlen(salt_str));
    }
    
    // PBKDF2 parameters
    const int iterations = 10000;
    unsigned char key[KEY_SIZE];
    
    // Derive key from password
    if (!PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_SIZE, 
                          iterations, EVP_sha256(), KEY_SIZE, key)) {
        return NULL;
    }
    
    // Allocate buffer for salt + key
    unsigned char* combined = (unsigned char*)malloc(SALT_SIZE + KEY_SIZE);
    memcpy(combined, salt, SALT_SIZE);
    memcpy(combined + SALT_SIZE, key, KEY_SIZE);
    
    // Base64 encode for storage
    char* result = base64_encode(combined, SALT_SIZE + KEY_SIZE);
    free(combined);
    
    return result;
}

// Verify a password against a stored hash
int verify_password(const char* password, const char* stored_hash) {
    if (!password || !stored_hash) {
        return 0;
    }
    
    // Decode the stored hash
    int decoded_len;
    unsigned char* decoded = base64_decode(stored_hash, &decoded_len);
    
    if (decoded_len != SALT_SIZE + KEY_SIZE) {
        free(decoded);
        return 0; // Invalid hash format
    }
    
    // Extract salt and key from the decoded data
    unsigned char* salt = decoded;
    unsigned char* stored_key = decoded + SALT_SIZE;
    
    // PBKDF2 parameters
    const int iterations = 10000;
    unsigned char key[KEY_SIZE];
    
    // Derive key from the provided password
    if (!PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_SIZE, 
                          iterations, EVP_sha256(), KEY_SIZE, key)) {
        free(decoded);
        return 0;
    }
    
    // Compare the derived key with the stored key
    int result = (memcmp(key, stored_key, KEY_SIZE) == 0);
    
    free(decoded);
    
    return result;
}

// Generate a secure random token
char* generate_secure_token(size_t length) {
    if (length <= 0) {
        return NULL;
    }
    
    unsigned char* buffer = (unsigned char*)malloc(length);
    
    if (!RAND_bytes(buffer, length)) {
        free(buffer);
        return NULL;
    }
    
    // Convert to base64 for printable characters
    char* result = base64_encode(buffer, length);
    free(buffer);
    
    // Truncate to the requested length if needed
    if (result && strlen(result) > length) {
        result[length] = '\0';
    }
    
    return result;
}

// Clean up encryption resources
void encryption_cleanup(void) {
    // Wipe the master key from memory
    memset(master_key, 0, KEY_SIZE);
    is_initialized = 0;
    
    // Clean up OpenSSL
    EVP_cleanup();
    ERR_free_strings();
}