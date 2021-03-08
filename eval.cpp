#include "eval.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include "bbinit.h"

namespace {

const int N_SPLITS = 31;
inline int pv_split_index(int empty) {
  return (60 - empty) / 2;
}

// Converts the packed bits for one side into an index for the pattern values
// array.
int PIECES_TO_INDEX[1024];

const int N_PATTERNS = 12;
const int N_WEIGHTS = 4*59049 + 19683 + 4*6561 + 2187 + 729 + 243;
enum PatternType {
  P25, E2X, TRAPZ, P33, PM, LINE2, LINE3, LINE4, DIAG8, DIAG7, DIAG6, DIAG5
};

struct SquareToPatterns {
  int ct;
  struct {
    int p, i;
  } patterns[9];
};

const SquareToPatterns squareToPatterns[65] = {
  {7, {{1, 0}, {1, 4}, {1, 8}, {1, 10}, {1, 16}, {1, 20}, {1, 36}}},  // A1
  {8, {{3, 0}, {243, 4}, {3, 8}, {1, 12}, {3, 16}, {3, 20}, {1, 26}, {1, 38}}},
  {7, {{9, 0}, {9, 8}, {3, 12}, {9, 16}, {9, 20}, {1, 30}, {1, 42}}},
  {8, {{27, 0}, {81, 2}, {27, 8}, {9, 12}, {27, 20}, {81, 22}, {1, 34}, {1, 46}}},
  {8, {{81, 0}, {27, 2}, {81, 8}, {27, 12}, {81, 20}, {27, 22}, {1, 35}, {81, 48}}},
  {7, {{9, 2}, {243, 8}, {81, 12}, {9, 18}, {9, 22}, {1, 31}, {243, 44}}},
  {8, {{3, 2}, {243, 5}, {729, 8}, {243, 12}, {3, 18}, {3, 22}, {1, 27}, {729, 40}}},
  {7, {{1, 2}, {1, 5}, {2187, 8}, {1, 11}, {1, 18}, {1, 22}, {2187, 37}}},  // A8
  {8, {{243, 0}, {3, 4}, {3, 10}, {1, 14}, {27, 16}, {243, 20}, {1, 24}, {1, 39}}},  // B1
  {9, {{729, 0}, {729, 4}, {6561, 8}, {6561, 10}, {81, 16}, {729, 20}, {3, 24}, {3, 26}, {3, 36}}},
  {6, {{2187, 0}, {729, 12}, {243, 16}, {9, 24}, {3, 30}, {3, 38}}},
  {7, {{6561, 0}, {19683, 2}, {2187, 12}, {27, 24}, {3, 34}, {3, 42}, {27, 48}}},
  {7, {{19683, 0}, {6561, 2}, {6561, 12}, {81, 24}, {3, 35}, {81, 44}, {3, 46}}},
  {6, {{2187, 2}, {19683, 12}, {243, 18}, {243, 24}, {3, 31}, {243, 40}}},
  {9, {{729, 2}, {729, 5}, {19683, 8}, {6561, 11}, {81, 18}, {729, 22}, {729, 24}, {3, 27}, {729, 37}}},
  {8, {{243, 2}, {3, 5}, {3, 11}, {1, 15}, {27, 18}, {243, 22}, {2187, 24}, {729, 41}}},  // B8
  {7, {{9, 4}, {9, 10}, {3, 14}, {729, 16}, {2187, 20}, {1, 28}, {1, 43}}},
  {6, {{2187, 4}, {729, 14}, {2187, 16}, {9, 26}, {3, 28}, {3, 39}}},
  {5, {{6561, 16}, {9, 28}, {9, 30}, {9, 36}, {9, 48}}},
  {4, {{27, 28}, {9, 34}, {9, 38}, {27, 44}}},
  {4, {{81, 28}, {9, 35}, {81, 40}, {9, 42}}},
  {5, {{6561, 18}, {243, 28}, {9, 31}, {243, 37}, {9, 46}}},
  {6, {{2187, 5}, {729, 15}, {2187, 18}, {9, 27}, {729, 28}, {243, 41}}},
  {7, {{9, 5}, {9, 11}, {3, 15}, {729, 18}, {2187, 22}, {2187, 28}, {243, 45}}},
  {8, {{27, 4}, {81, 6}, {27, 10}, {9, 14}, {6561, 20}, {19683, 21}, {1, 32}, {1, 47}}},
  {7, {{6561, 4}, {19683, 6}, {2187, 14}, {27, 26}, {3, 32}, {3, 43}, {3, 48}}},
  {4, {{27, 30}, {9, 32}, {9, 39}, {9, 44}}},
  {4, {{27, 32}, {27, 34}, {27, 36}, {27, 40}}},
  {4, {{81, 32}, {27, 35}, {81, 37}, {27, 38}}},
  {4, {{27, 31}, {243, 32}, {81, 41}, {27, 42}}},
  {7, {{6561, 5}, {19683, 7}, {2187, 15}, {27, 27}, {729, 32}, {81, 45}, {27, 46}}},
  {8, {{27, 5}, {81, 7}, {27, 11}, {9, 15}, {6561, 22}, {19683, 23}, {2187, 32}, {81, 49}}},
  {8, {{81, 4}, {27, 6}, {81, 10}, {27, 14}, {19683, 20}, {6561, 21}, {1, 33}, {1, 48}}},
  {7, {{19683, 4}, {6561, 6}, {6561, 14}, {81, 26}, {3, 33}, {3, 44}, {3, 47}}},
  {4, {{81, 30}, {9, 33}, {9, 40}, {9, 43}}},
  {4, {{27, 33}, {81, 34}, {27, 37}, {27, 39}}},
  {4, {{81, 33}, {81, 35}, {81, 36}, {27, 41}}},
  {4, {{81, 31}, {243, 33}, {81, 38}, {27, 45}}},
  {7, {{19683, 5}, {6561, 7}, {6561, 15}, {81, 27}, {729, 33}, {81, 42}, {27, 49}}},
  {8, {{81, 5}, {27, 7}, {81, 11}, {27, 15}, {19683, 22}, {6561, 23}, {2187, 33}, {81, 46}}},
  {7, {{9, 6}, {243, 10}, {81, 14}, {729, 17}, {2187, 21}, {1, 29}, {1, 44}}},
  {6, {{2187, 6}, {19683, 14}, {2187, 17}, {243, 26}, {3, 29}, {3, 40}}},
  {5, {{6561, 17}, {9, 29}, {243, 30}, {9, 37}, {9, 47}}},
  {4, {{27, 29}, {243, 34}, {9, 41}, {27, 43}}},
  {4, {{81, 29}, {243, 35}, {81, 39}, {9, 45}}},
  {5, {{6561, 19}, {243, 29}, {243, 31}, {243, 36}, {9, 49}}},
  {6, {{2187, 7}, {19683, 15}, {2187, 19}, {243, 27}, {729, 29}, {243, 38}}},
  {7, {{9, 7}, {243, 11}, {81, 15}, {729, 19}, {2187, 23}, {2187, 29}, {243, 42}}},
  {8, {{243, 1}, {3, 6}, {729, 10}, {243, 14}, {27, 17}, {243, 21}, {1, 25}, {1, 40}}},  // G1
  {9, {{729, 1}, {729, 6}, {6561, 9}, {19683, 10}, {81, 17}, {729, 21}, {3, 25}, {729, 26}, {3, 37}}},
  {6, {{2187, 1}, {729, 13}, {243, 17}, {9, 25}, {729, 30}, {3, 41}}},
  {7, {{6561, 1}, {19683, 3}, {2187, 13}, {27, 25}, {729, 34}, {3, 45}, {27, 47}}},
  {7, {{19683, 1}, {6561, 3}, {6561, 13}, {81, 25}, {729, 35}, {81, 43}, {3, 49}}},
  {6, {{2187, 3}, {19683, 13}, {243, 19}, {243, 25}, {729, 31}, {243, 39}}},
  {9, {{729, 3}, {729, 7}, {19683, 9}, {19683, 11}, {81, 19}, {729, 23}, {729, 25}, {729, 27}, {729, 36}}},
  {8, {{243, 3}, {3, 7}, {729, 11}, {243, 15}, {27, 19}, {243, 23}, {2187, 25}, {729, 38}}},  // G8
  {7, {{1, 1}, {1, 6}, {1, 9}, {2187, 10}, {1, 17}, {1, 21}, {1, 37}}},  // H1
  {8, {{3, 1}, {243, 6}, {3, 9}, {1, 13}, {3, 17}, {3, 21}, {2187, 26}, {1, 41}}},
  {7, {{9, 1}, {9, 9}, {3, 13}, {9, 17}, {9, 21}, {2187, 30}, {1, 45}}},
  {8, {{27, 1}, {81, 3}, {27, 9}, {9, 13}, {27, 21}, {81, 23}, {2187, 34}, {1, 49}}},
  {8, {{81, 1}, {27, 3}, {81, 9}, {27, 13}, {81, 21}, {27, 23}, {2187, 35}, {81, 47}}},
  {7, {{9, 3}, {243, 9}, {81, 13}, {9, 19}, {9, 23}, {2187, 31}, {243, 43}}},
  {8, {{3, 3}, {243, 7}, {729, 9}, {243, 13}, {3, 19}, {3, 23}, {2187, 27}, {729, 39}}},
  {7, {{1, 3}, {1, 7}, {2187, 9}, {2187, 11}, {1, 19}, {1, 23}, {2187, 36}}},  // H8
  {0, {}}
};

struct PatternInfo {
  int offset;
  int size;
  int offset2;
  int ct;
  PatternInfo(int o, int s, int o2, int c) {
    offset = o;
    size = s;
    offset2 = o2;
    ct = c;
  }
};

const PatternInfo patternInfo[N_PATTERNS] = {
  //offset  size  o2  ct
  {     0, 59049,  0, 8},  // corner 2x5
  { 59049, 59049,  8, 4},  // edge + 2x
  {118098, 59049, 12, 4},  // edge trapezoid
  {177147, 19683, 16, 4},  // corner 3x3
  {196830, 59049, 20, 4},  // corner M
  {255879,  6561, 24, 4},  // line 2
  {262440,  6561, 28, 4},  // line 3
  {269001,  6561, 32, 4},  // line 4
  {275562,  6561, 36, 2},  // diag 8
  {282123,  2187, 38, 4},  // diag 7
  {284310,   729, 42, 4},  // diag 6
  {285039,   243, 46, 4},  // diag 5
};

// Pattern value tables
int16_t **pvTable;
int16_t *s44Table;

// Reinterprets the given number n, written in its binary form, as a ternary
// number. For example, the input n=11 would give
// 1011 = 1 + 3 + 27 = 31
int binary_to_ternary(unsigned int n) {
  int result = 0;
  int ternary_place = 1;
  while (n) {
    result += ternary_place * (n & 1);
    n >>= 1;
    ternary_place *= 3;
  }
  return result;
}

void read_pattern_weights(std::string dir) {
  std::string line;
  std::string file_name = dir + "flippy_weights.txt";
  std::ifstream eval_table(file_name, std::ios_base::binary);

  if (eval_table.is_open()) {
    // int16_t* raw = new int16_t[31*N_WEIGHTS];

    for (int n = 0; n < N_SPLITS; n++) {
      eval_table.read(reinterpret_cast<char*>(pvTable[n]), N_WEIGHTS*sizeof(int16_t));
    }

    // Transform table to patterns per ply
    // for (int i = 0; i < N_SPLITS; i++) {
    //   for (int j = 0; j < N_PATTERNS; j++) {
    //     const PatternInfo& pi = patternInfo[j];
    //     for (int k = 0; k < pi.size; k++) {
    //       pvTable[i][pi.offset+k] = raw[(i/2)*N_WEIGHTS + pi.offset + k];
    //     }
    //   }
    // }

    // delete[] raw;
    eval_table.close();
  } else {
    std::cerr << "Error: could not open " << file_name << std::endl;
  }
}

void read_stability_table(std::string file_name, int16_t *table_array) {
  std::string line;
  std::ifstream eval_table(file_name);

  if (eval_table.is_open()) {
    for (int i = 0; i < 2048; i++) {
      getline(eval_table, line);
      for (int j = 0; j < 32; j++) {
        std::string::size_type sz = 0;
        table_array[32*i+j] = static_cast<int16_t>(std::stoi(line, &sz, 0));
        line = line.substr(sz);
      }
    }
    eval_table.close();
  } else {
    std::cerr << "Error: could not open " << file_name << std::endl;
  }
}

int pattern_index(int b, int w) {
  return PIECES_TO_INDEX[b] + 2*PIECES_TO_INDEX[w];
}

void eval_25(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int ulb = (int) ((black&0x1F) + ((black>>3)&0x3E0));
  int ulw = (int) ((white&0x1F) + ((white>>3)&0x3E0));
  int ul = pattern_index(ulb, ulw);

  uint64_t rvb = reflect_vert(black);
  uint64_t rvw = reflect_vert(white);
  int llb = (int) ((rvb&0x1F) + ((rvb>>3)&0x3E0));
  int llw = (int) ((rvw&0x1F) + ((rvw>>3)&0x3E0));
  int ll = pattern_index(llb, llw);

  uint64_t rhb = reflect_hor(black);
  uint64_t rhw = reflect_hor(white);
  int urb = (int) ((rhb&0x1F) + ((rhb>>3)&0x3E0));
  int urw = (int) ((rhw&0x1F) + ((rhw>>3)&0x3E0));
  int ur = pattern_index(urb, urw);

  uint64_t rbb = reflect_vert(rhb);
  uint64_t rbw = reflect_vert(rhw);
  int lrb = (int) ((rbb&0x1F) + ((rbb>>3)&0x3E0));
  int lrw = (int) ((rbw&0x1F) + ((rbw>>3)&0x3E0));
  int lr = pattern_index(lrb, lrw);

  uint64_t rotb = reflect_diag(black);
  uint64_t rotw = reflect_diag(white);
  int rulb = (int) ((rotb&0x1F) + ((rotb>>3)&0x3E0));
  int rulw = (int) ((rotw&0x1F) + ((rotw>>3)&0x3E0));
  int rul = pattern_index(rulb, rulw);

  uint64_t rotvb = reflect_vert(rotb);
  uint64_t rotvw = reflect_vert(rotw);
  int rllb = (int) ((rotvb&0x1F) + ((rotvb>>3)&0x3E0));
  int rllw = (int) ((rotvw&0x1F) + ((rotvw>>3)&0x3E0));
  int rll = pattern_index(rllb, rllw);

  uint64_t rothb = reflect_hor(rotb);
  uint64_t rothw = reflect_hor(rotw);
  int rurb = (int) ((rothb&0x1F) + ((rothb>>3)&0x3E0));
  int rurw = (int) ((rothw&0x1F) + ((rothw>>3)&0x3E0));
  int rur = pattern_index(rurb, rurw);

  uint64_t rotbb = reflect_vert(rothb);
  uint64_t rotbw = reflect_vert(rothw);
  int rlrb = (int) ((rotbb&0x1F) + ((rotbb>>3)&0x3E0));
  int rlrw = (int) ((rotbw&0x1F) + ((rotbw>>3)&0x3E0));
  int rlr = pattern_index(rlrb, rlrw);

  const PatternInfo& pi = patternInfo[P25];
  e->patterns[0] = pi.offset + ul;
  e->patterns[1] = pi.offset + ll;
  e->patterns[2] = pi.offset + ur;
  e->patterns[3] = pi.offset + lr;
  e->patterns[4] = pi.offset + rul;
  e->patterns[5] = pi.offset + rll;
  e->patterns[6] = pi.offset + rur;
  e->patterns[7] = pi.offset + rlr;
}

void eval_e2x(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int r1b = (int) ( (black & 0xFF) +
    ((black & 0x200) >> 1) + ((black & 0x4000) >> 5) );
  int r1w = (int) ( (white & 0xFF) +
    ((white & 0x200) >> 1) + ((white & 0x4000) >> 5) );
  int r1 = pattern_index(r1b, r1w);

  int r8b = (int) ( (black>>56) + ((black & 0x2000000000000) >> 41) +
    ((black & 0x40000000000000) >> 45) );
  int r8w = (int) ( (white>>56) + ((white & 0x2000000000000) >> 41) +
    ((white & 0x40000000000000) >> 45) );
  int r8 = pattern_index(r8b, r8w);

  int c1b = (int) (
    (((black & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56) + 
    ((black & 0x200) >> 1) + ((black & 0x2000000000000) >> 40) );
  int c1w = (int) (
    (((white & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56) +
    ((white & 0x200) >> 1) + ((white & 0x2000000000000) >> 40) );
  int c1 = pattern_index(c1b, c1w);

  int c8b = (int) (
    (((black & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56) +
    ((black & 0x4000) >> 6) + ((black & 0x40000000000000) >> 45) );
  int c8w = (int) (
    (((white & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56) +
    ((white & 0x4000) >> 6) + ((white & 0x40000000000000) >> 45) );
  int c8 = pattern_index(c8b, c8w);

  const PatternInfo& pi = patternInfo[E2X];
  e->patterns[8] = pi.offset + r1;
  e->patterns[9] = pi.offset + r8;
  e->patterns[10] = pi.offset + c1;
  e->patterns[11] = pi.offset + c8;
}

void eval_trapz(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int ulb = (int) (((black>>1)&0x3F) + ((black>>4)&0x3C0));
  int ulw = (int) (((white>>1)&0x3F) + ((white>>4)&0x3C0));
  int ul = pattern_index(ulb, ulw);

  uint64_t rvb = reflect_vert(black);
  uint64_t rvw = reflect_vert(white);
  int llb = (int) (((rvb>>1)&0x3F) + ((rvb>>4)&0x3C0));
  int llw = (int) (((rvw>>1)&0x3F) + ((rvw>>4)&0x3C0));
  int ll = pattern_index(llb, llw);

  uint64_t rdb = reflect_diag(black);
  uint64_t rdw = reflect_diag(white);
  int urb = (int) (((rdb>>1)&0x3F) + ((rdb>>4)&0x3C0));
  int urw = (int) (((rdw>>1)&0x3F) + ((rdw>>4)&0x3C0));
  int ur = pattern_index(urb, urw);

  uint64_t rbb = reflect_vert(rdb);
  uint64_t rbw = reflect_vert(rdw);
  int lrb = (int) (((rbb>>1)&0x3F) + ((rbb>>4)&0x3C0));
  int lrw = (int) (((rbw>>1)&0x3F) + ((rbw>>4)&0x3C0));
  int lr = pattern_index(lrb, lrw);

  const PatternInfo& pi = patternInfo[TRAPZ];
  e->patterns[12] = pi.offset + ul;
  e->patterns[13] = pi.offset + ll;
  e->patterns[14] = pi.offset + ur;
  e->patterns[15] = pi.offset + lr;
}

void eval_33(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int ulb = (int) ((black&7) + ((black>>5)&0x38) + ((black>>10)&0x1C0));
  int ulw = (int) ((white&7) + ((white>>5)&0x38) + ((white>>10)&0x1C0));
  int ul = pattern_index(ulb, ulw);

  uint64_t rvb = reflect_vert(black);
  uint64_t rvw = reflect_vert(white);
  int llb = (int) ((rvb&7) + ((rvb>>5)&0x38) + ((rvb>>10)&0x1C0));
  int llw = (int) ((rvw&7) + ((rvw>>5)&0x38) + ((rvw>>10)&0x1C0));
  int ll = pattern_index(llb, llw);

  uint64_t rhb = reflect_hor(black);
  uint64_t rhw = reflect_hor(white);
  int urb = (int) ((rhb&7) + ((rhb>>5)&0x38) + ((rhb>>10)&0x1C0));
  int urw = (int) ((rhw&7) + ((rhw>>5)&0x38) + ((rhw>>10)&0x1C0));
  int ur = pattern_index(urb, urw);

  uint64_t rbb = reflect_vert(rhb);
  uint64_t rbw = reflect_vert(rhw);
  int lrb = (int) ((rbb&7) + ((rbb>>5)&0x38) + ((rbb>>10)&0x1C0));
  int lrw = (int) ((rbw&7) + ((rbw>>5)&0x38) + ((rbw>>10)&0x1C0));
  int lr = pattern_index(lrb, lrw);

  const PatternInfo& pi = patternInfo[P33];
  e->patterns[16] = pi.offset + ul;
  e->patterns[17] = pi.offset + ll;
  e->patterns[18] = pi.offset + ur;
  e->patterns[19] = pi.offset + lr;
}

void eval_m(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int ulb = (int) ((black & 0x1F) + ((black >> 3) & 0x60) + ((black >> 9) & 0x80) + ((black >> 16) & 0x100) + ((black >> 23) & 0x200));
  int ulw = (int) ((white & 0x1F) + ((white >> 3) & 0x60) + ((white >> 9) & 0x80) + ((white >> 16) & 0x100) + ((white >> 23) & 0x200));
  int ul = pattern_index(ulb, ulw);

  uint64_t rvb = reflect_vert(black);
  uint64_t rvw = reflect_vert(white);
  int llb = (int) ((rvb & 0x1F) + ((rvb >> 3) & 0x60) + ((rvb >> 9) & 0x80) + ((rvb >> 16) & 0x100) + ((rvb >> 23) & 0x200));
  int llw = (int) ((rvw & 0x1F) + ((rvw >> 3) & 0x60) + ((rvw >> 9) & 0x80) + ((rvw >> 16) & 0x100) + ((rvw >> 23) & 0x200));
  int ll = pattern_index(llb, llw);

  uint64_t rhb = reflect_hor(black);
  uint64_t rhw = reflect_hor(white);
  int urb = (int) ((rhb & 0x1F) + ((rhb >> 3) & 0x60) + ((rhb >> 9) & 0x80) + ((rhb >> 16) & 0x100) + ((rhb >> 23) & 0x200));
  int urw = (int) ((rhw & 0x1F) + ((rhw >> 3) & 0x60) + ((rhw >> 9) & 0x80) + ((rhw >> 16) & 0x100) + ((rhw >> 23) & 0x200));
  int ur = pattern_index(urb, urw);

  uint64_t rbb = reflect_vert(rhb);
  uint64_t rbw = reflect_vert(rhw);
  int lrb = (int) ((rbb & 0x1F) + ((rbb >> 3) & 0x60) + ((rbb >> 9) & 0x80) + ((rbb >> 16) & 0x100) + ((rbb >> 23) & 0x200));
  int lrw = (int) ((rbw & 0x1F) + ((rbw >> 3) & 0x60) + ((rbw >> 9) & 0x80) + ((rbw >> 16) & 0x100) + ((rbw >> 23) & 0x200));
  int lr = pattern_index(lrb, lrw);

  const PatternInfo& pi = patternInfo[PM];
  e->patterns[20] = pi.offset + ul;
  e->patterns[21] = pi.offset + ll;
  e->patterns[22] = pi.offset + ur;
  e->patterns[23] = pi.offset + lr;
}

void eval_line2(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int r2 = pattern_index((int)((black >> 8) & 0xFF), (int)((white >> 8) & 0xFF));
  int r7 = pattern_index((int)((black >> 48) & 0xFF), (int)((white >> 48) & 0xFF));
  int c2 = pattern_index((int)((((black>>1) & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56),
                         (int)((((white>>1) & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56));
  int c7 = pattern_index((int)((((black<<1) & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56),
                         (int)((((white<<1) & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56));
  const PatternInfo& pi = patternInfo[LINE2];
  e->patterns[24] = pi.offset + r2;
  e->patterns[25] = pi.offset + r7;
  e->patterns[26] = pi.offset + c2;
  e->patterns[27] = pi.offset + c7;
}

void eval_line3(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int r2 = pattern_index((int)((black >> 16) & 0xFF), (int)((white >> 16) & 0xFF));
  int r7 = pattern_index((int)((black >> 40) & 0xFF), (int)((white >> 40) & 0xFF));
  int c2 = pattern_index((int)((((black>>2) & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56),
                         (int)((((white>>2) & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56));
  int c7 = pattern_index((int)((((black<<2) & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56),
                         (int)((((white<<2) & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56));

  const PatternInfo& pi = patternInfo[LINE3];
  e->patterns[28] = pi.offset + r2;
  e->patterns[29] = pi.offset + r7;
  e->patterns[30] = pi.offset + c2;
  e->patterns[31] = pi.offset + c7;
}

void eval_line4(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int r2 = pattern_index((int)((black >> 24) & 0xFF), (int)((white >> 24) & 0xFF));
  int r7 = pattern_index((int)((black >> 32) & 0xFF), (int)((white >> 32) & 0xFF));
  int c2 = pattern_index((int)((((black>>3) & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56),
                         (int)((((white>>3) & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56));
  int c7 = pattern_index((int)((((black<<3) & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56),
                         (int)((((white<<3) & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56));

  const PatternInfo& pi = patternInfo[LINE4];
  e->patterns[32] = pi.offset + r2;
  e->patterns[33] = pi.offset + r7;
  e->patterns[34] = pi.offset + c2;
  e->patterns[35] = pi.offset + c7;
}

void eval_diag8(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int d8 = pattern_index((int)(((black & 0x8040201008040201ULL) * 0x0101010101010101ULL) >> 56),
                         (int)(((white & 0x8040201008040201ULL) * 0x0101010101010101ULL) >> 56));
  int ad8 = pattern_index((int)(((black & 0x0102040810204080ULL) * 0x0101010101010101ULL) >> 56),
                          (int)(((white & 0x0102040810204080ULL) * 0x0101010101010101ULL) >> 56));

  const PatternInfo& pi = patternInfo[DIAG8];
  e->patterns[36] = pi.offset + d8;
  e->patterns[37] = pi.offset + ad8;
}

void eval_diag7(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int du7 = pattern_index((int)(((black & 0x0080402010080402ULL) * 0x0101010101010101ULL) >> 57),
                          (int)(((white & 0x0080402010080402ULL) * 0x0101010101010101ULL) >> 57));
  int dl7 = pattern_index((int)(((black & 0x4020100804020100ULL) * 0x0101010101010101ULL) >> 56),
                          (int)(((white & 0x4020100804020100ULL) * 0x0101010101010101ULL) >> 56));
  int adu7 = pattern_index((int)(((black & 0x0001020408102040ULL) * 0x0101010101010101ULL) >> 56),
                           (int)(((white & 0x0001020408102040ULL) * 0x0101010101010101ULL) >> 56));
  int adl7 = pattern_index((int)(((black & 0x0204081020408000ULL) * 0x0101010101010101ULL) >> 57),
                           (int)(((white & 0x0204081020408000ULL) * 0x0101010101010101ULL) >> 57));

  const PatternInfo& pi = patternInfo[DIAG7];
  e->patterns[38] = pi.offset + du7;
  e->patterns[39] = pi.offset + dl7;
  e->patterns[40] = pi.offset + adu7;
  e->patterns[41] = pi.offset + adl7;
}

void eval_diag6(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int du6 = pattern_index((int)(((black & 0x0000804020100804ULL) * 0x0101010101010101ULL) >> 58),
                          (int)(((white & 0x0000804020100804ULL) * 0x0101010101010101ULL) >> 58));
  int dl6 = pattern_index((int)(((black & 0x2010080402010000ULL) * 0x0101010101010101ULL) >> 56),
                          (int)(((white & 0x2010080402010000ULL) * 0x0101010101010101ULL) >> 56));
  int adu6 = pattern_index((int)(((black & 0x0000010204081020ULL) * 0x0101010101010101ULL) >> 56),
                           (int)(((white & 0x0000010204081020ULL) * 0x0101010101010101ULL) >> 56));
  int adl6 = pattern_index((int)(((black & 0x0408102040800000ULL) * 0x0101010101010101ULL) >> 58),
                           (int)(((white & 0x0408102040800000ULL) * 0x0101010101010101ULL) >> 58));

  const PatternInfo& pi = patternInfo[DIAG6];
  e->patterns[42] = pi.offset + du6;
  e->patterns[43] = pi.offset + dl6;
  e->patterns[44] = pi.offset + adu6;
  e->patterns[45] = pi.offset + adl6;
}

void eval_diag5(Board& b, Eval* e) {
  uint64_t black = b.get_bits(BLACK);
  uint64_t white = b.get_bits(WHITE);
  int du5 = pattern_index((int)(((black & 0x0000008040201008ULL) * 0x0101010101010101ULL) >> 59),
                          (int)(((white & 0x0000008040201008ULL) * 0x0101010101010101ULL) >> 59));
  int dl5 = pattern_index((int)(((black & 0x1008040201000000ULL) * 0x0101010101010101ULL) >> 56),
                          (int)(((white & 0x1008040201000000ULL) * 0x0101010101010101ULL) >> 56));
  int adu5 = pattern_index((int)(((black & 0x0000000102040810ULL) * 0x0101010101010101ULL) >> 56),
                           (int)(((white & 0x0000000102040810ULL) * 0x0101010101010101ULL) >> 56));
  int adl5 = pattern_index((int)(((black & 0x0810204080000000ULL) * 0x0101010101010101ULL) >> 59),
                           (int)(((white & 0x0810204080000000ULL) * 0x0101010101010101ULL) >> 59));

  const PatternInfo& pi = patternInfo[DIAG5];
  e->patterns[46] = pi.offset + du5;
  e->patterns[47] = pi.offset + dl5;
  e->patterns[48] = pi.offset + adu5;
  e->patterns[49] = pi.offset + adl5;
}

int eval_44sv(Board &b, Color c) {
  uint64_t sbits = b.get_bits(c);
  int ul = (int) ((sbits & 0xF) + ((sbits>>4) & 0xF0) +
                  ((sbits>>8) & 0xF00) + ((sbits>>12) & 0xF000));

  uint64_t rv = reflect_vert(sbits);
  int ll = (int) ((rv & 0xF) + ((rv>>4) & 0xF0) +
                  ((rv>>8) & 0xF00) + ((rv>>12) & 0xF000));

  uint64_t rh = reflect_hor(sbits);
  int ur = (int) ((rh & 0xF) + ((rh>>4) & 0xF0) +
                  ((rh>>8) & 0xF00) + ((rh>>12) & 0xF000));

  uint64_t rb = reflect_vert(rh);
  int lr = (int) ((rb & 0xF) + ((rb>>4) & 0xF0) +
                  ((rb>>8) & 0xF00) + ((rb>>12) & 0xF000));

  return s44Table[ul] + s44Table[ll] + s44Table[ur] + s44Table[lr];
}

}  // namespace

void Eval::update(Color c, int m, uint64_t flipped) {
  if (c == WHITE) {
    {
      const SquareToPatterns& sp = squareToPatterns[m];
      for (int n = 0; n < sp.ct; n++) {
        patterns[sp.patterns[n].i] += 2 * sp.patterns[n].p;
      }
    }
    while (flipped) {
      int b = bitscan_forward(flipped);
      const SquareToPatterns& sp = squareToPatterns[b];
      for (int n = 0; n < sp.ct; n++) {
        patterns[sp.patterns[n].i] += sp.patterns[n].p;
      }
      flipped &= flipped - 1;
    }
  } else {
    {
      const SquareToPatterns& sp = squareToPatterns[m];
      for (int n = 0; n < sp.ct; n++) {
        patterns[sp.patterns[n].i] += sp.patterns[n].p;
      }
    }
    while (flipped) {
      int b = bitscan_forward(flipped);
      const SquareToPatterns& sp = squareToPatterns[b];
      for (int n = 0; n < sp.ct; n++) {
        patterns[sp.patterns[n].i] -= sp.patterns[n].p;
      }
      flipped &= flipped - 1;
    }
  }
}

void Eval::undo(Color c, int m, uint64_t flipped) {
  if (c == WHITE) {
    {
      const SquareToPatterns& sp = squareToPatterns[m];
      for (int n = 0; n < sp.ct; n++) {
        patterns[sp.patterns[n].i] -= 2 * sp.patterns[n].p;
      }
    }
    while (flipped) {
      int b = bitscan_forward(flipped);
      const SquareToPatterns& sp = squareToPatterns[b];
      for (int n = 0; n < sp.ct; n++) {
        patterns[sp.patterns[n].i] -= sp.patterns[n].p;
      }
      flipped &= flipped - 1;
    }
  } else {
    {
      const SquareToPatterns& sp = squareToPatterns[m];
      for (int n = 0; n < sp.ct; n++) {
        patterns[sp.patterns[n].i] -= sp.patterns[n].p;
      }
    }
    while (flipped) {
      int b = bitscan_forward(flipped);
      const SquareToPatterns& sp = squareToPatterns[b];
      for (int n = 0; n < sp.ct; n++) {
        patterns[sp.patterns[n].i] += sp.patterns[n].p;
      }
      flipped &= flipped - 1;
    }
  }
}

void init_eval() {
  for (unsigned int i = 0; i < 1024; i++) {
    PIECES_TO_INDEX[i] = binary_to_ternary(i);
  }

  pvTable = new int16_t*[N_SPLITS];
  for (int i = 0; i < N_SPLITS; i++) {
    pvTable[i] = new int16_t[N_WEIGHTS];
  }
  s44Table = new int16_t[65536];

  std::cerr << "Reading eval tables." << std::endl;
  std::string dir = "Flippy_Resources/";
  read_pattern_weights(dir);
  read_stability_table("Flippy_Resources/s44table.txt", s44Table);
}

void init_evaluator(Board& b, Eval* e) {
  eval_25(b, e);
  eval_e2x(b, e);
  eval_trapz(b, e);
  eval_33(b, e);
  eval_m(b, e);
  eval_line2(b, e);
  eval_line3(b, e);
  eval_line4(b, e);
  eval_diag8(b, e);
  eval_diag7(b, e);
  eval_diag6(b, e);
  eval_diag5(b, e);
}

int heuristic(Board &b, Eval* e, Color c) {
  if (b.count(c) == 0 || b.count_empty() == 0)
    return score_game_end(b, c);

  int turn = pv_split_index(b.count_empty());
  int score = 0;

  int patterns = 0;
  for (int i = 0; i < N_OCC; i++) {
    patterns += pvTable[turn][e->patterns[i]];
  }
  if (c == BLACK)
    score += patterns;
  else
    score -= patterns;
  // uint64_t bm = b.get_bits(c);
  // uint64_t bo = b.get_bits(~c);
  // score += 1000 * (count_bits(bm & CORNERS) - count_bits(bo & CORNERS));
  // score += 50 * (count_bits(bm & EDGES) - count_bits(bo & EDGES));
  // score -= 150 * (count_bits(bm & X_CORNERS) - count_bits(bo & X_CORNERS));
  //score -= 50 * (count_bits(bm & ADJ_CORNERS) - count_bits(bo & ADJ_CORNERS));

  // int mobility = b.count_legal_moves(c);
  // int opp_mobility = b.count_legal_moves(~c);
  // score += 200 * (mobility - opp_mobility);
  // score += 2000 * (mobility - opp_mobility) / (std::min(opp_mobility, mobility) + 2);
  //score += 100 * ((64 - turn) / 16) * (b.count_potential_mobility(c) - b.count_potential_mobility(~c));

  return score;
}

int stability(Board &b, Color c) {
  return eval_44sv(b, c);
}

int score_game_end(Board& b, Color c) {
  int ourCt = b.count(c);
  int theirCt = b.count(~c);
  // If we were wiped out, create a large negative score tapered towards 0-64.
  if (ourCt == 0)
    return EVAL_SCALE_FACTOR * (-64 - theirCt) / 2;
  return EVAL_SCALE_FACTOR * (ourCt - theirCt);
}
