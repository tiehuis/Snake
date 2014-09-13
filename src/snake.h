#ifndef __SNAKE__
#define __SNAKE__

#define INIT_SPEED  40000
#define INIT_LENGTH 6
#define INIT_CAPACITY 32
#define SCORE_WINH  1
#define SNAKE_HEAD  "@"
#define SNAKE_BODY  "*"
#define BSCORE_FILE ".sbhs"
#define NSCORE_FILE ".snhs"
#define OPPOSITE 5

enum movement {
    DOWN  = 1, 
    RIGHT = 2, 
    LEFT  = 3,
    UP    = 4, 
};

enum colorset {
    GREEN = 1, 
    MAGENTA
};

int  get_hiscore();
void set_hiscore();
void reset_scores();
void refresh_allw();
void rand_fruit();
void print_score();
void end_ncenv();
void chsnake_collide();
void chborder_collide();
void upd_snake();
void chfruit_collide();
void draw_fruit();
void draw_snake();
void refresh_snake();
void clear_grid();
void pause_menu();
void keypress_event();
void game_loop();
void draw_static();
void init_start_var();
void init_ncenv();
void print_help();
void parse_options();

#endif
