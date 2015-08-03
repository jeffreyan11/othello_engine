#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "common.h"
#include "board.h"
#include "openings.h"
#include "endgame.h"
#include "hash.h"
#include "eval.h"
using namespace std;

#define USE_OPENING_BOOK false

class Player {

private:
    int maxDepth;
    int minDepth;
    int sortDepth;
    int endgameDepth;
    int attemptingDepth;
    uint64_t nodes;

    Endgame endgameSolver;

    #if USE_OPENING_BOOK
    Openings openingBook;
    #endif

    Hash transpositionTable;

    int turn;
    int timeLimit;

    Move* indexToMove[64];

    int pvs(Board &b, MoveList &moves, int &bestScore, int side, int depth);
    int pvs_h(Board &b, int side, int depth, int alpha, int beta);
    void sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth);

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
