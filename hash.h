#include "board.h"
#include "common.h"
#include <iostream>

struct BoardData {
    bitbrd taken;
    bitbrd black;
    int move;

    BoardData() {
        taken = 0;
        black = 0;
        move = -1;
    }

    BoardData(bitbrd t, bitbrd b, int m) {
        taken = t;
        black = b;
        move = m;
    }
};

class HashLL {

public:
    HashLL *next;
    BoardData cargo;

    HashLL(bitbrd t, bitbrd b, int m) {
        next = NULL;
        cargo = BoardData(t, b, m);
    }

    ~HashLL() {}
};

class Hash {

private:
    HashLL **table;
    int size;

    //void process(int index);
    uint32_t hash(const Board *b);

public:
    //TODO for testing
    int keys;

    Hash();
    Hash(int isize);
    ~Hash();

    void add(const Board *b, int move);
    int get(const Board *b);
};
