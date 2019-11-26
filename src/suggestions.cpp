// Suggestions algorithm

#ifndef SUGGESTIONS_CPP
#define SUGGESTIONS_CPP

// Includes
#include <algorithm>
#include <map>
#include <vector>
#include <unordered_set>
#include <string>

// Local includes
#include "data_structs_n_constants.h"
#include "game_manager.cpp"
#include "trie_manager.cpp"

int current_points;
vector <string> suggestions;
vector <Suggestion> suggestion_structs;

// This funciton gathers the cross-checks in a precise square on the
// board
vector <char>
get_square_cross_checks(vector <vector <Letter>> &g_board, int x, int y)
{
    Tnode *current;
    vector <char> cross_checks;
    string first_part;
    string second_part;

    // 3 cases: only up, only down, up and down

    if ((y - 1) >= 0 && g_board.at(y - 1).at(x).letter != ' ')
        first_part = get_downword(g_board, x, y - 1);

    check_prefix(current, dictionary, first_part);

    if ((y + 1) < BOARD_SIZE && g_board.at(y + 1).at(x).letter != ' ')
        second_part = get_downword(g_board, x, y + 1);

    for (Tnode *e : current->Tchildren)
        if (search_word(current, (e->letter + second_part)))
            cross_checks.push_back(e->letter);

    return cross_checks;
}

// Function to get all the cross-chekcs for each line (the int in the
// map in the line)
map <int, vector <char>>
get_cross_checks(vector <vector <Letter>> &g_board, int y)
{
    map <int, vector <char>> cross_checks;

    for (int x = 0; x < BOARD_SIZE; x++)
        // if (g_board.at(y).at(x).letter == ' ' &&
        if (check_updown_not_empty(g_board, x, y))
            cross_checks[x] = get_square_cross_checks(g_board, x, y);

    return cross_checks;
}

// We assume that the row always contains elements Function to get the
// anchors. Leftmost newly-covered square adjacent to a tile already
// on the board is the anchor. Potential anchor squares: they are the
// empty squares that are adjacent (vertically or horizontally) to
// filled squares. Limit: the maximum size of the left part. This is
// equal to the number of non-anchor squares to the left of the
// current anchor square.
map <int, int>
get_anchors(vector <vector <Letter>> &g_board,
            int y)
{
    // coord, limit
    int anch;
    map <int, int> anchors;

    if (first_turn && y == (anch = (BOARD_SIZE / 2))) {
        anchors[anch] = anch;
    } else {
        if (g_board.at(y).at(0).letter != ' ') anchors[0] = 0;

        for (int x = 1; x < BOARD_SIZE; x++) {
            if (g_board.at(y).at(x).letter != ' ' && g_board.at(y).at(x - 1).letter == ' ') {
                int limit = 1;
                for (int j = x - 2;
                     j >= 0 && (g_board.at(y).at(j).letter == ' ');
                     // && !cross_checks.count(j);
                     j--) {
                    limit++;
                }
                anchors[x] = limit;
            }
        }
    }
    return anchors;
}

void
extend_right_suggestion(vector <vector <Letter>> &g_board, int y, vector <char> &rack,
                        int square, map <int, vector <char>> &cross_checks,
                        bool dir, Tnode *dict, string partial_word);

void
find_next_letter_in_rack(vector <vector <Letter>> &g_board,
                         int y,
                         vector <char> &rack,
                         int square,
                         map <int, vector <char>> &cross_checks,
                         bool dir,
                         Tnode *dict,
                         string &partial_word)
{
    // We try to reach the end
    for (Tnode* current_node : dict->Tchildren) {
        // We search each letter in our rack
        auto it = find(rack.begin(), rack.end(), current_node->letter);
        if (it != rack.end()) { // if rack is not empty
            bool valid = true;  // to check if the cross-check is verified
            if (cross_checks.count(square)) {
                auto itc = find(cross_checks[square].begin(),
                                cross_checks[square].end(), *it);
                // if we find our letter *it in the cross-check for
                // that square
                if (itc == cross_checks[square].end()) valid = false;
            }
            if (valid) {
                char temp_c = *it;
                rack.erase(it);
                // we remove letter just checked so the recursion will
                // not be considered in the next execution
                extend_right_suggestion(g_board, y, rack, square + 1,
                                        cross_checks, dir, current_node,
                                        partial_word + temp_c);
                rack.push_back(temp_c); // we put the letter back in
            }
        }
    }
}

void
extend_right_suggestion(vector <vector <Letter>> &g_board,
                        int y,
                        vector <char> &rack,
                        int square,
                        map <int, vector <char>> &cross_checks,
                        bool dir,
                        Tnode *dict,
                        string partial_word)
{
    if (square >= BOARD_SIZE) return;

    char c_letter;
    if (square >= BOARD_SIZE || (c_letter = g_board.at(y).at(square).letter) == ' ') {
        if (dict->is_end) {     // If we already reached the end of
                                // the tree, add to the passible
                                // suggestions
            suggestion_structs.push_back({partial_word, square - (int) partial_word.size(), y, dir});
            // ( word, x, y, direction)
        }

        find_next_letter_in_rack(g_board, y, rack, square,
                                 cross_checks, dir, dict, partial_word);
    } else {
        // First case: normal attachment
        auto it = find_if(dict->Tchildren.begin(),
                          dict->Tchildren.end(),
                          find_letter(c_letter));
        if (it != dict->Tchildren.end())
            extend_right_suggestion(g_board, y, rack, square + 1,
                                    cross_checks, dir, *it,
                                    partial_word + c_letter);
        // Second case: upword
        find_next_letter_in_rack(g_board, y, rack, square,
                                 cross_checks, dir, dict, partial_word);
    }
}

// This funciotn finds the lef tpart of a suggestion, and for each of
// them, searches a right part
void
get_suggestions_for_anchor(vector <vector <Letter>> &g_board,
                           int y,
                           vector <char> &rack,
                           int anchor,
                           map <int, vector <char>> &cross_checks,
                           bool dir,
                           // Algorithm related
                           Tnode *dict,
                           string partial_word,
                           int limit)
{
    extend_right_suggestion(g_board, y, rack, anchor, cross_checks, dir, dict, partial_word);

    if (limit > 0) {
        for (Tnode* current_node : dict->Tchildren) {
            char temp_c = '\0';
            auto it = find(rack.begin(), rack.end(), current_node->letter);
            if (it != rack.end()) {
                temp_c = *it;
                rack.erase(it);

                get_suggestions_for_anchor(g_board, y, rack, anchor, cross_checks,
                                           dir, current_node, partial_word + temp_c, limit - 1);
                rack.push_back(temp_c);
            }
        }
    }
}

void
get_suggestions_direction(vector <vector <Letter>> &g_board,
                          vector <char> &rack,
                          bool dir)
{
    for (int y = 0; y < BOARD_SIZE; y++) {
        map <int, vector <char>> cross_checks = get_cross_checks(g_board, y);
        map <int, int> anchors = get_anchors(g_board, y); // , cross_checks);
        for (auto const &t : anchors) {
            int anchor = t.first, limit = t.second;
            get_suggestions_for_anchor(g_board, y, rack, anchor, cross_checks,
                                       dir, dictionary, "", limit);
        }
    }
}

bool
compare_by_direction(const Suggestion &a, const Suggestion &b)
{
    return (a.direction > b.direction);
}

int
get_points(vector <vector <Letter>> &g_board,
           Suggestion &sugg,
           Player &player,
           int &pts)
{
    vector <vector <Letter>> temp_board(g_board);
    Player temp_pl = player;
    temp_pl.points = 0;

    if (insert_word_to_board(temp_board, sugg.x, sugg.y, sugg.word, temp_pl)) {
        pts = temp_pl.points;
        return true;
    } else {
        pts = 0;
        return false;
    }
}

string
make_suggestion(Suggestion &sugg, int points)
{
    // | d xx yy board_size pts |
    // suggestion: d x y word pts
    string direction = (sugg.direction == HORIZONTAL) ? ">" : "V";
    string coords;
    if (sugg.direction == HORIZONTAL)
        coords = to_string(sugg.x+1) + " " + to_string(sugg.y+1);
    else
        coords = to_string(sugg.y+1) + " " + to_string(sugg.x+1);
    string pts = to_string(points);
    return direction + " " + coords + " " + sugg.word + " " + pts;
}

void
add_suggestion(vector <vector <Letter>> &g_board,
               Suggestion &sugg,
               int &max_points,
               Player &player)
{
    int current_pts;
    if (get_points(g_board, sugg, player, current_pts)) {
        if (current_pts > max_points) {
            max_points = current_pts;
            suggestions.clear();
            suggestions.push_back(make_suggestion(sugg, current_pts));
        } else if (current_pts == max_points) {
            suggestions.push_back(make_suggestion(sugg, current_pts));
        }
    }
}

// Second version
void
get_best_suggestions(vector <vector <Letter>> &g_board, Player &player)
{
    int max_pts = 0;

    auto it_sugg = suggestion_structs.begin();

    while (it_sugg != suggestion_structs.end()
           && (*it_sugg).direction == HORIZONTAL) {
        add_suggestion(g_board, *it_sugg, max_pts, player);
        it_sugg++;
    }

    transpose(g_board);
    while (it_sugg != suggestion_structs.end()
           && (*it_sugg).direction == VERTICAL) {
        add_suggestion(g_board, *it_sugg, max_pts, player);
        it_sugg++;
    }
    transpose(g_board);
    return;
}

void
uniquify_suggestions()
{
    unordered_set <string> temp_set;
    for (string sugg : suggestions) temp_set.insert(sugg);
    suggestions.assign(temp_set.begin(), temp_set.end());
}

// Main function that returns suggestions for a given board and a
// given player
vector <string>
get_suggestions(vector <vector <Letter>> &g_board,
                Player &player)
{
    vector <char> temp_rack = player.letters;
    suggestions.clear();
    suggestion_structs.clear();

    get_suggestions_direction(g_board, temp_rack, HORIZONTAL);

    transpose(g_board);
    get_suggestions_direction(g_board, temp_rack, VERTICAL);
    transpose(g_board);

    get_best_suggestions(g_board, player);

    uniquify_suggestions();
    return suggestions;
}

#endif
