#ifndef __PLAYER_H__
#define __PLAYER_H__

//#include <cstdint>
#include <iostream>
#include "common.h"
#include "board.h"
using namespace std;

//#define bits unsigned long long

class Player {

/*private:
    bits CORNERS;     //(0x8100000000000081)
    bits EDGES;       //(0x3C0081818181003C)
    bits ADJ_CORNERS; //(0x42C300000000C342)

    bits moveToBit(Move *m);*/

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
