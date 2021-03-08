#ifndef __HASH_H__
#define __HASH_H__

#include "board.h"
#include "common.h"

struct HashEntry {
  uint64_t taken;
  uint64_t black;
  int score;
  uint8_t selectivity;
  uint8_t color;
  uint8_t move;
  uint8_t turn;
  uint8_t depth;
  uint8_t nodeType;

  HashEntry() {
    setData(0, 0, WHITE, 0, 0, 0, 0, 0, 0);
  }
  HashEntry(uint64_t t, uint64_t b, Color c, int s, int sel, int m, uint8_t tu, int d, uint8_t nt) {
    setData(t, b, c, s, sel, m, tu, d, nt);
  }
  ~HashEntry() = default;

  void setData(uint64_t t, uint64_t b, Color c, int s, int sel, int m, uint8_t tu, int d,
    uint8_t nt) {
    taken = t;
    black = b;
    score = s;
    selectivity = sel;
    color = (uint8_t) c;
    move = (uint8_t) m;
    turn = tu;
    depth = (uint8_t) d;
    nodeType = nt;
  }
};

class HashNode {
 public:
  HashEntry entry1, entry2;

  HashNode() = default;
  HashNode(uint64_t t, uint64_t b, Color c, int s, int sel, int m, uint8_t tu, int d, uint8_t nt) {
    entry1 = HashEntry(t, b, c, s, sel, m, tu, d, nt);
  }
  ~HashNode() = default;
};

class Hash {
 public:
  // Creates a hashtable, with argument in number of bits for the bitmask
  // The table will have 2^bits entries
  Hash(uint32_t bits);
  ~Hash();

  // Adds a hash entry into the table.
  // Assumes that this key has been checked with get() and is not in the table.
  void add(Board &b, Color c, int score, int selectivity, int move, uint8_t turn, int depth, uint8_t node_type);
  // Get the move, if any, associated with a board b and player color c.
  HashEntry *get(Board &b, Color c);
  int hash_full();

  void resize(uint32_t bits);
  void clear();

 private:
  HashNode *table;
  uint32_t size;

  Hash(const Hash &other);
  Hash& operator=(const Hash &other);
};

#endif
