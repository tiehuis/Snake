#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <curses.h>
#include "config.h"

#if defined(__clang__) || (__GNUC__)
#   if (__GNUC__ >= 3) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 5))
#       define NORETURN__ __attribute__((noreturn))
#   endif
#endif

/* Initial capacity to use for snake arrays */
#define INIT_CAPACITY 32

/* Message to be displayed on opening help */
#define HELP_STR                                            \
    "Controls:\n"                                           \
    "    hjkl, ARROW_KEYS    Movement\n"                    \
    "    p                   Pause menu\n"                  \
    "    q                   Quit session\n"                \
    "\n"                                                    \
    "Usage:\n"                                              \
    "    snake [options]\n"                                 \
    "\n"                                                    \
    "Options:\n"                                            \
    "    -h, --help          Display help information\n"    \
    "    -b, --borders       Enable borders\n"              \
    "    -r, --reset         Reset all saved scores\n"

/* Global variables */
char *SCORE_FILE = NSCORE_FILE;
int hiscore;
int score;
int direction;
int speed;
int borders;
int xfr;
int yfr;

/* Snake variables */
int capacity;
int length;
int *xpos;
int *ypos;

WINDOW *scores;
WINDOW *win_game;

/* Function declarations (only those required are here currently) */
void game_main_loop(void) NORETURN__;
int hiscore_get(void);
void logic_generate_random_fruit(void);
void draw_blit_all(void);

/* Initialization and destruction routines */
void init_variables(void)
{
    srand((unsigned int)time(NULL));

    borders   = 0;
    score     = 0;
    hiscore   = hiscore_get();
    length    = INIT_LENGTH;
    capacity  = INIT_CAPACITY;
    direction = RIGHT;
    speed     = INIT_SPEED;
    logic_generate_random_fruit();
    xpos = malloc(sizeof(int) * capacity);
    ypos = malloc(sizeof(int) * capacity);

    int i;
    for (i = 0; i < length; i++) {
        ypos[i] = LINES/2;
        xpos[i] = COLS/2 - i;
    }
}

void init_ncurses(void)
{
    initscr();
    if (has_colors())
        start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    timeout(0);
    scores   = newwin(SCORE_WINH,         COLS, 0,          0);
    win_game = newwin(LINES - SCORE_WINH, COLS, SCORE_WINH, 0);
    draw_blit_all();

    /* Define all our color pairs */
    init_pair(GREEN,   COLOR_GREEN,   COLOR_BLACK);
    init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
}

void free_ncurses(void)
{
    erase();
    refresh();
    endwin();
}

/* Hiscore routines */
int hiscore_get(void)
{
    FILE *fd = fopen(SCORE_FILE, "r");
    if (fd == NULL) fd = fopen(SCORE_FILE, "w+");
    if (fscanf(fd, "%d", &hiscore) == EOF) hiscore = 0;
    fclose(fd);
    return hiscore;
}

void hiscore_set(void)
{
    if (score < hiscore) return;
    hiscore = score;
    FILE *fd = fopen(SCORE_FILE, "w+");
    fprintf(fd, "%d", hiscore);
    fclose(fd);
}

void hiscore_reset(void)
{
    FILE *fd = fopen(NSCORE_FILE, "w");
    freopen(BSCORE_FILE, "w", fd);
    fclose(fd);
}

void hiscore_print(void)
{
    printf("Your score this game was %d\n""The highscore is %d\n", score, hiscore);
    printf(score == hiscore ? "You set a new highscore!\n" : "");
}

/* Game logic */
void logic_snake_collision(void)
{
    int i;
    for (i = 1; i < length - 1; i++) {
        if (xpos[0] == xpos[i] && ypos[0] == ypos[i]) {
            free_ncurses();
            hiscore_set();
            hiscore_print();
            exit(EXIT_SUCCESS);
        }
    }
}

// Check if the snake has collided with the border
void logic_border_collision(void)
{
    if (borders == 0) {
        ypos[0] = ypos[0] < 1 ? LINES - SCORE_WINH - 2 :
            ypos[0] > LINES - SCORE_WINH - 2 ? 1 : ypos[0];
        xpos[0] = xpos[0] < 1 ? COLS - 2 :
            xpos[0] > COLS - 2 ? 1 : xpos[0];
    }
    else {
        if (ypos[0] < 1 || ypos[0] > LINES - SCORE_WINH - 1 ||
                xpos[0] < 1 || xpos[0] > COLS - 1) {
            free_ncurses();
            hiscore_set();
            hiscore_print();
            exit(EXIT_SUCCESS);
        }
    }
}

// Update the snakes position
void logic_update_snake(void)
{
    int i;
    for (i = length - 1; i > 0; i--) {
        ypos[i] = ypos[i - 1];
        xpos[i] = xpos[i - 1];
    }

    switch (direction) {
        case DOWN:
            ypos[0] += 1;
            break;
        case RIGHT:
            xpos[0] += 1;
            break;
        case UP:
            ypos[0] -= 1;
            break;
        case LEFT:
            xpos[0] -= 1;
            break;
    }
}

// Check if the snake has collided with the fruit
void logic_fruit_collision(void)
{
    if (xpos[0] == xfr && ypos[0] == yfr) {
        score += length++ << 1;
        mvwprintw(scores, 0, 1, "Score: %d", score);
        logic_generate_random_fruit();

        if (length == capacity) {
            capacity *= 2;
            xpos = realloc(xpos, sizeof(int) * capacity);
            ypos = realloc(ypos, sizeof(int) * capacity);
        }
    }
}

// sets the fruit position to a random value on the game board
void logic_generate_random_fruit(void)
{
    xfr = (rand() % (COLS - 2)) + 1;
    yfr = (rand() % (LINES - SCORE_WINH - 2)) + 1;
}

/* Drawing functions */
void draw_static(void)
{
    // on reset, ensure that the entire grid that was covered by 'score'...
    // ...is reverted back to empty chars
    // find a more elegant way to do this
    mvwprintw(scores, 0, 1,          "                  ");
    mvwprintw(scores, 0, 1,          "Score: %d",   score);
    mvwprintw(scores, 0, COLS/4 + 1, "Hiscore: %d", hiscore);
    if (borders == 1)
        mvwprintw(scores, 0, COLS/2, "Borders on!");
    box(win_game, 0, 0);
}

// Draw the fruit to the game window
void draw_fruit(void)
{
    wattron(win_game, COLOR_PAIR(MAGENTA));
    mvwprintw(win_game, yfr, xfr, "#");
    wattroff(win_game, COLOR_PAIR(MAGENTA));
}

// Update the snake, just drawing the parts necessary
void draw_snake(void)
{
    wattron(win_game, COLOR_PAIR(GREEN));
    mvwprintw(win_game, ypos[0], xpos[0], SNAKE_HEAD);
    mvwprintw(win_game, ypos[1], xpos[1], SNAKE_BODY);
    wattroff(win_game, COLOR_PAIR(GREEN));
}

// Redraw all parts of the snake
void draw_snake_all(void)
{
    int i;
    wattron(win_game, COLOR_PAIR(GREEN));
    mvwprintw(win_game, ypos[0], xpos[0], SNAKE_HEAD);
    for (i = 1; i < length; i++)
        mvwprintw(win_game, ypos[i], xpos[i], SNAKE_BODY);
    wattroff(win_game, COLOR_PAIR(GREEN));
}

// don't need to clear entire grid, just snake and fruit pos
// move into two seperate functions
void draw_clear_all(void)
{
    int i;
    for (i = 0; i < length; i++)
        mvwprintw(win_game, ypos[i], xpos[i], " ");
    mvwprintw(win_game, yfr, xfr, " ");
}

// Refresh all persistent game windows
void draw_blit_all(void)
{
    wrefresh(scores);
    wrefresh(win_game);
}

/* Pause menu */
void game_enter_pause(void)
{
    int pause_height = 5;
    int pause_width = 19;

    WINDOW *pause_win = newwin(pause_height, pause_width,
        LINES / 2 - pause_height / 2, COLS / 2 - pause_width / 2);

    box(pause_win, 0, 0);
    mvwprintw(pause_win, pause_height / 2 - 1, 2, "<p> to continue");
    mvwprintw(pause_win, pause_height / 2,     2, "<r> to restart");
    mvwprintw(pause_win, pause_height / 2 + 1, 2, "<q> to quit");
    wrefresh(pause_win);

    int ch;
    while ((ch = getch())) {
        switch (ch) {
            case 'p':
                werase(pause_win);
                wrefresh(pause_win);
                delwin(pause_win);
                draw_snake_all();
                draw_blit_all();
                return;
            case 'r':
                hiscore_set();
                draw_clear_all();
                free(xpos);
                free(ypos);
                init_variables();
                draw_static();
                draw_snake_all();
                draw_blit_all();
                game_main_loop();
                break;
            case 'q':
                free_ncurses();
                hiscore_set();
                hiscore_print();
                exit(EXIT_SUCCESS);
                break;
        }
    }
}

/* I/O controls */
void game_keypress_handler(void)
{
    int ch = getch();
    if (ch != ERR) {
        switch (ch) {
            case 'j':
            case KEY_DOWN:
                if (direction != UP)
                    direction = DOWN;
                break;
            case 'l':
            case KEY_RIGHT:
                if (direction != LEFT)
                    direction = RIGHT;
                break;
            case 'k':
            case KEY_UP:
                if (direction != DOWN)
                    direction = UP;
                break;
            case 'h':
            case KEY_LEFT:
                if (direction != RIGHT)
                    direction = LEFT;
                break;
            case 'p':
                game_enter_pause();
                break;
            case 'q':
                free_ncurses();
                hiscore_set();
                hiscore_print();
                exit(EXIT_SUCCESS);
                break;
        }
    }
}

/* Main game loop */
void game_main_loop(void)
{
    draw_static();
    draw_snake_all();

    while (1) {
        game_keypress_handler();
        mvwprintw(win_game, ypos[length - 1], xpos[length - 1], " ");
        draw_fruit();
        logic_fruit_collision();
        logic_update_snake();
        logic_border_collision();
        logic_snake_collision();
        draw_snake();
        draw_blit_all();
        switch(direction) {
              case RIGHT:
              case LEFT:
                  usleep(speed < UPPER_SPEED_LIMIT ? speed : speed--);
                  break;
              case UP:
              case DOWN:
                  usleep((speed < UPPER_SPEED_LIMIT ? speed : speed--) * FONT_HW_RATIO);
                  break;
        }
    }
}

/* Set options on game initialization */
void init_parse_options(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--speed") == 0) {
            if (i + 1 < argc) {
                char *endptr;
                long s_arg = strtol(argv[i + 1], &endptr, 10);

                if (*endptr == '\0') {
                    speed = s_arg < UPPER_SPEED_LIMIT ? UPPER_SPEED_LIMIT : s_arg;
                    continue;
                }
            }

            free_ncurses();
            printf("Invalid argument to option '-s'\n");
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--borders") == 0) {
            borders = 1;
            SCORE_FILE = BSCORE_FILE;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            free_ncurses();
            printf("%s\n", HELP_STR);
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--reset") == 0) {
            free_ncurses();
            hiscore_reset();
            printf("Hiscores successfully reset!\n");
            exit(EXIT_SUCCESS);
        }
    }
}

int main(int argc, char **argv)
{
    init_ncurses();
    init_variables();
    init_parse_options(argc, argv);
    game_main_loop();
}
