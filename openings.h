#ifndef __OPENINGS_H__
#define __OPENINGS_H__

#include "common.h"

struct Node {
  uint64_t taken, black;
  int move;
};

class Openings {
 public:
  Openings();
  ~Openings();

  int get(uint64_t taken, uint64_t black);

 private:
  Node *openings;
  int bookSize;

  int binary_search(uint64_t taken, uint64_t black);
  bool read_file();
};

#endif
