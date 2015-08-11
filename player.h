#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <chrono>
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
    int attemptingDepth;
    uint64_t nodes;

    Endgame endgameSolver;

    Openings openingBook;
    bool bookExhausted;

    Hash *transpositionTable;

    int turn;
    int timeLimit;
    std::chrono::high_resolution_clock::time_point timeElapsed;

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
    int oppSide;
    Eval *evaluater;
    bool otherHeuristic;

    Player(Side side);
    ~Player();

    Move *doMove(Move *opponentsMove, int msLeft);
    void setDepths(int sort, int min, int max, int end);
};

#endif
