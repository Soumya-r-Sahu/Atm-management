CC = gcc
CFLAGS = -Wall -Wextra -I./src
SRC = src/main.c src/card_num_validation.c src/database.c src/pin_validation.c src/utils/logger.c
OBJ = $(SRC:.c=.o)
EXEC = run

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
