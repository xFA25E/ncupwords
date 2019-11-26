// Includes
#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Local includes
#include "data_structs_n_constants.h"
#include "game_manager.cpp"
#include "suggestions.cpp"
#include "trie_manager.cpp"
#include "tui_manager.cpp"

using namespace std;

// We removed "e'" from dictionary, because it's useless
string filename = "dictionary.txt";

// Function that takes and manages specific run- arguments like -d and
// -h
void
parse_arguments(int argc, char **argv)
{

    for (int i = 0; i < argc; i++) {
        cout << argv[i] << endl;
        if (!strcmp("-h", argv[i])) {
            cout << "This program is an upwords game" << endl
                 << "Accepts the following flags:" << endl
                 << "  -d  dictionary  File containing dictionary words separated by newlines" << endl
                 << "                  (Default: dictionary.txt)" << endl
                 << "  -h              Show this help message" << endl;
            exit(0);
        }
        else if (!strcmp("-d", argv[i])) {
            if (i < (argc - 1)) {
                cout << "Setting dictionary file: " << argv[i + 1] << endl;
                filename = argv[i + 1];
            } else {
                cout << "Wrong usage. See -h for help" << endl;
                exit(1);
            }
        }
    }
}

// This function just prompts if a player wants to start a game
bool
accept_players(vector <string> names, int count)
{
    string answer;
    vector <string>::iterator it = names.begin();

    names.insert(it, "Start game with " + to_string(count) + " players:");

    show_message(names);
    answer = show_menu("Start game?", {"Yes", "No"});
    return (answer == "Yes") ? true : false;
}

// This function asks names of players and initializes players data
// structure
void
initialize_players()
{
    int count;
    vector <string> names;

    count = stoi(show_menu("How many players?", {"2", "3", "4"}), nullptr, 10);

    for (int i = 0; i < count; i++) {
        string name;
        while (name.empty())
            name = get_input("Player " + to_string(i + 1), 25);

        names.push_back(name);
    }

    if (accept_players(names, count))
        make_players(names);
    else
        initialize_players();
    return;
}

// Function prompts ta tho player if he want to exchange letter
bool
ask_exchange_letter(Player &player)
{
    string temp_string;
    char temp_char;
    do {
        temp_string = get_input("Insert letter to exchange (empty to cancel)", 1);
        temp_char = (temp_string.empty() ? ' ' : toupper(temp_string.at(0)));
    } while (!is_letter_correct(player.letters, temp_char));
    temp_string.clear();
    temp_string = "Exchange letter ";
    temp_string += temp_char;
    temp_string += "? [/no]";
    if (temp_char != ' ' && get_input(temp_string, 2) != "no") {
        if (!exchange_letter(player.letters, temp_char)) {
            show_message({"The bucket is empty.", "We are really sorry! :("});
            return false;
        } else
            return true;
    } else
        return false;
}

// Prompts to check, if the word is correct in that spon and
// direction. Inserts the word if the player really wants it
bool
check_n_insert(string word, Player &player, int x, int y)
{
    if (check_word(word, player, x, y)) {
        string prompt = "Insert " + word + " ";
        if (w_direction == VERTICAL)
            prompt += "vertically at (x " + to_string(1 + y) + " y " + to_string(1 + x);
        else
            prompt += "horizontally at (x " + to_string(1 + x) + " y " + to_string(1 + y);
        prompt += ") [/no]";

        if (get_input(prompt, 2) != "no"
            && insert_word_to_board(board, x, y, word, player)) {
            if (first_turn) first_turn = false;
            return true;
        } else
            return false;
    } else {
        show_message({"The word '" + word + "' is not valid"});
        return false;
    }
}

bool
ask_word_insertion(Player &player)
{
    string prompt = "Insert word (";
    prompt += (w_direction == VERTICAL) ? "vertical)" : "horizontal)";
    string word = get_input(prompt, BOARD_SIZE);
    transform(word.begin(), word.end(), word.begin(), ::toupper); // Upcase string
    int x = board_cursor_x;
    int y = board_cursor_y;
    bool result;

    if (w_direction == VERTICAL) {
        transpose(board);
        result = check_n_insert(word, player, y, x);
        transpose(board);
    } else
        result = check_n_insert(word, player, x, y);

    return result;
}

// every turn
void
player_play(Player &player, int player_index)
{
    bool player_loop = true;
    vector <string> suggestions;

    player.passed = false;
    get_letters(player);
    // Temp
    string temp_hand;
    // Temp

    int ch;
    update_screen(board, players, suggestions, player_index);
    while (player_loop) {
        switch (ch = getch()) {
        case KEY_UP:
            move_board_cursor(-1, 0);
            break;
        case KEY_DOWN:
            move_board_cursor(1, 0);
            break;
        case KEY_LEFT:
            move_board_cursor(0, -1);
            break;
        case KEY_RIGHT:
            move_board_cursor(0, 1);
            break;
        case 'h':
            show_message({"h for help", "d to change insertion direction",
                          "i to insert", "p to pass", "e to exchange",
                          "s for suggestions", "arrows to move"});
            break;
        case 'i':
            player_loop = !ask_word_insertion(player);
            break;
        case 'd':
            toggle_w_direction();
            break;
        case 'p':
            if (get_input("Pass? [/no]", 2) != "no") {
                player.passed = true;
                player_loop = false;
            }
            break;
        case 'e':
            player_loop = !ask_exchange_letter(player);
            break;
        case 's':
            suggestions = get_suggestions(board, player);
            break;
            // Temporary god mode
        // case 'c':
            // temp_hand = get_input("Insert hand", 7);
            // player.letters.clear();
            // for (char c : temp_hand) player.letters.push_back(toupper(c));
            // get_letters(player);
            // break;
        default:
            break;
        }
        update_screen(board, players, suggestions, player_index);
    }
    return;
}

// Game loop - returns the winner
Player
game_loop()
{
    bool game_is_over = false;
    int player_count = players.size();
    int player_turn = 0;

    while (!game_is_over) {
        player_play(players.at(player_turn), player_turn);
        player_turn = (player_turn + 1) % player_count;
        game_is_over = is_game_over();
    }

    return get_winner();
}

// First function to be called after the main. It initializes the
// borad, dictionary, calls the game_loop(), displays the winner
void
start_game()
{
    make_board();
    make_bucket();
    initialize_players();
    first_turn = true;

    // Only once, only you (maybe clear this later)
    clear();
    mvprintw(current_height/2, current_width/2 - 9, "Loading dictionary...");
    refresh();
    // #############################
    make_dictionary(filename);  // #
    // #############################
    clear();
    refresh();


    set_minimum_width(get_names_width(players));
    initialize_windows();

    Player winner = game_loop();
    vector <string> winner_message = {
        "The winner is " + winner.name + ",",
        "with " + to_string(winner.points) + " points."
    };
    show_message(winner_message);

    destroy_board();
    destroy_bucket();
    destroy_players();
    destroy_dictionary();
    destroy_windows();
}

// Main function, manages the menu and settings
int
main(int argc, char **argv)
{
    string option;
    string setting;

    parse_arguments(argc, argv);
    srand((unsigned int) time(NULL));
    init_tui();

    while (option != "Exit") {
        option = show_menu("UPWORDS", {"Start Game",
                                       "Settings",
                                       "Exit"});

        if (option == "Start Game")
            start_game();
        else if (option == "Settings") {
            while (setting != "To Main Menu") {
                setting = show_menu("Settings", {"Board Size",
                                                 "Player Hand",
                                                 "To Main Menu"});
                if (setting == "Board Size") {
                    setting = show_menu("Board Size", {"10", "12", "14", "16", "18"});
                    BOARD_SIZE = stoi(setting, nullptr, 10);
                } else if (setting == "Player Hand") {
                    setting = show_menu("Player Hand", {"7", "10", "13"});
                    PLAYER_HAND = stoi(setting, nullptr, 10);
                }
            }
            setting.clear();
        }
    }

    end_tui();
    return 0;
}
