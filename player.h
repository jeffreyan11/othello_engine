#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "board.h"
#include "common.h"
#include "endgame.h"
#include "hash.h"
#include "openings.h"
#include "search.h"

class Player {
 public:
  Board game;
  Color mySide;
  bool otherHeuristic;
  Hash *transpositionTable;
  int turn;
  int baseSelectivity;
  int bufferPerMove;

  Player(Color side, bool use_book, int tt_bits);
  ~Player();

  // Processes opponent's last move and selects a best move to play.
  int do_move(int opponents_move, int ms_left);
  void set_depths(int max, int end);
  uint64_t get_nodes();
  void set_position(uint64_t taken_bits, uint64_t black_bits);

 private:
  int maxDepth;
  int endgameDepth;
  // Always use the endgame solver with this many empty squares left, since it is faster.
  int forceEgDepth;
  // The last max depth achieved, for entering endgame solver
  int lastMaxDepth;

  uint64_t nodes;

  Endgame endgameSolver;

  Openings *openingBook;
  bool bookExhausted;

  int timeLimit;
  TimePoint timeElapsed;
};

#endif
