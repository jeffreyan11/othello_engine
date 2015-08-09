#include <iostream>
#include "hash.h"

using namespace std;

// Creates a hashtable, with argument in number of bits for the bitmask
// The table will have 2^bits entries
Hash::Hash(int bits) {
    size = 1 << bits;
    bitMask = 1;
    for (int i = 0; i < bits - 1; i++)
        bitMask |= bitMask << 1;

    table = new HashLL* [size];
    for(int i = 0; i < size; i++) {
        table[i] = NULL;
    }
    keys = 0;
}

Hash::~Hash() {
    for(int i = 0; i < size; i++) {
        HashLL* temp = table[i];
        if (temp != NULL)
            delete temp;
    }
    delete[] table;
}

/**
 * @brief Adds key (b,ptm) and item move into the hashtable.
 * Assumes that this key has been checked with get and is not in the table.
*/
void Hash::add(Board &b, int score, int move, int ptm, uint8_t turn,
        int depth, uint8_t nodeType) {
    uint32_t index = b.getHashCode() & bitMask;
    HashLL *node = table[index];
    if (node == NULL) {
        keys++;
        table[index] = new HashLL(b.getTaken(), b.getBits(CBLACK), score, move,
            ptm, turn, depth, nodeType);
        return;
    }
    else if (node->entry2.taken == 0) {
        keys++;
        node->entry2.setData(b.getTaken(), b.getBits(CBLACK), score, move,
            ptm, turn, depth, nodeType);
    }
    // Always update the same position with newer information
    if (node->entry1.taken == b.getTaken()
     && node->entry1.black == b.getBits(CBLACK)
     && node->entry1.ptm == (uint8_t) ptm) {
        node->entry1.setData(b.getTaken(), b.getBits(CBLACK), score, move,
            ptm, turn, depth, nodeType);
    }
    else if (node->entry2.taken == b.getTaken()
          && node->entry2.black == b.getBits(CBLACK)
          && node->entry2.ptm == (uint8_t) ptm) {
        node->entry2.setData(b.getTaken(), b.getBits(CBLACK), score, move,
            ptm, turn, depth, nodeType);
    }
    else {
        BoardData *toReplace = NULL;
        // Prioritize entries with a higher depth, but also from a more
        // recent search space
        int score1 = 4*(turn - node->entry1.turn) + depth - node->entry1.depth;
        int score2 = 4*(turn - node->entry2.turn) + depth - node->entry2.depth;
        if (score1 >= score2)
            toReplace = &(node->entry1);
        else
            toReplace = &(node->entry2);
        if (score1 <= 0 && score2 <= 0)
            toReplace = NULL;

        if (toReplace != NULL) {
            toReplace->setData(b.getTaken(), b.getBits(CBLACK), score, move,
                ptm, turn, depth, nodeType);
        }
        else return;
    }
}

/**
 * @brief Get the move, if any, associated with a board b and player to move.
*/
BoardData *Hash::get(Board &b, int ptm) {
    uint32_t index = b.getHashCode() & bitMask;
    HashLL *node = table[index];

    if (node == NULL)
        return NULL;

    if (node->entry1.taken == b.getTaken()
     && node->entry1.black == b.getBits(CBLACK)
     && node->entry1.ptm == (uint8_t) ptm)
        return &(node->entry1);

    if (node->entry2.taken == b.getTaken()
     && node->entry2.black == b.getBits(CBLACK)
     && node->entry2.ptm == (uint8_t) ptm)
        return &(node->entry2);

    return NULL;
}
