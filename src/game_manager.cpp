// Main game functions

#ifndef GAME_MANAGER_CPP
#define GAME_MANAGER_CPP

// Includes
#include <algorithm>
#include <map>
#include <vector>
#include <stdlib.h>

// Local includes
#include "data_structs_n_constants.h"
#include "trie_manager.cpp"

// Board manager

// Board declaration
vector <vector <Letter>> board;

void
make_board()
{
    board.resize(BOARD_SIZE);
    for (int i = 0; i < BOARD_SIZE; i++) {
        board.at(i).resize(BOARD_SIZE);
        for (int j = 0; j < BOARD_SIZE; j++) {
            board.at(i).at(j) = {' ', 0};
        }
    }
    return;
}

void
destroy_board()
{
    board.clear();
}

// Insertion direction of words
bool w_direction = HORIZONTAL;
// After d key is pressed
void toggle_w_direction() { w_direction = !w_direction; }

// Bucket manager

vector <char> bucket;

int myrandom (int i) { return rand()%i; }

// Function for bucket initialization
void
make_bucket()
{
    map <int, vector <char>> letters_map;

    letters_map[15] = {'O'};
    letters_map[14] = {'A'};
    letters_map[12] = {'I'};
    letters_map[11] = {'E'};
    letters_map[6]  = {'C', 'R', 'S', 'T'};
    letters_map[5]  = {'L', 'N', 'M', 'U'};
    letters_map[3]  = {'B', 'D', 'F', 'P', 'V'};
    letters_map[2]  = {'G', 'H', 'Z'};
    letters_map[1]  = {'Q'};

    for (auto const &t : letters_map)
        for (int i = 0; i < t.first; i++)
            for (auto c : t.second)
                bucket.push_back(c);

    random_shuffle(bucket.begin(), bucket.end(), myrandom);
    return;
}

void
destroy_bucket()
{
    bucket.clear();
}

// Function that refills the hand of the player if the bucket is not
// empty
bool
get_letters(Player &player)
{
    if (bucket.empty()) return false;

    while ((int) player.letters.size() < PLAYER_HAND
           && !bucket.empty()) {
        // random_shuffle(bucket.begin(), bucket.end());
        player.letters.push_back(bucket.back());
        bucket.pop_back();
    }
    return true;
}

// Player manager

vector <Player> players;

// This function takes the data, from a string of names, and puts it
// into the datastruct
void
make_players(vector <string> player_names)
{
    for (string name : player_names) {
        Player new_player;
        new_player.name = name;
        new_player.points = 0;
        new_player.passed = false;
        get_letters(new_player);

        players.push_back(new_player);
    }
    return;
}

void
destroy_players()
{
    players.clear();
}

// Game manager

bool first_turn = true;

// Transpose board
void
transpose(vector <vector <Letter>> &brd)
{
    Letter temp;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = i; j < BOARD_SIZE; j++) {

            temp = brd.at(i).at(j);
            brd.at(i).at(j) = brd.at(j).at(i);
            brd.at(j).at(i) = temp;
        }
    }
}

bool
is_letter_correct(vector <char> letters, char letter)
{
    return (find(letters.begin(), letters.end(), letter) != letters.end()
            || letter == ' ') ;
}

// Function that manages letter exchange. If bucket is not empty, it
// swaps given letter with the one from the bucket
bool
exchange_letter(vector <char> &letters, char letter)
{
    if (bucket.empty()) return false;

    vector <char>::iterator found = find(letters.begin(), letters.end(), letter);
    char temp = *found;
    *found = bucket.back();
    bucket.pop_back();
    bucket.push_back(temp);
    random_shuffle(bucket.begin(), bucket.end());
    return true;
}

// Gets the word passing through a precise square
string
get_downword(vector <vector <Letter>> &brd, int x, int y)
{
    int temp_y = y;
    string new_downword;

    while (temp_y >= 0
           && brd.at(temp_y).at(x).letter != ' ') { // Find upword start
        temp_y--;
    }
    temp_y++;

    while (temp_y < BOARD_SIZE
           && brd.at(temp_y).at(x).letter != ' ') { // Find upword end
        new_downword.push_back(brd.at(temp_y).at(x).letter);
        temp_y++;
    }
    return new_downword;
}

// Get the downword than search it in the dictionary
bool
check_downword(vector <vector <Letter>> &brd, int x, int y, char letter)
{
    char prev_letter = brd.at(y).at(x).letter;
    string new_downword;

    brd.at(y).at(x).letter = letter;
    new_downword = get_downword(brd, x, y);
    brd.at(y).at(x).letter = prev_letter;

    return search_word(dictionary, new_downword);
}

// All the actions that have to be done when a letter is placed
void
place_letter(vector <vector <Letter>> &brd, int x, int y,
             vector <char> &letters, vector <char>::iterator letter)
{
    brd.at(y).at(x).letter = *letter;
    letters.erase(letter);
    brd.at(y).at(x).layer++;
}

// First turn conditions. The word must have tiles placed in the
// middle of the board
bool
check_first_turn(int x, int y, string word)
{
    int middle_x = BOARD_SIZE / 2;
    int middle_y = BOARD_SIZE / 2;
    bool even = (BOARD_SIZE % 2 == 0);
    bool found = false;
    int size = ((int) word.size()) + x;

    if (even) {
        if (middle_y == y || (middle_y - 1) == y) {
            for (int i = x; i < size && !found; i++) {
                if (middle_x == i || (middle_x - 1) == i) {
                    found = true;
                }
            }
        }
    } else if (middle_y == y) {
        for (int i = x; i < size && !found; i++) {
            if (middle_x == i) {
                found = true;
            }
        }
    }
    return found;
}

// Looks if the square under and above are empty
bool
check_updown_not_empty(vector <vector <Letter>> &brd, int x, int y)
{
    // Regola del corto circuito
    return ((y > 0 && brd.at(y - 1).at(x).letter != ' ')
            || (y < (BOARD_SIZE - 1) && brd.at(y + 1).at(x).letter != ' '));
}

// This function checks if the word is valid in that place, places it,
// and adds points to the player
bool
insert_word_to_board(vector <vector <Letter>> &virt_board,
                     int x, int y, string word, Player &player)
{
    // Checker variables
    bool word_connected = false; // Check if the word that we are
                                 // trying to insert is attached to at
                                 // least another word
    unsigned int upwords_count = 0; // This variable contains the
                                    // number of upwords in the new
                                    // word that we are trying to
                                    // insert.
    bool letter_placed = false;     // Check if at least one letter was placed

    // First turn check for the center of board
    if (first_turn && !check_first_turn(x, y, word)) return false;
    // If word is empty or it's bigger then board size, don't put the word
    if (word.empty() || word.size() > (unsigned int) BOARD_SIZE) return false;
    // If the word is not in the dictionary, don't put the word
    if (!search_word(dictionary, word)) return false;

    // Check left part
    if (x > 0 && virt_board.at(y).at(x - 1).letter != ' ') return false;
    // Check right part
    int last_letter = x + word.size() - 1;
    if (last_letter >= BOARD_SIZE) return false;
    if (last_letter != (BOARD_SIZE - 1)
        && virt_board.at(y).at(last_letter + 1).letter != ' ') return false;

    for (char chr : word) {
        vector <char>::iterator hand_val = find(player.letters.begin(), player.letters.end(), chr);
        char board_val = virt_board.at(y).at(x).letter;
        bool in_hand = (hand_val != player.letters.end());

        if (in_hand) {
            if (board_val == ' ') {
                if (check_updown_not_empty(virt_board, x, y)) {
                    if (check_downword(virt_board, x, y, *hand_val)) {
                        place_letter(virt_board, x, y, player.letters, hand_val);
                        letter_placed = true;
                        word_connected = true;
                        player.points += 3;
                    } else {
                        return false;
                    }
                } else {
                    place_letter(virt_board, x, y, player.letters, hand_val);
                    letter_placed = true;
                    player.points += 2;
                }
            } else if (board_val != chr) {
                if (virt_board.at(y).at(x).layer < 5) {
                    if (check_updown_not_empty(virt_board, x, y)) {
                        if (check_downword(virt_board, x, y, *hand_val)) {
                            place_letter(virt_board, x, y, player.letters, hand_val);
                            letter_placed = true;
                            upwords_count++;
                            word_connected = true;
                            player.points += 2;
                        } else {
                            return false;
                        }
                    } else {
                        place_letter(virt_board, x, y, player.letters, hand_val);
                        letter_placed = true;
                        upwords_count++;
                        word_connected = true;
                        player.points += 1;
                    }
                } else {
                    return false;
                }
            } else {
                word_connected = true;
            }
        } else if (board_val != chr) {
            return false;
        } else {
            player.points += 1;
            word_connected = true;
        }
        x++;
    }
    // If all letters were used, add 20 points
    if (player.letters.empty()) player.points += 20;

    // if (!first_turn && !word_connected) return false;
    // Thanks De Morgan and Carlo for boolean algebra
    if (!(first_turn || word_connected)) return false;
    if (upwords_count == word.size()) return false;

    if (!letter_placed) return false;

    return true;
}

// Creates a temporary board so we can work on that in a not
// destructive way
bool
check_word(string word, Player &player, int x, int y)
{
    vector <vector <Letter>> temp_board(board);
    Player temp_player = player;

    return insert_word_to_board(temp_board, x, y, word, temp_player);
}

// Check if the condition for a game over occur
bool
is_game_over()
{
    bool all_passed = true;
    bool not_empty_hands = true;
    for (Player p : players) {
        all_passed = (all_passed && p.passed);
        not_empty_hands = (not_empty_hands && (bool) p.letters.size());
    }
    return ((!not_empty_hands && bucket.empty()) || all_passed);
}

// Just returns the Player struct with more points
Player
get_winner()
{
    Player winner = players.at(0);
    winner.points -= (winner.letters.size() * 5);

    for (Player &current : players) {
        current.points -= (current.letters.size() * 5);
        if (current.points > winner.points) winner = current;
    }

    return winner;
}

#endif
