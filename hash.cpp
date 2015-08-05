#include <iostream>
#include "hash.h"

using namespace std;

Hash::Hash() {
    table = new HashLL* [1000000];
    size = 1000000;
    for(int i = 0; i < size; i++) {
        table[i] = NULL;
    }
    keys = 0;
}

Hash::Hash(int isize) {
    table = new HashLL* [isize];
    size = isize;
    for(int i = 0; i < size; i++) {
        table[i] = NULL;
    }
    keys = 0;
}

Hash::~Hash() {
    for(int i = 0; i < size; i++) {
        HashLL* temp = table[i];
        while(temp != NULL) {
            HashLL *temp2 = temp->next;
            delete temp;
            temp = temp2;
        }
    }
    delete[] table;
}

/**
 * @brief Adds key (b,ptm) and item move into the hashtable.
 * Assumes that this key has been checked with get and is not in the table.
*/
void Hash::add(Board &b, int score, int move, int ptm, int turn,
        int depth, uint8_t nodeType) {
    keys++;
    uint32_t index = b.getHashCode() % size;
    HashLL *node = table[index];
    if(node == NULL) {
        table[index] = new HashLL(b.getTaken(), b.getBits(CBLACK), score, move,
            ptm, turn, depth, nodeType);
        return;
    }

    do {
        if(node->cargo.taken == b.getTaken()
        && node->cargo.black == b.getBits(CBLACK)
        && node->cargo.ptm == (uint8_t) ptm) {
            node->cargo.setData(b.getTaken(), b.getBits(CBLACK), score, move, ptm,
                turn, depth, nodeType);
            return;
        }
        node = node->next;
    } while (node != NULL);

    node = new HashLL(b.getTaken(), b.getBits(CBLACK), score, move, ptm,
        turn, depth, nodeType);
}

/**
 * @brief Get the move, if any, associated with a board b and player to move.
*/
BoardData *Hash::get(Board &b, int ptm) {
    uint32_t index = b.getHashCode() % size;
    HashLL *node = table[index];

    if(node == NULL)
        return NULL;

    do {
        if(node->cargo.taken == b.getTaken()
        && node->cargo.black == b.getBits(CBLACK)
        && node->cargo.ptm == (uint8_t) ptm)
            return &(node->cargo);
        node = node->next;
    }
    while(node != NULL);

    return NULL;
}

void Hash::clean(int turn) {
    for(int i = 0; i < size; i++) {
        HashLL *node = table[i];
        if(node == NULL)
            continue;
        while(node->cargo.turn <= turn) {
            keys--;
            table[i] = node->next;
            delete node;
            node = table[i];
            if(node == NULL)
                break;
        }
    }
}

void Hash::test() {
    int zeros = 0;
    int threes = 0;

    for(int i = 0; i < size; i++) {
        int linked = 0;
        HashLL* node = table[i];
        if(node == NULL)
            zeros++;
        else {
            linked++;
            while(node->next != NULL) node = node->next;
            if(linked >= 3) threes++;
        }
    }

    cout << "zeros: " << zeros << endl;
    cout << "threes: " << threes << endl;
}
