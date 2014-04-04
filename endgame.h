#ifndef __ENDGAME_H__
#define __ENDGAME_H__

#include <chrono>
#include <unordered_map>
#include "common.h"
#include "board.h"

int endgame(Board &b, vector<int> &moves, Side s, int depth,
    int alpha, int beta, int endgameTimeMS, unordered_map<Board, int, BoardHashFunc> &endgame_table);
int endgame_h(Board &b, Side s, Side mine, int depth, int alpha, int beta,
    unordered_map<Board, int, BoardHashFunc> &endgame_table);

#endif
