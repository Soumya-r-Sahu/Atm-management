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

#endif // HASH_UTILS_H