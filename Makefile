CC		= gcc
PROG	= snake
SRC		= src/snake.c
LIBS	= -lcurses

all: snake

snake:
	$(CC) -o $(PROG) $(SRC) $(LIBS)

clean:
	@rm -f $(PROG)
