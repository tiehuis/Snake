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
    int borders;
} WIN;

// rename this so it doesn't have so many similar clashes in code
typedef struct score1 {
    char *score_file;
    int hiscore;
    int score;
} SCORE;

// declare an instance of fruit, snake and score
SNAKE sn1; SNAKE *snake = &sn1;
FRUIT fr1; FRUIT *fruit = &fr1;
SCORE sc1; SCORE *sscore = &sc1;

// Main game windows

WIN win1; WIN *scores = &win1;
WIN win2; WIN *win_game = &win2;

// Define color pairs 
void def_colors()
{
    init_pair(GREEN,   COLOR_GREEN,   COLOR_BLACK);
    init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
}

// Return the current sscore->hiscore on file, creating the file if it doesn't exist
int get_hiscore(SCORE *_score)
{
    FILE *fd = fopen(_score->score_file, "r");
    if (fd == NULL) fd = fopen(_score->score_file, "w+");
    if (fscanf(fd, "%d", &_score->hiscore) == EOF) _score->hiscore = 0;
    fclose(fd);
    return _score->hiscore;
}

// Set the sscore->hiscore
void set_hiscore(SCORE *_score)
{
    if (_score->score < _score->hiscore) return;
    _score->hiscore = _score->score;
    FILE *fd = fopen(_score->score_file, "w+");
    fprintf(fd, "%d", _score->hiscore);
    fclose(fd);
}

void reset_scores()
{
    FILE *fd = fopen(NSCORE_FILE, "w");
    freopen(BSCORE_FILE, "w", fd);
    fclose(fd);
}

// sets the fruit position to a random value on the game board
void rand_fruit(FRUIT *_fruit, WIN *_win_game)
{ 
    _fruit->x = rand() % (_win_game->x - 2) + 1;
    _fruit->y = rand() % (_win_game->y - 2) + 1;
}

// Run procedures before ending the game
// Split this function into 3 parts: end_session(), print_score(), and just a standard exit(0);

void print_score(SCORE *_score)
{
    printf("Your score this game was %d\n"
           "The highscore is %d\n", 
            _score->score, _score->hiscore);
    printf(_score->score == _score->hiscore ?
        "You set a new highscore!\n" : "");
}

void end_ncenv()
{        
    erase();
    refresh();
    endwin();
}

// Check if the snake has collided with itself
void chsnake_collide(SNAKE *_snake, SCORE *_score)
{
    int i;    
    for (i = 1; i < _snake->length - 1; i++) {
        if (_snake->xs[0] == _snake->xs[i] && _snake->ys[0] == _snake->ys[i]) {
            end_ncenv();
            set_hiscore(_score);
            print_score(_score);
            exit(EXIT_SUCCESS);
        }
    }
}

// Check if the snake has collided with the border
void chborder_collide(WIN *_win_game, SNAKE *_snake, SCORE *_score)
{
    if (_win_game->borders == 0) {
        _snake->ys[0] = _snake->ys[0] < 1 ? _win_game->y - 2 : 
            _snake->ys[0] > _win_game->y - 2 ? 1 : _snake->ys[0];
        _snake->xs[0] = _snake->xs[0] < 2 ? _win_game->x - 1 : 
            _snake->xs[0] > _win_game->x - 2 ? 1 : _snake->xs[0];
    }
    else {
        if (_snake->ys[0] < 1 || _snake->ys[0] > _win_game->y - 1 || 
                _snake->xs[0] < 1 || _snake->xs[0] > _win_game->x - 1) {
            end_ncenv();
            set_hiscore(_score);
            print_score(_score);
            exit(EXIT_SUCCESS);
        }
    }
}

// Update the snakes position
void upd_snake(SNAKE *_snake)
{
    int i;
    for (i = _snake->length - 1; i > 0; i--) {
        _snake->ys[i] = _snake->ys[i - 1];
        _snake->xs[i] = _snake->xs[i - 1];
    }

    switch (_snake->direction) {
        case DOWN:
            _snake->ys[0] += 1;
            break;
        case RIGHT:
            _snake->xs[0] += 1;
            break;
        case UP:
            _snake->ys[0] -= 1;
            break;
        case LEFT:
            _snake->xs[0] -= 1;
            break;
    }
}

void increase_score(SCORE *_score, int _value)
{
    _score->score += _value;
}

// Check if the snake has collided with the fruit
void chfruit_collide(WIN *_scores, SNAKE *_snake, FRUIT *_fruit, SCORE *_score)
{
    if (_snake->xs[0] == _fruit->x && _snake->ys[0] == _fruit->y) {
        increase_score(_score, 2 * _snake->length++);
        mvwprintw(_scores->window, 0, 1, "Score: %d", _score->score);
        rand_fruit(_fruit);
        _snake->xs = realloc(_snake->xs, sizeof(int) * _snake->length);
        _snake->ys = realloc(_snake->ys, sizeof(int) * _snake->length);
    }
}

// Draw the fruit to the game window
void draw_fruit(FRUIT *_fruit) 
{
    wattron(win_game->window, COLOR_PAIR(MAGENTA));
    mvwprintw(win_game->window, _fruit->y, _fruit->x, "#");
    wattroff(win_game->window, COLOR_PAIR(MAGENTA));
}

// Update the snake, just drawing the parts necessary
void draw_snake(SNAKE *_snake)
{
    wattron(win_game->window, COLOR_PAIR(GREEN));
    mvwprintw(win_game->window, _snake->ys[0], _snake->xs[0], SNAKE_HEAD);
    mvwprintw(win_game->window, _snake->ys[1], _snake->xs[1], SNAKE_BODY);
    wattroff(win_game->window, COLOR_PAIR(GREEN));
}

// Redraw all parts of the snake
void refresh_snake(WIN *_win_game, SNAKE *_snake)
{
    int i;
    wattron(_win_game->window, COLOR_PAIR(GREEN));
    mvwprintw(_win_game->window, _snake->ys[0], _snake->xs[0], SNAKE_HEAD);
    for (i = 1; i < _snake->length; i++) 
        mvwprintw(_win_game->window, _snake->ys[i], _snake->xs[i], SNAKE_BODY);
    wattroff(_win_game->window, COLOR_PAIR(GREEN));
}

// don't need to clear entire grid, just snake and fruit pos
// move into two seperate functions
void clear_grid(WIN *_win_game, SNAKE *_snake, FRUIT *_fruit)
{
    int i;
    for (i = 0; i < _snake->length; i++) {
        mvwprintw(_win_game->window, _snake->ys[i], _snake->xs[i], " ");
    }
    mvwprintw(_win_game->window, _fruit->y, _fruit->x, " ");
}

// Initialize the pause menu and wait for user interaction
void pause_menu(SNAKE *_snake, FRUIT *_fruit, SCORE *_score)
{
    int ch;
    const int pause_height = 5;
    const int pause_width = 19;

    // don't need a WIN type here
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
                // return appropriate code here and act on it in main loop to avoid passing values
                werase(pause_win);
                wrefresh(pause_win);
                delwin(pause_win);
                refresh_snake(_snake);
                refresh_allw();
                return;
            case 'r':
                set_hiscore(_score);                  
                clear_grid(_snake, _fruit);
                free(_snake->xs);
                free(_snake->ys);
                init_start_var();
                draw_static();
                refresh_snake(_snake);
                refresh_allw();
                game_loop();
                break;
            case 'q':
                end_ncenv();
                set_hiscore(_score);
                print_score(_score);
                exit(EXIT_SUCCESS);
                break;
        }
    }
}

// Check for keypress and process if one has occured
void keypress_event(SNAKE *_snake)
{
    int ch = getch();
    if (ch != ERR) {
        switch (ch) {
            case 'j':
            case KEY_DOWN:
                // maybe return values here
                if (_snake->direction != UP) 
                    _snake->direction = DOWN; 
                break;
            case 'l':
            case KEY_RIGHT:
                if (_snake->direction != LEFT) 
                    _snake->direction = RIGHT; 
                break;
            case 'k': 
            case KEY_UP:
                if (_snake->direction != DOWN) 
                    _snake->direction = UP; 
                break;
            case 'h':
            case KEY_LEFT:
                if (_snake->direction != RIGHT) 
                    _snake->direction = LEFT; 
                break;
            case 'p':
                pause_menu();
                break;
            case 'q':
                end_ncenv();
                set_hiscore(_score);
                print_score(_score);
                exit(EXIT_SUCCESS);
                break;
        }
    }
}

// The main game loop; calls other functions as needed
void game_loop(WIN *_win_game, WIN *_scores, SNAKE *_snake, FRUIT *_fruit, SCORE *_score)
{
    while (1) {
        keypress_event(_snake);
        mvwprintw(_win_game->window, _snake->ys[_snake->length - 1], _snake->xs[_snake->length - 1], " ");
        draw_fruit(_fruit);
        chfruit_collide(_snake, _fruit);
        upd_snake(_snake);
        chborder_collide(_snake);
        chsnake_collide(_snake);
        draw_snake(_snake);
        wrefresh(_win_game);
        wrefresh(_scores);
        usleep(_snake->speed < 15000 ? _snake->speed : _snake->speed--);
    }
}

// Draw non-changing/static portions of the windows
void draw_static(WIN *_scores, WIN *_win_game, SCORE *_score, )
{
    // on reset, ensure that the entire grid that was covered by 'sscore->score'...
    // ...is reverted back to empty chars
    // find a more elegant way to do this
    mvwprintw(_scores->window, 0, 1,          "                  ");
    mvwprintw(_scores->window, 0, 1,          "Score: %d",   _score->score);
    mvwprintw(_scores->window, 0, COLS/4 + 1, "Hiscore: %d", _score->hiscore);
    if (_win_game->borders == 1)
        mvwprintw(_scores->window, 0, COLS/2, "Borders on!");
    box(_win_game->window, 0, 0);
}

// Initialize start variables to values
void init_start_var(SNAKE *_snake, FRUIT *_fruit, SCORE *_score)
{
    // decide whether to reseed on each game, or on every session
    // should move this elsewhere

    _score->score     = 0;
    _score->hiscore   = get_hiscore(_score);
    _snake->length    = INIT_LENGTH;
    _snake->direction = RIGHT;
    _snake->speed     = INIT_SPEED;
    rand_fruit(_fruit);
    _snake->xs = malloc(sizeof(int) * _snake->length);
    _snake->ys = malloc(sizeof(int) * _snake->length);

    int i;
    for (i = 0; i < _snake->length; i++) {
        _snake->ys[i] = LINES/2;
        _snake->xs[i] = COLS/2 - i;
    }
}

// Initialize the ncurses environment
void init_ncenv(WIN *_scores, WIN *_win_game)
{
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    timeout(0);

    // init window values
    _scores->x = COLS;
    _scores->y = 1;
    _scores->x_offset = 0;
    _scores->y_offset = 0;
    _scores->borders = 0;

    _win_game->x = COLS;
    _win_game->y = LINES - scores->y;
    _win_game->x_offset = 0;
    _win_game->y_offset = scores->y;

    _scores->window   = newwin(_scores->y,   _scores->x,   _scores->y_offset,   _scores->x_offset);
    _win_game->window = newwin(_win_game->y, _win_game->x, _win_game->y_offset, _win_game->x_offset);
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
void parse_options(int argc, char **argv, WIN *_win_game, SCORE *_score)
{
    int i;

    // not happy with border setting here
    _win_game->borders = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--borders") == 0) {
            _win_game->borders = 1;
            _score->score_file = BSCORE_FILE;
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
    // declaring non-static variables

    sscore->score_file = NSCORE_FILE;
    parse_options(argc, argv);
    srand((unsigned int)time(NULL));

    init_ncenv(sscores, win_game);
    init_start_var(snake, fruit, sscores);
    def_colors();
    draw_static(scores, win_game, sscores);
    refresh_snake(snake);
    game_loop();
    exit(EXIT_FAILURE);
}
