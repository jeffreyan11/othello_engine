#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <cstdint>
#include <iostream>
#include "common.h"
#include "board.h"
#include "openings.h"
using namespace std;

class Player {

private:
    int maxDepth;
    int minDepth;
    bitbrd CORNERS;     //(0x8100000000000081)
    bitbrd EDGES;       //(0x3C0081818181003C)
    bitbrd ADJ_CORNERS; //(0x42C300000000C342)
    Openings openingBook;

    int countSetBits(bitbrd b);
    void deleteMoveVector(vector<Move *> v);
    void sort(vector<Move *> &moves, vector<int> &scores, int left, int right);
    void swap(vector<Move *> &moves, vector<int> &scores, int i, int j);
    int partition(vector<Move *> &moves, vector<int> &scores, int left, int
        right, int pindex);

public:
    Board game;
    Side mySide;
    Side oppSide;

    Player(Side side);
    ~Player();
    
    Move *doMove(Move *opponentsMove, int msLeft);
    int heuristic (Board *b);
    int negascout(Board *b, Side side, int depth, int alpha, int beta);

    // Flag to tell if the player is running within the test_minimax context
    bool testingMinimax;
};

#endif
