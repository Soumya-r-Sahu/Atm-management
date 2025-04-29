#include "memory_utils.h"
#include "../common/error_handler.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global error message buffer
static char g_error_message[256] = {0};

// Memory allocation tracking structure
typedef struct MemoryAlloc {
    void* ptr;               // Pointer to allocated memory
    size_t size;             // Size of allocation
    char file[128];          // Source file
    int line;                // Line number
    char description[64];    // Description
    struct MemoryAlloc* next;// Next in list
} MemoryAlloc;

// Head of the allocation tracking list
static MemoryAlloc* alloc_list = NULL;

// Stats
static size_t total_allocations = 0;
static size_t current_allocations = 0;
static size_t peak_memory_usage = 0;
static size_t current_memory_usage = 0;

// Thread safety would require mutex/lock here in a multi-threaded environment

// Safe memory allocation with error handling
void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL && size > 0) {
        snprintf(g_error_message, sizeof(g_error_message), 
                "Memory allocation failed for %zu bytes", size);
        writeErrorLog(g_error_message);
    }
    return ptr;
}

// Set error message
void error_set(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_error_message, sizeof(g_error_message), format, args);
    va_end(args);
    
    // Log the error
    writeErrorLog(g_error_message);
}

// Get last error message
const char* error_get(void) {
    return g_error_message;
}

// Safe memory reallocation
void* safe_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL && size > 0) {
        snprintf(g_error_message, sizeof(g_error_message), 
                "Memory reallocation failed for %zu bytes", size);
        writeErrorLog(g_error_message);
    }
    return new_ptr;
}

// Safe string duplication
char* safe_strdup(const char* str) {
    if (str == NULL) {
        snprintf(g_error_message, sizeof(g_error_message), 
                "Attempted to duplicate NULL string");
        writeErrorLog(g_error_message);
        return NULL;
    }
    
    char* new_str = strdup(str);
    if (new_str == NULL) {
        snprintf(g_error_message, sizeof(g_error_message), 
                "String duplication failed for %zu bytes", strlen(str) + 1);
        writeErrorLog(g_error_message);
    }
    return new_str;
}

// Safe free - checks for NULL pointer
void safe_free(void* ptr) {
    if (ptr != NULL) {
        free(ptr);
    }
}

// Track memory allocation for debugging
void track_allocation(void* ptr, size_t size, const char* file, int line, const char* description) {
    if (!ptr) return;
    
    MemoryAlloc* alloc = (MemoryAlloc*)malloc(sizeof(MemoryAlloc));
    if (!alloc) {
        // Can't track this allocation, but at least log it
        writeErrorLog("Failed to track memory allocation");
        return;
    }
    
    // Fill in allocation details
    alloc->ptr = ptr;
    alloc->size = size;
    strncpy(alloc->file, file, sizeof(alloc->file) - 1);
    alloc->file[sizeof(alloc->file) - 1] = '\0';
    alloc->line = line;
    strncpy(alloc->description, description, sizeof(alloc->description) - 1);
    alloc->description[sizeof(alloc->description) - 1] = '\0';
    
    // Add to list
    alloc->next = alloc_list;
    alloc_list = alloc;
    
    // Update stats
    total_allocations++;
    current_allocations++;
    current_memory_usage += size;
    if (current_memory_usage > peak_memory_usage) {
        peak_memory_usage = current_memory_usage;
    }
}

// Track memory deallocation for debugging
void track_deallocation(void* ptr) {
    if (!ptr) return;
    
    // Find allocation in list
    MemoryAlloc** pp = &alloc_list;
    MemoryAlloc* alloc = alloc_list;
    
    while (alloc) {
        if (alloc->ptr == ptr) {
            // Remove from list
            *pp = alloc->next;
            
            // Update stats
            current_allocations--;
            current_memory_usage -= alloc->size;
            
            // Free tracking structure
            free(alloc);
            return;
        }
        pp = &alloc->next;
        alloc = alloc->next;
    }
    
    // Pointer not found in tracking list
    writeInfoLog("Attempted to free untracked memory");
}

// Print memory usage statistics
void print_memory_stats(void) {
    char msg[256];
    sprintf(msg, "Memory Stats: Total: %zu, Current: %zu, Peak Usage: %zu bytes, Current Usage: %zu bytes",
            total_allocations, current_allocations, peak_memory_usage, current_memory_usage);
    writeInfoLog(msg);
    
    // Print any memory leaks
    if (current_allocations > 0) {
        sprintf(msg, "WARNING: %zu memory leaks detected", current_allocations);
        writeErrorLog(msg);
        
        // List all leaks
        MemoryAlloc* alloc = alloc_list;
        while (alloc) {
            sprintf(msg, "Leak: %zu bytes at %p for '%s' from %s:%d",
                    alloc->size, alloc->ptr, alloc->description, alloc->file, alloc->line);
            writeErrorLog(msg);
            alloc = alloc->next;
        }
    }
}

// Cleanup all tracked memory at program exit
void cleanup_memory(void) {
    writeInfoLog("Performing memory cleanup");
    
    // Free all remaining allocations
    while (alloc_list) {
        void* ptr = alloc_list->ptr;
        writeInfoLog("Cleaning up unfreed memory");
        free(ptr); // Free the actual memory
        
        // Remove from list
        MemoryAlloc* next = alloc_list->next;
        free(alloc_list); // Free the tracking structure
        alloc_list = next;
    }
    
    // Reset stats
    current_allocations = 0;
    current_memory_usage = 0;
}