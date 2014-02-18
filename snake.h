#ifndef __SNAKE__
#define __SNAKE__

// just storing function prototypes currently

int get_hiscore();
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
