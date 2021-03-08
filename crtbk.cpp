#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "board.h"
#include "eval.h"
#include "openings.h"
#include "player.h"

using namespace std;

vector<Node> openings;
vector<string> stringForm;

bool read_file();
void write_file();

// Checks if a position already exists in the book. If so, it returns the move
// in the book. Otherwise, it returns -1.
int contains(uint64_t taken, uint64_t black) {
  for (unsigned int i = 0; i < openings.size(); i++) {
    if ((openings[i].taken == taken) && (openings[i].black == black)) {
      return openings[i].move;
    }
  }
  return -1;
}

// Given a board and the best move for that board, adds it to the book
void add(Board &b, int move) {
  Node nnn;
  nnn.move = move;
  nnn.taken = b.occupied();
  nnn.black = b.get_bits(BLACK);

  std::stringstream stream;
  stream << "0x" << std::setfill('0') << std::setw(16) << std::hex << b.occupied();
  std::string taken( stream.str() );
  std::stringstream stream2;
  stream2 << "0x" << std::setfill('0') << std::setw(16) << std::hex << b.get_bits(BLACK);
  std::string black( stream2.str() );

  std::stringstream t;
  t << taken << " " << black << " " << move;
  std::string result(t.str());
  stringForm.push_back(result);
  openings.push_back(nnn);
  cout << result << endl;
}

// Recursively builds the book, considering all possible positions to a depth
// and finding the best move
void next(Player &p, Board &b, Color color, int plies) {
  p.game = b;
  Board temp = b.copy();

  // Consider transpositions or previous book
  int exists = contains(b.occupied(), b.get_bits(BLACK));
  if (exists != -1) {
    std::cout << "already exists. traversing." << std::endl;
    temp.do_move(color, exists);
  }
  // Find a best move for the current position
  else {
    int m = p.do_move(MOVE_NULL, -1);
    add(b, m);
    temp.do_move(color, m);
  }

  if (plies <= 0) {
    return;
  }

  // for all legal replies
  ArrayList legalMoves = temp.legal_movelist(~color);
  for (int i = 0; i < legalMoves.size(); i++) {
    Board copy = temp.copy();
    copy.do_move(~color, legalMoves.get(i));

    next(p, copy, color, plies-1);
  }
}

int main(int argc, char **argv) {
  // Read existing book, if any
  read_file();
  init_eval();

  Player p(BLACK, false, 20);
  Player p2(WHITE, false, 20);
  p.set_depths(24, 36);
  p2.set_depths(24, 36);
  Board b;
  // consider black
  next(p, b, BLACK, 4);
  // consider white
  ArrayList lm = b.legal_movelist(BLACK);
  for (int i = 0; i < 4; i++) {
    Board copy = b.copy();
    copy.do_move(BLACK, lm.get(i));

    next(p2, copy, WHITE, 4);
  }

  write_file();
  return 0;
}

bool read_file() {
  std::string line;
  std::ifstream openingbk("newbook.txt");

  if (openingbk.is_open()) {
    // Discard the first line, which contains the size
    getline(openingbk, line);
    while (getline(openingbk, line)) {
      stringForm.push_back(std::string(line));
      std::string::size_type sz = 0;
      Node temp;
      temp.taken = std::stoull(line, &sz, 0);
      line = line.substr(sz);
      temp.black = std::stoull(line, &sz, 0);
      line = line.substr(sz);
      int m = std::stoi(line, &sz, 0);
      temp.move = m;
      openings.push_back(temp);
    }
    openingbk.close();
    return true;
  }
  return false;
}

void write_file() {
  // Sort
  std::sort(stringForm.begin(), stringForm.end());

  ofstream out;
  out.open("newbook.txt");
  out << stringForm.size() << endl;
  for (unsigned int i = 0; i < stringForm.size(); i++) {
    out << stringForm[i] << endl;
  }
  out.close();
}
