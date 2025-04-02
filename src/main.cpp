#include <curses.h>
#include <menu.h>
#include <string.h>
#include <cstdlib>
#include <string>
#include "../include/bible_database.h"
#include <iostream>
#include "../include/utils.h"
#include <locale.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 4

void printVector(const std::vector<std::string> &vec)
{
    std::cout << "[ ";
    for (const std::string &val : vec)
    {
        std::cout << val << " ";
    }
    std::cout << "]" << std::endl;
}

int main()
{

    setlocale(LC_ALL, "");

    const char *text =

        "   .-.\n"
        " __| |__\n"
        "[__   __]\n"
        "   | |\n"
        "   | |\n"
        "   | |\n"
        "   '-'\n";

    BibleDatabase db("../data/bible.db");

    // Get all books
    std::vector<std::string> oldTestamentBooks = db.get_old_testament_books();
    oldTestamentBooks = reorderVector(oldTestamentBooks, 13, 3);

    std::vector<std::string> newTestamentBooks = db.get_new_testament_books();
    newTestamentBooks = reorderVector(newTestamentBooks, 9, 3);

    ITEM **my_items;
    int c;
    MENU *my_menu;
    WINDOW *my_menu_win;
    int n_choices, i;

    //     /* Initialize curses */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);

    std::vector<std::string> testaments = {"The Old Testament", "The New Testament"};

    int testament_choices = testaments.size();
    ITEM **testament_items = (ITEM **)calloc(testament_choices + 1, sizeof(ITEM *));
    for (int i = 0; i < testament_choices; ++i)
        testament_items[i] = new_item(testaments[i].c_str(), testaments[i].c_str());

    if (testament_items == nullptr)
    {
        std::cerr << "Memory allocation failed!" << std::endl;
        return 1;
    }

    //     /* Crate menu */
    MENU *testament_menu = new_menu((ITEM **)testament_items);

    // /* Set menu option not to show the description */
    menu_opts_off(testament_menu, O_SHOWDESC);

    // // /* Create the window to be associated with the menu */
    WINDOW *testament_menu_win = newwin(0, COLS, 20, 0);
    keypad(testament_menu_win, TRUE);

    // /* Set main window and sub window */
    set_menu_win(testament_menu, testament_menu_win);
    set_menu_sub(testament_menu, derwin(testament_menu_win, 2, 68, 0, 0));
    set_menu_format(testament_menu, 1, 2);
    set_menu_mark(testament_menu, " * ");

    // // /* Print a border around the main window and print a title */
    // box(testament_menu_win, 0, 0);

    attron(COLOR_PAIR(2));
    mvprintw(LINES - 3, 0, "Use PageUp and PageDown to scroll");
    mvprintw(LINES - 2, 0, "Use Arrow Keys to navigate (F1 to Exit)");
    mvprintw(0, (COLS) / 2, "KJV Bible");
    // mvprintw(2, 0, "The Old Testament");
    attroff(COLOR_PAIR(2));

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Calculate the x position to center the ASCII art
    int startX = (cols) / 2;

    // Print each line of the ASCII art centered
    char *line = strtok(strdup(text), "\n"); // Split into lines
    int y = 2;                               // Start from the center row, adjust as needed

    while (line != NULL)
    {
        mvprintw(y, startX, "%s", line);
        line = strtok(NULL, "\n");
        y++;
    }

    refresh();

    // //     /* Post the menu */
    post_menu(testament_menu);
    wrefresh(testament_menu_win);

    //     /* Create items */
    // n_choices = ARRAY_SIZE(choices);

    n_choices = oldTestamentBooks.size();
    std::cout << n_choices << std::endl;

    my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));
    for (i = 0; i < n_choices; ++i)
        my_items[i] = new_item(oldTestamentBooks[i].c_str(), oldTestamentBooks[i].c_str());

    if (my_items == nullptr)
    {
        std::cerr << "Memory allocation failed!" << std::endl;
        return 1;
    }

    // // //     /* Crate menu */
    my_menu = new_menu((ITEM **)my_items);

    // // /* Set menu option not to show the description */
    menu_opts_off(my_menu, O_SHOWDESC);

    // // // /* Create the window to be associated with the menu */
    my_menu_win = newwin(0, COLS, 4, 0);
    keypad(my_menu_win, TRUE);

    // // /* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 14, 68, 0, 0));
    set_menu_format(my_menu, 14, 3);
    set_menu_mark(my_menu, " * ");

    // // // /* Print a border around the main window and print a title */
    // // // box(my_menu_win, 0, 0);

    attron(COLOR_PAIR(2));
    mvprintw(LINES - 3, 0, "Use PageUp and PageDown to scroll");
    mvprintw(LINES - 2, 0, "Use Arrow Keys to navigate (F1 to Exit)");
    mvprintw(0, (COLS) / 2, "KJV Bible");
    mvprintw(2, 0, "The Old Testament");
    attroff(COLOR_PAIR(2));
    refresh();

    // // //     /* Post the menu */
    post_menu(my_menu);
    wrefresh(my_menu_win);

    while ((c = wgetch(testament_menu_win)) != KEY_F(1))
    {
        switch (c)
        {
        case KEY_DOWN:
            menu_driver(testament_menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(testament_menu, REQ_UP_ITEM);
            break;
        case KEY_LEFT:
            menu_driver(testament_menu, REQ_LEFT_ITEM);
            break;
        case KEY_RIGHT:
            menu_driver(testament_menu, REQ_RIGHT_ITEM);
            break;
        case KEY_NPAGE:
            menu_driver(testament_menu, REQ_SCR_DPAGE);
            break;
        case KEY_PPAGE:
            menu_driver(testament_menu, REQ_SCR_UPAGE);
            break;
        }
        wrefresh(testament_menu_win);
    }

    //     /* Unpost and free all the memory taken up */
    unpost_menu(testament_menu);
    free_menu(testament_menu);
    for (i = 0; i < testament_choices; ++i)
        free_item(testament_items[i]);
    endwin();

    while ((c = wgetch(my_menu_win)) != KEY_F(1))
    {
        switch (c)
        {
        case KEY_DOWN:
            menu_driver(my_menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(my_menu, REQ_UP_ITEM);
            break;
        case KEY_LEFT:
            menu_driver(my_menu, REQ_LEFT_ITEM);
            break;
        case KEY_RIGHT:
            menu_driver(my_menu, REQ_RIGHT_ITEM);
            break;
        case KEY_NPAGE:
            menu_driver(my_menu, REQ_SCR_DPAGE);
            break;
        case KEY_PPAGE:
            menu_driver(my_menu, REQ_SCR_UPAGE);
            break;
        }
        wrefresh(my_menu_win);
    }

    //     /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    for (i = 0; i < n_choices; ++i)
        free_item(my_items[i]);
    endwin();
}