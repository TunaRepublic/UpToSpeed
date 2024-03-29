#include <iostream>
#include <random>
#include <string>
#include <cstring>
#include <ctype.h>
#include <fstream>
#include <filesystem>
#include <pwd.h>
#include <unistd.h>
#include "pcg/pcg_random.hpp"
#include <chrono>
#include <ncurses.h>
#include "mode.h"
#include "options.h"

#define PROGRAM_NAME "UpToSpeed"
#define N_LETTERS 26
#define ERROR_CIRCLE "\u2B24"
#define HIGH_SCORE_DIR_NAME ".uptospeed"
#define HIGH_SCORE_FILENAME "highscore.txt"

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
    int digit_count = 0;
    while (1) {
        c_in = getch();
        if (isdigit(c_in)) {
            ++digit_count;
            printw("%c", c_in);
            num = 10*num + c_in - '0';
        } else if (c_in == (char) KEY_BACKSPACE && digit_count) { // If Delete is pressed, delete the previous character entered
            --digit_count;
            num /= 10;
            printw("\b \b");
        } else if (c_in == 10 && num) { // If Enter is pressed, finish reading the input
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

    // Generate a uniform distribution for lowercase-uppercase
    std::uniform_int_distribution<int> low_upp_dist(0,1);

    // Initialize the ncurses window
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    
    // Define string used to print messages on screen
    string msg_str;

    // Print welcome message
    msg_str = "Welcome to ";
    print_msg(msg_str);
    msg_str = PROGRAM_NAME;
    attron(A_BOLD);
    print_msg(msg_str);
    attroff(A_BOLD);
    msg_str = "! Press Q to exit.\n";
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
        // Select the mode
        print_msg("Please select a mode:\n");
        mode = (Mode) option_menu(mode_options,mode_options_len,true);
        clear();

        // Select the number of rounds
        print_msg("Enter the number of rounds: ");
        select_rounds(&n_rounds);
        clear();

        // Choose among lowercase, uppercase or both
        print_msg("Choose the type of letters:");
        int low_upp_selection = option_menu(low_upp_options,low_upp_options_len,false);
        switch (low_upp_selection) {
            case 3: // Case unsensistive
                case_sens = false;
                break;
            case 2: // Both lowercase and uppercase
                case_sens = true;
                low_upp_both = true;
                break;
            default:
                case_sens = true;
                low_upp_both = false;
                low_upp_case = low_upp_selection;
                break;
        }
        clear();

        // Choose if timer is to be used
        print_msg("Use timer?\n");
        use_timer = !((bool) option_menu(timer_options,timer_options_len,false));
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
                char lett;
                if (!case_sens) {
                    lett = lett_num + 'A';
                } else if (low_upp_both) {
                    bool lett_low_upp = low_upp_dist(rng);
                    if (!lett_low_upp) {
                        lett = lett_num + 'a';
                    } else {
                        lett = lett_num + 'A';
                    }
                } else if (!low_upp_case) {
                    lett = lett_num + 'a';
                } else {
                    lett = lett_num + 'A';
                }

                // Retrieve the correct hand to use
                char hand_char = hand[lett_num] ? 'R' : 'L';

                // Print the question
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
                    while (lett_input != lett) {
                        if (!case_sens && (lett_input-'a'+'A' == lett || lett_input-'A'+'a' == lett)) {
                            break;
                        }
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
                if (lett_input != lett) {
                    if (!case_sens && (lett_input-'a'+'A' == lett || lett_input-'A'+'a' == lett)) {
                        ++score;
                    }
                } else {
                    ++score;
                }

                // If mode==PRACTICE, insert a newline to show the next question below the previous one
                if (mode == PRACTICE) { 
                    printw("\n");
                    refresh();
                }
            }

            clear();

            // Print end message
            if (mode == TEST) {
                msg_str = "Test";
            } else if (mode == PRACTICE) {
                msg_str = "Practice";
            }
            msg_str += " completed!\n\n";
            print_msg(msg_str); 

            // Print the score and the high score if mode==TEST
            if (mode == TEST) {
                msg_str = "Score: ";
                msg_str += to_string(score);
                msg_str += '/';
                msg_str += to_string(n_rounds);
                print_msg(msg_str);

                // Put the cache directory in the user's home directory
                const char *highscore_dir_parent = ".";
                if (!(highscore_dir_parent = getenv("HOME"))) {
                    highscore_dir_parent = getpwuid(getuid())->pw_dir;
                }

                string highscore_dir_path = highscore_dir_parent;
                highscore_dir_path += "/";
                highscore_dir_path += HIGH_SCORE_DIR_NAME;
                string highscore_file_path = highscore_dir_path;
                highscore_file_path += "/";
                highscore_file_path += HIGH_SCORE_FILENAME;

                bool dir_fail = false;

                // Create the cache directory
                try {
                    if (filesystem::is_directory(highscore_dir_parent)) {
                        if (!filesystem::is_directory(highscore_dir_path)) {
                            ifstream test_dir(highscore_dir_path);
                            if (test_dir) {
                                test_dir.close();
                                dir_fail = true;
                                msg_str = " (Error: can't create directory ";
                                msg_str += highscore_dir_path;
                                msg_str += " because a file of the same name already exists)\n";
                                if (color_avail) {
                                    attron(COLOR_PAIR(1));
                                }
                                print_msg(msg_str);
                                if (color_avail) {
                                    attroff(COLOR_PAIR(1));
                                }
                            } else {
                                test_dir.close();
                                filesystem::create_directory(highscore_dir_path);
                            }
                        }
                    } else {
                        dir_fail = true;
                        msg_str = " (Error: can't find directory ";
                        msg_str += highscore_dir_parent;
                        msg_str += " to create the high score file ";
                        msg_str += highscore_file_path;
                        msg_str += ")\n";
                        if (color_avail) {
                            attron(COLOR_PAIR(1));
                        }
                        print_msg(msg_str);
                        if (color_avail) {
                            attroff(COLOR_PAIR(1));
                        }
                    }
                }
                catch (filesystem::filesystem_error& err) {
                    dir_fail = true;                    
                    msg_str = " (Error: can't read/write in directory ";
                    msg_str += highscore_dir_path;
                    msg_str += " to use the high score file ";
                    msg_str += HIGH_SCORE_FILENAME;
                    msg_str += ")\n";
                    if (color_avail) {
                        attron(COLOR_PAIR(1));
                    }
                    print_msg(msg_str);
                    if (color_avail) {
                        attroff(COLOR_PAIR(1));
                    }
                }

                // Access or create the high score file
                if (!dir_fail) {
                    fstream highscore_file(highscore_file_path, fstream::in | fstream::out | fstream::app);
                    if (highscore_file) {
                        int high_score;
                        string high_score_str;
                        if (filesystem::is_empty(highscore_file_path)) {
                            highscore_file << 0 << endl;
                            highscore_file.seekg(0);
                        }  
                        getline(highscore_file,high_score_str);
                        high_score = stoi(high_score_str);
                        // Update the score
                        if (score > high_score) {
                            highscore_file.close();
                            highscore_file.open(highscore_file_path, fstream::out);
                            if (highscore_file) {
                                high_score = score;
                                highscore_file << high_score << endl;
                                msg_str = " (New high score!)\n";
                                print_msg(msg_str);
                            } else {
                                msg_str = " (Error: could not write to the high score file ";
                                msg_str += highscore_file_path;
                                msg_str += ")\n";
                                if (color_avail) {
                                    attron(COLOR_PAIR(1));
                                }
                                print_msg(msg_str);
                                if (color_avail) {
                                    attroff(COLOR_PAIR(1));
                                }
                            }
                        } else {
                            msg_str = " (High score: ";
                            msg_str += to_string(high_score);
                            msg_str += ")\n";
                            print_msg(msg_str);
                        }
                        highscore_file.close();
                    } else {
                        msg_str = " (Error: could not access or create the high score file ";
                        msg_str += highscore_file_path;
                        msg_str += ")\n";
                        if (color_avail) {
                            attron(COLOR_PAIR(1));
                        }
                        print_msg(msg_str);
                        if (color_avail) {
                            attroff(COLOR_PAIR(1));
                        }      
                    }
                }
            }

            // Print the elapsed time and the typing frequency
            if (use_timer) {
                float elapsed_time = ((end_time-start_time).count())/1000.;
                msg_str = "Elapsed time: ";
                print_msg(msg_str);
                printw("%.2f",elapsed_time);
                msg_str = " sec\n";
                print_msg(msg_str);
                float freq = n_rounds/elapsed_time;
                msg_str = "Typing frequency: ";
                print_msg(msg_str);
                printw("%.2f",freq);
                msg_str = " lett/sec";
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
            int option_choice = option_menu(final_options,final_options_len,true);
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
