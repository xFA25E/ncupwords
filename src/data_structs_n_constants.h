// The file with all the data structures and constants that are used
// in the upwords program.

#ifndef DATA_STRUCTS_N_CONSTANTS_H
#define DATA_STRUCTS_N_CONSTANTS_H

#include <string>
#include <vector>

using namespace std;

// Constants
// Direction of insertion of the word
#define HORIZONTAL true
#define VERTICAL   false

// Text color styles
#define NAME_COLOR 1            // Player name colors
#define POINTS_COLOR 2          // Player points colors
#define ALETTER_COLOR 3         // Active letter colors
#define SUGGESTION_COLOR 4      // Suggestions words colors
#define HLETTER_COLOR 5         // Hand letters colors
#define MESSAGE_COLOR 6         // Message box colors

// Some parameters
int BOARD_SIZE  = 10,
    PLAYER_HAND = 7;

// Data structures

struct Tnode {
    char letter;
    bool is_end;
    vector <Tnode*> Tchildren;
};

struct Letter {
    char letter;
    unsigned int layer;
};

struct Player {
    string name;
    vector <char> letters;
    int points;
    bool passed;
};

struct Suggestion {
    string word;
    int x;
    int y;
    bool direction;
};

#endif
