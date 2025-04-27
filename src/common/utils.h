#ifndef UTILS_H
#define UTILS_H

// String validation function
int is_valid_string(const char *str);

// Secure PIN entry function that masks input with asterisks
void secure_pin_entry(char *pin, int max_length);

// Clear input buffer to prevent input synchronization issues
void clear_input_buffer();

// Validate that an integer is within a specific range
int validate_int_range(int value, int min, int max);

// Generate a random number in a given range (inclusive)
int generate_random_in_range(int min, int max);

#endif // UTILS_H