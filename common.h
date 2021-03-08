#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <string>

#define PRINT_SEARCH_INFO false

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::high_resolution_clock::time_point;

const int INFTY = (1 << 30);
const int MOVE_NULL = 64;
const int MOVE_BROKEN = -2;
const int OPENING_NOT_FOUND = -3;
const uint8_t PV_NODE = 0;
const uint8_t CUT_NODE = 1;
const uint8_t ALL_NODE = 2;

enum Color {
  WHITE, BLACK
};

constexpr Color operator~(Color c) { return Color(c ^ 1); }

constexpr int move_row(int m) { return m >> 3; }
constexpr int move_col(int m) { return m & 7; }
constexpr int move_xy(int x, int y) { return 8 * y + x; }

// Bitboard functions
int count_bits(uint64_t i);
int bitscan_forward(uint64_t i);
int bitscan_reverse(uint64_t i);
uint64_t reflect_vert(uint64_t i);
uint64_t reflect_hor(uint64_t i);
uint64_t reflect_diag(uint64_t i);

// Utility functions
uint64_t get_time_elapsed(TimePoint start_time);
std::string print_move(int move);

class ArrayList {
 public:
  int data[32];
  int length;

  ArrayList() {
    clear();
  }
  ~ArrayList() = default;

  void add(int m) {
    data[length] = m;
    length++;
  }
  void remove(int i) {
    length--;
    data[i] = data[length];
  }

  int get(int i) { return data[i]; }
  void set(int i, int val) { data[i] = val; }
  int last() { return data[length-1]; }
  void clear() { length = 0; }
  int size() { return length; }

  void swap(int i, int j) {
    int temp = data[i];
    data[i] = data[j];
    data[j] = temp;
  }
};

// Retrieves the next move with the highest score starting from given index
// using a partial selection sort.
int next_move(ArrayList &moves, ArrayList &scores, int index);

#endif
