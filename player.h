#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <cstdint>
#include <iostream>
#include "common.h"
#include "board.h"
using namespace std;

class Player {

private:
    int maxDepth;
    bitbrd CORNERS;     //(0x8100000000000081)
    bitbrd EDGES;       //(0x3C0081818181003C)
    bitbrd ADJ_CORNERS; //(0x42C300000000C342)

    int countSetBits(bitbrd b);
    void deleteMoveVector(vector<Move *> v);

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
