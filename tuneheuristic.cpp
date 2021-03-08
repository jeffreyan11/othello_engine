#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "player.h"
#include "board.h"
#include "endgame.h"
#include "eval.h"
#include "common.h"
#include "patternbuilder.h"

using namespace std;

const int BOOK_SIZE = 8200;
// const int BOOK_SIZE = 7177;
const int hashSize = 10;
const int maxDepth = 9;
const int endgameDepth = 14;

thor_game *savedGames[2*BOOK_SIZE];
vector<string> positions;
int wins;
int losses;
int draws;

bool readFile();
void writeFile();
void freemem();

void play(const string &position, int saveIndex) {
  thor_game *result1 = new thor_game();
  thor_game *result2 = new thor_game();

  std::string pos = position;
  std::string::size_type sz = 0;
  uint64_t takenBits = std::stoull(pos, &sz, 0);
  pos = pos.substr(sz);
  uint64_t blackBits = std::stoull(pos, &sz, 0);
  pos = pos.substr(sz);
  Board b(takenBits^blackBits, blackBits);

  int movenum = 0;
  for (movenum = 0; movenum < 6; movenum++) {
    result1->moves[movenum] = std::stoi(pos, &sz, 0);
    result2->moves[movenum] = std::stoi(pos, &sz, 0);
    pos = pos.substr(sz);
  }

  // Run game on one side
  Player black(BLACK, false, hashSize);
  Player white(WHITE, false, hashSize);
  black.set_depths(maxDepth+2, endgameDepth);
  white.set_depths(maxDepth, endgameDepth);
  black.otherHeuristic = true;
  black.game = b;
  white.game = b;

  int m = MOVE_NULL;
  bool passed = false;
  while (true) {
    m = black.do_move(m, -1);
    // two passes in a row is end of game
    if (m == MOVE_NULL) {
      if (passed)
        break;
      passed = true;
    }
    else {
      result1->moves[movenum] = m;
      movenum++;
      passed = false;
    }

    m = white.do_move(m, -1);
    if (m == MOVE_NULL) {
      if (passed)
        break;
      passed = true;
    }
    else {
      result1->moves[movenum] = m;
      movenum++;
      passed = false;
    }
  }

  int bf = black.game.count(BLACK);
  int wf = black.game.count(WHITE);
  cout << bf << " " << wf << endl;
  if (bf > wf)
    wins++;
  else if (wf > bf)
    losses++;
  else
    draws++;
  result1->final = (bf - wf + 64) / 2;
  savedGames[2*saveIndex] = result1;

  // Run game on the other side
  Player black2(BLACK, false, hashSize);
  Player white2(WHITE, false, hashSize);
  black2.set_depths(maxDepth, endgameDepth);
  white2.set_depths(maxDepth+2, endgameDepth);
  white2.otherHeuristic = true;
  black2.game = b;
  white2.game = b;

  m = MOVE_NULL;
  passed = false;
  movenum = 6;
  while (true) {
    m = black2.do_move(m, -1);
    // two passes in a row is end of game
    if (m == MOVE_NULL) {
      if (passed)
        break;
      passed = true;
    }
    else {
      result2->moves[movenum] = m;
      movenum++;
      passed = false;
    }

    m = white2.do_move(m, -1);
    if (m == MOVE_NULL) {
      if (passed)
        break;
      passed = true;
    }
    else {
      result2->moves[movenum] = m;
      movenum++;
      passed = false;
    }
  }

  bf = black2.game.count(BLACK);
  wf = black2.game.count(WHITE);
  cout << bf << " " << wf << endl;
  if (bf > wf)
    losses++;
  else if (wf > bf)
    wins++;
  else
    draws++;
  result2->final = (bf - wf + 64) / 2;
  savedGames[2*saveIndex+1] = result2;
  cout << "" << wins << "-" << losses << "-" << draws << endl;
}

int main(int argc, char **argv) {
  readFile();
  cout << "Files read" << endl;

  init_eval();

  wins = 0;
  losses = 0;
  draws = 0;
  cout << "Score from other heuristic POV" << endl;

  for (int i = 0; i < BOOK_SIZE; i++) {
    play(positions[i], i);
  }

  cout << "Final result: " << wins << "-" << losses << "-" << draws << endl;

  writeFile();
  freemem();

  return 0;
}

bool readFile() {
  std::string line;
  std::ifstream cfile("perft6.txt");
  // std::ifstream cfile("perft6_balanced.txt");

  if (cfile.is_open()) {
    while (getline(cfile, line)) {
      positions.push_back(string(line));
    }
    cfile.close();
    return true;
  }
  return false;
}

void writeFile() {
  ofstream out;

  out.open("tuneoutput.txt");
  for (int i = 0; i < 2*BOOK_SIZE; i++) {
    thor_game *a = savedGames[i];
    out << a->final;
    for (int j = 0; j < 60; j++)
      out << " " << a->moves[j];

    out << endl;
  }
}

void freemem() {
  for (int i = 0; i < 2*BOOK_SIZE; i++)
    delete savedGames[i];
}
