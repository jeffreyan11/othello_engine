#ifndef __ENDGAME_H__
#define __ENDGAME_H__

#include <chrono>
#include "common.h"
#include "board.h"
#include "hash.h"

#define END_SHLLW 10

class Endgame {

private:
    Hash endgame_table;

    int bitScanForward(bitbrd bb);

public:
    int endgameTimeMS;
    int mySide;

    Endgame();
    ~Endgame();

    int endgame(Board &b, MoveList &moves, int depth);
    int endgame_h(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);
    int endgame_shallow(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);
    int endgame4(Board &b, int s, int alpha, int beta, bool passedLast);
    int endgame3(Board &b, int s, int alpha, int beta, bool passedLast);
    int endgame2(Board &b, int s, int alpha, int beta, bool passedLast);
    int endgame1(Board &b, int s, int alpha);
};

#endif
