#include <iostream>
#include "endhash.h"

EndHash::EndHash(int isize) {
    table = new EndgameEntry* [isize];
    size = isize;
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
void EndHash::add(const Board &b, int score, int move, int ptm, int depth) {
    uint32_t h = hash(b, ptm);
    unsigned int index = (unsigned int) (h % size);
    EndgameEntry *node = table[index];
    if (node == NULL) {
        table[index] = new EndgameEntry(b.taken, b.black, score, move, ptm,
            depth);
        keys++;
        return;
    }
    // Replacement strategy
    else if (depth >= node->depth) {
        node->setEntry(b.taken, b.black, score, move, ptm, depth);
    }
}

// Get the move, if any, associated with a board b and player to move.
EndgameEntry *EndHash::get(const Board &b, int ptm) {
    uint32_t h = hash(b, ptm);
    unsigned int index = (unsigned int) (h % size);
    EndgameEntry *node = table[index];

    if (node == NULL)
        return NULL;

    if (node->taken == b.taken && node->black == b.black
                && node->ptm == (uint8_t) ptm)
        return node;

    return NULL;
}

// Hashes a board position using the FNV hashing algorithm.
uint32_t EndHash::hash(const Board &b, int ptm) {
    uint32_t h = 2166136261UL;
    h ^= b.taken & 0xFFFFFFFF;
    h *= 16777619;
    h ^= (b.taken >> 32);
    h *= 16777619;
    h ^= (uint32_t) ptm;
    h *= 16777619;
    h ^= b.black & 0xFFFFFFFF;
    h *= 16777619;
    h ^= (b.black >> 32);
    h *= 16777619;
    return h;
}
