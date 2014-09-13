CC 	   ?= gcc
CFLAGS += -Wall
PROG	= snake
SRC		= src/snake.c
LIBS	= -lcurses

all: snake

snake: src/snake.c src/snake.h
	$(CC) $(CFLAGS) -o $(PROG) $(SRC) $(LIBS)

clean:
	@rm -f $(PROG)
