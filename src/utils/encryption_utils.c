#include "encryption_utils.h"
#include "../common/error_handler.h"
#include "../utils/memory_utils.h"
#include "../common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// For a simple implementation, we'll provide a basic encryption mechanism
// In a real-world scenario, you'd use a proper cryptographic library like OpenSSL

// Master encryption key (in a real system, this would be securely stored)
static unsigned char master_key[32] = {0};

// Generate a master key from a master password
static int derive_master_key(const char* master_password) {
    if (!master_password) {
        return 0;
    }
    
    // Simple key derivation (not for production use)
    size_t passlen = strlen(master_password);
    for (size_t i = 0; i < 32; i++) {
        master_key[i] = master_password[i % passlen] ^ ((i * 7) & 0xFF);
    }
    
    return 1;
}

// Load or create a master key
int encryption_init(const char* masterKeyPath) {
    const char* keyPath = masterKeyPath ? 
        masterKeyPath : 
        (isTestingMode() ? "testing/master.key" : "data/master.key");
    
    FILE* keyFile = fopen(keyPath, "rb");
    
    if (keyFile) {
        // Key exists, read it
        if (fread(master_key, 1, 32, keyFile) != 32) {
            SET_ERROR(ERR_FILE_ACCESS, "Failed to read master key file");
            fclose(keyFile);
            return 0;
        }
        fclose(keyFile);
    } else {
        // Key doesn't exist, create a new one
        // In a real system, this would use a secure random number generator
        srand(time(NULL));
        for (int i = 0; i < 32; i++) {
            master_key[i] = rand() & 0xFF;
        }
        
        // Save the key
        keyFile = fopen(keyPath, "wb");
        if (!keyFile) {
            SET_ERROR(ERR_FILE_ACCESS, "Failed to create master key file");
            return 0;
        }
        
        if (fwrite(master_key, 1, 32, keyFile) != 32) {
            SET_ERROR(ERR_FILE_ACCESS, "Failed to write master key file");
            fclose(keyFile);
            return 0;
        }
        
        fclose(keyFile);
    }
    
    return 1;
}

// Simplified XOR-based encryption (NOT secure for production use)
int encrypt_data(const unsigned char* plaintext, size_t plaintext_len,
                const unsigned char* associated_data, size_t associated_data_len,
                unsigned char* ciphertext, size_t* ciphertext_len,
                unsigned char* tag) {
    if (!plaintext || !ciphertext || !ciphertext_len || !tag) {
        SET_ERROR(ERR_INVALID_INPUT, "Invalid encryption parameters");
        return 0;
    }
    
    // Simple encryption - XOR with master key
    for (size_t i = 0; i < plaintext_len; i++) {
        ciphertext[i] = plaintext[i] ^ master_key[i % 32];
    }
    
    *ciphertext_len = plaintext_len;
    
    // Generate a simple tag (not secure, just for demonstration)
    unsigned int hash = 0x12345678;
    for (size_t i = 0; i < plaintext_len; i++) {
        hash = ((hash << 5) + hash) ^ plaintext[i];
    }
    
    // Include associated data in tag if provided
    if (associated_data && associated_data_len > 0) {
        for (size_t i = 0; i < associated_data_len; i++) {
            hash = ((hash << 5) + hash) ^ associated_data[i];
        }
    }
    
    // Store the tag (typically 16 bytes for AES-GCM)
    for (int i = 0; i < 16; i++) {
        tag[i] = (hash >> (i * 8)) & 0xFF;
    }
    
    return 1;
}

// Decrypt data with the same XOR-based approach
int decrypt_data(const unsigned char* ciphertext, size_t ciphertext_len,
                const unsigned char* associated_data, size_t associated_data_len,
                const unsigned char* tag,
                unsigned char* plaintext, size_t* plaintext_len) {
    if (!ciphertext || !plaintext || !plaintext_len || !tag) {
        SET_ERROR(ERR_INVALID_INPUT, "Invalid decryption parameters");
        return 0;
    }
    
    // Simple decryption - XOR with master key
    for (size_t i = 0; i < ciphertext_len; i++) {
        plaintext[i] = ciphertext[i] ^ master_key[i % 32];
    }
    
    *plaintext_len = ciphertext_len;
    
    // Verify the tag (this is a simple check, not secure)
    unsigned int hash = 0x12345678;
    for (size_t i = 0; i < ciphertext_len; i++) {
        hash = ((hash << 5) + hash) ^ plaintext[i];
    }
    
    // Include associated data in tag if provided
    if (associated_data && associated_data_len > 0) {
        for (size_t i = 0; i < associated_data_len; i++) {
            hash = ((hash << 5) + hash) ^ associated_data[i];
        }
    }
    
    // Verify the tag
    unsigned char calculated_tag[16];
    for (int i = 0; i < 16; i++) {
        calculated_tag[i] = (hash >> (i * 8)) & 0xFF;
    }
    
    if (memcmp(calculated_tag, tag, 16) != 0) {
        SET_ERROR(ERR_AUTHENTICATION, "Data authentication failed during decryption");
        return 0;
    }
    
    return 1;
}

// Simple Base64 encoding for encrypted data
static char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char* base64_encode(const unsigned char* data, size_t length) {
    size_t encoded_length = ((length + 2) / 3) * 4;
    char* encoded = (char*)MALLOC(encoded_length + 1, "Base64 encoded data");
    
    if (!encoded) {
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for base64 encoding");
        return NULL;
    }
    
    size_t i, j = 0;
    for (i = 0; i < length; i += 3) {
        unsigned int val = data[i] << 16;
        if (i + 1 < length) val |= data[i + 1] << 8;
        if (i + 2 < length) val |= data[i + 2];
        
        encoded[j++] = base64_chars[(val >> 18) & 0x3F];
        encoded[j++] = base64_chars[(val >> 12) & 0x3F];
        encoded[j++] = (i + 1 < length) ? base64_chars[(val >> 6) & 0x3F] : '=';
        encoded[j++] = (i + 2 < length) ? base64_chars[val & 0x3F] : '=';
    }
    
    encoded[j] = '\0';
    return encoded;
}

static unsigned char* base64_decode(const char* data, size_t* length) {
    if (!data || !length) {
        SET_ERROR(ERR_INVALID_INPUT, "Invalid base64 decode parameters");
        return NULL;
    }
    
    size_t encoded_length = strlen(data);
    if (encoded_length % 4 != 0) {
        SET_ERROR(ERR_INVALID_INPUT, "Invalid base64 encoded string length");
        return NULL;
    }
    
    size_t padding = 0;
    if (encoded_length > 0) {
        if (data[encoded_length - 1] == '=') padding++;
        if (data[encoded_length - 2] == '=') padding++;
    }
    
    *length = (encoded_length / 4) * 3 - padding;
    unsigned char* decoded = (unsigned char*)MALLOC(*length, "Base64 decoded data");
    
    if (!decoded) {
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for base64 decoding");
        return NULL;
    }
    
    for (size_t i = 0, j = 0; i < encoded_length;) {
        uint32_t val = 0;
        for (int k = 0; k < 4; k++) {
            val <<= 6;
            if (data[i] >= 'A' && data[i] <= 'Z') {
                val |= data[i] - 'A';
            }
            else if (data[i] >= 'a' && data[i] <= 'z') {
                val |= data[i] - 'a' + 26;
            }
            else if (data[i] >= '0' && data[i] <= '9') {
                val |= data[i] - '0' + 52;
            }
            else if (data[i] == '+') {
                val |= 62;
            }
            else if (data[i] == '/') {
                val |= 63;
            }
            else if (data[i] == '=') {
                val <<= 6;
            }
            else {
                FREE(decoded);
                SET_ERROR(ERR_INVALID_INPUT, "Invalid character in base64 encoded string");
                return NULL;
            }
            i++;
        }
        
        if (j < *length) decoded[j++] = (val >> 16) & 0xFF;
        if (j < *length) decoded[j++] = (val >> 8) & 0xFF;
        if (j < *length) decoded[j++] = val & 0xFF;
    }
    
    return decoded;
}

// Encrypt a string and return base64-encoded result
char* encrypt_string(const char* plaintext) {
    if (!plaintext) {
        SET_ERROR(ERR_INVALID_INPUT, "Null plaintext for encryption");
        return NULL;
    }
    
    size_t plaintext_len = strlen(plaintext);
    unsigned char* ciphertext = (unsigned char*)MALLOC(plaintext_len, "Ciphertext buffer");
    unsigned char tag[16];
    size_t ciphertext_len;
    
    if (!ciphertext) {
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for ciphertext");
        return NULL;
    }
    
    if (!encrypt_data((const unsigned char*)plaintext, plaintext_len, 
                      NULL, 0, ciphertext, &ciphertext_len, tag)) {
        FREE(ciphertext);
        return NULL;
    }
    
    // Prepare the combined buffer (tag + ciphertext)
    unsigned char* combined = (unsigned char*)MALLOC(16 + ciphertext_len, "Combined buffer");
    if (!combined) {
        FREE(ciphertext);
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for combined buffer");
        return NULL;
    }
    
    // Copy tag and ciphertext to combined buffer
    memcpy(combined, tag, 16);
    memcpy(combined + 16, ciphertext, ciphertext_len);
    
    // Base64 encode the combined data
    char* encoded = base64_encode(combined, 16 + ciphertext_len);
    
    // Clean up
    FREE(ciphertext);
    FREE(combined);
    
    return encoded;
}

// Decrypt a base64-encoded encrypted string
char* decrypt_string(const char* encrypted_b64) {
    if (!encrypted_b64) {
        SET_ERROR(ERR_INVALID_INPUT, "Null encrypted data for decryption");
        return NULL;
    }
    
    // Base64 decode
    size_t decoded_len;
    unsigned char* decoded = base64_decode(encrypted_b64, &decoded_len);
    
    if (!decoded || decoded_len <= 16) {
        FREE(decoded);
        SET_ERROR(ERR_INVALID_INPUT, "Invalid or corrupted encrypted data");
        return NULL;
    }
    
    // Extract tag and ciphertext
    unsigned char tag[16];
    memcpy(tag, decoded, 16);
    
    size_t ciphertext_len = decoded_len - 16;
    unsigned char* ciphertext = (unsigned char*)MALLOC(ciphertext_len, "Extracted ciphertext");
    
    if (!ciphertext) {
        FREE(decoded);
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for extracted ciphertext");
        return NULL;
    }
    
    memcpy(ciphertext, decoded + 16, ciphertext_len);
    FREE(decoded); // Done with the combined data
    
    // Decrypt
    unsigned char* plaintext = (unsigned char*)MALLOC(ciphertext_len + 1, "Decrypted plaintext");
    
    if (!plaintext) {
        FREE(ciphertext);
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for plaintext");
        return NULL;
    }
    
    size_t plaintext_len;
    if (!decrypt_data(ciphertext, ciphertext_len, NULL, 0, tag, plaintext, &plaintext_len)) {
        FREE(ciphertext);
        FREE(plaintext);
        return NULL;
    }
    
    // Ensure null termination
    plaintext[plaintext_len] = '\0';
    
    // Convert to string
    char* result = (char*)plaintext;
    
    // Clean up
    FREE(ciphertext);
    
    return result;
}

// Simple password hashing (SHA-256 would be better in a real system)
char* hash_password(const char* password, const char* salt) {
    if (!password) {
        SET_ERROR(ERR_INVALID_INPUT, "Null password for hashing");
        return NULL;
    }
    
    // Generate a salt if not provided
    char generated_salt[17];
    if (!salt) {
        for (int i = 0; i < 16; i++) {
            generated_salt[i] = 'a' + (rand() % 26);
        }
        generated_salt[16] = '\0';
        salt = generated_salt;
    }
    
    // Combine salt and password
    size_t salt_len = strlen(salt);
    size_t pass_len = strlen(password);
    char* combined = (char*)MALLOC(salt_len + pass_len + 1, "Combined salt and password");
    
    if (!combined) {
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for combined salt and password");
        return NULL;
    }
    
    strcpy(combined, salt);
    strcat(combined, password);
    
    // Simple hash function (not secure, just for demonstration)
    // In a real system, use bcrypt, Argon2, or similar
    unsigned int hash = 0x12345678;
    size_t combined_len = salt_len + pass_len;
    
    for (size_t i = 0; i < combined_len; i++) {
        hash = ((hash << 5) + hash) ^ combined[i];
        // Additional iterations to make it slower
        for (int j = 0; j < 1000; j++) {
            hash = ((hash << 5) + hash) ^ j;
        }
    }
    
    FREE(combined);
    
    // Format the result as salt + "$" + hash
    char hash_str[33];
    sprintf(hash_str, "%08x%08x%08x%08x", hash, hash >> 8, hash >> 16, hash >> 24);
    
    char* result = (char*)MALLOC(salt_len + 1 + 32 + 1, "Hashed password");
    if (!result) {
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for hash result");
        return NULL;
    }
    
    strcpy(result, salt);
    strcat(result, "$");
    strcat(result, hash_str);
    
    return result;
}

// Verify a password against a stored hash
int verify_password(const char* password, const char* stored_hash) {
    if (!password || !stored_hash) {
        SET_ERROR(ERR_INVALID_INPUT, "Invalid parameters for password verification");
        return 0;
    }
    
    // Extract salt from the stored hash
    const char* dollar = strchr(stored_hash, '$');
    if (!dollar) {
        SET_ERROR(ERR_INVALID_INPUT, "Invalid stored hash format");
        return 0;
    }
    
    size_t salt_len = dollar - stored_hash;
    char* salt = (char*)MALLOC(salt_len + 1, "Extracted salt");
    
    if (!salt) {
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for salt extraction");
        return 0;
    }
    
    strncpy(salt, stored_hash, salt_len);
    salt[salt_len] = '\0';
    
    // Hash the provided password with the extracted salt
    char* computed_hash = hash_password(password, salt);
    FREE(salt);
    
    if (!computed_hash) {
        return 0; // Error already set by hash_password
    }
    
    // Compare the hashes
    int result = (strcmp(computed_hash, stored_hash) == 0);
    FREE(computed_hash);
    
    return result;
}

// Generate a secure random token
char* generate_secure_token(size_t length) {
    if (length == 0) {
        SET_ERROR(ERR_INVALID_INPUT, "Token length must be greater than zero");
        return NULL;
    }
    
    unsigned char* random_data = (unsigned char*)MALLOC(length, "Random token data");
    
    if (!random_data) {
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for token data");
        return NULL;
    }
    
    // Generate random bytes
    // In a real system, use a cryptographically secure RNG
    srand(time(NULL));
    for (size_t i = 0; i < length; i++) {
        random_data[i] = rand() & 0xFF;
    }
    
    // Convert to hex string
    char* token = (char*)MALLOC(length * 2 + 1, "Hex token string");
    
    if (!token) {
        FREE(random_data);
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for token string");
        return NULL;
    }
    
    for (size_t i = 0; i < length; i++) {
        sprintf(token + i * 2, "%02x", random_data[i]);
    }
    
    FREE(random_data);
    return token;
}

// Clean up encryption resources
void encryption_cleanup(void) {
    // Clear the master key
    memset(master_key, 0, sizeof(master_key));
}