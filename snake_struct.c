/*
 *  TODO: 
 *        Improve highscore storing; keep a history of values and not just the highest. Protect by...
 *        ... encoding in a different non-readable format.
 *        Might change back some of the justifying on some function calls with longer arguments
 *        Refactor code to not rely on static variables
 *        Optimizations
 */

#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include "snake.h"

#define INIT_SPEED  40000
#define INIT_LENGTH 6
#define SCORE_WINH  1
#define SNAKE_HEAD  "@"
#define SNAKE_BODY  "*"
#define BSCORE_FILE ".sbhs"
#define NSCORE_FILE ".snhs"

// type enumerations
enum movement {
    DOWN, 
    RIGHT, 
    UP, 
    LEFT
};

enum colorset {
    GREEN = 1, 
    MAGENTA
};

// snake
typedef struct snake {
    int length;
    int direction;
    int speed;
    int *xs;
    int *ys;
} SNAKE;

// fruit
typedef struct fruit {
    int x;
    int y;
} FRUIT;

// change code to make use of the offsets
typedef struct win {
    WINDOW *window;
    int x;
    int y;
    int x_offset;
    int y_offset;
} WIN;

// alter game to make use of this data
typedef struct game {
    WIN *game;
    WIN *data;
    int borders;
} GAME;

typedef struct score1 {
    char *score_file;
    int hiscore;
    int score;
} SCORE;

// declare an instance of fruit, and snake
SNAKE sn1;
SNAKE *snake = &sn1;
FRUIT fr1;
FRUIT *fruit = &fr1;
SCORE sc1;
SCORE *sscore = &sc1;

// Declaration of static variables
int borders;

// Main game windows
WINDOW *scores;
WINDOW *win_game;

// Define color pairs 
void def_colors()
{
    init_pair(GREEN,   COLOR_GREEN,   COLOR_BLACK);
    init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
}

// Return the current sscore->hiscore on file, creating the file if it doesn't exist
int get_hiscore()
{
    FILE *fd = fopen(sscore->score_file, "r");
    if (fd == NULL) fd = fopen(sscore->score_file, "w+");
    if (fscanf(fd, "%d", &sscore->hiscore) == EOF) sscore->hiscore = 0;
    fclose(fd);
    return sscore->hiscore;
}

// Set the sscore->hiscore
void set_hiscore()
{
    if (sscore->score < sscore->hiscore) return;
    sscore->hiscore = sscore->score;
    FILE *fd = fopen(sscore->score_file, "w+");
    fprintf(fd, "%d", sscore->hiscore);
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
    fruit->x = (rand() % (COLS - 2)) + 1;
    fruit->y = (rand() % (LINES - SCORE_WINH - 2)) + 1;
}

// Run procedures before ending the game
// Split this function into 3 parts: end_session(), print_score(), and just a standard exit(0);

void print_score()
{
    printf("Your score this game was %d\n"
           "The highscore is %d\n", 
            sscore->score, sscore->hiscore);
    printf(sscore->score == sscore->hiscore ?
        "You set a new highscore!\n" : "");
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
    for (i = 1; i < snake->length - 1; i++) {
        if (snake->xs[0] == snake->xs[i] && snake->ys[0] == snake->ys[i]) {
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
        snake->ys[0] = snake->ys[0] < 1 ? LINES - SCORE_WINH - 2 : 
            snake->ys[0] > LINES - SCORE_WINH - 2 ? 1 : snake->ys[0];
        snake->xs[0] = snake->xs[0] < 1 ? COLS - 2 : 
            snake->xs[0] > COLS - 2 ? 1 : snake->xs[0];
    }
    else {
        if (snake->ys[0] < 1 || snake->ys[0] > LINES - SCORE_WINH - 1 || 
                snake->xs[0] < 1 || snake->xs[0] > COLS - 1) {
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
    for (i = snake->length - 1; i > 0; i--) {
        snake->ys[i] = snake->ys[i - 1];
        snake->xs[i] = snake->xs[i - 1];
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
}

// Check if the snake has collided with the fruit
void chfruit_collide()
{
    if (snake->xs[0] == fruit->x && snake->ys[0] == fruit->y) {
        sscore->score += snake->length++ << 1;
        mvwprintw(scores, 0, 1, "Score: %d", sscore->score);
        rand_fruit();
        snake->xs = realloc(snake->xs, sizeof(int) * snake->length);
        snake->ys = realloc(snake->ys, sizeof(int) * snake->length);
    }
}

// Draw the fruit to the game window
void draw_fruit() 
{
    wattron(win_game, COLOR_PAIR(MAGENTA));
    mvwprintw(win_game, fruit->y, fruit->x, "#");
    wattroff(win_game, COLOR_PAIR(MAGENTA));
}

// Update the snake, just drawing the parts necessary
void draw_snake()
{
    wattron(win_game, COLOR_PAIR(GREEN));
    mvwprintw(win_game, snake->ys[0], snake->xs[0], SNAKE_HEAD);
    mvwprintw(win_game, snake->ys[1], snake->xs[1], SNAKE_BODY);
    wattroff(win_game, COLOR_PAIR(GREEN));
}

// Redraw all parts of the snake
void refresh_snake()
{
    int i;
    wattron(win_game, COLOR_PAIR(GREEN));
    mvwprintw(win_game, snake->ys[0], snake->xs[0], SNAKE_HEAD);
    for (i = 1; i < snake->length; i++) 
        mvwprintw(win_game, snake->ys[i], snake->xs[i], SNAKE_BODY);
    wattroff(win_game, COLOR_PAIR(GREEN));
}

// don't need to clear entire grid, just snake and fruit pos
// move into two seperate functions
void clear_grid()
{
    int i;
    for (i = 0; i < snake->length; i++) {
        mvwprintw(win_game, snake->ys[i], snake->xs[i], " ");
    }
    mvwprintw(win_game, fruit->y, fruit->x, " ");
}

// Initialize the pause menu and wait for user interaction
void pause_menu()
{
    int ch;
    int pause_height = 5;
    int pause_width = 19;

    WINDOW *pause_win = newwin(pause_height, pause_width, 
        LINES / 2 - pause_height / 2, COLS / 2 - pause_width / 2);

    box(pause_win, 0, 0);
    mvwprintw(pause_win, pause_height / 2 - 1, 2, "<p> to continue");
    mvwprintw(pause_win, pause_height / 2,     2, "<r> to restart");
    mvwprintw(pause_win, pause_height / 2 + 1, 2, "<q> to quit");
    wrefresh(pause_win);

    while (ch = getch()) {
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
                free(snake->xs);
                free(snake->ys);
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
        mvwprintw(win_game, snake->ys[snake->length - 1], snake->xs[snake->length - 1], " ");
        draw_fruit();
        chfruit_collide();
        upd_snake();
        chborder_collide();
        chsnake_collide();
        draw_snake();
        refresh_allw();
        usleep(snake->speed < 15000 ? snake->speed : snake->speed--);
    }
}

// Draw non-changing/static portions of the windows
void draw_static()
{
    // on reset, ensure that the entire grid that was covered by 'sscore->score'...
    // ...is reverted back to empty chars
    // find a more elegant way to do this
    mvwprintw(scores, 0, 1,          "                  ");
    mvwprintw(scores, 0, 1,          "Score: %d",   sscore->score);
    mvwprintw(scores, 0, COLS/4 + 1, "Hiscore: %d", sscore->hiscore);
    if (borders == 1)
        mvwprintw(scores, 0, COLS/2, "Borders on!");
    box(win_game, 0, 0);
}

// Initialize start variables to values
void init_start_var()
{
    // decide whether to reseed on each game, or on every session
    // should move this elsewhere

    sscore->score     = 0;
    sscore->hiscore   = get_hiscore();
    snake->length    = INIT_LENGTH;
    snake->direction = RIGHT;
    snake->speed     = INIT_SPEED;
    rand_fruit();
    snake->xs = malloc(sizeof(int) * snake->length);
    snake->ys = malloc(sizeof(int) * snake->length);

    int i;
    for (i = 0; i < snake->length; i++) {
        snake->ys[i] = LINES/2;
        snake->xs[i] = COLS/2 - i;
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
            sscore->score_file = BSCORE_FILE;
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
    sscore->score_file = NSCORE_FILE;
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
