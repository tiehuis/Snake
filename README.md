#Snake

Your favourite snake game written in ncurses.
To compile:
```
gcc snake.c -o snake -lcurses
```
Requires the ncurses library and a linux\unix-like environment.


### To Fix
If another program (i.e. tmux) has occupied a row of your terminal then the collision detection for the borders will be slightly off.
