#ifndef __ENDHASH_H__
#define __ENDHASH_H__

#include "board.h"
#include "common.h"

struct EndgameEntry {
    bitbrd taken;
    bitbrd black;
    int8_t score;
    uint8_t move;
    uint8_t ptm;
    uint8_t depth;

    EndgameEntry(bitbrd t, bitbrd b, int s, int m, int p, int d) {
        taken = t;
        black = b;
        score = (int8_t) s;
        move = (uint8_t) m;
        ptm = (uint8_t) p;
        depth = (uint8_t) d;
    }

    void setEntry(bitbrd t, bitbrd b, int s, int m, int p, int d) {
        taken = t;
        black = b;
        score = (int8_t) s;
        move = (uint8_t) m;
        ptm = (uint8_t) p;
        depth = (uint8_t) d;
    }
};

class EndHash {

private:
    EndgameEntry **table;
    int size;

    uint32_t hash(const Board &b, int ptm);

public:
    int keys;

    EndHash(int isize);
    ~EndHash();

    void add(const Board &b, int score, int move, int ptm, int depth);
    EndgameEntry *get(const Board &b, int ptm);
};

#endif
