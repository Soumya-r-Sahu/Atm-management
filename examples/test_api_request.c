/**
 * @file test_api_request.c
 * @brief Example: REST API call test
 * 
 * This file provides examples of making REST API calls to the
 * Core Banking System's API endpoints.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../include/global.h"
#include "../include/error_codes.h"

// Structure to hold response data from curl
struct MemoryStruct {
    char *memory;
    size_t size;
};

/**
 * @brief Callback function for curl to write received data
 */
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

/**
 * @brief Example of getting account information via REST API
 */
void test_get_account_info() {
    printf("===== Testing GET Account Info API =====\n");
    
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    
    // Initialize the memory structure
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if (curl) {
        // Set URL for the request
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/api/accounts/ACC123456789");
        
        // Set callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        
        // Set authentication header
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Authorization: Bearer YOUR_TOKEN_HERE");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // Execute the request
        res = curl_easy_perform(curl);
        
        // Check for errors
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Process the response
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            
            printf("HTTP Response Code: %ld\n", http_code);
            
            if (http_code == 200) {
                printf("Response: %s\n", chunk.memory);
                
                // In a real application, we would parse the JSON response here
                printf("(This is a placeholder - in a real implementation, JSON parsing would occur here)\n");
            } else {
                printf("API call failed with HTTP code %ld\n", http_code);
                printf("Error response: %s\n", chunk.memory);
            }
        }
        
        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    free(chunk.memory);
}

/**
 * @brief Example of creating a new transaction via REST API
 */
void test_create_transaction() {
    printf("\n===== Testing POST Create Transaction API =====\n");
    
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    
    // Transaction data in JSON format
    const char *transaction_json = "{"
        "\"account_id\": \"ACC123456789\","
        "\"transaction_type\": \"CREDIT\","
        "\"amount\": 500.00,"
        "\"description\": \"API Test Deposit\","
        "\"reference\": \"REF123456\""
    "}";
    
    // Initialize the memory structure
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if (curl) {
        // Set URL for the request
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/api/transactions");
        
        // Set POST method
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        
        // Set POST data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, transaction_json);
        
        // Set callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        
        // Set authentication header
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Authorization: Bearer YOUR_TOKEN_HERE");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // Execute the request
        res = curl_easy_perform(curl);
        
        // Check for errors
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Process the response
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            
            printf("HTTP Response Code: %ld\n", http_code);
            
            if (http_code == 201) { // Created
                printf("Transaction created successfully\n");
                printf("Response: %s\n", chunk.memory);
                
                // In a real application, we would parse the JSON response here
                printf("(This is a placeholder - in a real implementation, JSON parsing would occur here)\n");
            } else {
                printf("API call failed with HTTP code %ld\n", http_code);
                printf("Error response: %s\n", chunk.memory);
            }
        }
        
        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    free(chunk.memory);
}

int main() {
    printf("REST API Test Application\n");
    printf("========================\n\n");
    
    // Test GET request for account information
    test_get_account_info();
    
    // Test POST request for creating a transaction
    test_create_transaction();
    
    return 0;
}
