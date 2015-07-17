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

const int endgameTime[26] = {0,
25, 30, 30, 40, 40, 40, 40, 50, 50, 50, // 1-10
50, 50, 75, 100, 150, // 11-15
300, 600, 1500, 3000, 7500, // 16 - 20
20000, 50000, 120000, 300000, 700000};

class Player {

private:
    int maxDepth;
    int minDepth;
    int sortDepth;
    int endgameDepth;
    int attemptingDepth;

    Endgame endgameSolver;

    #if USE_OPENING_BOOK
    Openings openingBook;
    #endif

    Hash killer_table;

    int turn;
    int totalTimePM;

    Move* indexToMove[64];

    int pvs(Board *b, MoveList &moves, int &bestScore, int side, int depth);
    int pvs_h(Board *b, int side, int depth, int alpha, int beta);
    void sortSearch(Board *b, MoveList &moves, MoveList &scores, int side,
        int depth);

    void sort(MoveList &moves, MoveList &scores, int left, int right);
    void swap(MoveList &moves, MoveList &scores, int i, int j);
    int partition(MoveList &moves, MoveList &scores, int left, int
        right, int pindex);

public:
    Board game;
    int mySide;
    int oppSide;
    Eval *evaluater;

    Player(Side side);
    ~Player();

    Move *doMove(Move *opponentsMove, int msLeft);
};

#endif
