#ifndef __ENDGAME_H__
#define __ENDGAME_H__

#include "common.h"
#include "board.h"
#include "hash.h"
#include "eval.h"

#define END_SHLLW 8
#define USE_BESTMOVE_TABLE true
#define USE_STABILITY true
#define USE_REGION_PAR false

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

    int dispatch(Board &b, int s, int depth, int alpha, int beta);

    void sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth);
    int pvs(Board &b, int side, int depth, int alpha, int beta);

    int nextMoveShallow(int *moves, int *scores, int size, int index);

public:
    int endgameTimeMS;
    int mySide;
    unsigned long long nodes;

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
