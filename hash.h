#ifndef __HASH_H__
#define __HASH_H__

#include "board.h"
#include "common.h"

struct BoardData {
    bitbrd taken;
    bitbrd black;
    int score;
    uint8_t move;
    uint8_t ptm;
    uint8_t turn;
    uint8_t depth;
    uint8_t nodeType;

    BoardData() {
        taken = 0;
        black = 0;
        score = 0;
        move = 0;
        ptm = 0;
        turn = 0;
        depth = 0;
    }

    BoardData(bitbrd t, bitbrd b, int s, int m, int p, int tu, int d, uint8_t nt) {
        taken = t;
        black = b;
        score = s;
        move = (uint8_t) m;
        ptm = (uint8_t) p;
        turn = (uint8_t) tu;
        depth = (uint8_t) d;
        nodeType = nt;
    }

    void setData(bitbrd t, bitbrd b, int s, int m, int p, int tu, int d,
        uint8_t nt) {
        taken = t;
        black = b;
        score = s;
        move = (uint8_t) m;
        ptm = (uint8_t) p;
        turn = (uint8_t) tu;
        depth = (uint8_t) d;
        nodeType = nt;
    }
};

class HashLL {

public:
    HashLL *next;
    BoardData cargo;

    HashLL(bitbrd t, bitbrd b, int s, int m, int ptm, int tu, int d, uint8_t nt) {
        next = NULL;
        cargo = BoardData(t, b, s, m, ptm, tu, d, nt);
    }

    ~HashLL() {}
};

class Hash {

private:
    HashLL **table;
    int size;

    uint32_t hash(Board &b);

public:
    int keys;
    void test();

    Hash();
    Hash(int isize);
    ~Hash();

    void add(Board &b, int score, int move, int ptm, int turn, int depth,
        uint8_t nodeType);
    BoardData *get(Board &b, int ptm);
    void clean(int turn);
};

#endif
