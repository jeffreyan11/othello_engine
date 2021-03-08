#include "hash.h"

#include <cstring>

Hash::Hash(uint32_t bits) {
  if (bits < 10) bits = 10;
  size = 1 << bits;
  table = new HashNode[size];
}

Hash::~Hash() {
  delete[] table;
}

void Hash::add(Board &b, Color c, int score, int selectivity, int move, uint8_t turn, int depth, uint8_t node_type) {
  uint32_t index = b.hash() & (size-1);
  HashNode *node = &(table[index]);
  if (node->entry1.taken == 0) {
    node->entry1.setData(b.occupied(), b.get_bits(BLACK), c, score, selectivity, move, turn, depth, node_type);
    return;
  }
  if (node->entry2.taken == 0) {
    node->entry2.setData(b.occupied(), b.get_bits(BLACK), c, score, selectivity, move, turn, depth, node_type);
    return;
  }
  // Always update the same position with newer information
  if (node->entry1.taken == b.occupied()
   && node->entry1.black == b.get_bits(BLACK)
   && node->entry1.color == (uint8_t) c) {
    node->entry1.setData(b.occupied(), b.get_bits(BLACK), c, score, selectivity, move, turn, depth, node_type);
  } else if (node->entry2.taken == b.occupied()
      && node->entry2.black == b.get_bits(BLACK)
      && node->entry2.color == (uint8_t) c) {
    node->entry2.setData(b.occupied(), b.get_bits(BLACK), c, score, selectivity, move, turn, depth, node_type);
  } else {
    HashEntry *to_replace = nullptr;
    // Prioritize entries with a higher depth, but also from a more
    // recent search space
    int score1 = 8 * (turn - node->entry1.turn) + 2 * (selectivity - node->entry1.selectivity) + depth - node->entry1.depth;
    int score2 = 8 * (turn - node->entry2.turn) + 2 * (selectivity - node->entry2.selectivity) + depth - node->entry2.depth;
    if (score1 >= score2) {
      to_replace = &(node->entry1);
    } else {
      to_replace = &(node->entry2);
    }
    if (score1 <= 0 && score2 <= 0) {
      to_replace = nullptr;
    }

    if (to_replace != nullptr) {
      to_replace->setData(b.occupied(), b.get_bits(BLACK), c, score, selectivity, move, turn, depth, node_type);
    }
  }
}

HashEntry *Hash::get(Board &b, Color c) {
  uint32_t index = b.hash() & (size-1);
  HashNode *node = &(table[index]);

  if (node->entry1.taken == b.occupied()
   && node->entry1.black == b.get_bits(BLACK)
   && node->entry1.color == (uint8_t) c) {
    return &(node->entry1);
  }

  if (node->entry2.taken == b.occupied()
   && node->entry2.black == b.get_bits(BLACK)
   && node->entry2.color == (uint8_t) c) {
    return &(node->entry2);
  }

  return nullptr;
}

int Hash::hash_full() {
  int used = 0;
  for (int i = 0; i < 500; i++) {
    used += (table[i].entry1.taken) != 0;
    used += (table[i].entry2.taken) != 0;
  }
  return used;
}

void Hash::resize(uint32_t bits) {
  if (bits < 10) bits = 10;
  delete[] table;
  size = 1 << bits;
  table = new HashNode[size];
}

void Hash::clear() {
  std::memset(static_cast<void*>(table), 0, size * sizeof(HashNode));
}
