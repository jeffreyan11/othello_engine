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
        setData(0, 0, 0, 0, 0, 0, 0, 0);
    }

    BoardData(bitbrd t, bitbrd b, int s, int m, int p, uint8_t tu, int d,
        uint8_t nt) {
        setData(t, b, s, m, p, tu, d, nt);
    }

    void setData(bitbrd t, bitbrd b, int s, int m, int p, uint8_t tu, int d,
        uint8_t nt) {
        taken = t;
        black = b;
        score = s;
        move = (uint8_t) m;
        ptm = (uint8_t) p;
        turn = tu;
        depth = (uint8_t) d;
        nodeType = nt;
    }
};

class HashLL {

public:
    BoardData entry1;
    BoardData entry2;

    HashLL() {}

    HashLL(bitbrd t, bitbrd b, int s, int m, int ptm, uint8_t tu, int d,
        uint8_t nt) {
        entry1 = BoardData(t, b, s, m, ptm, tu, d, nt);
    }

    ~HashLL() {}
};

class Hash {

private:
    HashLL **table;
    int size;
    uint32_t bitMask;

public:
    int keys;

    Hash(int isize);
    ~Hash();

    void add(Board &b, int score, int move, int ptm, uint8_t turn, int depth,
        uint8_t nodeType);
    BoardData *get(Board &b, int ptm);
};

#endif
