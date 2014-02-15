#include <stdlib.h>
#include <unistd.h>
#include <curses.h>

// global state variables
int hiscore, score;
int length, direction;
int speed, borders;
int xfr, yfr;
int *xpos, *ypos;
WINDOW *win_game, *scores;

int get_hiscore()
{
    FILE *f = fopen(".shs", "r");
    if (fscanf(f, "%d", &hiscore) == EOF) 
        hiscore = 0;
    fclose(f);
    return hiscore;
}

void refresh_allw()
{
    wrefresh(scores);
    wrefresh(win_game);
}

void init_start_var()
{
    // initialize random seed
    srand((unsigned int)time(NULL));

    // initialize window sizes
    scores = newwin(1, COLS, 0, 0);
    win_game = newwin(LINES - 1, COLS, 1, 0);
    refresh_allw();

    // initialize game state variables
    hiscore = get_hiscore();
    score = 0;
    length = 6;
    direction = 1;
    speed = 40000;
    borders = 0;
    xfr = rand() % COLS;
    yfr = rand() % LINES;
    xpos = malloc(sizeof(int) * length);
    ypos = malloc(sizeof(int) * length);

    // set snake initial values
    int i;
    for (i = 0; i < length; i++) {
        ypos[i] = LINES/2;
        xpos[i] = (COLS/2) - i;
    }
}

void endgame()
{        
    if (score > hiscore) {
        FILE *fd = fopen(".shs", "w+");
        fprintf(fd, "%d", score);
    }

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
    nodelay(stdscr, TRUE);
    curs_set(0);
    timeout(0);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
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
    if (borders == 0) {
        ypos[0] = ypos[0] < 1 ? LINES - 3 : ypos[0] > LINES - 3 ? 1 : ypos[0];
        xpos[0] = xpos[0] < 1 ? COLS - 2  : xpos[0] > COLS - 2  ? 1 : xpos[0];
    }
    else {
        if (ypos[0] < 1 || ypos[0] > LINES - 2 || xpos[0] < 1 || xpos[0] > COLS - 2)
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

    if (direction == 0) {
        ypos[0] += 1;}
    else if (direction == 1) {
        xpos[0] += 1;}
    else if (direction == 2) {
        ypos[0] -= 1;}
    else {
        xpos[0] -= 1;}
}

// ensure that fruit is always drawn in the parameters of the game board
void chfruit_collide()
{
    if (xpos[0] == xfr && ypos[0] == yfr) {
        score += length++ << 1;
        mvwprintw(scores, 0, 1, "Score: %d", score);
        xfr = rand() % (COLS - 1) + 1;
        yfr = rand() % (LINES - 2) + 1;
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

void keypress_event()
{
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
}

void game_tick()
{
    while (1) {
        keypress_event();
        box(win_game, 0, 0);
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

void draw_game_info()
{
    mvwprintw(scores, 0, 1, "Score: %d", score);
    mvwprintw(scores, 0, 31, "Hiscore: %d", hiscore);
}

void parse_options(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "-b") == 0) {
        borders = 1;
        mvwprintw(scores, 0, 61, "Borders on!");
    }
}

int main(int argc, char **argv)
{
    init_ncenv();
    init_start_var();
    parse_options(argc, argv);
    draw_game_info();
    game_tick();
    // never reached
    exit(EXIT_FAILURE);
}
