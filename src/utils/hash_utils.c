#include "hash_utils.h"
#include "../utils/logger.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>  // Added for uint32_t, uint8_t, etc.

// Simple SHA-256 implementation for educational purposes
// In a real-world application, use a cryptographic library like OpenSSL

// SHA-256 constants
#define SHA256_BLOCK_SIZE 64
#define SHA256_DIGEST_SIZE 32

// Circular right rotation
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

// SHA-256 functions
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

// SHA-256 round constants
static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// SHA-256 context structure
typedef struct {
    uint32_t state[8];          // Current state
    uint64_t total_bits;        // Total bits processed
    uint8_t buffer[64];         // Input buffer
    uint8_t buffer_idx;         // Buffer index
} sha256_ctx;

// Initialize SHA-256 context
static void sha256_init(sha256_ctx* ctx) {
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
    ctx->total_bits = 0;
    ctx->buffer_idx = 0;
}

// Process a 64-byte block
static void sha256_process_block(sha256_ctx* ctx, const uint8_t* block) {
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;
    int i;

    // Prepare the message schedule
    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i * 4] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8) |
               ((uint32_t)block[i * 4 + 3]);
    }

    for (i = 16; i < 64; i++) {
        w[i] = SIG1(w[i - 2]) + w[i - 7] + SIG0(w[i - 15]) + w[i - 16];
    }

    // Initialize working variables
    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    // Main loop
    for (i = 0; i < 64; i++) {
        t1 = h + EP1(e) + CH(e, f, g) + k[i] + w[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    // Update the state
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

// Update SHA-256 context with data
static void sha256_update(sha256_ctx* ctx, const uint8_t* data, size_t len) {
    size_t i;
    
    // Update the total number of bits
    ctx->total_bits += len * 8;
    
    // Process the data
    for (i = 0; i < len; i++) {
        ctx->buffer[ctx->buffer_idx++] = data[i];
        
        // Process the block if the buffer is full
        if (ctx->buffer_idx == 64) {
            sha256_process_block(ctx, ctx->buffer);
            ctx->buffer_idx = 0;
        }
    }
}

// Finalize the SHA-256 operation and get the digest
static void sha256_final(sha256_ctx* ctx, uint8_t* digest) {
    uint8_t pad[72];
    uint64_t bits;
    int pad_len;
    int i;
    
    // Calculate padding length
    pad_len = (ctx->buffer_idx < 56) ? (56 - ctx->buffer_idx) : (120 - ctx->buffer_idx);
    
    // Prepare padding
    memset(pad, 0, pad_len);
    pad[0] = 0x80; // Append a '1' bit
    
    // Append the length in bits
    bits = ctx->total_bits;
    for (i = 0; i < 8; i++) {
        pad[pad_len + i] = (bits >> (56 - i * 8)) & 0xff;
    }
    
    // Process the final block(s)
    sha256_update(ctx, pad, pad_len + 8);
    
    // Copy the final state to the output digest
    for (i = 0; i < 8; i++) {
        digest[i * 4] = (ctx->state[i] >> 24) & 0xff;
        digest[i * 4 + 1] = (ctx->state[i] >> 16) & 0xff;
        digest[i * 4 + 2] = (ctx->state[i] >> 8) & 0xff;
        digest[i * 4 + 3] = ctx->state[i] & 0xff;
    }
}

// Compute SHA-256 hash of a string
char* sha256_hash(const char* input) {
    if (input == NULL) {
        writeErrorLog("NULL input provided to sha256_hash");
        return NULL;
    }
    
    sha256_ctx ctx;
    uint8_t digest[SHA256_DIGEST_SIZE];
    char* hex_digest = (char*)malloc(SHA256_DIGEST_SIZE * 2 + 1);
    
    if (hex_digest == NULL) {
        writeErrorLog("Memory allocation failed in sha256_hash");
        return NULL;
    }
    
    // Initialize context
    sha256_init(&ctx);
    
    // Update with input data
    sha256_update(&ctx, (const uint8_t*)input, strlen(input));
    
    // Finalize and get digest
    sha256_final(&ctx, digest);
    
    // Convert digest to hex string
    for (int i = 0; i < SHA256_DIGEST_SIZE; i++) {
        sprintf(&hex_digest[i * 2], "%02x", digest[i]);
    }
    
    return hex_digest;
}