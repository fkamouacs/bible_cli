#include "../include/menu.h"
#include <ncurses.h>
#include <iostream>

// Initialize ncurses
void initNcurses()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Title
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Highlights
    init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Verse numbers
    curs_set(0);                             // Hide cursor
}
