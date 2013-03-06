#ifndef __EXAMPLEPLAYER_H__
#define __EXAMPLEPLAYER_H__

#include <iostream>
#include "common.h"
#include "board.h"
using namespace std;

class ExamplePlayer {

public:
    ExamplePlayer(Side side);
    ~ExamplePlayer();
    
    Move *doMove(Move *opponentsMove, int msLeft);
};

#endif
