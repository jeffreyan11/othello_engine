#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "common.h"
#include "board.h"
#include "endgame.h"
#include "player.h"

namespace {

// Prints a usage statement.
void usage() {
  std::cerr << "Usage: testsuites    [test type] [option]" << std::endl;
  std::cerr << "Test types: perft    [depth]" << std::endl;
  std::cerr << "            bench    [depth] [sel]" << std::endl;
  std::cerr << "            ffo      [n] tests the first n positions" << std::endl;
  std::cerr << "            eg       [14|16|18|20|22] test 500 positions to given depth" << std::endl;
  std::cerr << "            eval_acc [depth] test eg eval accuracy @ depth" << std::endl;
}

}  // namespace

std::vector<std::string> split(const std::string &s, char d);

uint64_t perft(Board &b, Color c, int depth, bool passed);
void bench(std::string file, int depth, int sel);
uint64_t ffo(std::string file);
void egtest(std::string file);
void eval_acc(int depth);

int main(int argc, char **argv) {
  // if (argc != 3) {
  //   usage();
  //   return 1;
  // }

  init_eval();

  if (std::string(argv[1]) == "perft") {
    auto start_time = Clock::now();
    Board b;
    int plies = std::stoi(argv[2]);
    uint64_t nodes = perft(b, BLACK, plies, false);
    uint64_t time_ms = get_time_elapsed(start_time);
    std::cerr << "Nodes: " << nodes << " | NPS: " << 1000 * nodes / time_ms << std::endl;
    std::cerr << "Time: " << time_ms << " ms" << std::endl;
  } else if (std::string(argv[1]) == "bench") {
    int depth = std::stoi(argv[2]);
    int sel = 1;
    if (argc == 4) sel = std::stoi(argv[3]);
    bench("Flippy_Resources/bench.txt", depth, sel);
  } else if (std::string(argv[1]) == "ffo") {
    uint64_t ms = 0;
    int positions = std::stoi(argv[2]);
    resize_endhash(14);
    for (int i = 0; i < positions; i++) {
      std::string file_name = "ffotest/end";
      file_name += std::to_string(40 + i);
      file_name += ".pos";
      ms += ffo(file_name);
    }
    std::cerr << "Time: " << ms / 1000.0 << " s" << std::endl;
  } else if (std::string(argv[1]) == "eg") {
    int max_depth = std::stoi(argv[2]);
    switch (max_depth) {
      case 14:
        resize_endhash(6);
        egtest("ffotest/eg_13_14.txt");
        break;
      case 16:
        resize_endhash(7);
        egtest("ffotest/eg_15_16.txt");
        break;
      case 18:
        resize_endhash(8);
        egtest("ffotest/eg_17_18.txt");
        break;
      case 20:
        resize_endhash(10);
        egtest("ffotest/eg_19_20.txt");
        break;
      case 22:
        resize_endhash(12);
        egtest("ffotest/eg_21_22.txt");
        break;
    }
  } else if (std::string(argv[1]) == "eval_acc") {
    int depth = std::stoi(argv[2]);
    eval_acc(depth);
  } else {
    usage();
    return 1;
  }

  return 0;
}

void bench(std::string file, int depth, int sel) {
  std::vector<std::string> positions;
  std::ifstream cfile(file);
  uint64_t total_time = 0;
  uint64_t total_nodes = 0;

  if (cfile.is_open()) {
    std::string line;
    while (getline(cfile, line)) {
      positions.push_back(line);
    }
  }

  auto overhead = Clock::now();
  for (unsigned int i = 0; i < positions.size(); i++) {
    std::string line = positions[i];
    char board[64];
    const char *read = line.c_str();
    for (int j = 0; j < 64; j++)
      board[j] = read[j];

    std::vector<std::string> parts = split(line, ' ');

    Color side = BLACK;
    if (parts[1] == "Black") {
      side = BLACK;
    }
    else {
      side = WHITE;
    }

    Board b(board);
    Player p(side, false, std::min(20, depth));
    p.set_depths(depth, 0);
    p.baseSelectivity = sel;
    p.game = b;
    auto start_time = Clock::now();
    p.do_move(MOVE_NULL, -1);
    uint64_t ms = get_time_elapsed(start_time);
    total_time += ms;
    total_nodes += p.get_nodes();
  }

  std::cerr << "Nodes: " << total_nodes << std::endl;
  std::cerr << "Total time: " << total_time << std::endl;
  std::cerr << "NPS: " << 1000 * total_nodes / total_time << std::endl;
  std::cerr << "Time with overhead: " << get_time_elapsed(overhead) << std::endl;
}

// Sets up a solve of one position of the FFO suite, given a string with
// the location of the FFO test position file.
uint64_t ffo(std::string file) {
  std::string line;
  std::string ffostring;
  std::ifstream cfile(file);
  char board[64];

  if (cfile.is_open()) {
    getline(cfile, line);
    const char *read = line.c_str();
    for (int i = 0; i < 64; i++)
      board[i] = read[i];

    getline(cfile, line);
    getline(cfile, ffostring);
    std::cerr << ffostring << std::endl;
  }

  const char *read_color = line.c_str();
  Color side = BLACK;
  if (read_color[0] == 'B') {
    std::cerr << "Solving for black: ";
    side = BLACK;
  }
  else {
    std::cerr << "Solving for white: ";
    side = WHITE;
  }

  Board b(board);
  Eval e;
  init_evaluator(b, &e);

  ArrayList lm = b.legal_movelist(side);
  int empties = b.count_empty();
  std::cerr << empties << " empty" << std::endl;

  Endgame eg;
  int score;
  auto start_time = Clock::now();
  int m = eg.solve_endgame(b, &e, side, lm, false, empties, 100000000, &score);
  uint64_t ms = get_time_elapsed(start_time);
  std::cerr << "Solution: " << print_move(m) << " Score: " << score << " Time: " << ms << " ms" << std::endl;
  std::cerr << std::endl;
  return ms;
}

void egtest(std::string file) {
  std::vector<std::string> positions;
  std::ifstream cfile(file);
  uint64_t total_time = 0;
  uint64_t total_nodes = 0;
  uint64_t min_time = 1 << 30;
  uint64_t max_time = 0;

  if (cfile.is_open()) {
    std::string line;
    while (getline(cfile, line)) {
      positions.push_back(line);
    }
  }

  auto overhead = Clock::now();
  for (unsigned int i = 0; i < positions.size(); i++) {
    if (i % 100 == 0) {
      std::cerr << "Position " << i+1 << std::endl;
    }
    std::string line = positions[i];
    char board[64];
    const char *read = line.c_str();
    for (int j = 0; j < 64; j++)
      board[j] = read[j];

    std::vector<std::string> parts = split(line, ' ');

    Color side = (parts[1] == "Black") ? BLACK : WHITE;
    int move_sol = std::stoi(parts[2]);
    int score_sol = std::stoi(parts[3]);
    if (side == WHITE) score_sol = -score_sol;

    Board b(board);
    Eval e;
    init_evaluator(b, &e);

    ArrayList lm = b.legal_movelist(side);
    int empties = b.count_empty();

    Endgame eg;
    int score;
    auto start_time = Clock::now();
    int m = eg.solve_endgame(b, &e, side, lm, false, empties, 100000000, &score);
    uint64_t ms = get_time_elapsed(start_time);
    total_time += ms;
    total_nodes += eg.nodes;
    if (ms < min_time) min_time = ms;
    if (ms > max_time) max_time = ms;
    if (/*m != move_sol || */score != score_sol) {
      std::cerr << "Error: incorrect solution " << move_sol << " " << m << " " << score_sol << " " << score << std::endl;
      return;
    }
  }

  std::cerr << "Nodes: " << total_nodes << std::endl;
  std::cerr << "Total time: " << total_time << std::endl;
  std::cerr << "Min time: " << min_time << " max time: " << max_time << " avg time: " << total_time / 500 << std::endl;
  std::cerr << "NPS: " << 1000 * total_nodes / total_time << std::endl;
  std::cerr << "Time with overhead: " << get_time_elapsed(overhead) << std::endl;
}

void eval_acc(int depth) {
  std::vector<std::string> positions;
  std::ifstream cfile("ffotest/eg_17_18.txt");
  std::string line;
  while (getline(cfile, line))
    positions.push_back(line);
  cfile.close();
  cfile.open("ffotest/eg_19_20.txt");
  while (getline(cfile, line))
    positions.push_back(line);
  cfile.close();
  cfile.open("ffotest/eg_21_22.txt");
  while (getline(cfile, line))
    positions.push_back(line);

  uint64_t nodes = 0;
  auto overhead = Clock::now();
  int lin_err = 0;
  int sq_err = 0;
  for (unsigned int i = 0; i < positions.size(); i++) {
    std::string pos = positions[i];
    char board[64];
    const char *read = pos.c_str();
    for (int j = 0; j < 64; j++)
      board[j] = read[j];

    std::vector<std::string> parts = split(pos, ' ');

    Color side = BLACK;
    if (parts[1] == "Black") {
      side = BLACK;
    } else {
      side = WHITE;
    }

    // int move_sol = std::stoi(parts[2]);
    int score_sol = std::stoi(parts[3]);
    if (side == WHITE) score_sol = -score_sol;

    Board b(board);
    ArrayList lm = b.legal_movelist(side);

    Hash transpositionTable(12);
    SearchInfo search_info;
    search_info.time_limit = 0;
    search_info.tt = &transpositionTable;
    search_info.other_heuristic = true;
    Eval e;
    init_evaluator(b, &e);
    int ss_score;
    for (int ss_depth = 1; ss_depth <= depth; ss_depth++) {
      int ss_move = pvs_best_move(b, &e, side, lm, &ss_score, ss_depth, &search_info);
      lm.swap(ss_move, 0);
    }
    ss_score /= EVAL_SCALE_FACTOR;

    nodes += search_info.nodes;
    lin_err += score_sol - ss_score;
    sq_err += (ss_score - score_sol) * (ss_score - score_sol);
  }

  std::cerr << "Nodes: " << nodes << std::endl;
  std::cerr << "Linear error: " << (double) lin_err / positions.size() << std::endl;
  std::cerr << "Squared error: " << (double) sq_err / positions.size() << std::endl;
  std::cerr << "Time with overhead: " << get_time_elapsed(overhead) << std::endl;
}

/*
 * Array of PERFT results from http://www.aartbik.com/MISC/reversi.html
 *
 DEPTH  #LEAF NODES   #FULL-DEPTH  #HIGHER
==========================================
   1            4
   2           12
   3           56
   4          244
   5         1396
   6         8200
   7        55092
   8       390216
   9      3005288
  10     24571284
  11    212258800  =    212258572  +    228
  12   1939886636  =   1939886052  +    584
  13  18429641748  =  18429634780  +   6968 
  14 184042084512  = 184042061172  +  23340
*/

// Performs a PERFT, which enumerates all possible lines of play up to a
// certain number of plies. Useful for debugging the move generator and testing
// speed/performance.
uint64_t perft(Board &b, Color c, int depth, bool passed) {
  if (depth == 0)
    return 1;

  uint64_t nodes = 0;
  ArrayList lm = b.legal_movelist(c);

  if (lm.size() == 0) {
    if (passed) return 1;

    nodes += perft(b, ~c, depth-1, true);
    return nodes;
  }

  for (int i = 0; i < lm.size(); i++) {
    Board copy = b.copy();
    copy.do_move(c, lm.get(i));
    nodes += perft(copy, ~c, depth-1, false);
  }

  return nodes;
}

// Splits a string s with delimiter d.
std::vector<std::string> split(const std::string &s, char d) {
    std::vector<std::string> v;
    std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, d)) {
        v.push_back(item);
    }
    return v;
}
