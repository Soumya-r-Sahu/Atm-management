#include <stdio.h>
#include <string.h>

// Hashing key
#define HASH_KEY "ATM_HASH_KEY"

// Function to hash a string using XOR
void hashString(const char *input, char *output) {
    size_t keyLen = strlen(HASH_KEY);
    size_t inputLen = strlen(input);

    for (size_t i = 0; i < inputLen; i++) {
        output[i] = input[i] ^ HASH_KEY[i % keyLen];
    }
    output[inputLen] = '\0'; // Null-terminate the hashed string
}