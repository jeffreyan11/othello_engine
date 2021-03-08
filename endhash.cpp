#include "endhash.h"

#include <cstring>

EndHash::EndHash(uint32_t bits) {
  if (bits < 10) bits = 10;
  size = 1 << bits;
  table = new EndgameEntry[size];
}

EndHash::~EndHash() {
  delete[] table;
}

void EndHash::add(Board &b, Color c, int score, int move, int depth) {
  uint32_t index = b.hash() & (size-1);
  EndgameEntry *node = &(table[index]);
  // Replacement strategy
  if (depth + 2 >= node->depth) {
    node->set_entry(b.get_bits(WHITE), b.get_bits(BLACK), c, score, move, depth);
  }
}

// Get the move, if any, associated with a board b and player to move.
EndgameEntry *EndHash::get(Board &b, Color c) {
  uint32_t index = b.hash() & (size-1);
  EndgameEntry *node = &(table[index]);

  if (node->white == b.get_bits(WHITE)
   && node->black == b.get_bits(BLACK)
   && node->color == (uint8_t) c) {
    return node;
  }

  return nullptr;
}

int EndHash::hash_full() {
  int used = 0;
  for (int i = 0; i < 1000; i++) {
    used += table[i].depth > 0;
  }
  return used;
}

void EndHash::resize(uint32_t bits) {
  if (bits < 10) bits = 10;
  delete[] table;
  size = 1 << bits;
  table = new EndgameEntry[size];
}

void EndHash::clear() {
  std::memset(static_cast<void*>(table), 0, size * sizeof(EndgameEntry));
}
