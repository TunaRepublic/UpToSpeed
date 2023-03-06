#include <iostream>
#include <random>
#include <string>
#include <cstring>
#include <ctype.h>
#include "pcg/pcg_random.hpp"
#include <chrono>
#include <ncurses.h>
#include "mode.h"
#include "options.h"

#define PROGRAM_NAME "UpToSpeed"

#define N_LETTERS 26
#define ERROR_CIRCLE "\u2B24"

using namespace std;

char row[N_LETTERS] = {
                        'm', //A
                        'b', //B
                        'b', //C
                        'm', //D
                        't', //E
                        'm', //F
                        'm', //G
                        'm', //H
                        't', //I
                        'm', //J
                        'm', //K
                        'm', //L
                        'b', //M
                        'b', //N
                        't', //O
                        't', //P
                        't', //Q
                        't', //R
                        'm', //S
                        't', //T
                        't', //U
                        'b', //V
                        't', //W
                        'b', //X
                        't', //Y
                        'b' //Z 
                      };

short int digit[N_LETTERS] = {
                               4, //A
                               1, //B
                               1, //C
                               2, //D
                               2, //E
                               1, //F
                               1, //G
                               1, //H
                               2, //I
                               1, //J
                               2, //K
                               3, //L
                               2, //M
                               1, //N
                               3, //O
                               4, //P
                               4, //Q
                               1, //R
                               3, //S
                               1, //T
                               1, //U
                               1, //V
                               3, //W
                               3, //X
                               1, //Y
                               4 //Z
                             };
bool hand[N_LETTERS] = { 
                         0, //A
                         1, //B
                         0, //C
                         0, //D
                         0, //E
                         0, //F
                         0, //G
                         1, //H
                         1, //I
                         1, //J
                         1, //K
                         1, //L
                         1, //M
                         1, //N
                         1, //O
                         1, //P
                         0, //Q
                         0, //R
                         0, //S
                         0, //T
                         1, //U
                         0, //V
                         0, //W
                         0, //X
                         1, //Y
                         0 //Z
                       }; 

// Prints the message to the screen
void print_msg(string msg_str) {
    const int len = msg_str.length();
    char *msg_array = new char[len+1];
    strcpy(msg_array,msg_str.c_str());
    printw("%s", msg_array);
    refresh();
    delete[] msg_array;
}

void select_rounds(int *n_rounds) {
    char c_in;
    int num = 0;
    while (1) {
        c_in = getch();
        if (isdigit(c_in)) {
            printw("%c", c_in);
            num = 10*num + c_in - '0';
        } else if (c_in == 127) { // If Delete is pressed, delete the previous character entered
            //delch();
            printw("%c", c_in);
        } else if (c_in == 10) { // If Enter is pressed, finish reading the input
            break;
        }
        refresh();
    }
    *n_rounds = num;
}

int main() {

    // Seed with a real random value
    pcg_extras::seed_seq_from<std::random_device> seed_source;

    // Create the random number engine
    pcg32 rng(seed_source);

    // Generate a uniform distribution for the letters
    std::uniform_int_distribution<int> lett_dist(0,N_LETTERS-1);

    // Initialize the ncurses window
    initscr();
    noecho();
    cbreak();
    
    // Define string used to print messages on screen
    string msg_str;

    // Print welcome message
    msg_str = "Welcome to ";
    msg_str += PROGRAM_NAME;
    msg_str += "!\n";
    print_msg(msg_str);

    // Initialize choice of colors
    bool color_avail = has_colors();
    if (color_avail) {
        start_color();
        use_default_colors();
        init_pair(1,COLOR_RED,-1);
    } else {
        msg_str = "Note: This terminal does not support colors.\n\n";
        print_msg(msg_str);
    }

    bool main_menu_restart = true;
    while (main_menu_restart) {
        // Selects the mode
        print_msg("Please select a mode:\n");
        mode = (Mode) option_menu(mode_options,mode_options_len);
        clear();

        // Select the number of rounds
        print_msg("Enter the number of rounds: ");
        // echo();
        // scanw("%d",&n_rounds);
        // noecho();
        select_rounds(&n_rounds);
        clear();

        // Choose if timer is to be used
        print_msg("Use timer?\n");
        use_timer = !((bool) option_menu(timer_options,timer_options_len));
        clear();

        int score = 0;
        chrono::milliseconds start_time, end_time;
        const char *out_str;
        bool restart = true;
        while (restart) {
            // Loop through the rounds
            for (int round_i = 0; round_i < n_rounds; round_i++) {
                // Select the letter
                int lett_num = lett_dist(rng);
                char lett = lett_num + 'A';

                // Retrieve the correct hand to use
                char hand_char = hand[lett_num] ? 'R' : 'L';

                // Prints the question
                clear();
                msg_str = lett;
                msg_str += " (";
                msg_str += hand_char;
                msg_str += to_string(digit[lett_num]);
                msg_str += row[lett_num];
                msg_str += ") : ";
                print_msg(msg_str);

                // If it's the first round, start the timer
                if (use_timer && !round_i) {
                    start_time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch());
                }

                // Retrieve the input from the user
                char lett_input;
                lett_input = getch();

                // If mode==PRACTICE and the answer is wrong, display an error circle and keep checking the user input until the answer is correct.
                if (mode == PRACTICE) {
                    while (lett_input != lett && lett_input-'a'+'A' != lett) {
                        clear();
                        string err_str;
                        err_str = ERROR_CIRCLE;
                        err_str += ' ';
                        if (color_avail) {
                            attron(COLOR_PAIR(1));
                        }
                        print_msg(err_str);
                        if (color_avail) {
                            attroff(COLOR_PAIR(1));
                        }
                        print_msg(msg_str);
                        lett_input = getch();
                    }
                }

                // If it's the last round, end the timer
                if (use_timer && round_i == n_rounds-1) {
                    end_time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch());
                }

                // If mode==TEST, check if answer is correct and update the score
                if (lett_input == lett || lett_input-'a'+'A' == lett) {
                    ++score;
                }

                // If mode==PRACTICE, insert a newline to show the next question below the previous one
                if (mode == PRACTICE) { 
                    printw("\n");
                    refresh();
                }
            }

            clear();

            // Print the score if mode==TEST
            if (mode == TEST) {
                msg_str = "Score: ";
                msg_str += to_string(score);
                msg_str += '/';
                msg_str += to_string(n_rounds);
                msg_str += '\n';
                print_msg(msg_str);
            }

            // Print the elapsed time and the typing frequency
            if (use_timer) {
                double elapsed_time = ((end_time-start_time).count())/1000.;
                msg_str = "Elapsed time: ";
                msg_str += to_string(elapsed_time);
                msg_str += " sec\n";
                print_msg(msg_str);
                double freq = n_rounds/elapsed_time;
                msg_str = "Typing frequency: ";
                msg_str += to_string(freq);
                msg_str += " lett/sec";
                if (mode == PRACTICE) {
                    msg_str += " (only correct answers counted)\n";
                } else if (mode == TEST) {
                    msg_str += " (all keystrokes counted)\n";
                }
                print_msg(msg_str);
            }

            // Show the final menu and decide whether to restart or exit
            const char *final_options[] = {
                "Restart",
                "Main menu",
                "Exit"
            };
            int final_options_len = sizeof(final_options)/sizeof(char *);
            int option_choice = option_menu(final_options,final_options_len);
            if (!option_choice) {
                restart = true;
            } else if (option_choice == 1) {
                restart = false;
                main_menu_restart = true;
            } else {
                restart = false;
                main_menu_restart = false;
            }

            // Reset the score
            score = 0;

            clear();
        }
    }

    // Close the ncurses window
    endwin();

    return 0;
}
