/**
 * @file test_iso8583.c
 * @brief Example: Parse and build ISO 8583 messages
 * 
 * This file provides examples of parsing and building ISO 8583 messages
 * for financial transaction processing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/global.h"
#include "../application/iso8583_parser.c"
#include "../application/iso8583_builder.c"

// Sample ISO 8583 message (hexadecimal representation for a balance inquiry)
const unsigned char sample_iso_message[] = {
    0x30, 0x31, 0x30, 0x30, // Message type (0100 - Authorization request)
    0x82, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Primary bitmap
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Secondary bitmap
    0x16, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, // Field 2: PAN
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Field 3: Processing code
    0x00, 0x00, 0x00, 0x00, 0x10, 0x05, 0x23, // Field 7: Transmission date & time
    0x12, 0x34, 0x56, // Field 11: STAN
    0x10, 0x05, 0x23, // Field 12: Local time
    0x10, 0x05, // Field 13: Local date
    0x06, 0x12, 0x34, 0x56, // Field 32: Acquiring institution ID
    0x41, 0x54, 0x4d, 0x31, 0x32, 0x33, 0x34, 0x35, // Field 41: Terminal ID
    0x84, 0x61, 0x98, // Field 49: Currency code
};

/**
 * @brief Example of parsing an ISO 8583 message
 */
void test_parse_iso_message() {
    printf("===== Testing ISO 8583 Message Parsing =====\n");
    
    void* parsed_data = malloc(1024); // Allocate space for parsed data
    if (!parsed_data) {
        printf("Memory allocation failed\n");
        return;
    }
    
    int result = parse_iso_message(sample_iso_message, sizeof(sample_iso_message), parsed_data);
    
    if (result == SUCCESS) {
        printf("Successfully parsed ISO 8583 message\n");
        
        // Example of extracting fields
        char pan[20];
        if (get_iso_field(parsed_data, 2, pan, sizeof(pan)) > 0) {
            printf("Field 2 (PAN): %s\n", pan);
        }
        
        // Validate fields
        if (validate_iso_fields(parsed_data) == SUCCESS) {
            printf("ISO 8583 fields validated successfully\n");
        } else {
            printf("ISO 8583 field validation failed\n");
        }
    } else {
        printf("Failed to parse ISO 8583 message: Error %d\n", result);
    }
    
    free(parsed_data);
}

/**
 * @brief Example of building an ISO 8583 message
 */
void test_build_iso_message() {
    printf("\n===== Testing ISO 8583 Message Building =====\n");
    
    // Create a new message
    void* message = create_iso_message(ISO_MSG_AUTH_REQUEST);
    if (!message) {
        printf("Failed to create ISO 8583 message\n");
        return;
    }
    
    // Set fields in the message
    
    // Field 2: Primary Account Number (PAN)
    const char pan[] = "4111111111111111";
    set_iso_field(message, 2, pan, strlen(pan));
    
    // Field 3: Processing Code (balance inquiry)
    const char proc_code[] = "300000";
    set_iso_field(message, 3, proc_code, strlen(proc_code));
    
    // Field 7: Transmission Date and Time
    const char datetime[] = "0510194622";
    set_iso_field(message, 7, datetime, strlen(datetime));
    
    // Field 11: System Trace Audit Number
    const char stan[] = "123456";
    set_iso_field(message, 11, stan, strlen(stan));
    
    // Field 32: Acquiring Institution ID
    const char acq_id[] = "12345";
    set_iso_field(message, 32, acq_id, strlen(acq_id));
    
    // Field 41: Card Acceptor Terminal ID
    const char terminal_id[] = "ATM12345";
    set_iso_field(message, 41, terminal_id, strlen(terminal_id));
    
    // Field 49: Transaction Currency Code
    const char currency[] = "840"; // USD
    set_iso_field(message, 49, currency, strlen(currency));
    
    // Build the final message
    unsigned char output[1024];
    int msg_len = build_iso_message(message, output, sizeof(output));
    
    if (msg_len > 0) {
        printf("Successfully built ISO 8583 message of %d bytes\n", msg_len);
        
        printf("Message Hex Dump: ");
        for (int i = 0; i < msg_len && i < 64; i++) {
            printf("%02X ", output[i]);
            if ((i + 1) % 16 == 0) printf("\n                  ");
        }
        printf("\n");
    } else {
        printf("Failed to build ISO 8583 message\n");
    }
    
    // Clean up
    // TODO: Implement and call free_iso_message(message);
}

int main() {
    printf("ISO 8583 Message Test Application\n");
    printf("=================================\n\n");
    
    // Test parsing
    test_parse_iso_message();
    
    // Test building
    test_build_iso_message();
    
    return 0;
}
