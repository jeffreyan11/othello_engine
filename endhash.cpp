#include <iostream>
#include "endhash.h"

// Creates a endgame hashtable, with argument in number of bits for the bitmask
// The table will have 2^bits entries
EndHash::EndHash(int bits) {
    size = 1 << bits;
    bitMask = 1;
    for (int i = 0; i < bits - 1; i++)
        bitMask |= bitMask << 1;
    table = new EndgameEntry* [size];
    for (int i = 0; i < size; i++) {
        table[i] = NULL;
    }
    keys = 0;
}

EndHash::~EndHash() {
    for (int i = 0; i < size; i++) {
        if (table[i] != NULL)
            delete table[i];
    }
    delete[] table;
}

// Adds key (b, ptm) and item move into the hashtable.
// Assumes that this key has been checked with get and is not in the table.
void EndHash::add(Board &b, int score, int move, int ptm, int depth) {
    uint32_t h = hash(b);
    unsigned int index = (unsigned int) (h & bitMask);
    EndgameEntry *node = table[index];
    if (node == NULL) {
        table[index] = new EndgameEntry(b.getBits(CWHITE), b.getBits(CBLACK),
            score, move, ptm, depth);
        keys++;
        return;
    }
    // Replacement strategy
    else if (depth + 2 >= node->depth) {
        node->setEntry(b.getBits(CWHITE), b.getBits(CBLACK), score, move, ptm, depth);
    }
}

// Get the move, if any, associated with a board b and player to move.
EndgameEntry *EndHash::get(Board &b, int ptm) {
    uint32_t h = hash(b);
    unsigned int index = (unsigned int) (h & bitMask);
    EndgameEntry *node = table[index];

    if (node == NULL)
        return NULL;

    if (node->white == b.getBits(CWHITE) && node->black == b.getBits(CBLACK)
                && node->ptm == (uint8_t) ptm)
        return node;

    return NULL;
}

// Hashes a board position using the FNV hashing algorithm.
uint32_t EndHash::hash(Board &b) {
    uint32_t h = 2166136261UL;
    h ^= b.getTaken() & 0xFFFFFFFF;
    h *= 16777619;
    h ^= (b.getTaken() >> 32);
    h *= 16777619;
    h ^= b.getBits(CBLACK) & 0xFFFFFFFF;
    h *= 16777619;
    h ^= (b.getBits(CBLACK) >> 32);
    h *= 16777619;
    return h;
}
