// Main dictionary loader functions

#ifndef TRIE_MANAGER_CPP
#define TRIE_MANAGER_CPP

// Includes
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

// Local includes
#include "data_structs_n_constants.h"

using namespace std;

Tnode *dictionary;

// Function used in sort() to compare node letters
bool
trie_cmp(const Tnode* a, const Tnode* b)
{
    return a->letter < b->letter;
}

// This struct is used to create lexical closures. We initialize a
// struct with a certain letter and save it to `l` field. This struct
// has an operator `()` overloaded. This acts like a
// function: it takes some value as an argument, and compare it to the
// `l` field in struct.
struct find_letter {
    char l;
    find_letter(char letter) : l(letter) {}
    bool operator () ( const Tnode *m ) const
        {
            return m->letter == l;
        }
};

bool
check_prefix(Tnode *&last_letter, Tnode *root, string prefix)
{
    Tnode *current = root;

    for (char letter : prefix) {
        vector <Tnode*>::iterator it = find_if(current->Tchildren.begin(),
                                               current->Tchildren.end(),
                                               find_letter(letter));
        if (it == current->Tchildren.end()) return false;
        current = *it;
    }
    last_letter = current;
    return true;
}


// Given a trie root and a word `str`, check if trie contains the word `str`
bool
search_word(Tnode *root, string str)
{
    transform(str.begin(), str.end(), str.begin(), ::toupper); // Upcase string
    Tnode *last_letter;

    if (check_prefix(last_letter, root, str))
        return last_letter->is_end;
    else
        return false;
}

// Given a trie root, deletes it from memory, forever :(
void
delete_trie(Tnode *&root) {
    if (root == nullptr) return;
    for (Tnode* child : root->Tchildren) {
        delete_trie(child);
    }
    root->Tchildren.clear();
    delete root;
    root = nullptr;
    return;
}

// Given a trie node and a letter, check if the letter exists in a
// node. If the letter exists, return letter node. Otherwise, create
// new node with a letter, insert it into the trie, and return it.
Tnode*
insert_char(Tnode *node, char letter)
{
    // Check in the letter is in the node children.
    vector<Tnode*>::iterator found_val = find_if(node->Tchildren.begin(),
                                                 node->Tchildren.end(),
                                                 find_letter(letter));

    if (found_val == node->Tchildren.end()) { // If letter is not in the node children
        Tnode *new_letter = new Tnode;        // Create new node with given letter
        new_letter->letter = letter;
        new_letter->is_end = false;
        // new_letter->Tchildren = {};

        vector<Tnode*>::iterator iter = node->Tchildren.begin();
        vector<Tnode*>::iterator end = node->Tchildren.end();
        int size = node->Tchildren.size();

        // Sorted insertion
        while (size != 0
               && iter != end
               && new_letter->letter < (*iter)->letter)
            iter++;

        if (iter == end)
            node->Tchildren.push_back(new_letter);
        else
            node->Tchildren.insert(iter, new_letter);

        // Maybe we will need it in the future. :)
        // node->Tchildren.push_back(new_letter); // Push new node to trie
        // sort(node->Tchildren.begin(), node->Tchildren.end(), trie_cmp); // Sort nodes
        return new_letter;
    } else                      // Otherwise return found node
        return *found_val;
}

// Given a trie root and a word, insert word in the trie.
void
insert_word(Tnode *root, string word)
{
    transform(word.begin(), word.end(), word.begin(), ::toupper); // Upcase string
    Tnode *current_node = root;

    bool is_ascii = true;
    for (char letter : word)
        is_ascii = (is_ascii && ('A' <= letter && letter <= 'Z'));

    if (is_ascii) {
        for (char letter : word) {
            current_node = insert_char(current_node, letter);
        }
        current_node->is_end = true;
    }
    return;
}

// Given a string containing dictionary filename, for every line
// (word) in a dictionary add word to trie.
Tnode*
create_trie(string filename)
{
    string word;
    Tnode *root = new Tnode;
    root->is_end = false;
    ifstream dict(filename); // Create input file stream

    if (dict.is_open()) { // If we can read from it
        while (getline(dict, word)) { // read line (word)
            if (word.size() <= (unsigned int) BOARD_SIZE) {
                insert_word(root, word); // insert line (word) into trie
            }
        }
    }
    return root;
}

void
make_dictionary(string filename)
{
    dictionary = create_trie(filename);
    return;
}

void
destroy_dictionary()
{
    delete_trie(dictionary);
    return;
}

#endif
