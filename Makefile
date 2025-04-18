CC = gcc
CFLAGS = -Wall -Wextra -I./src
SRC = src/main.c src/database.c src/pin_validation.c src/utils/logger.c
OBJ = $(SRC:.c=.o)
EXEC = run
TEST_EXEC = test

all: $(EXEC) $(TEST_EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^

$(TEST_EXEC): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC) $(TEST_EXEC)

.PHONY: all clean