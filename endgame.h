#ifndef __ENDGAME_H__
#define __ENDGAME_H__

#include "common.h"
#include "board.h"
#include "endhash.h"
#include "eval.h"

#define USE_STABILITY true
#define USE_REGION_PAR false
#define USE_ALL_TABLE false

class Endgame {

private:
    EndHash *endgame_table;
    EndHash *killer_table;
    #if USE_ALL_TABLE
    EndHash *all_table;
    #endif

    Eval *evaluater;

    int hashHits;
    int hashCuts;
    int firstFailHigh;
    int failHighs;
    int searchSpaces;

    bool isWLD;

    #if USE_REGION_PAR
    int region_parity;
    #endif

    int dispatch(Board &b, int s, int depth, int alpha, int beta);
    int endgameDeep(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);
    int endgameMedium(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);
    int endgameShallow(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);
    int endgame4(Board &b, int s, int alpha, int beta, bool passedLast);
    int endgame3(Board &b, int s, int alpha, int beta, bool passedLast);
    int endgame2(Board &b, int s, int alpha, int beta);
    int endgame1(Board &b, int s, int alpha, int legalMove);

    void sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth);
    int pvs(Board &b, int side, int depth, int alpha, int beta);
    /*void rootSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth);
    int rootPVS(Board &b, int s, int depth, int alpha, int beta);*/

    int nextMoveShallow(int *moves, int *scores, int size, int index);

public:
    unsigned long long nodes;

    Endgame();
    ~Endgame();

    int solveEndgame(Board &b, MoveList &moves, int s, int depth, int timeLimit,
        Eval *eval, int *exactScore = NULL);
};

#endif
