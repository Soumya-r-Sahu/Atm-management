/**
 * @file mac_generator.c
 * @brief Message Authentication Code (MAC) generator for ISO8583 messages
 * 
 * This file provides functionality to generate and validate MACs for ISO8583 messages
 * to ensure message integrity and authenticity.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// A simple XOR-based MAC generator (for demonstration purposes)
// In a production environment, use a cryptographically secure algorithm like HMAC-SHA256
void generate_simple_mac(const unsigned char* message, size_t length, const char* key, unsigned char* mac_out) {
    if (!message || !key || !mac_out || length == 0) {
        return;
    }
    
    size_t key_len = strlen(key);
    if (key_len == 0) {
        return;
    }
    
    // Initialize MAC with zeros
    unsigned char mac[8] = {0};
    
    // Simple XOR-based MAC (not secure, for demonstration only)
    for (size_t i = 0; i < length; i++) {
        mac[i % 8] ^= message[i] ^ key[i % key_len];
    }
    
    // Additional scrambling
    for (int i = 0; i < 7; i++) {
        mac[i+1] ^= mac[i];
    }
    
    // Copy the result to output
    memcpy(mac_out, mac, 8);
}

// Function to generate a MAC for an ISO8583 message
// In a real implementation, this would use cryptographic algorithms
int generate_iso8583_mac(const unsigned char* message, size_t length, unsigned char* mac_out) {
    if (!message || !mac_out || length < 12) {
        return -1;
    }
    
    // In a real implementation, use a secure key management system
    const char* mac_key = "CB5_M4C_K3Y_2023";
    
    // Generate the MAC
    generate_simple_mac(message, length, mac_key, mac_out);
    
    return 0;
}

// Function to validate a MAC in an ISO8583 message
int validate_iso8583_mac(const unsigned char* message, size_t length, const unsigned char* mac) {
    if (!message || !mac || length < 20) {
        return -1;
    }
    
    // Calculate the MAC for the message
    unsigned char calculated_mac[8];
    
    // In a real implementation, the message portion used for MAC calculation
    // would exclude the MAC field itself
    // For simplicity, we'll assume the MAC is at the end of the message
    generate_iso8583_mac(message, length - 8, calculated_mac);
    
    // Compare the calculated MAC with the provided MAC
    if (memcmp(calculated_mac, mac, 8) == 0) {
        return 0; // MAC is valid
    } else {
        return -1; // MAC is invalid
    }
}

// Add MAC to an ISO8583 message as field 128
int add_mac_to_iso8583(unsigned char* message, size_t* length, size_t max_length) {
    if (!message || !length || *length < 12 || max_length < *length + 10) {
        return -1;
    }
    
    // Generate the MAC
    unsigned char mac[8];
    if (generate_iso8583_mac(message, *length, mac) != 0) {
        return -1;
    }
    
    // Set bit 128 in the bitmap
    // This assumes the message format starts with 4 bytes MTI + 8 bytes primary bitmap
    unsigned char* bitmap = message + 4;
    bitmap[15] |= 0x01; // Set bit 128 in the last byte of secondary bitmap
    
    // Append the MAC to the message
    memcpy(message + *length, mac, 8);
    *length += 8;
    
    return 0;
}
