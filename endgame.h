#ifndef __ENDGAME_H__
#define __ENDGAME_H__

#include <chrono>
#include "common.h"
#include "board.h"
#include "endhash.h"
#include "eval.h"

#define USE_STABILITY false
#define USE_ALL_TABLE true

struct EndgameStatistics;

class Endgame {

private:
    EndHash *endgame_table;
    EndHash *killer_table;
    #if USE_ALL_TABLE
    EndHash *all_table;
    #endif

    Eval *evaluater;

    EndgameStatistics *egStats;

    std::chrono::high_resolution_clock::time_point timeElapsed;
    int timeout;
    bool isWLD;

    int dispatch(Board &b, int s, int depth, int alpha, int beta);
    int endgameDeep(Board &b, int s, int depth, int alpha, int beta,
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

    int nextMoveShallow(int *moves, int *scores, int size, int index);

public:
    unsigned long long nodes;

    Endgame();
    ~Endgame();

    int solveEndgame(Board &b, MoveList &moves, bool isSorted, int s, int depth,
        int timeLimit, Eval *eval, int *exactScore = NULL);
    int solveWLD(Board &b, MoveList &moves, bool isSorted, int s, int depth,
        int timeLimit, Eval *eval, int *exactScore = NULL);
    int solveEndgameWithWindow(Board &b, MoveList &moves, bool isSorted, int s,
        int depth, int alpha, int beta, int timeLimit, Eval *eval,
        int *exactScore = NULL);
    void resetEGTable();
};

#endif
