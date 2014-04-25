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
    int mySide;

    Endgame();
    ~Endgame();

    int endgame(Board &b, MoveList &moves, int depth);
    int endgame_h(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);
    int endgame10(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);
    int endgame5(Board &b, int s, int depth, int alpha, int beta,
            bool passedLast);

    int result_solve(Board &b, MoveList &moves, int depth);
    int rs_h(Board &b, int s, int depth, int alpha, int beta);
};

#endif
