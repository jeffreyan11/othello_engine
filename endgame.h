#ifndef __ENDGAME_H__
#define __ENDGAME_H__

#include <chrono>
#include "common.h"
#include "board.h"
#include "hash.h"

class Endgame {

private:
    Hash endgame_table;

public:
    int endgameTimeMS;
    Side mySide;

    Endgame();
    ~Endgame();

    int endgame(Board &b, vector<int> &moves, int depth);
    int endgame_h(Board &b, Side s, int depth, int alpha, int beta);
};

#endif
