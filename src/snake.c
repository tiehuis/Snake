/*
 *  TODO:
 *        Improve highscore storing; keep a history of values and not just the highest. Protect by...
 *        ... encoding in a different non-readable format.
 *        Might change back some of the justifying on some function calls with longer arguments
 *        Create header
 *        Optimizations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <curses.h>
#include "snake.h"

// score file to write out to; default to non-border
char *SCORE_FILE = NSCORE_FILE;

// Declaration of static variables
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

// Main game windows
WINDOW *scores;
WINDOW *win_game;

// Define color pairs
void def_colors()
{
    init_pair(GREEN,   COLOR_GREEN,   COLOR_BLACK);
    init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
}

// Return the current hiscore on file, creating the file if it doesn't exist
int get_hiscore()
{
    FILE *fd = fopen(SCORE_FILE, "r");
    if (fd == NULL) fd = fopen(SCORE_FILE, "w+");
    if (fscanf(fd, "%d", &hiscore) == EOF) hiscore = 0;
    fclose(fd);
    return hiscore;
}

// Set the hiscore
void set_hiscore()
{
    if (score < hiscore) return;
    hiscore = score;
    FILE *fd = fopen(SCORE_FILE, "w+");
    fprintf(fd, "%d", hiscore);
    fclose(fd);
}

void reset_scores()
{
    FILE *fd = fopen(NSCORE_FILE, "w");
    freopen(BSCORE_FILE, "w", fd);
    fclose(fd);
}

// Refresh all persistent game windows
void refresh_allw()
{
    wrefresh(scores);
    wrefresh(win_game);
}

// sets the fruit position to a random value on the game board
void rand_fruit()
{
    xfr = (rand() % (COLS - 2)) + 1;
    yfr = (rand() % (LINES - SCORE_WINH - 2)) + 1;
}

// Run procedures before ending the game
// Split this function into 3 parts: end_session(), print_score(), and just a standard exit(0);

void print_score()
{
    printf("Your score this game was %d\n""The highscore is %d\n", score, hiscore);
    printf(score == hiscore ? "You set a new highscore!\n" : "");
}

void end_ncenv()
{
    erase();
    refresh();
    endwin();
}

// Check if the snake has collided with itself
void chsnake_collide()
{
    int i;
    for (i = 1; i < length - 1; i++) {
        if (xpos[0] == xpos[i] && ypos[0] == ypos[i]) {
            end_ncenv();
            set_hiscore();
            print_score();
            exit(EXIT_SUCCESS);
        }
    }
}

// Check if the snake has collided with the border
void chborder_collide()
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
            end_ncenv();
            set_hiscore();
            print_score();
            exit(EXIT_SUCCESS);
        }
    }
}

// Update the snakes position
void upd_snake()
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
void chfruit_collide()
{
    if (xpos[0] == xfr && ypos[0] == yfr) {
        score += length++ << 1;
        mvwprintw(scores, 0, 1, "Score: %d", score);
        rand_fruit();

        if (length == capacity) {
            capacity *= 2;
            xpos = realloc(xpos, sizeof(int) * capacity);
            ypos = realloc(ypos, sizeof(int) * capacity);
        }
    }
}

// Draw the fruit to the game window
void draw_fruit()
{
    wattron(win_game, COLOR_PAIR(MAGENTA));
    mvwprintw(win_game, yfr, xfr, "#");
    wattroff(win_game, COLOR_PAIR(MAGENTA));
}

// Update the snake, just drawing the parts necessary
void draw_snake()
{
    wattron(win_game, COLOR_PAIR(GREEN));
    mvwprintw(win_game, ypos[0], xpos[0], SNAKE_HEAD);
    mvwprintw(win_game, ypos[1], xpos[1], SNAKE_BODY);
    wattroff(win_game, COLOR_PAIR(GREEN));
}

// Redraw all parts of the snake
void refresh_snake()
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
void clear_grid()
{
    int i;
    for (i = 0; i < length; i++)
        mvwprintw(win_game, ypos[i], xpos[i], " ");
    mvwprintw(win_game, yfr, xfr, " ");
}

// Initialize the pause menu and wait for user interaction
void pause_menu()
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
                refresh_snake();
                refresh_allw();
                return;
            case 'r':
                set_hiscore();
                clear_grid();
                free(xpos);
                free(ypos);
                init_start_var();
                draw_static();
                refresh_snake();
                refresh_allw();
                game_loop();
                break;
            case 'q':
                end_ncenv();
                set_hiscore();
                print_score();
                exit(EXIT_SUCCESS);
                break;
        }
    }
}

// Check for keypress and process if one has occured
void keypress_event()
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
                pause_menu();
                break;
            case 'q':
                end_ncenv();
                set_hiscore();
                print_score();
                exit(EXIT_SUCCESS);
                break;
        }
    }
}

// The main game loop; calls other functions as needed
void game_loop()
{
    while (1) {
        keypress_event();
        mvwprintw(win_game, ypos[length - 1], xpos[length - 1], " ");
        draw_fruit();
        chfruit_collide();
        upd_snake();
        chborder_collide();
        chsnake_collide();
        draw_snake();
        refresh_allw();
        switch(direction) {
              case RIGHT:
              case LEFT:
                  usleep(speed < 15000 ? speed : speed--);
                  break;
              case UP:
              case DOWN:
                  usleep((speed < 15000 ? speed : speed--) * 1.5);
                  break;
        }
    }
}

// Draw non-changing/static portions of the windows
void draw_static()
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

// Initialize start variables to values
void init_start_var()
{
    // decide whether to reseed on each game, or on every session
    // should move this elsewhere

    score     = 0;
    hiscore   = get_hiscore();
    length    = INIT_LENGTH;
    capacity  = INIT_CAPACITY;
    direction = RIGHT;
    speed     = INIT_SPEED;
    rand_fruit();
    xpos = malloc(sizeof(int) * capacity);
    ypos = malloc(sizeof(int) * capacity);

    int i;
    for (i = 0; i < length; i++) {
        ypos[i] = LINES/2;
        xpos[i] = COLS/2 - i;
    }
}

// Initialize the ncurses environment
void init_ncenv()
{
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    timeout(0);

    scores   = newwin(SCORE_WINH,         COLS, 0,          0);
    win_game = newwin(LINES - SCORE_WINH, COLS, SCORE_WINH, 0);
    refresh_allw();
}

// print details explaining how the options available
void print_help()
{
    printf("Controls:\n"
           "    hjkl, ARROW_KEYS    Movement\n"
           "    p                   Pause menu\n"
           "    q                   Quit session\n"
           "\n");

    printf("Usage:\n"
           "    snake [options]\n"
           "\n");

    printf("Options:\n"
           "    -h, --help          Display help information\n"
           "    -b, --borders       Enable borders\n"
           "    -r, --reset         Reset all saved scores\n"
           "\n");
}

// Parse the intput options
// Add some more options, such as --help, etc
void parse_options(int argc, char **argv)
{
    int i;

    // not happy with border setting here
    borders = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--borders") == 0) {
            borders = 1;
            SCORE_FILE = BSCORE_FILE;
        }

        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            exit(EXIT_SUCCESS);
        }

        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--reset") == 0) {
            reset_scores();
            printf("Hiscores successfully reset!\n");
            exit(EXIT_SUCCESS);
        }
    }
}

// Entry point for program
// Create a function which calls the functions which are needed for reseting state.
// i.e: init_ncenv(), init_start_var(), draw_static(), refresh_snake();
// Seperate all once-called functions vs possible ones called more than once
int main(int argc, char **argv)
{
    parse_options(argc, argv);
    srand((unsigned int)time(NULL));

    init_ncenv();
    init_start_var();
    def_colors();
    draw_static();
    refresh_snake();
    game_loop();
    exit(EXIT_FAILURE);
}
