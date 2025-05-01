#ifndef HASH_UTILS_H
#define HASH_UTILS_H

/**
 * Computes a SHA-256 hash of the input string
 * 
 * @param input The string to hash
 * @return A dynamically allocated string containing the hex representation 
 *         of the hash. The caller is responsible for freeing this memory.
 *         Returns NULL if hashing fails.
 */
char* sha256_hash(const char* input);

/**
 * Securely compares two hashes in constant time to prevent timing attacks
 * 
 * @param hash1 First hash to compare
 * @param hash2 Second hash to compare
 * @return 1 if hashes match, 0 otherwise
 */
int secure_hash_compare(const char* hash1, const char* hash2);

#endif // HASH_UTILS_H