CC	   ?= clang
CFLAGS += -Wall -Wextra -pedantic -O2
PROG   := snake
SRC	   := src/snake.c
LIBS	= -lcurses

all: snake

snake: src/snake.c src/config.h
	$(CC) $(CFLAGS) -o snake src/snake.c -lcurses

clean:
	rm -f snake
