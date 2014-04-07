#include "board.h"
#include "common.h"
#include <iostream>

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
    uint32_t hash(const Board *b);

public:
    //TODO for testing
    int keys;

    Hash();
    Hash(int isize);
    ~Hash();

    void add(const Board *b, int move, int turn);
    int get(const Board *b, int turn);
    void clean(int turn);
};
