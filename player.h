#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "common.h"
#include "board.h"
#include "openings.h"
#include "endgame.h"
#include "hash.h"
#include "eval.h"

#define COLLECT_MPC_STATS false

class Player {

private:
    int maxDepth;
    int minDepth;
    int sortDepth;
    int endgameDepth;
    // The depth currently being attempted. Helps with hash entry aging.
    int attemptingDepth;
    // The last max depth achieved, for entering endgame solver
    int lastMaxDepth;

    uint64_t nodes;
    #if COLLECT_MPC_STATS
    uint64_t MPCdone[21];
    uint64_t MPCfail[21];
    #endif

    Move *indexToMove[64];

    Endgame endgameSolver;

    Openings openingBook;
    bool bookExhausted;

    Hash *transpositionTable;

    int turn;
    int timeLimit;
    OthelloTime timeElapsed;

    int getBestMoveIndex(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth);
    int pvs(Board &b, int side, int depth, int alpha, int beta);
    unsigned int sortMoves(Board &b, MoveList &legalMoves, int s, int depth,
        int alpha, bool isPVNode);
    void sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth);

public:
    Board game;
    int mySide;
    Eval *evaluater;
    bool otherHeuristic;

    Player(Side side);
    ~Player();

    Move *doMove(Move *opponentsMove, int msLeft);
    void setDepths(int sort, int min, int max, int end);
    uint64_t getNodes();
};

#endif
