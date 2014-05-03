#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <chrono>
#include <iostream>
#include "common.h"
#include "board.h"
#include "openings.h"
#include "endgame.h"
#include "hash.h"
#include "eval.h"
using namespace std;

#define USE_OPENING_BOOK false

const int endgameTime[22] = {25, 30, 30, 40, 40, 40, 40, 50, 50, 50, // 1-10
50, 75, 100, 200, 500,
1500, 4000, 12000, 30000, 75000,
200000, 500000};

class Player {

private:
    int maxDepth;
    int minDepth;
    int sortDepth;
    int endgameDepth;
    int attemptingDepth;

    Eval *evaluater;
    Endgame endgameSolver;

    #if USE_OPENING_BOOK
    Openings openingBook;
    #endif

    Hash killer_table;

    int turn;
    int totalTimePM;

    Move* indexToMove[64];

    int pvs(Board *b, MoveList &moves, MoveList &scores, int side, int depth,
        int alpha, int beta);
    int pvs_h(Board *b, int &topScore, int side, int depth,
        int alpha, int beta);

    void sort(MoveList &moves, MoveList &scores, int left, int right);
    void swap(MoveList &moves, MoveList &scores, int i, int j);
    int partition(MoveList &moves, MoveList &scores, int left, int
        right, int pindex);

public:
    Board game;
    int mySide;
    int oppSide;

    Player(Side side);
    ~Player();

    Move *doMove(Move *opponentsMove, int msLeft);
};

#endif
