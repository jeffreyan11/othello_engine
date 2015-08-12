#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "common.h"
#include "board.h"
#include "openings.h"
#include "endgame.h"
#include "hash.h"
#include "eval.h"

class Player {

private:
    int maxDepth;
    int minDepth;
    int sortDepth;
    int endgameDepth;
    // The depth currently being attempted. Helps with hash entry aging.
    int attemptingDepth;
    // A hard limit on the depth, in case we broke on the previous turn
    int depthLimit;
    uint64_t nodes;

    Endgame endgameSolver;

    Openings openingBook;
    bool bookExhausted;

    Hash *transpositionTable;

    int turn;
    int timeLimit;
    OthelloTimer timeElapsed;

    int getBestMoveIndex(Board &b, MoveList &moves, int &bestScore, int side,
        int depth);
    int pvs(Board &b, int side, int depth, int alpha, int beta);
    void sortMoves(Board &b, MoveList &legalMoves, int s, int depth,
        bool isPVNode);
    void sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth);
    Move *indexToMove(int index);

public:
    Board game;
    int mySide;
    Eval *evaluater;
    bool otherHeuristic;

    Player(Side side);
    ~Player();

    Move *doMove(Move *opponentsMove, int msLeft);
    void setDepths(int sort, int min, int max, int end);
};

#endif
