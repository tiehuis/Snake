#ifndef CONFIG_H
#define CONFIG_H

/* Movement in blocks per second */
#define INIT_SPEED  100000

/* Length of the snake on start */
#define INIT_LENGTH 6

/* Number of rows score window occupies */
#define SCORE_WINH  1

/* Character used to display snake head */
#define SNAKE_HEAD  "@"

/* Character used to display snake body */
#define SNAKE_BODY  "*"

/* Highscore (bordered) location save */
#define BSCORE_FILE ".sbhs"

/* Highscore (non-bordered) location save */
#define NSCORE_FILE ".snhs"

/* Define the ratio of width to height here for your specific font. i.e.
 * 12x16 would be 16/12 = 1.33f */
#define FONT_HW_RATIO 1.33f

/* Maximum speed allowed ingame */
#define UPPER_SPEED_LIMIT 15000

enum movement {
    DOWN  = 1, RIGHT, LEFT, UP
};

enum colorset {
    GREEN = 1, MAGENTA
};

#endif
