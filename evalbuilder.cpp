#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include "board.h"
#include "common.h"
#include "endgame.h"
#include "eval.h"
#include "patternbuilder.h"
#include "player.h"
#include "search.h"
using namespace std;

#define TSPLITS 7
#define IOFFSET 10
#define TURNSPERDIV 8
const int N_SPLITS = 61;
const int split_limit = 0;

const double reg_lambda = 0.0075;
// const double reg_lambda = 0;
const auto update = [](double* entry, double f, double l_rate) {
  *entry += (f - *entry * reg_lambda) * l_rate;
};
const auto freq_l_rate = [](double l_rate, double freq) -> double {
  return l_rate * freq;
};

struct Sample {
  Board b;
  int score;

  Sample() = default;
  Sample(string& board_str, string& score_str)
    : b(board_str), score(std::stoi(score_str)) {}
};

struct PatternSample {
  int score;
  Eval e;

  PatternSample() = default;
  PatternSample(int s)
    : score(s) {}
};

thor_game **games;
unsigned int totalSize;

std::vector<std::string> endgame_pos;

const int N_PATTERNS = 12;
const int N_WEIGHTS = 4*59049 + 19683 + 4*6561 + 2187 + 729 + 243;
enum PatternType {
  P25, E2X, TRAPZ, P33, PM, LINE2, LINE3, LINE4, DIAG8, DIAG7, DIAG6, DIAG5
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

double **pvTable;
double *freq;

std::vector<std::string> split(const std::string &s, char d);
std::vector<string> read_all_training_data();
void read_all_tables(bool new_file = false);
void write_weights();
void freemem();

// Calculate regularization term for gradient descent
double regularization();
// Get random bench positions
void randomPositions();


double pos_err(Eval& e, int t, int score) {
  double result = 0.0;
  for (int i = 0; i < N_OCC; i++) {
    result += pvTable[t][e.patterns[i]];
  }
  return result / 11.0 - score;
}

void iter(PatternSample& data, double l_rate, int t) {
  double err = pos_err(data.e, t, data.score);
  for (int i = 0; i < N_OCC; i++) {
    update(&(pvTable[t][data.e.patterns[i]]), -err, freq_l_rate(l_rate, freq[data.e.patterns[i]]));
  }
}

std::vector<double> lin_error(std::vector<PatternSample>& positions, int t) {
  std::vector<double> errors;
  for (auto& s : positions) {
    errors.push_back(pos_err(s.e, t, s.score));
  }
  return errors;
}

double total_error(std::vector<PatternSample>& positions, int t, bool use_reg) {
  double error = 0.0;
  for (auto& s : positions) {
    double abs_err = abs(pos_err(s.e, t, s.score));
    error += abs_err * abs_err;
    // Mirror board
    // Board m(reflect_vert(reflect_hor(s.b.get_bits(WHITE))), reflect_vert(reflect_hor(s.b.get_bits(BLACK))));
    // abs_err = abs(pos_err(m, t, s.score));
    // error += abs_err * abs_err;
    // Mirror colors
    // Board m(s.b.get_bits(BLACK), s.b.get_bits(WHITE));
    // abs_err = abs(pos_err(m, t, -s.score));
    // error += abs_err * abs_err;
    // Mirror both
    // m = Board(reflect_vert(reflect_hor(s.b.get_bits(BLACK))), reflect_vert(reflect_hor(s.b.get_bits(WHITE))));
    // abs_err = abs(pos_err(m, t, -s.score));
    // error += abs_err * abs_err;
  }
  // error /= 2;
  if (use_reg)
    error += regularization() / 2;
  return error / positions.size();
}

void searchFeatures(std::vector<Sample>& positions, int t, double base_l_rate, int n_epochs) {
  if (positions.size() < 100000) {
    cerr << "Too few positions: " << positions.size() << endl;
    return;
  }
  cerr << "Searching features with " << positions.size() << " positions." << endl;
  std::default_random_engine rng(time(NULL));
  std::default_random_engine det(time(NULL));
  // Split training, validation. No test set necessary since selfplay is the real test.
  std::vector<Sample> training_raw, validation_raw;
  std::uniform_int_distribution<int> data_split(0, 99);
  for (auto& s : positions) {
    int r = data_split(det);
    if (r < 85)
      training_raw.push_back(s);
    else
      validation_raw.push_back(s);
  }
  // cerr << "Training: " << training_raw.size() << " validation: " << validation_raw.size() << endl;

  std::cerr << "Getting patterns from data" << std::endl;
  std::vector<PatternSample> training, validation;
  for (auto& s : training_raw) {
    PatternSample ps(s.score);
    init_evaluator(s.b, &(ps.e));
    training.push_back(ps);
  }
  for (auto& s : validation_raw) {
    PatternSample ps(s.score);
    init_evaluator(s.b, &(ps.e));
    validation.push_back(ps);
  }

  // Get all pattern frequencies
  std::cerr << "Getting pattern frequences" << std::endl;
  // Reset frequencies
  for (int j = 0; j < N_WEIGHTS; j++) freq[j] = 0;
  for (auto& s : training) {
    // pos_err(s.b, i, s.score, true);
    for (int j = 0; j < N_OCC; j++) {
      freq[s.e.patterns[j]]++;
    }
  }
  for (int j = 0; j < N_WEIGHTS; j++) {
    if (freq[j] > 0.5) {
      freq[j] = 7 / std::max(7.0, sqrt(freq[j]));
    }
  }

  std::uniform_int_distribution<int> rand_train(0, (int) training.size() - 1);
  // EPOCH_SIZE ~ training.size / 2
  // learning rate ~ [25, 200]
  // epoch ~ [50, 150]
  const int EPOCH_SIZE = training.size();
  double l_rate = base_l_rate / sqrt(training.size());

  double prev_error = 100000;
  int err_inc = 0;
  int stop = 0;
  for (int epoch = 0; epoch < n_epochs; epoch++) {
    // Calculate error
    double abs_err = total_error(validation, t, false);
    // double abs_err = total_error(training, false);
    if (epoch % 5 == 0)
      cerr << "Error: " << abs_err << endl;
    if (abs_err > prev_error) {
      err_inc++;
      if (err_inc >= 2) {
        cerr << "Error increased, lowering learning rate." << endl;
        l_rate /= 4;
        stop++;
        if (stop > 1) break;
      }
    } else {
      err_inc = 0;
    }
    prev_error = abs_err;
    for (int i = 0; i < EPOCH_SIZE; i++) {
      PatternSample& s = training[rand_train(rng)];
      iter(s, l_rate, t);
      // Mirror board
      // Sample m;
      // m.b = Board(reflect_vert(reflect_hor(s.b.get_bits(WHITE))), reflect_vert(reflect_hor(s.b.get_bits(BLACK))));
      // m.score = s.score;
      // iter(m, l_rate/4);
      // Mirror colors
      // m.b = Board(s.b.get_bits(BLACK), s.b.get_bits(WHITE));
      // m.score = -s.score;
      // iter(m, l_rate/2);
      // Mirror both
      // m.b = Board(reflect_vert(reflect_hor(s.b.get_bits(BLACK))), reflect_vert(reflect_hor(s.b.get_bits(WHITE))));
      // iter(m, l_rate/4);
    }
  }
}

void selectPositions() {
  std::default_random_engine rng(1234);
  std::uniform_int_distribution<int> distribution1(0, 1);
  // Generate a deterministic permutation
  int perm[16400];
  // Fisher-Yates shuffle
  for (unsigned int i = 0; i < 16400; i++) {
    std::uniform_int_distribution<int> distribution(0, i);
    int j = distribution(rng);
    perm[i] = perm[j];
    perm[j] = i;
  }
  // Default at section 0 is depth 13 to 14 with occasional 15.
  int section = 4;
  resize_endhash(5 + 2 * section);
  for (int i = 500 * section; i < 500 * (section + 1); i++) {
    cerr << "Selecting: " << i << endl;
    thor_game *game = games[perm[i]];
    if (game == NULL)
      continue;

    Board tracker;
    Color side = BLACK;
    // play opening moves
    int limit = 46 - 2 * section + distribution1(rng);
    for (int j = 0; j < limit; j++) {
      // If one side must pass it is not indicated in the database?
      if (!tracker.is_legal(side, game->moves[j])) {
        side = ~side;
      }
      tracker.do_move(side, game->moves[j]);
      side = ~side;
    }

    Endgame eg;
    if (tracker.count_empty() > 15 + section * 2) {
      // games[i] = NULL;
      continue;
    }

    ArrayList lm = tracker.legal_movelist(side);
    if (lm.size() == 0) {
      // games[i] = NULL;
      continue;
    }

    int score = 0;
    Eval e;
    init_evaluator(tracker, &e);
    int m = eg.solve_endgame(tracker, &e, side, lm, false, tracker.count_empty(), 10000000, &score);
    // We want everything from black's POV
    if (side == WHITE)
      score = -score;

    std::string test_pos = tracker.to_string() + " " + (side == BLACK ? "Black" : "White")
        + " " + std::to_string(m) + " " + std::to_string(score);
    // std::cerr << test_pos << std::endl;
    endgame_pos.push_back(test_pos);
  }

  ofstream out;
  out.open("eg.txt");
  for (unsigned int i = 0; i < endgame_pos.size(); i++) {
    out << endgame_pos[i] << endl;
  }
  out.close();
}

void read_book_positions(std::vector<string>& positions, std::string file_name) {
  std::string line;
  std::ifstream cfile(file_name);

  if (cfile.is_open()) {
    while (getline(cfile, line)) {
      positions.push_back(string(line));
    }
    cfile.close();
  }
  cerr << positions.size() << " positions read." << endl;
}

string board_to_tb(Board& b) {
  std::stringstream stream;
  stream << "0x" << std::setfill('0') << std::setw(16) << std::hex << b.occupied();
  std::string taken(stream.str());
  std::stringstream stream2;
  stream2 << "0x" << std::setfill('0') << std::setw(16) << std::hex << b.get_bits(BLACK);
  std::string black(stream2.str());

  std::string result = taken + " " + black;
  return result;
}

void perft_write(Board &b, Color c, int depth, bool passed, std::ofstream& out) {
  if (depth == 0) {
    out << board_to_tb(b) << endl;
    return;
  }

  ArrayList lm = b.legal_movelist(c);
  if (lm.size() == 0) {
    if (passed) {
      out << board_to_tb(b) << endl;
      return;
    }

    perft_write(b, ~c, depth-1, true, out);
    return;
  }

  for (int i = 0; i < lm.size(); i++) {
    Board copy = b.copy();
    copy.do_move(c, lm.get(i));
    perft_write(copy, ~c, depth-1, false, out);
  }
}

bool analyze_book_balance(const string &position, int depth, int bound) {
  std::string pos = position;
  std::string::size_type sz = 0;
  uint64_t takenBits = std::stoull(pos, &sz, 0);
  pos = pos.substr(sz);
  uint64_t blackBits = std::stoull(pos, &sz, 0);
  pos = pos.substr(sz);
  Board b(takenBits^blackBits, blackBits);

  Hash transpositionTable(10);

  SearchInfo search_info;
  search_info.root_age = 64 - b.count_empty();
  search_info.time_limit = 0;
  search_info.tt = &transpositionTable;
  search_info.other_heuristic = false;
  Eval e;
  init_evaluator(b, &e);
  int score = pvs(b, &e, BLACK, depth, -INFTY, INFTY, false, &search_info);
  score -= 1500;
  return abs(score) < bound;
}

int label_training(const string &position) {
  Board b(position.substr(0, 64));
  Color c = static_cast<Color>(std::stoi(position.substr(65, 1)));

  // Run game on one side
  Player p1(c, false, 12);
  Player p2(~c, false, 12);
  p1.set_depths(14, 22);
  p2.set_depths(14, 22);
  p1.game = b;
  p2.game = b;

  int m = MOVE_NULL;
  bool passed = false;
  while (true) {
    m = p1.do_move(m, -1);
    // two passes in a row is end of game
    if (m == MOVE_NULL) {
      if (passed) break;
      passed = true;
    } else {
      passed = false;
    }

    m = p2.do_move(m, -1);
    if (m == MOVE_NULL) {
      if (passed) break;
      passed = true;
    } else {
      passed = false;
    }
  }

  int bf = p1.game.count(BLACK);
  int wf = p1.game.count(WHITE);
  // cout << bf << "-" << wf << endl;
  return bf - wf;
}

void label_training_rec(const string &position, ofstream* out) {
  Board b(position.substr(0, 64));
  Color c = static_cast<Color>(std::stoi(position.substr(65, 1)));

  // Run game on one side
  Player p1(c, false, 14);
  Player p2(~c, false, 14);
  p1.set_depths(18, 26);
  p2.set_depths(18, 26);
  p1.game = b;
  p2.game = b;

  // Record game moves
  int game_moves[128];
  int turns = 0;

  int m = MOVE_NULL;
  bool passed = false;
  while (true) {
    m = p1.do_move(m, -1);
    game_moves[turns++] = m;
    // two passes in a row is end of game
    if (m == MOVE_NULL) {
      if (passed) break;
      passed = true;
    } else {
      passed = false;
    }

    m = p2.do_move(m, -1);
    game_moves[turns++] = m;
    if (m == MOVE_NULL) {
      if (passed) break;
      passed = true;
    } else {
      passed = false;
    }
  }

  int bf = p1.game.count(BLACK);
  int wf = p1.game.count(WHITE);
  *out << b.to_string() << " " << c << " " << bf - wf << endl;
  // Record all but the last move
  for (int i = 0; i < turns-3; i++) {
    if (game_moves[i] != MOVE_NULL) {
      b.do_move(c, game_moves[i]);
      c = ~c;
      *out << b.to_string() << " " << c << " " << bf - wf << endl;
    } else {
      c = ~c;
    }
  }
}

int pvs_prune(const string& position, int depth) {
  Board b(position.substr(0, 64));
  Color c = static_cast<Color>(std::stoi(position.substr(65, 1)));

  Hash transpositionTable(10);
  SearchInfo search_info;
  search_info.root_age = 64 - b.count_empty();
  search_info.time_limit = 0;
  search_info.tt = &transpositionTable;
  search_info.other_heuristic = false;
  Eval e;
  init_evaluator(b, &e);
  int score = pvs(b, &e, c, depth, -INFTY, INFTY, false, &search_info);
  return score;
}

std::string rand_mg_game(std::default_random_engine& rng, int min_ply, int max_ply, int* score, Color* final_color) {
  // Label position from this ply
  std::uniform_int_distribution<int> distribution2(min_ply, max_ply);
  int empty_end = distribution2(rng);
  // Randomness in generated data
  std::uniform_int_distribution<int> distribution1(49, 54);
  int empty_start = std::max(distribution1(rng), empty_end);
  // Low depth to simulate search tree?
  std::uniform_int_distribution<int> distribution3(1, std::max(20, std::min(10, empty_start - empty_end)));
  int low_depth_start = empty_end + distribution3(rng);
  Board b;
  Color c = BLACK;
  bool passed = false;
  while (b.count_empty() > empty_start) {
    ArrayList legal_movelist = b.legal_movelist(c);
    if (legal_movelist.size() == 0) {
      if (passed)
        break;
      passed = true;
      c = ~c;
      continue;
    }
    std::uniform_int_distribution<int> rand_move(0, legal_movelist.size()-1);
    b.do_move(c, legal_movelist.get(rand_move(rng)));
    c = ~c;
    passed = false;
  }

  if (passed) {
    return rand_mg_game(rng, min_ply, max_ply, score, final_color);
  }

  Player p1(c, false, 10);
  Player p2(~c, false, 10);
  Player* ptm = &p1;
  Player* other = &p2;
  p1.set_depths(10, 12);
  p2.set_depths(10, 12);
  p1.game = b;
  p2.game = b;

  int m = MOVE_NULL;
  while (b.count_empty() > empty_end) {
    if (b.count_empty() <= low_depth_start) {
      int search_depth = std::max(1, (low_depth_start - empty_end) / 2 - 1);
      p1.set_depths(search_depth, search_depth + 1);
      p2.set_depths(search_depth, search_depth + 1);
    }

    m = ptm->do_move(m, -1);
    b.do_move(c, m);
    c = ~c;
    // two passes in a row is end of game
    if (m == MOVE_NULL) {
      if (passed) break;
      passed = true;
    } else {
      passed = false;
    }
    Player* temp = ptm;
    ptm = other;
    other = temp;
  }

  ArrayList lm = b.legal_movelist(c);
  if (lm.size() == 0) {
    return rand_mg_game(rng, min_ply, max_ply, score, final_color);
  }

  std::string pos = b.to_string();
  *score = label_training(pos + " " + std::to_string(c));
  *final_color = c;
  return pos;
}

void rand_eg_game(std::default_random_engine& rng, int min_ply, int max_ply, std::ofstream* out) {
  std::uniform_int_distribution<int> distribution1(47, 52);
  int empty_start = distribution1(rng);
  Board b;
  Color c = BLACK;
  bool passed = false;
  while (b.count_empty() > empty_start) {
    ArrayList legal_movelist = b.legal_movelist(c);
    if (legal_movelist.size() == 0) {
      if (passed)
        break;
      passed = true;
      c = ~c;
      continue;
    }
    std::uniform_int_distribution<int> distribution2(0, legal_movelist.size()-1);
    b.do_move(c, legal_movelist.get(distribution2(rng)));
    c = ~c;
    passed = false;
  }

  if (passed) {
    rand_eg_game(rng, min_ply, max_ply, out);
    return;
  }

  std::uniform_int_distribution<int> distribution2(min_ply, max_ply);
  int empty_end = distribution2(rng) + 1;
  Player p1(c, false, 10);
  Player p2(~c, false, 10);
  Player* ptm = &p1;
  Player* other = &p2;
  p1.set_depths(10, 12);
  p2.set_depths(10, 12);
  p1.game = b;
  p2.game = b;

  // Low depth to simulate search tree?
  std::uniform_int_distribution<int> distribution3(-8, 8);
  int low_depth_start = max_ply + distribution3(rng);

  int m = MOVE_NULL;
  while (b.count_empty() > empty_end) {
    if (b.count_empty() <= low_depth_start) {
      if (b.count_empty() <= 15) {
        p1.set_depths(1, 1);
        p2.set_depths(1, 1);
      } else {
        p1.set_depths(2, 1);
        p2.set_depths(2, 1);
      }
    }

    m = ptm->do_move(m, -1);
    b.do_move(c, m);
    c = ~c;
    // two passes in a row is end of game
    if (m == MOVE_NULL) {
      if (passed) break;
      passed = true;
    } else {
      passed = false;
    }
    Player* temp = ptm;
    ptm = other;
    other = temp;
  }

  if (b.count_empty() == 0) {
    *out << b.to_string() << " " << c << " " << b.count(BLACK) - b.count(WHITE) << endl;
    return;
  }
  if (b.count_empty() > 26) {
    rand_eg_game(rng, min_ply, max_ply, out);
    return;
  }

  ArrayList lm = b.legal_movelist(c);
  while (lm.size() == 0) {
    if (passed) return;
    passed = true;
    c = ~c;
    lm = b.legal_movelist(c);
  }

  // Use the same hashtable for all solves
  Endgame eg;
  for (int i = 0; i < lm.size(); i++) {
    // passed = false;
    Board copy(b);
    copy.do_move(c, lm.get(i));
    ArrayList next_lm = copy.legal_movelist(~c);
    Color eg_color = ~c;

    if (next_lm.size() == 0) {
      next_lm = copy.legal_movelist(c);
      // passed = true;
      eg_color = c;
      if (next_lm.size() == 0) continue;
    }

    Eval e;
    init_evaluator(copy, &e);
    int score;
    eg.solve_endgame(copy, &e, eg_color, next_lm, false, copy.count_empty(), 10000000, &score);
    // We want everything from black's POV
    if (eg_color == WHITE)
      score = -score;
    // Always record as ~c's move regardless of whether a pass was handled.
    *out << copy.to_string() << " " << ~c << " " << score << endl;
  }
}

void rand_full_game(std::default_random_engine& rng, int min_ply, int max_ply, ofstream* out) {
  std::uniform_int_distribution<int> distribution1(min_ply / 2, max_ply / 2);
  int empty_end = distribution1(rng) * 2;
  Board b;
  Color c = BLACK;
  bool passed = false;
  while (b.count_empty() > empty_end) {
    ArrayList legal_movelist = b.legal_movelist(c);
    if (legal_movelist.size() == 0) {
      if (passed)
        break;
      passed = true;
      c = ~c;
      continue;
    }
    std::uniform_int_distribution<int> distribution2(0, legal_movelist.size()-1);
    b.do_move(c, legal_movelist.get(distribution2(rng)));
    c = ~c;
    passed = false;
  }

  ArrayList lm = b.legal_movelist(c);
  if (lm.size() == 0) {
    rand_full_game(rng, min_ply, max_ply, out);
    return;
  }

  std::string pos = b.to_string();
  label_training_rec(pos + " " + std::to_string(c), out);
}

std::vector<string> filter_empty(const std::vector<string>& positions, int min, int max) {
  cerr << "Filtering for positions from " << min << " to " << max << " empties" << endl;
  std::vector<string> filtered;
  for (unsigned int i = 0; i < positions.size(); i++) {
    std::vector<string> info = split(positions[i], ' ');
    std::string s = info[0];
    int n = (int) std::count(s.begin(), s.end(), '-');
    if (n >= min && n <= max)
      filtered.push_back(positions[i]);
  }
  std::cerr << "Filtered " << positions.size() << " to " << filtered.size() << " positions." << std::endl;
  return filtered;
}

std::vector<string> filter_remove_one(const std::vector<string>& positions, int empties) {
  cerr << "Filtering out positions with " << empties << " empties" << endl;
  std::vector<string> filtered;
  for (unsigned int i = 0; i < positions.size(); i++) {
    std::vector<string> info = split(positions[i], ' ');
    std::string s = info[0];
    int n = (int) std::count(s.begin(), s.end(), '-');
    if (n != empties)
      filtered.push_back(positions[i]);
  }
  std::cerr << "Filtered " << positions.size() << " to " << filtered.size() << " positions." << std::endl;
  return filtered;
}

std::vector<Sample> process_data(const std::vector<string>& positions) {
  std::vector<Sample> samples;
  samples.reserve(positions.size());
  for (const auto& s : positions) {
    std::vector<string> info = split(s, ' ');
    samples.push_back(Sample(info[0], info[2]));
  }
  return samples;
}

std::vector<Sample>** equalize_plies(std::vector<Sample>& data) {
  std::cerr << "Equalizing ply pairs";
  std::vector<Sample>** equalized = new std::vector<Sample>*[N_SPLITS];
  for (int i = 0; i < 31; i++) {
    std::cerr << ".";
    std::vector<Sample> even, odd;
    int even_t = 2 * i, odd_t = 2 * i + 1;
    // Separate the even and odd positions for this pair
    for (auto s : data) {
      int turn = 60 - s.b.count_empty();
      if (turn == even_t)
        even.push_back(s);
      else if (turn == odd_t)
        odd.push_back(s);
    }

    std::default_random_engine rng(time(NULL));
    if (i == 30) {
      std::vector<Sample>* pair = new std::vector<Sample>();
      pair->insert(pair->begin(), even.begin(), even.end());
      equalized[2*i] = pair;
    }
    else {
      // Remove random until equal number of even and odd positions.
      while (even.size() > odd.size()) {
        std::uniform_int_distribution<unsigned int> dist(0, even.size()-1);
        even[dist(rng)] = even[even.size()-1];
        even.pop_back();
      }
      while (odd.size() > even.size()) {
        std::uniform_int_distribution<unsigned int> dist(0, odd.size()-1);
        odd[dist(rng)] = odd[odd.size()-1];
        odd.pop_back();
      }
      std::vector<Sample>* pair = new std::vector<Sample>();
      pair->insert(pair->begin(), even.begin(), even.end());
      equalized[2*i] = pair;
      pair = new std::vector<Sample>();
      pair->insert(pair->begin(), odd.begin(), odd.end());
      equalized[2*i+1] = pair;
    }

    // Equalize white vs. black scores
    int total_score = 0;
    for (auto& s : *equalized[2*i])
      total_score += s.score;
    std::cerr << 10 * total_score / ((int) equalized[2*i]->size() + 1);
    if (i < 30) {
      total_score = 0;
      for (auto& s : *equalized[2*i+1])
        total_score += s.score;
      std::cerr << "," << 10 * total_score / ((int) equalized[2*i+1]->size() + 1);
    }
    // while (total_score < -64) {
    //   std::uniform_int_distribution<unsigned int> dist(0, pair->size()-1);
    //   unsigned int idx = dist(rng);
    //   while ((*pair)[idx].score >= 0) {
    //     idx = dist(rng);
    //   }
    //   total_score -= (*pair)[idx].score;
    //   (*pair)[idx] = (*pair)[pair->size()-1];
    //   pair->pop_back();
    // }
    // while (total_score > 64) {
    //   std::uniform_int_distribution<unsigned int> dist(0, pair->size()-1);
    //   unsigned int idx = dist(rng);
    //   while ((*pair)[idx].score <= 0) {
    //     idx = dist(rng);
    //   }
    //   total_score -= (*pair)[idx].score;
    //   (*pair)[idx] = (*pair)[pair->size()-1];
    //   pair->pop_back();
    // }
    // equalized[i] = pair;
  }
  std::cerr << ".done." << std::endl;
  return equalized;
}

int main(int argc, char **argv) {
  init_eval();

  if (argc == 2 && std::string(argv[1]) == "perft") {
    ofstream out;
    out.open("perft8.txt");
    Board b;
    perft_write(b, BLACK, 8, false, out);
    out.close();
    return 0;
  }

  if (argc == 2 && std::string(argv[1]) == "book") {
    std::vector<string> positions;
    // read_book_positions(positions, "perft8.txt");
    read_book_positions(positions, "perft8_balanced_2_new.txt");
    std::vector<string> valid;
    for (unsigned int i = 0; i < positions.size(); i++) {
      if (i % 5000 == 0) std::cerr << i+1 << std::endl;
      // if (analyze_book_balance(positions[i], 4, 8 * 1200)) {
      // if (analyze_book_balance(positions[i], 6, 4 * 1200)) {
      if (analyze_book_balance(positions[i], 14, 4 * 1200 / 3)) {
      // if (analyze_book_balance(positions[i], 16, 3 * 1200 / 2)) {
        valid.push_back(positions[i]);
      }
    }

    ofstream out;
    out.open("perft8_balanced_3_new.txt");
    for (unsigned int i = 0; i < valid.size(); i++) {
      out << valid[i] << endl;
    }
    out.close();
    return 0;
  }

  if (argc == 2 && std::string(argv[1]) == "label") {
    resize_endhash(8);
    std::string output_filename = "training200727.txt";

    std::ifstream labelled(output_filename);
    int lines = 0;
    if (labelled.is_open()) {
      lines = std::count(std::istreambuf_iterator<char>(labelled), 
                         std::istreambuf_iterator<char>(), '\n');
      labelled.close();
    }

    std::vector<string> positions;
    read_book_positions(positions, "randbook200727.txt");
    ofstream out;
    out.open(output_filename, std::ofstream::app);
    for (unsigned int i = lines; i < positions.size(); i++) {
      if (i % 100 == 0) std::cerr << i+1 << std::endl;
      int score = label_training(positions[i]);
      out << positions[i] << " " << score << endl;
    }
    out.close();
    return 0;
  }

  if (argc >= 2 && std::string(argv[1]) == "eg_training") {
    std::default_random_engine rng(time(NULL));
    resize_endhash(13);
    std::string output_filename;
    if (argc == 3) {
      output_filename = "training-eg" + std::string(argv[2]) + ".txt";
    } else {
      output_filename = "training-eg.txt";
    }
    ofstream out(output_filename, std::ofstream::app);
    for (int i = 0; i < 5000000; i++) {
      if (i % 1000 == 0) std::cerr << i+1 << std::endl;
      rand_eg_game(rng, 1, 22, &out);
    }
    out.close();
    return 0;
  }

  if (argc >= 2 && std::string(argv[1]) == "mg_training") {
    std::default_random_engine rng(time(NULL));
    resize_endhash(11);
    std::string output_filename;
    if (argc == 3) {
      output_filename = "training-mg" + std::string(argv[2]) + ".txt";
    } else {
      output_filename = "training-mg.txt";
    }
    ofstream out(output_filename, std::ofstream::app);
    for (int i = 0; i < 5000000; i++) {
      if (i % 100 == 0) std::cerr << i+1 << std::endl;
      int score = 0;
      Color c = BLACK;
      std::string pos = rand_mg_game(rng, 23, 52, &score, &c);
      out << pos << " " << c << " " << score << endl;
    }
    out.close();
    return 0;
  }

  if (argc >= 2 && std::string(argv[1]) == "full_training") {
    std::default_random_engine rng(time(NULL));
    resize_endhash(14);
    std::string output_filename;
    if (argc == 3) {
      output_filename = "training-full" + std::string(argv[2]) + ".txt";
    } else {
      output_filename = "training-full.txt";
    }
    ofstream out(output_filename, std::ofstream::app);
    for (int i = 0; i < 5000000; i++) {
      if (i % 100 == 0) std::cerr << i+1 << std::endl;
      rand_full_game(rng, 44, 50, &out);
    }
    out.close();
    return 0;
  }

  totalSize = 0;
  games = new thor_game*[217000];

  if (argc == 2 && std::string(argv[1]) == "eg_ffo") {
    readFlippyGame("games/tuneoutput-200723-eg.txt", totalSize, 16400, games);
    checkGames(totalSize, games);
    selectPositions();
    // randomPositions();
    return 0;
  }

  pvTable = new double*[N_SPLITS];
  for (int i = 0; i < N_SPLITS; i++) {
    pvTable[i] = new double[N_WEIGHTS];
  }
  freq = new double[N_WEIGHTS];
  for (int j = 0; j < N_WEIGHTS; j++) freq[j] = 0;

  if (argc == 2 && std::string(argv[1]) == "pvs_prune") {
    std::vector<string> positions;
    read_book_positions(positions, "games/training200727.txt");
    // read_book_positions(positions, "games/training200729.txt");
    // read_book_positions(positions, "games/training-mg200823.txt");
    // read_book_positions(positions, "games/training-eg200830.txt");
    read_all_tables();
    std::vector<int> error4, error3, error2, error1;
    int i = 0;
    for (const auto& p : positions) {
      if ((i % 10000) == 0) cerr << i << endl;
      // int score_4 = pvs_prune(p, 8);
      int score_3 = pvs_prune(p, 3);
      int score_2 = pvs_prune(p, 2);
      int score_1 = pvs_prune(p, 1);
      int score_0 = pvs_prune(p, 0);
      error1.push_back(score_1 - score_0);
      error2.push_back(score_2 - score_0);
      error3.push_back(score_3 - score_0);
      error4.push_back(0);
      i++;
    }
    std::sort(error1.begin(), error1.end());
    std::sort(error2.begin(), error2.end());
    std::sort(error3.begin(), error3.end());
    std::sort(error4.begin(), error4.end());
    // Look at 10%iles of high d - low d for beta margins
    // Look at 90%iles for alpha margins
    cerr << "  min    5     10     q1    med    q3     90     95    max" << endl;
    cerr << error1[0] << " " << error1[error1.size() / 20]
         << " " << error1[error1.size() / 10] << " " << error1[error1.size() / 4]
         << " " << error1[error1.size() / 2] << " " << error1[3 * error1.size() / 4]
         << " " << error1[9 * error1.size() / 10] << " " << error1[19 * error1.size() / 20] << " " << error1[error1.size()-1] << endl;
    cerr << error2[0] << " " << error2[error2.size() / 20]
         << " " << error2[error2.size() / 10] << " " << error2[error2.size() / 4]
         << " " << error2[error2.size() / 2] << " " << error2[3 * error2.size() / 4]
         << " " << error2[9 * error2.size() / 10] << " " << error2[19 * error2.size() / 20] << " " << error2[error2.size()-1] << endl;
    cerr << error3[0] << " " << error3[error3.size() / 20]
         << " " << error3[error3.size() / 10] << " " << error3[error3.size() / 4]
         << " " << error3[error3.size() / 2] << " " << error3[3 * error3.size() / 4]
         << " " << error3[9 * error3.size() / 10] << " " << error3[19 * error3.size() / 20] << " " << error3[error3.size()-1] << endl;
    cerr << error4[0] << " " << error4[error4.size() / 20]
         << " " << error4[error4.size() / 10] << " " << error4[error4.size() / 4]
         << " " << error4[error4.size() / 2] << " " << error4[3 * error4.size() / 4]
         << " " << error4[9 * error4.size() / 10] << " " << error4[19 * error4.size() / 20] << " " << error4[error4.size()-1] << endl;
    return 0;
  }

  if (argc >= 2 && std::string(argv[1]) == "bias") {
    bool new_file = false;
    if (argc == 3 && (std::string(argv[2]) == "new")) new_file = true;
    read_all_tables(new_file);
    std::cerr << "Calculating all weight biases" << std::endl;
    std::cerr << std::setprecision(3);
    for (int i = 0; i < N_SPLITS; i++) {
      for (int j = 0; j < N_PATTERNS; j++) {
        const PatternInfo& pi = patternInfo[j];
        double total = 0.0;
        for (int k = 0; k < pi.size; k++) {
          total += pvTable[i][pi.offset + k];
        }
        double avg_bias = total / pi.size;
        std::cerr << avg_bias << " ";

        // Remove bias
        for (int k = 0; k < pi.size; k++) {
          pvTable[i][pi.offset + k] -= avg_bias;
        }
      }
      std::cerr << std::endl;
    }
    // write_weights();
    return 0;
  }

  // Print weights alongside frequencies for debugging.
  // if (argc == 2 && std::string(argv[1]) == "print_w") {
  //   read_all_tables();

  //   int frequencies[243];
  //   for (int i = 0; i < 243; i++) frequencies[i] = 0;
  //   std::vector<string> positions = read_all_training_data();
  //   std::vector<Sample> training_data = process_data(positions);
  //   positions.clear();
  //   std::vector<Sample>** equalized_data = equalize_plies(training_data);
  //   training_data.clear();
  //   for (Sample& s : *(equalized_data[28])) {
  //     uint64_t black = s.b.get_bits(BLACK);
  //     uint64_t white = s.b.get_bits(WHITE);
  //     int du5 = bitsToPI( (int)(((black & 0x0000008040201008ULL) * 0x0101010101010101ULL) >> 59),
  //                (int)(((white & 0x0000008040201008ULL) * 0x0101010101010101ULL) >> 59) );
  //     int dl5 = bitsToPI( (int)(((black & 0x1008040201000000ULL) * 0x0101010101010101ULL) >> 56),
  //                (int)(((white & 0x1008040201000000ULL) * 0x0101010101010101ULL) >> 56) );
  //     int adu5 = bitsToPI( (int)(((black & 0x0000000102040810ULL) * 0x0101010101010101ULL) >> 56),
  //               (int)(((white & 0x0000000102040810ULL) * 0x0101010101010101ULL) >> 56) );
  //     int adl5 = bitsToPI( (int)(((black & 0x0810204080000000ULL) * 0x0101010101010101ULL) >> 59),
  //               (int)(((white & 0x0810204080000000ULL) * 0x0101010101010101ULL) >> 59) );

  //     frequencies[du5]++;
  //     frequencies[dl5]++;
  //     frequencies[adu5]++;
  //     frequencies[adl5]++;
  //   }

  //   for (int i = 28; i < 29; i++) {
  //     const PatternInfo& pi = patternInfo[11];
  //     for (int k = 0; k < pi.size; k++) {
  //       std::cerr << frequencies[k] << "," << pvTable[i][pi.offset + k] << " ";
  //       if (k % 9 == 8) std::cerr << std::endl;
  //     }
  //     std::cerr << std::endl;
  //   }
  //   return 0;
  // }

  read_all_tables();
  // for (int i = 0; i < N_SPLITS; i++) {
  //   for (int j = 0; j < N_WEIGHTS; j++) {
  //     pvTable[i][j] /= 1.01;
  //   }
  // }
  // write_weights();
  // return 0;

  std::vector<string> positions = read_all_training_data();

  std::vector<Sample> training_data = process_data(positions);
  positions.clear();

  std::vector<Sample>** equalized_data = equalize_plies(training_data);
  training_data.clear();

  std::cerr << "Shuffling data..." << std::endl;
  for (int i = 0; i < N_SPLITS; i++) {
    std::cerr << "Split " << i << ": " << equalized_data[i]->size() << std::endl;
    std::random_shuffle(equalized_data[i]->begin(), equalized_data[i]->end());
  }

  // Print stats on error bias
  // for (int i = 18; i < 30; i++) {
  //   std::vector<Sample> primary;
  //   primary.insert(primary.begin(), equalized_data[2*i]->begin(), equalized_data[2*i]->end());
  //   primary.insert(primary.end(), equalized_data[2*i+1]->begin(), equalized_data[2*i+1]->end());
  //   std::vector<PatternSample> pattern_data;
  //   for (auto& s : primary) {
  //     PatternSample ps(s.score);
  //     init_evaluator(s.b, &(ps.e));
  //     pattern_data.push_back(ps);
  //   }
  //   std::vector<double> errors = lin_error(pattern_data, 2*i);
  //   double mean = 0.0;
  //   for (double e : errors) mean += e;
  //   mean /= errors.size();
  //   std::sort(errors.begin(), errors.end());
  //   std::cerr << "mean: " << mean << " med: " << errors[errors.size() / 2]
  //             << " q1: " << errors[errors.size() / 4] << " q3: " << errors[3 * errors.size() / 4]
  //             << " 10: " << errors[errors.size() / 10] << " 90: " << errors[9 * errors.size() / 10] << std::endl;
  // }

  for (int i = 0; i < 31; i++) {
    std::cerr << "Turns " << 2*i << " and " << 2*i+1 << std::endl;
    // Train paired plies together
    if (i < 30) {
      std::vector<Sample> primary;
      primary.insert(primary.begin(), equalized_data[2*i]->begin(), equalized_data[2*i]->end());
      primary.insert(primary.end(), equalized_data[2*i+1]->begin(), equalized_data[2*i+1]->end());
      int primary_size = primary.size() / 2;
      int eg_factor = std::max(0, i - 18);
      int eg_factor2 = std::max(0, i - 10);
      // 2 plies before
      if (i > 1 && i < 29) {
        int additional_size = 228 - 12 * eg_factor2;
        additional_size = additional_size * primary_size / 1000;
        additional_size = std::min(additional_size, (int) equalized_data[2*i-2]->size());
        primary.insert(primary.end(), equalized_data[2*i-2]->begin(), equalized_data[2*i-2]->begin() + additional_size);
      }
      // 1 ply before
      if (i > 0) {
        int additional_size = 560 - 10 * eg_factor2 - eg_factor * eg_factor;
        additional_size = additional_size * primary_size / 1000;
        additional_size = std::min(additional_size, (int) equalized_data[2*i-1]->size());
        primary.insert(primary.end(), equalized_data[2*i-1]->begin(), equalized_data[2*i-1]->begin() + additional_size);
      }
      // 1 ply after
      if (i < 30) {
        int additional_size = 560 - 10 * eg_factor2 - eg_factor * eg_factor;
        additional_size = additional_size * primary_size / 1000;
        additional_size = std::min(additional_size, (int) equalized_data[2*i+2]->size());
        primary.insert(primary.end(), equalized_data[2*i+2]->begin(), equalized_data[2*i+2]->begin() + additional_size);
      }
      // 2 plies after
      if (i < 29) {
        int additional_size = 228 - 12 * eg_factor2;
        additional_size = additional_size * primary_size / 1000;
        additional_size = std::min(additional_size, (int) equalized_data[2*i+3]->size());
        primary.insert(primary.end(), equalized_data[2*i+3]->begin(), equalized_data[2*i+3]->begin() + additional_size);
      }

      searchFeatures(primary, /*turn=*/2*i, 0.2, 250);
      // searchFeatures(primary, /*turn=*/2*i+1, 0.02, 250);

      // Copy paired weights
      for (int j = 0; j < N_WEIGHTS; j++) {
        pvTable[2*i+1][j] = pvTable[2*i][j];
      }
    }
    // Train paired plies separately
    else {
      searchFeatures((*equalized_data[2*i]), /*turn=*/2*i, 0.2, 250);
      if (i < 30)
        searchFeatures((*equalized_data[2*i+1]), /*turn=*/2*i+1, 0.02, 250);
    }
  }

  // Zero weights for patterns that never appear (carry-over from previous tuning methods)
  // for (int i = 0; i < 30; i++) {
  //   for (int j = 0; j < N_WEIGHTS; j++) {
  //     if (freq[2*i][j] + freq[2*i+1][j] < 0.5) {
  //       pvTable[2*i][j] = 0;
  //       pvTable[2*i+1][j] = 0;
  //     }
  //   }
  // }

  write_weights();

  freemem();
  return 0;
}

std::vector<string> read_all_training_data() {
  std::vector<string> positions;
  read_book_positions(positions, "training-eg.txt");
  read_book_positions(positions, "training-mg.txt");
  return positions;
}

void read_all_tables(bool new_file) {
  std::string line;
  std::string file_name = "Flippy_Resources/flippy_weights.txt";
  if (new_file) file_name = "Flippy_Resources/new/flippy_weights.txt";
  std::ifstream eval_table(file_name, ios_base::binary);

  if (eval_table.is_open()) {
    int16_t* raw = new int16_t[31*N_WEIGHTS];
    eval_table.read(reinterpret_cast<char*>(raw), 31*N_WEIGHTS*sizeof(int16_t));
    for (int n = 0; n < N_SPLITS; n++) {
      for (int i = 0; i < N_WEIGHTS; i++) {
        pvTable[n][i] = raw[(n/2)*N_WEIGHTS + i] / 100.0;
      }
    }
    delete[] raw;

    // int16_t* raw = new int16_t[N_SPLITS*N_WEIGHTS];
    // eval_table.read(reinterpret_cast<char*>(raw), N_SPLITS*N_WEIGHTS*sizeof(int16_t));
    // for (int n = 0; n < N_SPLITS; n++) {
    //   for (int i = 0; i < N_WEIGHTS; i++) {
    //     pvTable[n][i] = raw[n*N_WEIGHTS + i] / 100.0;
    //   }
    // }
    // delete[] raw;

    eval_table.close();
  } else {
    std::cerr << "Error: could not open " << file_name << std::endl;
  }
}

void write_weights() {
  cerr << "Writing new weights." << endl;
  ofstream out;
  out.open("Flippy_Resources/new/flippy_weights.txt", ios_base::binary);
  for (int n = 0; n < 31; n++) {
    for (int i = 0; i < N_WEIGHTS; i++) {
      double a = pvTable[2*n][i] * 100.0;
      if (a > 0) a += 0.1;
      if (a < 0) a -= 0.1;
      int16_t b = (int16_t) a;
      out.write(reinterpret_cast<char *>(&b), sizeof(int16_t));
    }
  }
  out.close();
}

double regularization() {
  double reg = 0.0;
  for (int n = 0; n < N_SPLITS; n++) {
    for (int q = 0; q < N_WEIGHTS; q++) {
      double a = pvTable[n][q];
      reg += a * a;
    }
  }
  return reg * reg_lambda;
}

void randomPositions() {
  std::default_random_engine rng(1234);
  std::uniform_int_distribution<int> distribution1(0, 40);
  // Generate a deterministic permutation
  int perm[16400];
  // Fisher-Yates shuffle
  for (unsigned int i = 0; i < 16400; i++) {
    std::uniform_int_distribution<int> distribution(0, i);
    int j = distribution(rng);
    perm[i] = perm[j];
    perm[j] = i;
  }
  for (int i = 10000; i < 10100; i++) {
    cerr << "Selecting: " << i << endl;
    thor_game *game = games[perm[i]];
    if (game == NULL)
      continue;

    Board tracker;
    Color side = BLACK;
    // play opening moves
    int limit = 6 + distribution1(rng);
    for (int j = 0; j < limit; j++) {
      // If one side must pass it is not indicated in the database?
      if (!tracker.is_legal(side, game->moves[j])) {
        side = ~side;
      }
      tracker.do_move(side, game->moves[j]);
      side = ~side;
    }

    ArrayList lm = tracker.legal_movelist(side);
    if (lm.size() == 0) {
      // games[i] = NULL;
      continue;
    }

    std::string test_pos = tracker.to_string() + " " + (side == BLACK ? "Black" : "White");
    // std::cerr << test_pos << std::endl;
    endgame_pos.push_back(test_pos);
  }

  ofstream out;
  out.open("bench.txt");
  for (unsigned int i = 0; i < endgame_pos.size(); i++) {
    out << endgame_pos[i] << endl;
  }
  out.close();
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

void freemem() {
  for (unsigned int i = 0; i < totalSize; i++) {
    delete games[i];
  }
  delete[] games;
}
