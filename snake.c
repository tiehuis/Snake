#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <curses.h>

#define INIT_SPEED  40000
#define INIT_LENGTH 6
#define SCORE_FILE  ".shs"

enum movement {DOWN, RIGHT, UP, LEFT};

int    hiscore, score;
int    length,  direction;
int    speed,   borders;
int    xfr,     yfr;
int    *xpos,   *ypos;
WINDOW *scores, *win_game;

void color_defs()
{
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
}

int get_hiscore()
{
    FILE *fd = fopen(SCORE_FILE, "r");
    if (fscanf(fd, "%d", &hiscore) == EOF) 
        hiscore = 0;
    fclose(fd);
    return hiscore;
}

void set_hiscore()
{
    if (score > hiscore) {
        FILE *fd = fopen(SCORE_FILE, "w+");
        fprintf(fd, "%d", score);
        fclose(fd);
    }
}

void refresh_allw()
{
    wrefresh(scores);
    wrefresh(win_game);
}

void init_start_var()
{
    srand((unsigned int)time(NULL));

    scores   = newwin(1, COLS, 0, 0);
    win_game = newwin(LINES - 1, COLS, 1, 0);
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

void endgame()
{        
    set_hiscore();
    erase();
    refresh();
    endwin();
    printf("Your score was %d\nThe highscore is %d\n", score, hiscore);
    exit(EXIT_SUCCESS);
}

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


void chsnake_collide()
{
    int i;    
    for (i = 1; i < length - 1; i++)
        if (ypos[0] == ypos[i] && xpos[0] == xpos[i])
            endgame();
}

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

void draw_fruit() 
{
    wattron(win_game, COLOR_PAIR(2));
    mvwprintw(win_game, yfr, xfr, "#");
    wattroff(win_game, COLOR_PAIR(2));
}

void draw_snake()
{
    wattron(win_game, COLOR_PAIR(1));
    mvwprintw(win_game, ypos[0], xpos[0], "@");
    mvwprintw(win_game, ypos[1], xpos[1], "*");
    wattroff(win_game, COLOR_PAIR(1));
}

void pause_menu()
{
    int ch;
    int pause_height = 8;
    int pause_width = 40;

    WINDOW *pause_win = newwin(pause_height, pause_width, 
        LINES / 2 - pause_height / 2, COLS / 2 - pause_width / 2);

    curs_set(true);
    box(pause_win, 0, 0);

    wattron(pause_win, COLOR_PAIR(3));
    mvwprintw(pause_win, pause_height / 2 - 1, 3, "<p> to continue the game");
    mvwprintw(pause_win, pause_height / 2, 3, "<q> to quit this game");
    wattron(pause_win, COLOR_PAIR(3));
    wrefresh(pause_win);

    while (ch = getch()) {
        switch (ch) {
            case 'p':
                delwin(pause_win);
                refresh_allw();
                curs_set(false);
                return;
            case 'q':
                endgame();
                break;
        }
    }
}

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

void draw_static()
{
    mvwprintw(scores, 0, 1, "Score: %d", score);
    mvwprintw(scores, 0, 31, "Hiscore: %d", hiscore);
    box(win_game, 0, 0);
}

void parse_options(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "-b") == 0) {
        borders = true;
        mvwprintw(scores, 0, 61, "Borders on!");
    }
}

int main(int argc, char **argv)
{
    init_ncenv();
    color_defs();
    init_start_var();
    parse_options(argc, argv);
    draw_static();
    game_tick();
    exit(EXIT_FAILURE);
}
