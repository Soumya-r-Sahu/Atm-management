# Makefile for Core Banking System Menu Structure

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -I../..

# Source files
MENU_SOURCES = \
	menu_utils.c \
	main_menu.c \
	customer_menu.c \
	account_menu.c \
	transaction_menu.c \
	beneficiary_menu.c \
	bill_payment_menu.c \
	atm_operations_menu.c \
	admin_menu.c \
	cbs_admin_menu.c

TEST_SOURCE = menu_test.c

# Object files
MENU_OBJECTS = $(MENU_SOURCES:.c=.o)
TEST_OBJECT = $(TEST_SOURCE:.c=.o)

# Target executables
TEST_EXECUTABLE = menu_test

# Default target
all: $(TEST_EXECUTABLE)

# Test executable
$(TEST_EXECUTABLE): $(MENU_OBJECTS) $(TEST_OBJECT)
	$(CC) $(CFLAGS) -o $@ $^

# Object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(MENU_OBJECTS) $(TEST_OBJECT) $(TEST_EXECUTABLE)

# Run test
run: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

# Dependencies
menu_utils.o: menu_utils.c menu_utils.h
main_menu.o: main_menu.c menu_system.h menu_types.h menu_utils.h
customer_menu.o: customer_menu.c menu_system.h menu_types.h menu_utils.h
account_menu.o: account_menu.c menu_system.h menu_types.h menu_utils.h
transaction_menu.o: transaction_menu.c menu_system.h menu_types.h menu_utils.h
beneficiary_menu.o: beneficiary_menu.c menu_system.h menu_types.h menu_utils.h
bill_payment_menu.o: bill_payment_menu.c menu_system.h menu_types.h menu_utils.h
atm_operations_menu.o: atm_operations_menu.c menu_system.h menu_types.h menu_utils.h
admin_menu.o: admin_menu.c menu_system.h menu_types.h menu_utils.h
cbs_admin_menu.o: cbs_admin_menu.c menu_system.h menu_types.h menu_utils.h
menu_test.o: menu_test.c menu_system.h menu_types.h menu_utils.h

.PHONY: all clean run
