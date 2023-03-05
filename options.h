#include <cstring>
#include <ncurses.h>

const char *mode_options[] = {
        "Practice",
        "Test"
};
int mode_options_len = sizeof(mode_options)/sizeof(char *);
const char *timer_options[] = {
        "Yes",
        "No"
};
int timer_options_len = sizeof(timer_options)/sizeof(char *);
bool use_timer;
int n_rounds;

// Prints the menu options
void print_menu(int highlight, const char *options[], int n_options)
{
    int x, y, i;	

    x = 5;
    y = 5;
    for(i = 0; i < n_options; ++i) {
        if(i == highlight) {
            mvprintw(y,x,"[");
            attron(A_REVERSE);
            ++x;
            mvprintw(y, x, "%s", options[i]);
            attroff(A_REVERSE);
            x += strlen(options[i]);
            mvprintw(y,x,"]");
        }
        else {
            mvprintw(y,x,"[");
            ++x;
            mvprintw(y, x, "%s", options[i]);
            x += strlen(options[i]);
            mvprintw(y,x,"]");
        }
        x += 5;
    }
    refresh();
}

// Prints the menu with options given in the array options[]
int option_menu(const char *options[],int n_options) {
    int highlight = 0;
    bool select = false;
    int c_in;

    keypad(stdscr, TRUE);
    curs_set(0);
    while(1) {
        print_menu(highlight,options,n_options);
        c_in = getch();
        switch(c_in) {
            case KEY_LEFT: // Left arrow key
                if(!highlight)
                    highlight = n_options-1;
                else
                    --highlight;
                break;
            case KEY_RIGHT: // Right arrow key
                if(highlight == n_options-1)
                    highlight = 0;
                else 
                    ++highlight;
                break;
            case 10: // Enter key
                select = true;
                break;
        }
        // Exit the loop if the user has selected an option
        if (select) {
            break;
        }
    }
    refresh();
    curs_set(1);
    return highlight;
}