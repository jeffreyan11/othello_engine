#ifndef __ENDGAME_H__
#define __ENDGAME_H__

#include <chrono>
#include "common.h"
#include "board.h"
#include "hash.h"
#include "eval.h"

#define END_SHLLW 9
#define USE_BESTMOVE_TABLE true
#define USE_STABILITY true
#define USE_REGION_PAR false

#define COUNT_NODES true

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

const int STAB_THRESHOLD[45] = {
/*
    64, 64, 64, 64, 8,
    10, 12, 14, 16, 18,
    20, 22, 24, 26, 28,
    30, 32, 34, 36, 38,
    40, 42, 44, 46, 48,
    50, 52, 54, 56, 58,
    60, 60, 62, 62, 64,
    64, 64, 64, 64, 64,
    64, 64, 64, 64, 64
*/
    64, 64, 64, 64, 10,
    12, 14, 16, 18, 20,
    22, 24, 26, 28, 30,
    32, 34, 36, 38, 40,
    42, 44, 46, 48, 50,
    52, 54, 56, 58, 58,
    60, 60, 62, 62, 64,
    64, 64, 64, 64, 64,
    64, 64, 64, 64, 64
};

class Endgame {

private:
    #if USE_BESTMOVE_TABLE
    Hash *endgame_table;
    #endif
    Hash killer_table;

    Eval *evaluater;

    #if USE_REGION_PAR
    int region_parity;
    #endif

    void sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth);
    int pvs(Board &b, int side, int depth, int alpha, int beta);

    void sort(MoveList &moves, MoveList &scores, int left, int right);
    void swap(MoveList &moves, MoveList &scores, int i, int j);
    int partition(MoveList &moves, MoveList &scores, int left, int
        right, int pindex);

public:
    int endgameTimeMS;
    int mySide;
    #if COUNT_NODES
    unsigned long long nodes;
    #endif

    Endgame();
    ~Endgame();

    int endgame(Board &b, MoveList &moves, int depth, Eval *eval);
    int endgame_h(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);
    int endgame_shallow(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);
    int endgame4(Board &b, int s, int alpha, int beta, bool passedLast);
    int endgame3(Board &b, int s, int alpha, int beta, bool passedLast);
    int endgame2(Board &b, int s, int alpha, int beta);
    int endgame1(Board &b, int s, int alpha, int legalMove);
};

#endif
