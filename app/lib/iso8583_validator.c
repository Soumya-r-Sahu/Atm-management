/**
 * @file iso8583_validator.c
 * @brief Validates ISO8583 messages against schema and business rules
 * 
 * This file provides comprehensive validation for ISO8583 messages
 * to ensure they comply with message format and business rules.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/global.h"
#include "../include/error_codes.h"

// Forward declaration for the ISO8583_Message structure from iso8583_parser.c
typedef struct {
    char message_type[5];     // Message type (0100, 0110, etc.)
    uint8_t bitmap[16];       // Primary and secondary bitmap (128 bits)
    void* field_data[129];    // Field data (1-128, 0 unused)
    size_t field_length[129]; // Length of each field
} ISO8583_Message;

// Error codes specific to validation
#define ERROR_INVALID_FIELD_FORMAT 1001
#define ERROR_FIELD_TOO_LONG 1002
#define ERROR_FIELD_TOO_SHORT 1003
#define ERROR_INVALID_FIELD_VALUE 1004
#define ERROR_INCONSISTENT_FIELDS 1005
#define ERROR_MISSING_MANDATORY_FIELD 1006

/**
 * @brief Validates an ISO8583 message against schema
 * 
 * @param message The ISO8583 message to validate
 * @param error_field Output parameter to indicate which field had an error
 * @param error_detail Output buffer for error details
 * @param error_detail_size Size of the error_detail buffer
 * @return int 0 if valid, error code if invalid
 */
int validate_iso8583_schema(const void* message, int* error_field, char* error_detail, size_t error_detail_size) {
    if (!message || !error_field || !error_detail) {
        return ERROR_INVALID_PARAMETER;
    }
    
    const ISO8583_Message* iso_msg = (const ISO8583_Message*)message;
    
    // Check message type
    if (strlen(iso_msg->message_type) != 4) {
        *error_field = 0; // Message type is field 0
        snprintf(error_detail, error_detail_size, "Invalid message type length: %s", iso_msg->message_type);
        return ERROR_INVALID_FIELD_FORMAT;
    }
    
    // Check if message type is valid
    if (iso_msg->message_type[0] != '0' || 
        (iso_msg->message_type[1] != '1' && iso_msg->message_type[1] != '2' &&
         iso_msg->message_type[1] != '4' && iso_msg->message_type[1] != '8') ||
        (iso_msg->message_type[2] != '0' && iso_msg->message_type[2] != '1') ||
        (iso_msg->message_type[3] != '0')) {
        
        *error_field = 0; // Message type is field 0
        snprintf(error_detail, error_detail_size, "Invalid message type value: %s", iso_msg->message_type);
        return ERROR_INVALID_FIELD_VALUE;
    }
    
    // For each field in the bitmap, validate it
    for (int field = 1; field <= 128; field++) {
        int byte_pos = (field - 1) / 8;
        int bit_pos = 7 - ((field - 1) % 8);
        
        if (iso_msg->bitmap[byte_pos] & (1 << bit_pos)) {
            // Field is present, validate it
            if (!iso_msg->field_data[field]) {
                *error_field = field;
                snprintf(error_detail, error_detail_size, "Field %d is marked present in bitmap but has no data", field);
                return ERROR_MISSING_MANDATORY_FIELD;
            }
            
            // Validate field format based on field number
            switch (field) {
                case 2: // PAN - LLVAR numeric
                    if (iso_msg->field_length[field] < 1 || iso_msg->field_length[field] > 19) {
                        *error_field = field;
                        snprintf(error_detail, error_detail_size, "Field %d (PAN) has invalid length: %zu", 
                                field, iso_msg->field_length[field]);
                        return ERROR_INVALID_FIELD_FORMAT;
                    }
                    // Check if all characters are numeric
                    for (size_t i = 0; i < iso_msg->field_length[field]; i++) {
                        if (((char*)iso_msg->field_data[field])[i] < '0' || 
                            ((char*)iso_msg->field_data[field])[i] > '9') {
                            *error_field = field;
                            snprintf(error_detail, error_detail_size, "Field %d (PAN) contains non-numeric character", field);
                            return ERROR_INVALID_FIELD_VALUE;
                        }
                    }
                    break;
                
                case 3: // Processing Code - fixed 6 digits numeric
                    if (iso_msg->field_length[field] != 6) {
                        *error_field = field;
                        snprintf(error_detail, error_detail_size, "Field %d (Processing Code) has invalid length: %zu", 
                                field, iso_msg->field_length[field]);
                        return ERROR_INVALID_FIELD_FORMAT;
                    }
                    // Check if all characters are numeric
                    for (size_t i = 0; i < 6; i++) {
                        if (((char*)iso_msg->field_data[field])[i] < '0' || 
                            ((char*)iso_msg->field_data[field])[i] > '9') {
                            *error_field = field;
                            snprintf(error_detail, error_detail_size, "Field %d (Processing Code) contains non-numeric character", field);
                            return ERROR_INVALID_FIELD_VALUE;
                        }
                    }
                    break;
                
                case 4: // Transaction Amount - fixed 12 digits numeric
                    if (iso_msg->field_length[field] != 12) {
                        *error_field = field;
                        snprintf(error_detail, error_detail_size, "Field %d (Amount) has invalid length: %zu", 
                                field, iso_msg->field_length[field]);
                        return ERROR_INVALID_FIELD_FORMAT;
                    }
                    // Check if all characters are numeric
                    for (size_t i = 0; i < 12; i++) {
                        if (((char*)iso_msg->field_data[field])[i] < '0' || 
                            ((char*)iso_msg->field_data[field])[i] > '9') {
                            *error_field = field;
                            snprintf(error_detail, error_detail_size, "Field %d (Amount) contains non-numeric character", field);
                            return ERROR_INVALID_FIELD_VALUE;
                        }
                    }
                    break;
                
                // Add more field validations as needed...
            }
        }
    }
    
    return 0; // All validations passed
}

/**
 * @brief Validates business rules for an ISO8583 message
 * 
 * @param message The ISO8583 message to validate
 * @param error_field Output parameter to indicate which field had an error
 * @param error_detail Output buffer for error details
 * @param error_detail_size Size of the error_detail buffer
 * @return int 0 if valid, error code if invalid
 */
int validate_iso8583_business_rules(const void* message, int* error_field, char* error_detail, size_t error_detail_size) {
    if (!message || !error_field || !error_detail) {
        return ERROR_INVALID_PARAMETER;
    }
    
    const ISO8583_Message* iso_msg = (const ISO8583_Message*)message;
    
    // Check message type specific validations
    if (strcmp(iso_msg->message_type, "0100") == 0 || strcmp(iso_msg->message_type, "0110") == 0) {
        // Authorization request/response
        
        // Check for required fields
        int required_fields[] = {2, 3, 7, 11, 0}; // 0 terminates the list
        for (int i = 0; required_fields[i] != 0; i++) {
            int field = required_fields[i];
            int byte_pos = (field - 1) / 8;
            int bit_pos = 7 - ((field - 1) % 8);
            
            if (!(iso_msg->bitmap[byte_pos] & (1 << bit_pos))) {
                *error_field = field;
                snprintf(error_detail, error_detail_size, "Required field %d is missing for message type %s", 
                        field, iso_msg->message_type);
                return ERROR_MISSING_MANDATORY_FIELD;
            }
        }
        
        // Check processing code specific validations
        char proc_code[7] = {0};
        if (iso_msg->field_data[3]) {
            strncpy(proc_code, (char*)iso_msg->field_data[3], 6);
            proc_code[6] = '\0';
            
            if (strcmp(proc_code, "000000") != 0 && // Purchase
                strcmp(proc_code, "010000") != 0 && // Cash Withdrawal
                strcmp(proc_code, "200000") != 0 && // Return/Refund
                strcmp(proc_code, "210000") != 0 && // Deposit
                strcmp(proc_code, "300000") != 0 && // Balance Inquiry
                strcmp(proc_code, "400000") != 0 && // Fund Transfer
                strcmp(proc_code, "920000") != 0 && // Pin Change
                strcmp(proc_code, "940000") != 0) { // Pin Verification
                
                *error_field = 3;
                snprintf(error_detail, error_detail_size, "Invalid processing code: %s", proc_code);
                return ERROR_INVALID_FIELD_VALUE;
            }
            
            // If this is a financial transaction, ensure amount field is present
            if ((strcmp(proc_code, "000000") == 0 || // Purchase
                 strcmp(proc_code, "010000") == 0 || // Cash Withdrawal
                 strcmp(proc_code, "200000") == 0 || // Return/Refund
                 strcmp(proc_code, "210000") == 0 || // Deposit
                 strcmp(proc_code, "400000") == 0) && // Fund Transfer
                strcmp(iso_msg->message_type, "0100") == 0) {
                
                int byte_pos = (4 - 1) / 8;
                int bit_pos = 7 - ((4 - 1) % 8);
                
                if (!(iso_msg->bitmap[byte_pos] & (1 << bit_pos))) {
                    *error_field = 4;
                    snprintf(error_detail, error_detail_size, "Amount field (4) is required for financial transaction with processing code %s", 
                            proc_code);
                    return ERROR_MISSING_MANDATORY_FIELD;
                }
            }
        }
    } else if (strcmp(iso_msg->message_type, "0200") == 0 || strcmp(iso_msg->message_type, "0210") == 0) {
        // Financial transaction request/response
        
        // Check for required fields
        int required_fields[] = {2, 3, 4, 7, 11, 41, 0}; // 0 terminates the list
        for (int i = 0; required_fields[i] != 0; i++) {
            int field = required_fields[i];
            int byte_pos = (field - 1) / 8;
            int bit_pos = 7 - ((field - 1) % 8);
            
            if (!(iso_msg->bitmap[byte_pos] & (1 << bit_pos))) {
                *error_field = field;
                snprintf(error_detail, error_detail_size, "Required field %d is missing for message type %s", 
                        field, iso_msg->message_type);
                return ERROR_MISSING_MANDATORY_FIELD;
            }
        }
        
        // Additional validations for financial transactions
        
        // For funds transfer, check that source and destination accounts are present
        char proc_code[7] = {0};
        if (iso_msg->field_data[3]) {
            strncpy(proc_code, (char*)iso_msg->field_data[3], 6);
            proc_code[6] = '\0';
            
            if (strcmp(proc_code, "400000") == 0) { // Fund Transfer
                int byte_pos_102 = (102 - 1) / 8;
                int bit_pos_102 = 7 - ((102 - 1) % 8);
                int byte_pos_103 = (103 - 1) / 8;
                int bit_pos_103 = 7 - ((103 - 1) % 8);
                
                if (!(iso_msg->bitmap[byte_pos_102] & (1 << bit_pos_102)) ||
                    !(iso_msg->bitmap[byte_pos_103] & (1 << bit_pos_103))) {
                    *error_field = (!(iso_msg->bitmap[byte_pos_102] & (1 << bit_pos_102))) ? 102 : 103;
                    snprintf(error_detail, error_detail_size, "Fields 102 and 103 are required for funds transfer");
                    return ERROR_MISSING_MANDATORY_FIELD;
                }
            }
        }
    }
    
    // Check timestamp fields for validity
    if (iso_msg->field_data[7]) {
        char timestamp[11] = {0};
        strncpy(timestamp, (char*)iso_msg->field_data[7], 10);
        timestamp[10] = '\0';
        
        if (strlen(timestamp) != 10) {
            *error_field = 7;
            snprintf(error_detail, error_detail_size, "Invalid timestamp format in field 7");
            return ERROR_INVALID_FIELD_FORMAT;
        }
        
        // Extract components
        int month = (timestamp[0] - '0') * 10 + (timestamp[1] - '0');
        int day = (timestamp[2] - '0') * 10 + (timestamp[3] - '0');
        int hour = (timestamp[4] - '0') * 10 + (timestamp[5] - '0');
        int minute = (timestamp[6] - '0') * 10 + (timestamp[7] - '0');
        int second = (timestamp[8] - '0') * 10 + (timestamp[9] - '0');
        
        // Validate components
        if (month < 1 || month > 12 || day < 1 || day > 31 ||
            hour < 0 || hour > 23 || minute < 0 || minute > 59 ||
            second < 0 || second > 59) {
            *error_field = 7;
            snprintf(error_detail, error_detail_size, "Invalid date or time in timestamp: %s", timestamp);
            return ERROR_INVALID_FIELD_VALUE;
        }
    }
    
    return 0; // All validations passed
}
