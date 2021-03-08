#include "player.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include "eval.h"

using namespace std;

Player::Player(Color side, bool use_book, int tt_bits) {
  maxDepth = 50;
  endgameDepth = 50;
  forceEgDepth = 16;
  lastMaxDepth = 0;

  mySide = side;
  turn = 4;
  otherHeuristic = false;
  baseSelectivity = 1;
  bufferPerMove = 20;

  openingBook = nullptr;
  bookExhausted = true;
  if (use_book) {
    openingBook = new Openings();
    // Set to false to turn on book
    bookExhausted = false;
  }

  // Initialize transposition table with 2^20 = 1 million array slots and
  // 2 * 2^20 = 2 million entries
  transpositionTable = new Hash(tt_bits);
}

Player::~Player() {
  if (openingBook != nullptr) {
    delete openingBook;
  }
  delete transpositionTable;
}

int Player::do_move(int opponents_move, int ms_left) {
  // Register opponent's move
  if (opponents_move != MOVE_NULL) {
    game.do_move(~mySide, opponents_move);
  }
  // If opponent is passing and it isn't the start of the game
  else if (turn != 4) {
    // TODO a temporary hack to prevent opening book from crashing
    bookExhausted = true;
  }

  // We can easily count how many moves have been made from the number of
  // empty squares
  int empties = game.count_empty();
  turn = 64 - empties;
  #if PRINT_SEARCH_INFO
  cerr << endl;
  cerr << empties << " empty squares. Time left: " << ms_left << " ms" << endl;
  #endif

  // Timing
  // 10 min per move for "infinite" time
  int time_allotment = 600000;
  timeLimit = 0;
  if (ms_left != -1) {
    // Buffer time: to prevent losses on time at short time controls
    int buffer_time = 100 + bufferPerMove * empties;
    ms_left -= buffer_time;
    time_allotment = std::max(1, ms_left);

    // Base fair time usage off of number of moves left
    int moves_left = min(max(1, (empties - forceEgDepth) / 2), 18);
    time_allotment /= moves_left;
    #if PRINT_SEARCH_INFO
    cerr << "Allotted time: " << time_allotment / 1000.0 << " s" << endl;
    #endif
    // Use up to 5x fair time
    timeLimit = std::min(time_allotment * 5, ms_left);
  }

  // Check opening book
  if (!bookExhausted) {
    int book_move = openingBook->get(game.occupied(), game.get_bits(BLACK));
    if (book_move != OPENING_NOT_FOUND) {
      #if PRINT_SEARCH_INFO
      cerr << "Opening book: bestmove " << print_move(book_move) << endl << endl;
      #endif
      game.do_move(mySide, book_move);
      return book_move;
    }
    else
      bookExhausted = true;
  }


  // Find and test all legal moves
  ArrayList legal_moves = game.legal_movelist(mySide);
  if (legal_moves.size() <= 0) {
    // TODO a temporary hack to prevent opening book from crashing
    bookExhausted = true;
    #if PRINT_SEARCH_INFO
    cerr << "No legal moves. Passing." << endl << endl;
    #endif
    return MOVE_NULL;
  }

  if (legal_moves.size() == 1) {
    #if PRINT_SEARCH_INFO
    cerr << "One legal move: " << print_move(legal_moves.get(0)) << endl << endl;
    #endif
    game.do_move(mySide, legal_moves.get(0));
    return legal_moves.get(0);
  }


  Eval e;
  init_evaluator(game, &e);
  int my_move = MOVE_BROKEN;

  // Endgame solver: if we are within sight of the end and we have enough
  // time to do a perfect solve (estimated by lastMaxDepth) or have unlimited
  // time. Always use endgame solver for the last forceEgDepth plies since it
  // is faster and for more accurate results.
  int eg_potential_depth = 4;
  if (lastMaxDepth > forceEgDepth - 2)
    eg_potential_depth -= (lastMaxDepth - forceEgDepth + 2) / 2;
  if (empties <= endgameDepth
   && (lastMaxDepth + eg_potential_depth >= empties || ms_left == -1 || empties <= forceEgDepth)) {
    #if PRINT_SEARCH_INFO
    cerr << "Endgame solver: depth " << empties << endl;
    #endif

    int eg_timeLimit = std::max(1, std::min(timeLimit - 40, 2 * time_allotment));
    my_move = endgameSolver.solve_endgame(game, &e, mySide, legal_moves, false, empties, eg_timeLimit);

    if (my_move != MOVE_BROKEN) {
      game.do_move(mySide, my_move);
      return my_move;
    }
    // Otherwise, we broke out of the endgame solver.
    endgameDepth -= 2;
    time_allotment = time_allotment / 2;
    timeLimit -= eg_timeLimit;
  }

  // Start timers
  auto start_time = Clock::now();
  timeElapsed = Clock::now();
  uint64_t time_span = 0;

  // Iterative deepening
  int root_depth = 1;
  lastMaxDepth = 1;
  int prev_best = MOVE_NULL, new_best = MOVE_NULL;
  int best_score = 0;
  double time_adjustment_factor = 1.0;

  SearchInfo search_info;
  search_info.root_age = turn;
  search_info.selectivity = baseSelectivity;
  search_info.time_limit = timeLimit;
  search_info.tt = transpositionTable;
  search_info.other_heuristic = otherHeuristic;
  search_info.search_start = timeElapsed;
  do {
    #if PRINT_SEARCH_INFO
    cerr << "Depth " << root_depth << ": ";
    #endif

    time_adjustment_factor = (2 * time_adjustment_factor + 1) / 3;
    prev_best = new_best;

    new_best = pvs_best_move(game, &e, mySide, legal_moves, &best_score, root_depth, &search_info);
    if (new_best == MOVE_BROKEN) {
      #if PRINT_SEARCH_INFO
      cerr << " Broken out of search!" << endl;
      #endif
      time_span = get_time_elapsed(start_time);
      break;
    }
    lastMaxDepth = root_depth;

    // Switch new PV to be searched first
    legal_moves.swap(0, new_best);
    root_depth++;
    my_move = legal_moves.get(0);
    time_span = get_time_elapsed(start_time);
    if (new_best == prev_best) {
      time_adjustment_factor *= 0.8;
    } else {
      if (time_adjustment_factor < 1.0) time_adjustment_factor = 1.0;
      time_adjustment_factor *= 1.7;
      if (time_adjustment_factor > 2.5) time_adjustment_factor = 2.5;
    }

    #if PRINT_SEARCH_INFO
    cerr << "time " << time_span
         << " bestmove " << print_move(my_move)
         << " score " << ((double) best_score) / EVAL_SCALE_FACTOR
         << " nodes " << search_info.nodes << " nps " << 1000 * search_info.nodes / time_span << endl;
    #endif
  // Continue while we think we can finish the next depth within our
  // allotted time for this move. Based on a crude estimate of branch factor.
  } while (time_span < 0.85 * time_allotment * time_adjustment_factor
        && root_depth <= maxDepth
        && lastMaxDepth < empties);

  // Iterate selectivity
  int sel = 2;
  while (lastMaxDepth >= empties
   && time_span < 0.7 * time_allotment * time_adjustment_factor
   && sel < NO_SELECTIVITY) {
    #if PRINT_SEARCH_INFO
    cerr << "Selectivity " << sel << ": ";
    #endif

    time_adjustment_factor = (2 * time_adjustment_factor + 1) / 3;
    prev_best = new_best;

    search_info.selectivity = sel;
    new_best = pvs_best_move(game, &e, mySide, legal_moves, &best_score, root_depth, &search_info);
    if (new_best == MOVE_BROKEN) {
      #if PRINT_SEARCH_INFO
      cerr << " Broken out of search!" << endl;
      #endif
      time_span = get_time_elapsed(start_time);
      break;
    }

    // Switch new PV to be searched first
    legal_moves.swap(0, new_best);
    sel++;
    my_move = legal_moves.get(0);
    time_span = get_time_elapsed(start_time);
    if (new_best == prev_best) {
      time_adjustment_factor *= 0.8;
    } else {
      if (time_adjustment_factor < 1.0) time_adjustment_factor = 1.0;
      time_adjustment_factor *= 1.7;
      if (time_adjustment_factor > 2.5) time_adjustment_factor = 2.5;
    }

    #if PRINT_SEARCH_INFO
    cerr << "time " << time_span
         << " bestmove " << print_move(my_move)
         << " score " << ((double) best_score) / EVAL_SCALE_FACTOR
         << " nodes " << search_info.nodes << " nps " << 1000 * search_info.nodes / time_span << endl;
    #endif
  }

  lastMaxDepth += sel - 2;
  lastMaxDepth += 0.5 - log(time_adjustment_factor) / log(1.4);
  if (sel >= NO_SELECTIVITY) lastMaxDepth += 4;
  // The best move should be at the front of the list.
  my_move = legal_moves.get(0);

  // WLD confirmation at high depths
  int wld_potential_depth = 6;
  if (lastMaxDepth > forceEgDepth - 2)
    wld_potential_depth -= (lastMaxDepth - forceEgDepth + 2) / 2;
  if (empties <= endgameDepth + 2
  && (lastMaxDepth + wld_potential_depth >= empties || ms_left == -1)
  && empties > forceEgDepth
  && time_span < (uint64_t) 3 * std::min(timeLimit, time_allotment) / 4) {
    int WLDMove = endgameSolver.solve_wld(game, &e, mySide, legal_moves, true,
      empties, time_allotment);

    if (WLDMove != MOVE_BROKEN) {
      if (WLDMove != -1 && my_move != WLDMove) {
        #if PRINT_SEARCH_INFO
        cerr << "Move changed to " << print_move(WLDMove) << endl;
        #endif
        my_move = WLDMove;
      }
    }
    // If we broke out of WLD here next move's endgame solver isn't likely
    // to be successful...
    else {
      lastMaxDepth -= 4;
    }
  }

  time_span = get_time_elapsed(start_time);
  #if PRINT_SEARCH_INFO
  cerr << "Total time: " << time_span / 1000.0 << " s" << endl;
  cerr << "Nodes: " << search_info.nodes << " | NPS: " << 1000 * search_info.nodes / time_span << endl;
  cerr << "Hashfull: " << transpositionTable->hash_full() << endl;
  cerr << "Playing " << print_move(my_move) << ". Score: " << ((double) best_score) / EVAL_SCALE_FACTOR << endl << endl;
  #endif

  nodes = search_info.nodes;
  game.do_move(mySide, my_move);

  return my_move;
}

void Player::set_depths(int max, int end) {
  maxDepth = max;
  endgameDepth = end;
}

uint64_t Player::get_nodes() {
  return nodes;
}

void Player::set_position(uint64_t taken_bits, uint64_t black_bits) {
  game = Board(taken_bits & ~black_bits, black_bits);
  bookExhausted = true;
  turn = 64 - game.count_empty();
}
