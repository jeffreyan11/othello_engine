#include "endhash.h"

// Creates a endgame hashtable, with argument in number of bits for the bitmask
// The table will have 2^bits entries
EndHash::EndHash(int bits) {
    size = 1 << bits;
    bitMask = 1;
    for (int i = 0; i < bits - 1; i++)
        bitMask |= bitMask << 1;

    table = new EndgameEntry[size];
    keys = 0;
}

EndHash::~EndHash() {
    delete[] table;
}

// Adds key (b, ptm) and item move into the hashtable.
// Assumes that this key has been checked with get and is not in the table.
void EndHash::add(Board &b, int score, int move, int ptm, int depth) {
    uint32_t index = b.getHashCode() & bitMask;
    EndgameEntry *node = &(table[index]);
    if (node->depth == 0) {
        keys++;
        node->setEntry(b.getBits(CWHITE), b.getBits(CBLACK), score, move, ptm, depth);
    }
    // Replacement strategy
    else if (depth + 2 >= node->depth) {
        node->setEntry(b.getBits(CWHITE), b.getBits(CBLACK), score, move, ptm, depth);
    }
}

// Get the move, if any, associated with a board b and player to move.
EndgameEntry *EndHash::get(Board &b, int ptm) {
    uint32_t index = b.getHashCode() & bitMask;
    EndgameEntry *node = &(table[index]);

    if (node->white == b.getBits(CWHITE) && node->black == b.getBits(CBLACK)
                && node->ptm == (uint8_t) ptm)
        return node;

    return NULL;
}