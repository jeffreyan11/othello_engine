#ifndef __ENDGAME_H__
#define __ENDGAME_H__

#include "common.h"
#include "board.h"
#include "endhash.h"
#include "eval.h"
#include "hash.h"

#define USE_STABILITY false

struct EndgameStatistics;

/**
 * @brief This class contains a large number of functions to help solve the
 * endgame for a game result or perfect play.
 */
class Endgame {

private:
    EndHash *endgameTable;
    EndHash *killerTable;
    EndHash *allTable;
    Hash *transpositionTable;

    Eval *evaluater;

    EndgameStatistics *egStats;
    // For replacement strategy in sort search hash table
    int sortBranch;

    OthelloTimer timeElapsed;
    int timeout;

    int endgameAspiration(Board &b, MoveList &moves, int s, int depth,
        int alpha, int beta, int &exactScore);
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
};

#endif
