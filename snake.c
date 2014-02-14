#include <stdlib.h>
#include <unistd.h>
#include <curses.h>

inline void endgame()
{         
    erase();
    refresh();
    endwin();
    exit(EXIT_SUCCESS);
}

int main()
{
    initscr();
    start_color();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);
    timeout(0);
    
    srand((unsigned int)time(NULL));

    // variables
    int score = 0;
    int length = 6;
    int direction = 1;
    int speed = 40000;

    // fruit position
    int xfr = rand() % COLS;
    int yfr = rand() % LINES;

    // snake array
    int *xpos = malloc(sizeof(int) * length);
    int *ypos = malloc(sizeof(int) * length);

    // initialize snake start values
    int i;
    for (i = 0; i < length; i++) {
        ypos[i] = LINES/2;
        xpos[i] = (COLS/2) - i;
    }

    // initialize windows
    WINDOW *win_game = newwin(LINES - 1, COLS, 1, 0);
    WINDOW *scores = newwin(1, COLS, 0, 0);
    box(win_game, 0, 0);
    wrefresh(scores);
    wrefresh(win_game);

    // initialize colors
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
    
    while (1) {
        // get keypress
        int ch = getch();
        if (ch != ERR) {
            switch (ch) {
                case 'j':
                    if (direction != 2) direction = 0; break;
                case 'l':
                    if (direction != 3) direction = 1; break;
                case 'k': 
                    if (direction != 0) direction = 2; break;
                case 'h':
                    if (direction != 1) direction = 3; break;
                case 'q':
                    endgame();
            }
        }
        
        // redraw border
        box(win_game, 0, 0);

        // redraw fruit
        wattron(win_game, COLOR_PAIR(2));
        mvwprintw(win_game, yfr, xfr, "#");
        wattroff(win_game, COLOR_PAIR(2));

        // print game details
        mvwprintw(scores, 0, 1, "Score: %d", score);

        // redraw fruit if out of game bounds
        if (xfr > COLS - 2 || yfr > LINES - 2) {
            xfr = rand() % (COLS - 2) + 1;
            yfr = rand() % (LINES - 3) + 1;
        }

        // get a new fruit and update score
        if (xpos[0] == xfr && ypos[0] == yfr) {
            score += 2*length++;
            xfr = rand() % (COLS - 3) + 1;
            yfr = rand() % (LINES - 5) + 1;
            xpos = realloc(xpos, sizeof(int) * length);
            ypos = realloc(ypos, sizeof(int) * length);
        }
        
        // reset tail square
        mvwprintw(win_game, ypos[length - 1], xpos[length - 1], " ");

        // move snake
        for (i = length - 1; i > 0; i--) {
            ypos[i] = ypos[i - 1];
            xpos[i] = xpos[i - 1];
        }

        // increment snake direction
        if      (direction == 0) {ypos[0] += 1;}
        else if (direction == 1) {xpos[0] += 1;}
        else if (direction == 2) {ypos[0] -= 1;}
        else                     {xpos[0] -= 1;}

        // no borders
        ypos[0] = ypos[0] < 1 ? LINES - 3 : ypos[0] > LINES - 3 ? 1 : ypos[0];
        xpos[0] = xpos[0] < 1 ? COLS - 2  : xpos[0] > COLS - 2  ? 1 : xpos[0];

        // check collisions
        for (i = 1; i < length - 1; i++)
            if (ypos[0] == ypos[i] && xpos[0] == xpos[i])
                endgame();

        // update snake
        wattron(win_game, COLOR_PAIR(1));
        mvwprintw(win_game, ypos[0], xpos[0], "@");
        mvwprintw(win_game, ypos[1], xpos[1], "*");
        wattroff(win_game, COLOR_PAIR(1));

        // refresh game window
        wrefresh(win_game);
        wrefresh(scores);

        // pause
        usleep(speed < 15000 ? speed : speed--);
    }
}
