#ifndef __EVAL_H__
#define __EVAL_H__

#include <string>
#include "board.h"

const int EVAL_SCALE_FACTOR = 1100;
const int N_OCC = 8 + 4*10 + 2;

struct Eval {
  // All pattern indexes for the pattern value table, including offsets.
  int patterns[N_OCC];

  // Update patterns based on a move and flipped bitmask.
  void update(Color c, int m, uint64_t flipped);
  void undo(Color c, int m, uint64_t flipped);
};

void init_eval();
void init_evaluator(Board& b, Eval* e);

int heuristic(Board &b, Eval* e, Color c);
int stability(Board &b, Color c);
int score_game_end(Board& b, Color c);

#endif
