#ifndef __HASH_H__
#define __HASH_H__

#include "board.h"
#include "common.h"
#include <iostream>

#define USE_HASH64 false

struct BoardData {
    bitbrd taken;
    bitbrd black;
    int move;
    int turn;

    BoardData() {
        taken = 0;
        black = 0;
        move = -1;
        turn = 0;
    }

    BoardData(bitbrd t, bitbrd b, int m, int tu) {
        taken = t;
        black = b;
        move = m;
        turn = tu;
    }
};

class HashLL {

public:
    HashLL *next;
    BoardData cargo;

    HashLL(bitbrd t, bitbrd b, int m, int tu) {
        next = NULL;
        cargo = BoardData(t, b, m, tu);
    }

    ~HashLL() {}
};

class Hash {

private:
    HashLL **table;
    int size;

    //void process(int index);
    #if USE_HASH64
    bitbrd hash(const Board *b);
    #else
    uint32_t hash(const Board *b);
    #endif

public:
    //TODO for testing
    int keys;
    void test();

    Hash();
    Hash(int isize);
    ~Hash();

    void add(const Board *b, int move, int turn);
    int get(const Board *b);
    void clean(int turn);
};

#endif
