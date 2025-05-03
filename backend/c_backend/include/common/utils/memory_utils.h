#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <stddef.h>

/**
 * Safe memory allocation with error handling
 * 
 * @param size Size of memory to allocate in bytes
 * @param description Description of the allocation (for logging)
 * @return Pointer to allocated memory or NULL if allocation failed
 */
void* safe_malloc(size_t size, const char* description);

/**
 * Safe memory reallocation with error handling
 * 
 * @param ptr Existing pointer to reallocate
 * @param size New size to allocate
 * @param description Description of the allocation (for logging)
 * @return Pointer to reallocated memory or NULL if reallocation failed
 */
void* safe_realloc(void* ptr, size_t size, const char* description);

/**
 * Safe memory deallocation with NULL check
 * 
 * @param ptr Pointer to memory that should be freed
 * @return 1 on success, 0 if pointer was already NULL
 */
int safe_free(void** ptr);

/**
 * Track memory allocation for debugging
 * 
 * @param ptr Pointer to allocated memory
 * @param size Size of allocated memory
 * @param file Source file where allocation occurred
 * @param line Line number where allocation occurred
 * @param description Description of the allocation
 */
void track_allocation(void* ptr, size_t size, const char* file, int line, const char* description);

/**
 * Track memory deallocation for debugging
 * 
 * @param ptr Pointer that was freed
 */
void track_deallocation(void* ptr);

/**
 * Print memory usage statistics
 */
void print_memory_stats(void);

/**
 * Cleanup all tracked memory at program exit
 */
void cleanup_memory(void);

// Convenience macros for memory management with tracking
#define MALLOC(size, desc) safe_malloc(size, desc)
#define REALLOC(ptr, size, desc) safe_realloc(ptr, size, desc)
#define FREE(ptr) safe_free((void**)&(ptr))

#endif // MEMORY_UTILS_H