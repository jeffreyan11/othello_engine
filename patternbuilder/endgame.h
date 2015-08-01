#ifndef __ENDGAME_H__
#define __ENDGAME_H__

#include "common.h"
#include "board.h"
#include "hash.h"

#define END_SHLLW 11
#define USE_BESTMOVE_TABLE false
#define USE_STABILITY false
#define STAB_ASP 16
#define STAB_UP 48
#define USE_REGION_PAR false

const int QUADRANT_ID[64] = {
1, 1, 1, 1, 2, 2, 2, 2,
1, 1, 1, 1, 2, 2, 2, 2,
1, 1, 1, 1, 2, 2, 2, 2,
1, 1, 1, 1, 2, 2, 2, 2,
4, 4, 4, 4, 8, 8, 8, 8,
4, 4, 4, 4, 8, 8, 8, 8,
4, 4, 4, 4, 8, 8, 8, 8,
4, 4, 4, 4, 8, 8, 8, 8
};

class Endgame {

private:
    #if USE_BESTMOVE_TABLE
    Hash *endgame_table;
    #endif
    Hash killer_table;

    #if USE_REGION_PAR
    int region_parity;
    #endif

    int pvs(Board &b, MoveList &moves, MoveList &scores, int side, int depth,
        int alpha, int beta);
    int pvs_h(Board &b, int &topScore, int side, int depth,
        int alpha, int beta);

    int bitScanForward(bitbrd bb);
    int countSetBitsLow(bitbrd i);
    void sort(MoveList &moves, MoveList &scores, int left, int right);
    void swap(MoveList &moves, MoveList &scores, int i, int j);
    int partition(MoveList &moves, MoveList &scores, int left, int
        right, int pindex);

public:
    int mySide;

    Endgame();
    ~Endgame();

    int endgame(Board &b, MoveList &moves, int depth);
    int endgame_score(Board &b, MoveList &moves, int depth);
    int endgame_h(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);
    int endgame_shallow(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);
    int endgame4(Board &b, int s, int alpha, int beta, bool passedLast);
    int endgame3(Board &b, int s, int alpha, int beta, bool passedLast);
    int endgame2(Board &b, int s, int alpha, int beta);
    int endgame1(Board &b, int s, int alpha);
};

#endif