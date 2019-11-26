// Helper functions for tui_manager.cpp

#ifndef TUI_HELPER_CPP
#define TUI_HELPER_CPP

// Includes
#include <ncurses.h>

// Local includes
#include "data_structs_n_constants.h"
#include "game_manager.cpp"

// * NAMES WINDOW *

// This function prints player information in coordinates `coordy` and
// `coordx` on window `win`. Also, if the player is `selected` (e.g. it's
// his turn), reverses the foreground and background colors and adds
// "<>" brackets around player information.
void
mvwprint_player(WINDOW *win,
                int coordy,
                int coordx,
                Player player,
                bool selected)
{
    init_pair(NAME_COLOR, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(POINTS_COLOR, COLOR_YELLOW, COLOR_BLACK);

    wattron(win, COLOR_PAIR(NAME_COLOR));

    if (selected) {              // If selected,
        wattron(win, A_REVERSE); // Turn on REVERSE attribute
        mvwprintw(win, coordy, coordx, "< %s:", player.name.c_str()); // Print with brackets
    } else
        mvwprintw(win, coordy, coordx, "  %s:", player.name.c_str());

    wattroff(win, COLOR_PAIR(NAME_COLOR));
    wattron(win, COLOR_PAIR(POINTS_COLOR));

    if (selected) {                            // If selected,
        wprintw(win, " %3d >", player.points); // Print with brackets
        wattroff(win, A_REVERSE);              // Turn off REVERSE attribute
    } else
        wprintw(win, " %3d  ", player.points);

    wattroff(win, COLOR_PAIR(POINTS_COLOR));
    return;
}

// This function should distribute equaly player names and scores
// across the width of the window `nms_win`. Players are stored in
// `players` vector. `player_index` is indicates the current player
// index in the vector.
void
update_names_window(WINDOW *nms_win,
                    vector <Player> players,
                    int player_index)
{
    int coordx,                 // X coordinate of next field
        n_crd,                  // X coordinate of name inside a field
        empty_s,                // Remaining spaces to equalize fields
        width,                  // Window width
        f_width,                // Field width
        c_width,                // Current field width
        size,                   // Name length
        f_count,                // Field count
        i;                      // iterator

    wborder(nms_win,
            0, 0, 0, 0,         // borders l r t b
            ACS_LTEE, ACS_RTEE, ACS_LTEE, ACS_RTEE); // corderns lt rt lb rb

    width = getmaxx(nms_win) - 1; // Window width ('- 1' because we don't consider left border)
    f_count = players.size();     // Field count
    f_width = width / f_count;    // Name field width
    empty_s = width % f_count;    // Remaining empty spaces after
                                  // previous division that we should
                                  // distribute too.
    coordx = 0;
    // First name
    c_width = f_width + ((empty_s-- > 0) ? 1 : 0); // Get current field width
    size = players.at(0).name.size();
    n_crd = ((c_width - size - 10) / 2) + 1; // Name coordinates inside field
    mvwprint_player(nms_win, 1, coordx + n_crd, players.at(0), (0 == player_index)); // Print name
    coordx += c_width;  // Update next field X coordinate

    for (i = 1; i < f_count; i++) {
         // Print delimiters
        mvwaddch(nms_win, 0, coordx, ACS_TTEE);  // + up
        mvwaddch(nms_win, 1, coordx, ACS_VLINE); // |
        mvwaddch(nms_win, 2, coordx, ACS_BTEE);  // + down

        c_width = f_width + ((empty_s-- > 0) ? 1 : 0); // Get current field width
        size = players.at(i).name.size();
        n_crd = ((c_width - size - 10) / 2) + 1; // Name coordinates inside field
        mvwprint_player(nms_win, 1, coordx + n_crd, players.at(i), (i == player_index)); // Print naem

        coordx += c_width;      // Update next field X coordinate
    }
    return;
}

// * BOARD WINDOW *

// Draw board number coordinates at top border
void
draw_board_top_numbers(WINDOW *win, int coordy)
{
    int coordx;

    for (int x = 0; x < BOARD_SIZE; x++) {
        coordx = (x * 4) + 3;
        mvwprintw(win, coordy, coordx, "%4d", x + 1);
    }
    return;
}

void
draw_board_top_border(WINDOW *win, int coordy)
{
    int coordx;

    mvwaddch(win, coordy, 3, ACS_ULCORNER);
    mvwhline(win, coordy, 4, ACS_HLINE, 3);

    for (int x = 1; x < BOARD_SIZE; x++) {
        coordx = (x * 4) + 3;
        mvwaddch(win, coordy, coordx, ACS_TTEE);
        mvwhline(win, coordy, coordx + 1, ACS_HLINE, 3);
    }
    mvwaddch(win, coordy, coordx + 4, ACS_URCORNER);
    return;
}

// Draw line delimiter between rows.
void
draw_board_delimiter(WINDOW *win, int coordy)
{
    int coordx;

    mvwaddch(win, coordy, 3, ACS_LTEE);
    mvwhline(win, coordy, 4, ACS_HLINE, 3);

    for (int x = 1; x < BOARD_SIZE; x++) {
        coordx = (x * 4) + 3;
        mvwaddch(win, coordy, coordx, ACS_PLUS);
        mvwhline(win, coordy, coordx + 1, ACS_HLINE, 3);
    }
    mvwaddch(win, coordy, coordx + 4, ACS_RTEE);
    return;
}

// Draw board letters at given line
void
draw_board_line(WINDOW *win,
                vector <vector <Letter>> &board,
                int y,
                int coordy,
                int sel_y, int sel_x) // Current selected cell
{
    int coordx;
    unsigned int layer;

    init_pair(ALETTER_COLOR, COLOR_BLUE, COLOR_BLACK);

    mvwprintw(win, coordy, 0, "%2d", y + 1); // Draw row number
    mvwaddch(win, coordy, 3, ACS_VLINE);
    for (int x = 0; x < BOARD_SIZE; x++) {
        coordx = (x * 4) + 3;

        wattron(win, A_BOLD);
        layer = board.at(y).at(x).layer;
        if (layer < 5) wattron(win, COLOR_PAIR(ALETTER_COLOR)); // if cell is active, draw it with color
        if (y == sel_y && x == sel_x) { // if the cell being precessed is currently selected
            wattron(win, A_REVERSE);
            mvwprintw(win, coordy, coordx + 1, "<%c>", board.at(y).at(x).letter);
            wattroff(win, A_REVERSE);
        } else
            mvwprintw(win, coordy, coordx + 1, " %c ", board.at(y).at(x).letter);
        if (layer < 5) wattroff(win, COLOR_PAIR(ALETTER_COLOR));
        wattroff(win, A_BOLD);

        mvwaddch(win, coordy, coordx + 4, ACS_VLINE);
    }
    return;
}

void
draw_board_bottom_border(WINDOW *win, int coordy)
{
    int coordx;

    mvwaddch(win, coordy, 3, ACS_LLCORNER);
    mvwhline(win, coordy, 4, ACS_HLINE, 3);

    for (int x = 1; x < BOARD_SIZE; x++) {
        coordx = (x * 4) + 3;
        mvwaddch(win, coordy, coordx, ACS_BTEE);
        mvwhline(win, coordy, coordx + 1, ACS_HLINE, 3);
    }
    mvwaddch(win, coordy, coordx + 4, ACS_LRCORNER);
    return;
}

// Main function for updating game board on window
void
update_board_window(WINDOW *brd_win,
                    vector <vector <Letter>> &board,
                    int sel_y, int sel_x) // Current selected cell
{
    int coordy;
    int height = getmaxy(brd_win);

    draw_board_top_numbers(brd_win, 0); // Top number coordinates
    draw_board_top_border(brd_win, 1); // Top border
    draw_board_line(brd_win, board, 0, 2, // First line
                    sel_y, sel_x);
    for (int y = 1; y < BOARD_SIZE; y++) { // Rest of lines
        coordy = (y * 2) + 1;
        draw_board_delimiter(brd_win, coordy);
        draw_board_line(brd_win, board, y, coordy + 1,
                        sel_y, sel_x);
    }
    draw_board_bottom_border(brd_win, height - 1); // Bottom border

    // Insertion direction
    wattron(brd_win, A_BOLD);
    if (w_direction == HORIZONTAL) {
        mvwprintw(brd_win, 0, 0, "-->");
        mvwaddch(brd_win, 1, 0, ' ');
    } else {
        mvwprintw(brd_win, 0, 0, "   ");
        mvwaddch(brd_win, 0, 0, '|');
        mvwaddch(brd_win, 1, 0, 'v');
    }
    wattroff(brd_win, A_BOLD);
    return;
}

// This function clears suggestions from window `win`
void
clear_suggestions(WINDOW *win)
{
    int starty = 3;
    int startx = 2;
    int width = getmaxx(win) - 4;
    int height = getmaxy(win) - 4;

    for (int y = 0; y < height; y++) {
        mvwhline(win, starty + y, startx, ' ', width);
    }
    return;
}

// Main function for updating suggestions window. First, it clears the
// window. After that, it writes suggestions to it.
void
update_suggestions_window(WINDOW *sgg_win,
                          vector <string> suggestions)
{
    int width = getmaxx(sgg_win);
    int size = ((int) suggestions.size() < (getmaxy(sgg_win) - 4))
        ? suggestions.size() : (getmaxy(sgg_win) - 4);

    init_pair(SUGGESTION_COLOR, COLOR_CYAN, COLOR_BLACK);

    box(sgg_win, 0, 0);
    wattron(sgg_win, A_BOLD);
    mvwprintw(sgg_win, 1, (width - 11) / 2, "SUGGESTIONS");
    wattroff(sgg_win, A_BOLD);
    mvwaddch(sgg_win, 2, 0, ACS_LTEE);
    mvwhline(sgg_win, 2, 1, ACS_HLINE, width - 2);
    mvwaddch(sgg_win, 2, width - 1, ACS_RTEE);

    clear_suggestions(sgg_win);
    wattron(sgg_win, COLOR_PAIR(SUGGESTION_COLOR));
    for (int i = 0; i < size; i++ ) {
        mvwprintw(sgg_win, 3 + i, 2, "%s", suggestions.at(i).c_str());
    }
    wattroff(sgg_win, COLOR_PAIR(SUGGESTION_COLOR));
    return;
}

// Updates hand letters of the player
void
update_letters_window(WINDOW *ltt_win, vector <char> letters)
{
    int size = letters.size();

    init_pair(HLETTER_COLOR, COLOR_RED, COLOR_BLACK);

    if (size) {
        mvwaddch(ltt_win, 0, 0, letters.at(0) | COLOR_PAIR(HLETTER_COLOR));
        for (int x = 1; x < size; x++) {
            int coordx = ((x - 1) * 4) + 2;
            mvwaddch(ltt_win, 0, coordx, ACS_VLINE);
            mvwaddch(ltt_win, 0, coordx + 2, letters.at(x) | COLOR_PAIR(HLETTER_COLOR));
        }
    }
    return;
}

#endif
