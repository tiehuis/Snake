#include <stdlib.h>
#include <unistd.h>
#include <curses.h>

enum movement {DOWN, RIGHT, UP, LEFT};
enum colorset {GREEN = 1, MAGENTA};

typedef struct snake {
    int length;
    int direction;
    int speed;
    int *xs;
    int *ys;
} SNAKE;

typedef struct fruit {
    int x;
    int y;
} FRUIT;

typedef struct win {
    WINDOW *window;
    int x;
    int y;
    int x_offset;
    int y_offset;
    int border;
} WIN;

typedef struct score {
    char *path;
    int high;
    int current;
} SCORE;


#define HELP_MSG\
    "Controls:\n"\
    "    hjkl, ARROW_KEYS   Movement\n"\
    "    p                  Pause menu\n"\
    "    q                  Quit session\n"\
    "\n"\
    "Usage:\n"\
    "    snake [options]\n"\
    "\n"\
    "Options:\n"\
    "    -h, --help         Display help information\n"\
    "    -b, --borders      Enable borders\n"\
    "    -r, --reset        Reset all saved scores\n"\
    "\n"


// Don't worry about splitting into too many functions just yet.
int main(int argc, char **argv)
{
    // Declare initial variables we need to use.
    SNAKE _snake;  SNAKE *snake  = &_snake;
    FRUIT _fruit;  FRUIT *fruit  = &_fruit;
    SCORE _score;  SCORE *score  = &_score;
    WIN _scr_win;  WIN *scr_win  = &_scr_win;
    WIN _game_win; WIN *game_win = &_game_win;

    // set initial variables
    score->path = ".snhs";
    game_win->border = 0;
    srand((unsigned int)time(NULL));

    // counter variable
    int i;
    // variable to store keypress
    int ch;

    // parse options
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b" ) == 0 || strcmp(argv[i], "--borders") == 0) {
            game_win->border = 1;
            score->path      = ".sbhs";
        }
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf(HELP_MSG);
            exit(EXIT_SUCCESS);
        }
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--reset") == 0) {
            // reset hiscores
            printf("Hiscores successfully reset!\n");
            exit(EXIT_SUCCESS);
        }
    }

    // initialize the ncurses environment
    initscr(); start_color(); cbreak; noecho(); keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); curs_set(0); timeout(0);

    // initialize win values
    scr_win->x         = COLS;
    scr_win->y         = 1;
    scr_win->x_offset  = 0;
    scr_win->y_offset  = 0;
    scr_win->border    = 0;
    scr_win->window    = newwin(scr_win->y, scr_win->x, scr_win->y_offset, scr_win->x_offset);

    game_win->x        = COLS;
    game_win->y        = LINES - scr_win->y;
    game_win->x_offset = 0;
    game_win->y_offset = scr_win->y;
    game_win->window   = newwin(game_win->y, game_win->x, game_win->y_offset, game_win->x_offset);

    // initialize starting variables
    score->current = 0;
    score->high    = 0; //get hiscore
    
    snake->length = 6;
    snake->direction = RIGHT;
    snake->speed = 40000;
    snake->xs = malloc(sizeof(int) * snake->length);
    snake->ys = malloc(sizeof(int) * snake->length);

    fruit->x = rand() % (game_win->x - 2) + 1;
    fruit->y = rand() % (game_win->y - 2) + 1;

    // setting initial snake values
    for (i = 0; i < snake->length; i++) {
        snake->ys[i] = LINES/2;
        snake->xs[i] = COLS/2 - i;
    }

    // define colors to use
    init_pair(GREEN,   COLOR_GREEN,   COLOR_BLACK);
    init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);

    // draw static parts of the screen
    box(game_win->window, 0, 0);
    mvwprintw(scr_win->window, 0, 1,  "Score: %d", score->current);
    mvwprintw(scr_win->window, 0, 31, "Hiscore: %d", score->high);
    if (game_win->border == 1)
        mvwprintw(scr_win->window, 0, 61, "Borders on!");

    // draw the snake on the board before beginning
    wattron(game_win->window, COLOR_PAIR(GREEN));
    mvwprintw(game_win->window, snake->ys[0], snake->xs[0], "@");
    for (i = 1; i < snake->length; i++)
        mvwprintw(game_win->window, snake->ys[i], snake->xs[i], "*");
    wattroff(game_win->window, COLOR_PAIR(GREEN));

    // begin the main game loop
    while (1) {
        // check for a keypress
        if ((ch = getch()) != ERR) {
            switch (ch) {
                case 'j':
                case KEY_DOWN:
                    if (snake->direction != UP)
                        snake->direction = DOWN;
                    break;
                case 'l':
                case KEY_RIGHT:
                    if (snake->direction != LEFT)
                        snake->direction = RIGHT;
                    break;
                case 'k':
                case KEY_UP:
                    if (snake->direction != DOWN)
                        snake->direction = UP;
                    break;
                case 'h':
                case KEY_LEFT:
                    if (snake->direction != RIGHT)
                        snake->direction = LEFT;
                    break;
                case 'p':
                    // pause menu
                    break;
                case 'q':
                    erase();
                    refresh();
                    endwin();
                    exit(EXIT_SUCCESS);
                    break;
            }
        }


        // update snake
        // remove snake tail from grid
        mvwprintw(game_win->window, snake->ys[snake->length-1], snake->xs[snake->length-1], " ");

        for (i = snake->length - 1; i > 0; i--) {
            snake->xs[i] = snake->xs[i-1];
            snake->ys[i] = snake->ys[i-1];
        }

        switch (snake->direction) {
            case DOWN:
                snake->ys[0] += 1;
                break;
            case RIGHT:
                snake->xs[0] += 1;
                break;
            case UP:
                snake->ys[0] -= 1;
                break;
            case LEFT:
                snake->xs[0] -= 1;
                break;
        }

        // check collisions
        // border collision
        if (game_win->border == 0) {
            if (snake->ys[0] > game_win->y - 2)
                snake->ys[0] = 1;

            if (snake->ys[0] < 1)
                snake->ys[0] = game_win->y - 2;

            if (snake->xs[0] > game_win->x - 2)
                snake->xs[0] = 1;

            if (snake->xs[0] < 1)
                snake->xs[0] = game_win->x - 2;
        }
        else {
            if (snake->ys[0] < 1 || snake->ys[0] > game_win->y - 1 ||
                snake->xs[0] < 1 || snake->xs[0] > game_win->x - 1) {
                erase();
                refresh();
                endwin();
                exit(EXIT_SUCCESS);
            }
                
        }

        // snake collision
        for (i = 1; i < snake->length - 1; i++) {
            if (snake->xs[0] == snake->xs[i] && snake->ys[0] == snake->ys[i]) {
                erase();
                refresh();
                endwin();
                exit(EXIT_SUCCESS);
            }
        }

        // fruit collision
        if (snake->xs[0] == fruit->x && snake->ys[0] == fruit->y) {
            score->current += 2 * snake->length++;
            mvwprintw(scr_win->window, 0, 1, "Score: %d", score->current);
            snake->xs = realloc(snake->xs, sizeof(int) * snake->length);
            snake->ys = realloc(snake->ys, sizeof(int) * snake->length);
            fruit->x = rand() % (game_win->x - 2) + 1;
            fruit->y = rand() % (game_win->y - 2) + 1;
        }

        // draw fruit
        wattron(game_win->window, COLOR_PAIR(MAGENTA));
        mvwprintw(game_win->window, fruit->y, fruit->x, "#");
        wattroff(game_win->window, COLOR_PAIR(MAGENTA));

        // draw snake
        wattron(game_win->window, COLOR_PAIR(GREEN));
        mvwprintw(game_win->window, snake->ys[0], snake->xs[0], "@");
        mvwprintw(game_win->window, snake->ys[1], snake->xs[1], "*");
        wattroff(game_win->window, COLOR_PAIR(GREEN));

        // refresh windows
        wrefresh(game_win->window);
        wrefresh(scr_win->window);

        // sleep
        usleep(snake->speed < 15000 ? 15000 : snake->speed--);
    }
}
