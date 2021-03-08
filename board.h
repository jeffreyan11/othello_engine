#ifndef __BOARD_H__
#define __BOARD_H__

#include <string>
#include "common.h"

const int SQ_VAL[64] = {
  9, 1, 7, 6, 6, 7, 1, 9,
  1, 0, 2, 3, 3, 2, 0, 1,
  7, 2, 5, 4, 4, 5, 2, 7,
  6, 3, 4, 0, 0, 4, 3, 6,
  6, 3, 4, 0, 0, 4, 3, 6,
  7, 2, 5, 4, 4, 5, 2, 7,
  1, 0, 2, 3, 3, 2, 0, 1,
  9, 1, 7, 6, 6, 7, 1, 9
};

class Board {
 public:
  // Default constructor initializes to the starting position.
  Board();
  Board(uint64_t w, uint64_t b);
  // Sets the board state given an 8x8 char array where 'w' or 'O' indicates a
  // white piece and 'b' or 'X' indicates a black piece.
  Board(char data[]);
  Board(const std::string& data);
  ~Board() = default;
  Board copy();

  // Returns a simple on-the-fly Zobrist hash.
  uint32_t hash();

  // Do and undo move
  void do_move(Color c, int m);
  void do_move(Color c, int m, uint64_t mask);
  uint64_t get_do_move(Color c, int m);
  void undo_move(Color c, int m, uint64_t mask);

  // Legal moves
  bool is_legal(Color c, int m);
  ArrayList legal_movelist(Color c);
  uint64_t legal_moves(Color c);

  // Board heuristics
  int count_legal_moves(Color c);
  // Returns the potential mobility (frontier squares) of the given player,
  // defined as the number of empty squares adjacent to the opponent's pieces.
  int count_potential_mobility(Color c);
  // Gets an estimate of the number of stable discs of the given player.
  // Currently overestimates and needs to be fixed.
  int count_stability(Color c);

  // Utility functions
  uint64_t get_bits(Color c);
  uint64_t occupied();
  int count(Color c);
  int count_empty();
  std::string to_string();

 private:
  uint64_t pieces[2];

  // 16 x 256 zobrist table array
  static uint32_t **zobristTable;
  static uint32_t **init_zobrist_table();

  uint64_t north_fill(int m, uint64_t self, uint64_t pos);
  uint64_t south_fill(int m, uint64_t self, uint64_t pos);
  uint64_t east_fill(int m, uint64_t self, uint64_t pos);
  uint64_t west_fill(int m, uint64_t self, uint64_t pos);
  uint64_t ne_fill(int m, uint64_t self, uint64_t pos);
  uint64_t nw_fill(int m, uint64_t self, uint64_t pos);
  uint64_t sw_fill(int m, uint64_t self, uint64_t pos);
  uint64_t se_fill(int m, uint64_t self, uint64_t pos);
};

#endif
