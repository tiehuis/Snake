/*
 *  TODO: Rename variables, consolidate style and improve the program flow so it's natural to follow.
 *        Consolidate any constant values dependent on the board size and abstract them out so that...
          ... any board changes are automatically dealt with.
          Improve highscore storing; keep a history of values and not just the highest. Protect by...
          ... encoding in a different non-readable format.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <curses.h>

#define INIT_SPEED  40000
#define INIT_LENGTH 6
#define SCORE_WINH  1
#define SCORE_FILE  ".shs"

// type enumerations
enum movement {DOWN, RIGHT, UP, LEFT};
enum colorset {GREEN = 1, MAGENTA, RED};

// Declaration of static variables
int hiscore;
int score;
int length;
int direction;
int speed;
int borders;
int xfr;
int yfr;
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
    init_pair(RED,     COLOR_RED,     COLOR_BLACK);
}

// Return the current hiscore on file
int get_hiscore()
{
    FILE *fd = fopen(SCORE_FILE, "r");
    if (fscanf(fd, "%d", &hiscore) == EOF) 
        hiscore = 0;
    fclose(fd);
    return hiscore;
}

// Set the hiscore - split the check of this back into the main game loop
void set_hiscore()
{
    if (score > hiscore) {
        FILE *fd = fopen(SCORE_FILE, "w+");
        fprintf(fd, "%d", score);
        fclose(fd);
    }
}

// Refresh all persistent game windows
void refresh_allw()
{
    wrefresh(scores);
    wrefresh(win_game);
}

// Initialize start variables to values
void init_start_var()
{
    srand((unsigned int)time(NULL));

    scores   = newwin(SCORE_WINH, COLS, 0, 0);
    win_game = newwin(LINES - SCORE_WINH, COLS, SCORE_WINH, 0);
    refresh_allw();

    hiscore   = get_hiscore();
    score     = 0;
    length    = INIT_LENGTH;
    direction = RIGHT;
    speed     = INIT_SPEED;
    borders   = false;
    xfr = rand() % COLS;
    yfr = rand() % LINES;
    xpos = malloc(sizeof(int) * length);
    ypos = malloc(sizeof(int) * length);

    int i;
    for (i = 0; i < length; i++) {
        ypos[i] = LINES/2;
        xpos[i] = COLS/2 - i;
    }
}

// Run procedures before ending the game
void endgame()
{        
    set_hiscore();
    erase();
    refresh();
    endwin();
    printf("Your score was %d\nThe highscore is %d\n", score, hiscore);
    exit(EXIT_SUCCESS);
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
    curs_set(false);
    timeout(false);
}

// Check if the snake has collided with itself
void chsnake_collide()
{
    int i;    
    for (i = 1; i < length - 1; i++)
        if (xpos[0] == xpos[i] && ypos[0] == ypos[i])
            endgame();
}

// Check if the snake has collided with the border
void chborder_collide()
{
    if (borders == false) {
        ypos[0] = ypos[0] < 1 ? LINES - 3 : ypos[0] > LINES - 3 ? 1 : ypos[0];
        xpos[0] = xpos[0] < 1 ? COLS - 2  : xpos[0] > COLS - 2  ? 1 : xpos[0];
    }
    else {
        if (ypos[0] < 1 || ypos[0] > LINES - 2 || xpos[0] < 1 || xpos[0] > COLS - 1)
            endgame();
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
        xfr = rand() % (COLS - 1) + 1;
        yfr = rand() % (LINES - 3) + 3;
        xpos = realloc(xpos, sizeof(int) * length);
        ypos = realloc(ypos, sizeof(int) * length);
    }
}

// Draw the fruit to the game window
void draw_fruit() 
{
    wattron(win_game, COLOR_PAIR(MAGENTA));
    mvwprintw(win_game, yfr, xfr, "#");
    wattroff(win_game, COLOR_PAIR(MAGENTA));
}

// Draw the snake to the game window
void draw_snake()
{
    wattron(win_game, COLOR_PAIR(GREEN));
    mvwprintw(win_game, ypos[0], xpos[0], "@");
    mvwprintw(win_game, ypos[1], xpos[1], "*");
    wattroff(win_game, COLOR_PAIR(GREEN));
}

// Initialize the pause menu and wait for user interaction
void pause_menu()
{
    int ch;
    int pause_height = 8;
    int pause_width = 40;

    WINDOW *pause_win = newwin(pause_height, pause_width, 
        LINES / 2 - pause_height / 2, COLS / 2 - pause_width / 2);

    curs_set(true);
    box(pause_win, 0, 0);

    wattron(pause_win, COLOR_PAIR(RED));
    mvwprintw(pause_win, pause_height / 2 - 1, 3, "<p> to continue the game");
    mvwprintw(pause_win, pause_height / 2, 3, "<q> to quit this game");
    wattron(pause_win, COLOR_PAIR(RED));
    wrefresh(pause_win);

    // delete pause window entirely
    while (ch = getch()) {
        switch (ch) {
            case 'p':
                curs_set(false);
                werase(pause_win);
                wrefresh(pause_win);
                delwin(pause_win);
                refresh_allw();
                return;
            case 'q':
                endgame();
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
                if (direction != UP) direction = DOWN; 
                break;
            case 'l':
            case KEY_RIGHT:
                if (direction != LEFT) direction = RIGHT; 
                break;
            case 'k': 
            case KEY_UP:
                if (direction != DOWN) direction = UP; 
                break;
            case 'h':
            case KEY_LEFT:
                if (direction != RIGHT) direction = LEFT; 
                break;
            case 'p':
                pause_menu();
                break;
            case 'q':
                endgame();
                break;
        }
    }
}

// The main game loop; calls other functions as needed
void game_tick()
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
        usleep(speed < 15000 ? speed : speed--);
    }
}

// Draw non-changing/static portions of the windows
void draw_static()
{
    mvwprintw(scores, 0, 1, "Score: %d", score);
    mvwprintw(scores, 0, COLS/4 + 1, "Hiscore: %d", hiscore);
    box(win_game, 0, 0);
}

// Parse the intput options
void parse_options(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "-b") == 0) {
        borders = true;
        mvwprintw(scores, 0, COLS/2, "Borders on!");
    }
}

// Entry point for program
int main(int argc, char **argv)
{
    init_ncenv();
    def_colors();
    init_start_var();
    parse_options(argc, argv);
    draw_static();
    game_tick();
    exit(EXIT_FAILURE);
}
