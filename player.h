#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include "common.h"
#include "board.h"
#include "openings.h"
#include "endgame.h"
#include "hash.h"
using namespace std;

const bitbrd CORNERS = 0x8100000000000081;
const bitbrd EDGES = 0x3C0081818181003C;
const bitbrd ADJ_CORNERS = 0x4281000000008142;
const bitbrd X_CORNERS = 0x0042000000004200;

class Player {

private:
    int maxDepth;
    int minDepth;
    int sortDepth;
    int endgameDepth;
    int attemptingDepth;
    bool endgameSwitch;
    Openings openingBook;
    Endgame endgameSolver;

    int turn;
    int totalTimePM;
    int endgameTimeMS;

    Move* indexToMove[64];

    int mobilities[6561];

    int heuristic(Board *b);
    int pvs(Board *b, vector<int> &moves, vector<int> &scores,
        Side side, int depth, int alpha, int beta);
    int pvs_h(Board *b, int &topScore, Side side, int depth,
        int alpha, int beta);

    int countSetBits(bitbrd b);
    int boardToPV(Board *b);
    int mobilityEstimate(Board *b);
    int bitsToPI(bitbrd b, bitbrd w);

    void sort(vector<int> &moves, vector<int> &scores, int left, int right);
    void swap(vector<int> &moves, vector<int> &scores, int i, int j);
    int partition(vector<int> &moves, vector<int> &scores, int left, int
        right, int pindex);
    void readMobilities();

public:
    Board game;
    Side mySide;
    Side oppSide;

    Hash killer_table;

    Player(Side side);
    ~Player();
    
    Move *doMove(Move *opponentsMove, int msLeft);
};

#endif
