#ifndef __ENDHASH_H__
#define __ENDHASH_H__

#include "board.h"
#include "common.h"

struct EndgameEntry {
  uint64_t white, black;
  uint8_t color;
  int8_t score;
  uint8_t move;
  uint8_t depth;

  EndgameEntry() {
    set_entry(0, 0, WHITE, 0, 0, 0);
  }
  ~EndgameEntry() = default;

  void set_entry(uint64_t w, uint64_t b, Color c, int s, int m, int d) {
    white = w;
    black = b;
    color = (uint8_t) c;
    score = (int8_t) s;
    move = (uint8_t) m;
    depth = (uint8_t) d;
  }
};

class EndHash {
 public:
  // Creates a endgame hashtable, with argument in number of bits for the bitmask
  // The table will have 2^bits entries
  EndHash(uint32_t bits);
  ~EndHash();
  EndHash(const EndHash &other) = delete;
  EndHash& operator=(const EndHash &other) = delete;

  // Adds key (board, color) and item move into the hashtable.
  // Assumes that this key has been checked with get() and is not in the table.
  void add(Board &b, Color c, int score, int move, int depth);
  EndgameEntry *get(Board &b, Color c);
  int hash_full();

  void resize(uint32_t bits);
  void clear();

 private:
  EndgameEntry *table;
  uint32_t size;
};

#endif
