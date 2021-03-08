#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "board.h"
#include "common.h"
#include "eval.h"
#include "hash.h"

const int TIMEOUT = (1 << 20);
const int NO_SELECTIVITY = 5;

struct SearchInfo {
  uint64_t nodes;
  // Aging for transposition table replacement strategy
  int root_age;
  int selectivity;
  uint64_t time_limit;
  Hash* tt;
  bool other_heuristic;
  TimePoint search_start;

  SearchInfo()
    : nodes(0),
      root_age(0),
      selectivity(1),
      time_limit(0),
      tt(nullptr),
      other_heuristic(false) {}
};

// Helper function for the principal variation search.
// Uses alpha-beta pruning with a null-window search, a transposition table that
// stores moves from at least depth 4, and internal iterative deepening,
// fastest first, and a piece-square table for move ordering.
int pvs(Board &b, Eval* e, Color c, int depth, int alpha, int beta, bool passed_last, SearchInfo* search_info);

// Performs a principal variation null-window search. Returns the index of the best move.
int pvs_best_move(Board &b, Eval* e, Color c, ArrayList &moves, int* best_score, int depth, SearchInfo* search_info);

#endif
