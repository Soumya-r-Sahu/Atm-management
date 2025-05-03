#ifndef INPUT_VALIDATOR_H
#define INPUT_VALIDATOR_H

#include <stdbool.h>

// Card number validation (typically 6 digits in our system)
bool is_valid_card_number(int card_number);

// PIN validation (4 digits)
bool is_valid_pin(int pin);

// Amount validation for withdrawals and deposits
bool is_valid_amount(double amount);

// Money format validation (e.g., "$123.45")
bool is_valid_money_format(const char* money_string);

// Email format validation
bool is_valid_email(const char* email);

// Phone number format validation
bool is_valid_phone(const char* phone);

// Date format validation (YYYY-MM-DD)
bool is_valid_date(const char* date);

// Clear input buffer (when reading from console)
void clear_input_buffer(void);

// Get a validated integer input from user
int get_validated_int(const char* prompt, int min, int max);

// Get a validated double input from user
double get_validated_double(const char* prompt, double min, double max);

// Get a validated string input from user
void get_validated_string(const char* prompt, char* buffer, int max_length);

// Get a secured (non-echoed) input like PIN or password
void get_secured_input(const char* prompt, char* buffer, int max_length);

#endif // INPUT_VALIDATOR_H