#ifndef __ENDGAME_H__
#define __ENDGAME_H__

#include <chrono>
#include "common.h"
#include "board.h"
#include "hash.h"

int endgame(Board &b, vector<int> &moves, Side s, int depth,
    int alpha, int beta, int endgameTimeMS, Hash &endgame_table);
int endgame_h(Board &b, Side s, Side mine, int depth, int alpha, int beta,
    Hash &endgame_table);

#endif
