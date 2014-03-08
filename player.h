#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <iostream>
#include "common.h"
#include "board.h"
using namespace std;

class Player {

private:
    bitset<64> CORNERS;     //(0x8100000000000081)
    bitset<64> EDGES;       //(0x3C0081818181003C)
    bitset<64> ADJ_CORNERS; //(0x42C300000000C342)

    bitset<64> moveToBit(Move *m);

public:
    Board game;
    Side mySide;
    Side oppSide;

    Player(Side side);
    ~Player();
    
    Move *doMove(Move *opponentsMove, int msLeft);
    int heuristic (Move * nextMove);

    // Flag to tell if the player is running within the test_minimax context
    bool testingMinimax;
};

#endif
