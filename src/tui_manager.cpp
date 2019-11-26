// Main user interface manager.

#ifndef TUI_MANAGER_CPP
#define TUI_MANAGER_CPP

// Includes
#include <ncurses.h>
#include <string>
#include <vector>

// Local includes
#include "data_structs_n_constants.h"
#include "tui_helper.cpp"

#define MIN_HEIGHT (8 + (2 * BOARD_SIZE) + 4) // 32
#define MIN_WIDTH (6 + (4 * BOARD_SIZE) + 2 + 23) // 70
int width_mod = 0;              /* Minimum width modifier.
                                   Prevent from not visualizing names,
                                   because of small window width. */

// Sizes to which update_screen() function refers for update.
// These variables are updated by check_terminel_size() function. Don't change them manually!
int current_width;
int current_height;

// Game windows.
// These are automaticalli initialized by init_tui() function. Don't change them manually!
WINDOW *names_window;           // Window for player names. Also shows player turn.
WINDOW *board_window;           // Window for game board. Shows letters, yeah.
WINDOW *suggestions_window;     // Window for suggestions.
WINDOW *letters_window;         // Window for player hand letters.
WINDOW *message_window;         // Window for showing messages to player. (part of communication mechanism)
WINDOW *input_window;           // Window for player input. (Also part of communication mechanism)
WINDOW *menu_window;            // Window for showing a menu to player (communication)

// Board cursor. Current selected cell on a board Don't change these
// variables manually! These are just for reading! Use
// move_board_cursor() instead!
int board_cursor_x = 0;
int board_cursor_y = 0;

// Function used to calculate minimum window width based on names.
// It's purpose is to get width and then pass it to init_tui()
// function. This should set minimum width correctly.
int
get_names_width(vector <Player> &players)
{
    int size;
    int max = 0;

    for (Player p : players) {
        size = p.name.size();
        if (size > max) max = size;
    }

    return ((max + 10) * players.size()) + 1;
}

// Return true if terminal was resized. Used to check if we need to
// reinitialize all the windows with initialize_windows().
bool
terminal_size_changed()
{
    return (COLS != current_width || LINES != current_height);
}

// Loop, until user doesn't resize terminal to fit the game.
void
check_terminal_size()
{
    current_height = LINES;
    current_width = COLS;
    while (current_width < (MIN_WIDTH + width_mod)
           || current_height < MIN_HEIGHT) {
        clear();
        mvprintw(0, 0, "Please, resize terminal.");
        mvprintw(1, 0, "Minimal WIDTHxHEIGHT: %dx%d",
                 MIN_WIDTH + width_mod, MIN_HEIGHT);
        mvprintw(2, 0, "Current WIDTHxHEIGHT: %dx%d",
                 current_width, current_height);
        refresh();
        getch();
        current_height = LINES;
        current_width = COLS;
    }
    clear();
    refresh();
    return;
}

// Create new ncurses window.
WINDOW*
create_window(int height, int width, int starty, int startx)
{
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    wnoutrefresh(local_win);
    doupdate();
    return local_win;
}

// Destroy ncurses window
void
destroy_window(WINDOW *local_win)
{
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wnoutrefresh(local_win);
    doupdate();
    delwin(local_win);
    return;
}

// Initialize all the window's positions and sizes
void
initialize_windows()
{
     /* height, width,  starty, startx */
    int brd_wh, brd_ww, brd_wy, brd_wx; // Board window
    int sgg_wh, sgg_ww, sgg_wy, sgg_wx; // Suggestions window
    int ltt_wh, ltt_ww, ltt_wy, ltt_wx; // Letters window

    int unused_w;      // horizontal empty characters count
    int used_w;        // horizontal non empty characters count

    // Some calculations
    brd_wh = (BOARD_SIZE * 2) + 2;     // Board height
    brd_ww = (BOARD_SIZE * 4) + 4;     // Board width
    sgg_wh =  brd_wh - 1;              // Suggestions height
    sgg_ww = BOARD_SIZE + 14;          // Suggestions width: | > 10 10 word |
    used_w = brd_ww + sgg_ww;          // Used space (not whitespace)
    unused_w = current_width - used_w; // Whitespaces. These two are
                                       // used to put correctly board
                                       // and suggestions
    // names
    names_window = create_window(3, current_width, 2, 0);

    // board             names and title    coords      names and title
    brd_wy = ((current_height - 6 - (brd_wh + 2)) / 2) + 5;
    brd_wx = ((brd_ww * unused_w) / used_w) / 2; // proportion formula
    board_window = create_window(brd_wh, brd_ww, brd_wy, brd_wx);

    // suggestions
    sgg_wy = brd_wy + 1;
    //             after board        proportion formula
    sgg_wx = (brd_wx * 2) + brd_ww + (((sgg_ww * unused_w) / used_w) / 2);
    suggestions_window = create_window(sgg_wh, sgg_ww, sgg_wy, sgg_wx);

    // letters
    ltt_wh = 1;
    ltt_ww = (PLAYER_HAND - 1) * 4 + 1;
    ltt_wy = brd_wy + brd_wh + 1;
    ltt_wx = (current_width - ltt_ww) / 2;
    letters_window = create_window(ltt_wh, ltt_ww, ltt_wy, ltt_wx);

    return;
}

// function used to initialize tui with only one line. just call it
// with minimum `width` (that we get from names) and everything should
// be set correctly. Trust me.
void
init_tui()
{
    initscr();                  // Initiarize stdscr ncurses
    cbreak(); // Make every keypress available to the program (^C etc)
    noecho(); // Don't put pressed characters on screen
    keypad(stdscr, TRUE);       // Don't use strange keys like F1 ecc.
    curs_set(0);                // Don't show cursor
    if (has_colors()) start_color(); // Turn on colors
    current_width = COLS;
    current_height = LINES;
    clear();
    refresh();

    check_terminal_size();
    // initialize_windows();
    // box(stdscr, 0, 0);

    return;
}

void
set_minimum_width(int width)
{
    int mod = width - MIN_WIDTH;  // If minimum `width` is bigger then width needed by windows,
    if (mod > 0) width_mod = mod; // set width modifier. It will be used in check_terminal_size().
    return;
}

// Well, because endwin name is very counterintuitive, we use end_tui().
void
end_tui()
{
    endwin();                   // End ncurses session
    return;
}

// Destroy all the game windows. Used when terminal size changes. Just
// a shortcut for not writing 4 lines every time.
void
destroy_windows()
{
    destroy_window(names_window);
    destroy_window(board_window);
    destroy_window(suggestions_window);
    destroy_window(letters_window);
    return;
}

// Another shortcut, but used to refresh all the windows when updating the screen.
void
refresh_windows()
{
    wnoutrefresh(names_window);
    wnoutrefresh(board_window);
    wnoutrefresh(suggestions_window);
    wnoutrefresh(letters_window);
    doupdate();
    return;
}

// Function used to move cursor on the board. Remember, use only this
// function, don't change cursor manually!
void
move_board_cursor(int y_dir, int x_dir)
{
    int temp_x = (board_cursor_x + x_dir) % BOARD_SIZE,
        temp_y = (board_cursor_y + y_dir) % BOARD_SIZE;

    if (temp_x < 0) temp_x = BOARD_SIZE - 1;
    if (temp_y < 0) temp_y = BOARD_SIZE - 1;

    board_cursor_x = temp_x;
    board_cursor_y = temp_y;

    return;
}

// Main screen update function. Just call it with parameters
void
update_screen(vector <vector <Letter>> &board,
              vector <Player> players,
              vector <string> suggestions,
              unsigned int player_index)
{
    if (terminal_size_changed()) {
        check_terminal_size();
        destroy_windows();
        initialize_windows();
    }
    box(stdscr, 0, 0);

    attron(A_BOLD);
    mvprintw(1, (current_width - 7) / 2, "UPWORDS"); // Print title
    attroff(A_BOLD);

    update_names_window(names_window, players, player_index);
    update_board_window(board_window, board,
                        board_cursor_y, board_cursor_x);
    update_suggestions_window(suggestions_window, suggestions);
    update_letters_window(letters_window, players[player_index].letters);
    wnoutrefresh(stdscr);
    refresh_windows();

    return;
}

// Function used for communication with user. Just pass it a vector
// with strings. They will be displayed at the center of the screen,
// that will disappear at any keypress. Basically, it creates and
// destroys every time a new window. Be sure that your strings aren't
// longer thin terminal width, otherwise, the message won't be shown
// (function checks terminal sizes). If sizes are bigger, function
// will return false, otherwise, true.
bool
show_message(vector <string> message)
{
    string help_msg = "Press any key to continue...";
    int startx;
    int starty;
    int msg_height = message.size() + 4;
    int msg_width = help_msg.size() + 4;

    for (string str : message) {
        int temp = str.size() + 4;
        if (temp > msg_width) msg_width = temp;
    }

    if (msg_height <= current_height
        && msg_width <= current_width) {

        startx = (current_width - msg_width) / 2;
        starty = (current_height - msg_height) / 2;

        message_window = create_window(msg_height, msg_width, starty, startx);

        wattron(message_window, A_BOLD);
        box(message_window, 0, 0);
        wattroff(message_window, A_BOLD);

        init_pair(MESSAGE_COLOR, COLOR_GREEN, COLOR_BLACK);
        wattron(message_window, COLOR_PAIR(6));
        for (int i = 0; i < msg_height - 4; i++) {
            mvwprintw(message_window, 1 + i, 2, "%s", message.at(i).c_str());
        }
        mvwprintw(message_window, msg_height - 2, 2, "%s", help_msg.c_str());
        wattroff(message_window, COLOR_PAIR(6));

        wgetch(message_window);
        wnoutrefresh(message_window);
        doupdate();
        destroy_window(message_window);

        return true;            // Message was shown
    } else
        return false;           // Message wasn't shown
}

// Another function used for communication. It gets input from user.
// It receives a `prompt` string and a maximum input `width`. A small
// window will be shown at the center of the screen, where user can
// type input that will be returned as string. Backspaces and
// whitespaces are also implemented! It works by creating and
// destroying new window every time. Returns value after pressing
// Enter.
string
get_input(string prompt, int width)
{
    string result;
    int chr;
    int pos = 0;
    int coordx = prompt.size() + 4;
    int inp_width = prompt.size() + width + 6;
    int inp_height = 3;
    int starty;
    int startx;

    if (inp_width <= current_width
        && inp_height <= current_height) {

        starty = (current_height - inp_height) / 2;
        startx = (current_width - inp_width) / 2;

        input_window = create_window(inp_height, inp_width, starty, startx);

        box(input_window, 0, 0);
        mvwprintw(input_window, 1, 2, "%s:", prompt.c_str());

        while ((chr = wgetch(input_window)) != 10) { // Enter pressed

            if (chr == 127 && pos != 0) { // Backspace pressed
                mvwaddch(input_window, 1, coordx + pos - 1, ' ');
                result.pop_back();
                pos--;
                // If it's a printable character or a whitespace
            } else if ((('!' <= chr && chr <= '~') || chr == ' ')
                       && result.size() != (unsigned long) width) {
                mvwaddch(input_window, 1, coordx + pos, chr);
                result.push_back(chr);
                pos++;
            }
            wnoutrefresh(input_window);
            doupdate();
        }
        destroy_window(input_window);
    }
    return result;
}

void
update_menu(string title, vector <string> contents, int pos)
{
    int width = getmaxx(menu_window);

    box(menu_window, 0, 0);
    mvwprintw(menu_window, 1, 3, "%s", title.c_str());
    mvwaddch(menu_window, 2, 0, ACS_LTEE);
    mvwhline(menu_window, 2, 1, ACS_HLINE, width - 2);
    mvwaddch(menu_window, 2, width - 1, ACS_RTEE);

    for (int y = 0; y < (int) contents.size(); y++) {
        if (pos == y) wattron(menu_window, A_REVERSE);
        mvwprintw(menu_window, y + 3, 3, "%s", contents.at(y).c_str());
        if (pos == y) wattroff(menu_window, A_REVERSE);
    }
    wnoutrefresh(menu_window);
    doupdate();
}

// This function is also used for user intarction. It takes a title
// and a list of strings. The title will be printed as the title of
// the menu. The elements of the list will be printed on the screen
// for the user to choose one of them. Returns chosen item.
string
show_menu(string title, vector <string> contents)
{
    int chr;
    int pos = 0;
    int startx;
    int starty;
    int menu_width = title.size();
    int menu_height = contents.size() + 4;

    for (string s : contents)
        if (s.size() > (unsigned int) menu_width) menu_width = s.size();
    menu_width += 6;

    if (menu_width <= current_width
        && menu_height <= current_height) {

        starty = (current_height - menu_height) / 2;
        startx = (current_width - menu_width) / 2;

        menu_window = create_window(menu_height, menu_width, starty, startx);
        clear();
        refresh();
        update_menu(title, contents, pos);
        wnoutrefresh(menu_window);
        doupdate();

        while ((chr = getch()) != 10) { // Enter pressed
            if (chr == KEY_UP) {
                pos = ((pos - 1) < 0) ? (contents.size() - 1) : pos - 1;
            } else if (chr == KEY_DOWN) {
                pos = ((pos + 1) >= (int) contents.size()) ? 0 : pos + 1;
            }
            update_menu(title, contents, pos);
        }
        destroy_window(menu_window);
    }
    return contents.at(pos);
}

#endif
