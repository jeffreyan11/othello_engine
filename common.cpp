#include "common.h"

#include <iostream>

int count_bits(uint64_t i) {
  return __builtin_popcountll(i);
}

int bitscan_forward(uint64_t i) {
  return __builtin_ctzll(i);
}

int bitscan_reverse(uint64_t i) {
  return __builtin_clzll(i) ^ 63;
}

uint64_t reflect_vert(uint64_t i) {
  return __builtin_bswap64(i);
}

// Reflection algorithms from
// https://chessprogramming.wikispaces.com/Flipping+Mirroring+and+Rotating
uint64_t reflect_hor(uint64_t i) {
  const uint64_t k1 = 0x5555555555555555;
  const uint64_t k2 = 0x3333333333333333;
  const uint64_t k4 = 0x0f0f0f0f0f0f0f0f;
  i = ((i >> 1) & k1) | ((i & k1) << 1);
  i = ((i >> 2) & k2) | ((i & k2) << 2);
  i = ((i >> 4) & k4) | ((i & k4) << 4);
  return i;
}

uint64_t reflect_diag(uint64_t i) {
  uint64_t t;
  const uint64_t k1 = 0x5500550055005500;
  const uint64_t k2 = 0x3333000033330000;
  const uint64_t k4 = 0x0f0f0f0f00000000;
  t  = k4 & (i ^ (i << 28));
  i ^=       t ^ (t >> 28) ;
  t  = k2 & (i ^ (i << 14));
  i ^=       t ^ (t >> 14) ;
  t  = k1 & (i ^ (i <<  7));
  i ^=       t ^ (t >>  7) ;
  return i;
}

uint64_t get_time_elapsed(TimePoint start_time) {
  auto end_time = Clock::now();
  std::chrono::milliseconds time_span =
      std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time);
  return (uint64_t) time_span.count() + 1;
}

std::string print_move(int move) {
  int row = (move >> 3) + 1;
  char column = (char) ('a' + (move & 7));
  return std::string(1, column) + std::to_string(row);
}

int next_move(ArrayList &moves, ArrayList &scores, int index) {
  if (index >= moves.size())
    return MOVE_NULL;
  // Find the index of the next best move/score
  int best_index = index;
  for (int i = index + 1; i < moves.size(); i++) {
    if (scores.get(i) > scores.get(best_index))
      best_index = i;
  }
  // swap to the correct position
  moves.swap(best_index, index);
  scores.swap(best_index, index);
  // return the move
  return moves.get(index);
}
