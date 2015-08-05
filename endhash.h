#ifndef __ENDHASH_H__
#define __ENDHASH_H__

#include "board.h"
#include "common.h"

struct EndgameEntry {
    bitbrd white;
    bitbrd black;
    int8_t score;
    uint8_t move;
    uint8_t ptm;
    uint8_t depth;

    EndgameEntry(bitbrd w, bitbrd b, int s, int m, int p, int d) {
        setEntry(w, b, s, m, p, d);
    }

    void setEntry(bitbrd w, bitbrd b, int s, int m, int p, int d) {
        white = w;
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
    uint32_t bitMask;

    uint32_t hash(Board &b);

public:
    int keys;

    EndHash(int bits);
    ~EndHash();

    void add(Board &b, int score, int move, int ptm, int depth);
    EndgameEntry *get(Board &b, int ptm);
};

#endif
