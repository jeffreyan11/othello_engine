#include "board.h"

#include <random>
#include "bbinit.h"

const int BOARD_REGIONS[64] = {
  1, 1, 2, 2, 2, 2, 3, 3,
  1, 1, 2, 2, 2, 2, 3, 3,
  4, 4, 5, 5, 5, 5, 6, 6,
  4, 4, 5, 5, 5, 5, 6, 6,
  4, 4, 5, 5, 5, 5, 6, 6,
  4, 4, 5, 5, 5, 5, 6, 6,
  7, 7, 8, 8, 8, 8, 9, 9,
  7, 7, 8, 8, 8, 8, 9, 9
};

uint32_t **Board::init_zobrist_table() {
  std::mt19937 rng(612801529U);
  uint32_t **table = new uint32_t *[16];
  for (int i = 0; i < 16; i++)
    table[i] = new uint32_t[256];
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 256; j++) {
      table[i][j] = rng();
    }
  }
  return table;
}

uint32_t **Board::zobristTable = Board::init_zobrist_table();

uint32_t Board::hash() {
  // On-the-fly Zobrist hash calculation, using bytes as
  // the base unit. Idea from Richard Delorme's edax-reversi.
  const uint8_t *hash_strings = (const uint8_t *) (this);
  uint32_t hash = 0;
  for (int i = 0; i < 16; i++) {
    hash ^= zobristTable[i][hash_strings[i]];
  }
  return hash;
}

Board::Board() {
  pieces[WHITE] = 0x0000001008000000;
  pieces[BLACK] = 0x0000000810000000;
}

Board::Board(uint64_t w, uint64_t b) {
  pieces[WHITE] = w;
  pieces[BLACK] = b;
}

Board::Board(char data[]) {
  pieces[WHITE] = 0;
  pieces[BLACK] = 0;
  for (int i = 0; i < 64; i++) {
    if (data[i] == 'b' || data[i] == 'B' || data[i] == 'X') {
      pieces[BLACK] |= SQ_TO_BIT[i];
    } if (data[i] == 'w' || data[i] == 'W' || data[i] == 'O') {
      pieces[WHITE] |= SQ_TO_BIT[i];
    }
  }
}

Board::Board(const std::string& data) {
  pieces[WHITE] = 0;
  pieces[BLACK] = 0;
  for (int i = 0; i < 64; i++) {
    if (data[i] == 'b' || data[i] == 'B' || data[i] == 'X') {
      pieces[BLACK] |= SQ_TO_BIT[i];
    } if (data[i] == 'w' || data[i] == 'W' || data[i] == 'O') {
      pieces[WHITE] |= SQ_TO_BIT[i];
    }
  }
}

Board Board::copy() {
  return Board(pieces[WHITE], pieces[BLACK]);
}

void Board::do_move(Color c, int m) {
  // A NULL move means pass.
  if (m == MOVE_NULL) return;
  uint64_t mask = get_do_move(c, m);
  do_move(c, m, mask);
}

void Board::do_move(Color c, int m, uint64_t mask) {
  pieces[WHITE] ^= mask;
  pieces[BLACK] ^= mask;
  pieces[c] |= SQ_TO_BIT[m];
}

// This algorithm gets the mask by lookup with precalculated tables in each of
// the eight directions. A switch with the board region is used to only
// consider certain directions for efficiency.
uint64_t Board::get_do_move(Color c, int m) {
  uint64_t mask = 0;
  uint64_t pos = ~pieces[~c];
  uint64_t self = pieces[c];

  switch (BOARD_REGIONS[m]) {
    case 1:
      mask = south_fill(m, self, pos);
      mask |= east_fill(m, self, pos);
      mask |= se_fill(m, self, pos);
      break;
    case 2:
      mask = south_fill(m, self, pos);
      mask |= east_fill(m, self, pos);
      mask |= west_fill(m, self, pos);
      mask |= sw_fill(m, self, pos);
      mask |= se_fill(m, self, pos);
      break;
    case 3:
      mask = south_fill(m, self, pos);
      mask |= west_fill(m, self, pos);
      mask |= sw_fill(m, self, pos);
      break;
    case 4:
      mask = north_fill(m, self, pos);
      mask |= south_fill(m, self, pos);
      mask |= east_fill(m, self, pos);
      mask |= ne_fill(m, self, pos);
      mask |= se_fill(m, self, pos);
      break;
    case 5:
      mask = north_fill(m, self, pos);
      mask |= south_fill(m, self, pos);
      mask |= east_fill(m, self, pos);
      mask |= west_fill(m, self, pos);
      mask |= ne_fill(m, self, pos);
      mask |= nw_fill(m, self, pos);
      mask |= sw_fill(m, self, pos);
      mask |= se_fill(m, self, pos);
      break;
    case 6:
      mask = north_fill(m, self, pos);
      mask |= south_fill(m, self, pos);
      mask |= west_fill(m, self, pos);
      mask |= nw_fill(m, self, pos);
      mask |= sw_fill(m, self, pos);
      break;
    case 7:
      mask = north_fill(m, self, pos);
      mask |= east_fill(m, self, pos);
      mask |= ne_fill(m, self, pos);
      break;
    case 8:
      mask = north_fill(m, self, pos);
      mask |= east_fill(m, self, pos);
      mask |= west_fill(m, self, pos);
      mask |= ne_fill(m, self, pos);
      mask |= nw_fill(m, self, pos);
      break;
    case 9:
      mask = north_fill(m, self, pos);
      mask |= west_fill(m, self, pos);
      mask |= nw_fill(m, self, pos);
      break;
  }

  return mask;
}

void Board::undo_move(Color c, int m, uint64_t mask) {
  pieces[WHITE] ^= mask;
  pieces[BLACK] ^= mask;
  pieces[c] ^= SQ_TO_BIT[m];
}

bool Board::is_legal(Color c, int m) {
  uint64_t legal = legal_moves(c);
  if ((m < 0 || m > 63) && !legal)
    return true;
  return legal & SQ_TO_BIT[m];
}

ArrayList Board::legal_movelist(Color c) {
  ArrayList movelist;
  uint64_t mask = legal_moves(c);
  while (mask) {
    movelist.add(bitscan_forward(mask));
    mask &= mask-1;
  }
  return movelist;
}

// This method operates by checking in all eight directions, first for the line
// of pieces of the opposite color, then for the empty square to place the
// anchor once the line ends.
uint64_t Board::legal_moves(Color c) {
  uint64_t result = 0;
  uint64_t self = pieces[c];
  uint64_t opp = pieces[~c];

  uint64_t other = opp & 0x00FFFFFFFFFFFF00;
  // north and south
  uint64_t templ = other & (self << 8);
  uint64_t tempr = other & (self >> 8);
  templ |= other & (templ << 8);
  tempr |= other & (tempr >> 8);
  uint64_t maskl = other & (other << 8);
  uint64_t maskr = other & (other >> 8);
  templ |= maskl & (templ << 16);
  tempr |= maskr & (tempr >> 16);
  templ |= maskl & (templ << 16);
  tempr |= maskr & (tempr >> 16);
  result |= (templ << 8) | (tempr >> 8);

  other = opp & 0x7E7E7E7E7E7E7E7E;
  // east and west
  templ = other & (self << 1);
  tempr = other & (self >> 1);
  templ |= other & (templ << 1);
  tempr |= other & (tempr >> 1);
  maskl = other & (other << 1);
  maskr = other & (other >> 1);
  templ |= maskl & (templ << 2);
  tempr |= maskr & (tempr >> 2);
  templ |= maskl & (templ << 2);
  tempr |= maskr & (tempr >> 2);
  result |= (templ << 1) | (tempr >> 1);

  // ne and sw
  templ = other & (self << 7);
  tempr = other & (self >> 7);
  templ |= other & (templ << 7);
  tempr |= other & (tempr >> 7);
  maskl = other & (other << 7);
  maskr = other & (other >> 7);
  templ |= maskl & (templ << 14);
  tempr |= maskr & (tempr >> 14);
  templ |= maskl & (templ << 14);
  tempr |= maskr & (tempr >> 14);
  result |= (templ << 7) | (tempr >> 7);

  // nw and se
  templ = other & (self << 9);
  tempr = other & (self >> 9);
  templ |= other & (templ << 9);
  tempr |= other & (tempr >> 9);
  maskl = other & (other << 9);
  maskr = other & (other >> 9);
  templ |= maskl & (templ << 18);
  tempr |= maskr & (tempr >> 18);
  templ |= maskl & (templ << 18);
  tempr |= maskr & (tempr >> 18);
  result |= (templ << 9) | (tempr >> 9);

  return result & ~occupied();
}

int Board::count_legal_moves(Color c) {
  return count_bits(legal_moves(c));
}

int Board::count_potential_mobility(Color c) {
  uint64_t other = pieces[~c];
  uint64_t empty = ~occupied();

  uint64_t result = (other >> 8) | (other << 8);
  result &= empty;

  empty &= 0x7E7E7E7E7E7E7E7E;
  result |= (other << 1) | (other >> 1);
  result |= (other >> 7) | (other << 7);
  result |= (other >> 9) | (other << 9);
  result &= empty;

  return count_bits(result);
}

int Board::count_stability(Color c) {
  uint64_t result = 0;
  uint64_t self = pieces[c];
  uint64_t occ = occupied();

  // north and south
  // calculate full lines
  uint64_t border = self & 0xFF000000000000FF;
  uint64_t full = occ & (((occ << 8) & (occ >> 8)) | border);
  full &= (((full << 8) & (full >> 8)) | border);
  full &= (((full << 8) & (full >> 8)) | border);
  full &= (((full << 8) & (full >> 8)) | border);
  full &= (((full << 8) & (full >> 8)) | border);
  full = (full << 8) & (full >> 8);

  // propagate edge pieces, anywhere there are full lines,
  // to find single direction stability
  uint64_t tempM = border;
  tempM |= (((border << 8) | (border >> 8)) & full);
  tempM |= (((tempM << 8) | (tempM >> 8)) & full);
  tempM |= (((tempM << 8) | (tempM >> 8)) & full);
  tempM |= (((tempM << 8) | (tempM >> 8)) & full);
  tempM |= (((tempM << 8) | (tempM >> 8)) & full);
  tempM |= (((tempM << 8) | (tempM >> 8)) & full);
  result = tempM;

  // east and west
  border = self & 0x8181818181818181;
  full = occ & (((occ << 1) & (occ >> 1)) | border);
  full &= (((full << 1) & (full >> 1)) | border);
  full &= (((full << 1) & (full >> 1)) | border);
  full &= (((full << 1) & (full >> 1)) | border);
  full &= (((full << 1) & (full >> 1)) | border);
  full = (full << 1) & (full >> 1);

  tempM = border;
  tempM |= (((border << 1) | (border >> 1)) & full);
  tempM |= (((tempM << 1) | (tempM >> 1)) & full);
  tempM |= (((tempM << 1) | (tempM >> 1)) & full);
  tempM |= (((tempM << 1) | (tempM >> 1)) & full);
  tempM |= (((tempM << 1) | (tempM >> 1)) & full);
  tempM |= (((tempM << 1) | (tempM >> 1)) & full);
  result &= tempM;

  // ne and sw
  border = self & 0xFF818181818181FF;
  full = occ & (((occ << 7) & (occ >> 7)) | border);
  full &= (((full << 7) & (full >> 7)) | border);
  full &= (((full << 7) & (full >> 7)) | border);
  full &= (((full << 7) & (full >> 7)) | border);
  full &= (((full << 7) & (full >> 7)) | border);
  full = (full << 7) & (full >> 7);

  tempM = border;
  tempM |= (((border << 7) | (border >> 7)) & full);
  tempM |= (((tempM << 7) | (tempM >> 7)) & full);
  tempM |= (((tempM << 7) | (tempM >> 7)) & full);
  tempM |= (((tempM << 7) | (tempM >> 7)) & full);
  tempM |= (((tempM << 7) | (tempM >> 7)) & full);
  tempM |= (((tempM << 7) | (tempM >> 7)) & full);
  result &= tempM;

  // nw and se
  border = self & 0xFF818181818181FF;
  full = occ & (((occ << 9) & (occ >> 9)) | border);
  full &= (((full << 9) & (full >> 9)) | border);
  full &= (((full << 9) & (full >> 9)) | border);
  full &= (((full << 9) & (full >> 9)) | border);
  full &= (((full << 9) & (full >> 9)) | border);
  full = (full << 9) & (full >> 9);

  tempM = border;
  tempM = (((border << 9) | (border >> 9)) & full);
  tempM |= (((tempM << 9) | (tempM >> 9)) & full);
  tempM |= (((tempM << 9) | (tempM >> 9)) & full);
  tempM |= (((tempM << 9) | (tempM >> 9)) & full);
  tempM |= (((tempM << 9) | (tempM >> 9)) & full);
  tempM |= (((tempM << 9) | (tempM >> 9)) & full);
  result &= tempM;

  // Find corner stability
  tempM = self & CORNERS;
  tempM |= ((((tempM << 1) | (tempM >> 1)) & 0x7E7E7E7E7E7E7E7E) | (tempM << 8) | (tempM >> 8)) & self;

  uint64_t all_other = ((tempM << 1) | (tempM >> 1)) & ((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E;
  tempM |= ((tempM << 8) | (tempM >> 8)) & self & all_other;
  all_other = (((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E) & ((tempM << 8) | (tempM >> 8));
  tempM |= (((tempM << 1) | (tempM >> 1)) & 0x7E7E7E7E7E7E7E7E) & self & all_other;

  all_other = ((tempM << 1) | (tempM >> 1)) & ((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E;
  tempM |= ((tempM << 8) | (tempM >> 8)) & self & all_other;
  all_other = (((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E) & ((tempM << 8) | (tempM >> 8));
  tempM |= (((tempM << 1) | (tempM >> 1)) & 0x7E7E7E7E7E7E7E7E) & self & all_other;

  all_other = ((tempM << 1) | (tempM >> 1)) & ((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E;
  tempM |= ((tempM << 8) | (tempM >> 8)) & self & all_other;
  all_other = (((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E) & ((tempM << 8) | (tempM >> 8));
  tempM |= (((tempM << 1) | (tempM >> 1)) & 0x7E7E7E7E7E7E7E7E) & self & all_other;

  all_other = ((tempM << 1) | (tempM >> 1)) & ((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E;
  tempM |= ((tempM << 8) | (tempM >> 8)) & self & all_other;
  all_other = (((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E) & ((tempM << 8) | (tempM >> 8));
  tempM |= (((tempM << 1) | (tempM >> 1)) & 0x7E7E7E7E7E7E7E7E) & self & all_other;
  result |= tempM;

  all_other = ((tempM << 1) | (tempM >> 1)) & ((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E;
  tempM |= ((tempM << 8) | (tempM >> 8)) & self & all_other;
  all_other = (((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E) & ((tempM << 8) | (tempM >> 8));
  tempM |= (((tempM << 1) | (tempM >> 1)) & 0x7E7E7E7E7E7E7E7E) & self & all_other;

  all_other = ((tempM << 1) | (tempM >> 1)) & ((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E;
  tempM |= ((tempM << 8) | (tempM >> 8)) & self & all_other;
  all_other = (((tempM << 7) | (tempM >> 7)) & ((tempM << 9) | (tempM >> 9)) & 0x7E7E7E7E7E7E7E7E) & ((tempM << 8) | (tempM >> 8));
  tempM |= (((tempM << 1) | (tempM >> 1)) & 0x7E7E7E7E7E7E7E7E) & self & all_other;
  result |= tempM;

  return count_bits(result & self);
}

uint64_t Board::get_bits(Color c) {
  return pieces[c];
}

uint64_t Board::occupied() {
  return pieces[WHITE] | pieces[BLACK];
}

int Board::count(Color c) {
  return count_bits(pieces[c]);
}

int Board::count_empty() {
  return count_bits(~occupied());
}

std::string Board::to_string() {
  char *buf = new char[65];
  uint64_t occ = occupied();
  for (int i = 0; i < 64; i++) {
    if (occ & SQ_TO_BIT[i]) {
      if (pieces[BLACK] & SQ_TO_BIT[i]) {
        buf[i] = 'X';
      } else {
        buf[i] = 'O';
      }
    } else {
      buf[i] = '-';
    }
  }
  // null terminate
  buf[64] = 0;
  std::string result = std::string(buf);
  delete[] buf;
  return result;
}

//----------------------------do_move() helpers---------------------------------
uint64_t Board::north_fill(int m, uint64_t self, uint64_t pos) {
  uint64_t result = NORTHRAY[m];
  // Capture line ends with either own piece or empty square
  uint64_t block = result & pos;
  // If the line ends before the edge
  if (block) {
    int anchor = bitscan_reverse(block);
    // use multiplication to reduce branching
    // confirm line ender is an anchor piece
    return ((bool) (self & SQ_TO_BIT[anchor]))
    // and if so, we can return the captured pieces
          * (result & SOUTHRAY[anchor]);
  }
  return 0;
}

uint64_t Board::south_fill(int m, uint64_t self, uint64_t pos) {
  uint64_t result = SOUTHRAY[m];
  uint64_t block = result & pos;
  // Get single occupancy blocker mask, if the blocker is an anchor piece
  block &= -block & self;
  if (block) {
    int anchor = bitscan_forward(block);
    return (result & NORTHRAY[anchor]);
  }
  return 0;
}

uint64_t Board::east_fill(int m, uint64_t self, uint64_t pos) {
  uint64_t result = EASTRAY[m];
  uint64_t block = result & pos;
  block &= -block & self;
  if (block) {
    int anchor = bitscan_forward(block);
    return (result & WESTRAY[anchor]);
  }
  return 0;
}

uint64_t Board::west_fill(int m, uint64_t self, uint64_t pos) {
  uint64_t result = WESTRAY[m];
  uint64_t block = result & pos;
  if (block) {
    int anchor = bitscan_reverse(block);
    return ((bool) (self & SQ_TO_BIT[anchor]))
          * (result & EASTRAY[anchor]);
  }
  return 0;
}

uint64_t Board::ne_fill(int m, uint64_t self, uint64_t pos) {
  uint64_t result = NERAY[m];
  uint64_t block = result & pos;
  if (block) {
    int anchor = bitscan_reverse(block);
    return ((bool) (self & SQ_TO_BIT[anchor]))
          * (result & SWRAY[anchor]);
  }
  return 0;
}

uint64_t Board::nw_fill(int m, uint64_t self, uint64_t pos) {
  uint64_t result = NWRAY[m];
  uint64_t block = result & pos;
  if (block) {
    int anchor = bitscan_reverse(block);
    return ((bool) (self & SQ_TO_BIT[anchor]))
          * (result & SERAY[anchor]);
  }
  return 0;
}

uint64_t Board::sw_fill(int m, uint64_t self, uint64_t pos) {
  uint64_t result = SWRAY[m];
  uint64_t block = result & pos;
  block &= -block & self;
  if (block) {
    int anchor = bitscan_forward(block);
    return (result & NERAY[anchor]);
  }
  return 0;
}

uint64_t Board::se_fill(int m, uint64_t self, uint64_t pos) {
  uint64_t result = SERAY[m];
  uint64_t block = result & pos;
  block &= -block & self;
  if (block) {
    int anchor = bitscan_forward(block);
    return (result & NWRAY[anchor]);
  }
  return 0;
}
