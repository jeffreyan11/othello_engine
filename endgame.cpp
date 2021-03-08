#include <iostream>
#include "bbinit.h"
#include "endgame.h"

using namespace std;

#define USE_STABILITY false

namespace {

const int STAB_THRESHOLD[40] = {
  64, 64, 64, 64, 64,
  10, 12, 14, 16, 18,
  20, 22, 24, 26, 28,
  30, 32, 34, 36, 38,
  40, 42, 44, 46, 48,
  50, 52, 54, 56, 58,
  60, 60, 62, 62, 64,
  64, 64, 64, 64, 64
};

const int ROOT_SORT_DEPTHS[40] = { 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 4, 4, 6, 6, 8, 8, 10, 12,
  14, 16, 18, 18, 20, 20, 22, 22, 24, 24,
  26, 26, 28, 28, 30, 30, 32, 32
};

// Depths for sort searching. Indexed by depth.
const int ENDGAME_SORT_DEPTHS[40] = { 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 1, 2, 2,
  3, 3, 4, 4, 5, 5, 6, 6, 7, 8,
  9, 10, 11, 12, 13, 13, 14, 14, 15
};

const int END_MEDIUM = 11;
const int END_SHALLOW = 6;

const int SCORE_TIMEOUT = 65;
const int MOVE_FAIL_LOW = -1;

struct EndgameStatistics {
  uint64_t hashHits, hashCuts;
  uint64_t hashMoveAttempts, hashMoveCuts;
  uint64_t firstFailHighs, failHighs;
  uint64_t stability_cuts, stability_attempts;

  void reset() {
    hashHits = 0;
    hashCuts = 0;
    hashMoveAttempts = 0;
    hashMoveCuts = 0;
    firstFailHighs = 0;
    failHighs = 0;
    stability_cuts = 0;
    stability_attempts = 0;
  }
};

EndgameStatistics egStats;

// 16384 entries
EndHash endgameTable(9);
// 2^23 entries * 20 bytes/entry = 168 MB
EndHash cutTable(18);
// 2^22 entries * 20 bytes/entry = 84 MB
EndHash allTable(17);
// 2^16 array slots (2^17 entries) * 64 bytes/slot = 4 MB
Hash transpositionTable(11);

}  // namespace

void resize_endhash(uint32_t pv_bits) {
  endgameTable.resize(pv_bits);
  cutTable.resize(pv_bits + 9);
  allTable.resize(pv_bits + 8);
  transpositionTable.resize(pv_bits + 2);
}

Endgame::Endgame() {
  endgameTable.clear();
  cutTable.clear();
  allTable.clear();
  transpositionTable.clear();
}

int Endgame::solve_endgame(Board &b, Eval* e, Color c, ArrayList &moves, bool is_sorted,
  int depth, int time_limit, int *exact_score) {
  return solve_endgame_with_window(b, e, c, moves, is_sorted, depth, -64, 64,
    time_limit, exact_score);
}

int Endgame::solve_wld(Board &b, Eval* e, Color c, ArrayList &moves, bool is_sorted,
  int depth, int time_limit, int *exact_score) {
  #if PRINT_SEARCH_INFO
  cerr << "WLD solver: depth " << depth << endl;
  #endif
  int game_result = -2;
  int best_move = solve_endgame_with_window(b, e, c, moves, is_sorted, depth, -1, 1,
    time_limit, &game_result);

  #if PRINT_SEARCH_INFO
  if (best_move != MOVE_BROKEN) {
    if (game_result == 0) {
      cerr << "Game is draw" << endl;
    } else if (game_result <= -1) {
      cerr << "Game is loss" << endl;
    } else {
      cerr << "Game is win" << endl;
    }
  }
  #endif
  if (exact_score != nullptr)
    *exact_score = game_result;

  return best_move;
}

int Endgame::solve_endgame_with_window(Board &b, Eval* e, Color c, ArrayList &moves, bool is_sorted,
  int depth, int alpha, int beta, int time_limit, int *exact_score) {
  // if best move for this position has already been found and stored
  EndgameEntry *entry = endgameTable.get(b, c);
  if (entry != nullptr) {
    #if PRINT_SEARCH_INFO
    cerr << "Endgame hashtable hit." << endl;
    cerr << "Best move: " << print_move(entry->move);
    cerr << " Score: " << (int) (entry->score) << endl;
    #endif
    if (exact_score != nullptr)
      *exact_score = entry->score;
    return entry->move;
  }

  auto start_time = Clock::now();
  #if PRINT_SEARCH_INFO
  uint64_t time_span = 0;
  #endif

  nodes = 0;
  egStats.reset();
  searchStart = Clock::now();
  timeout = (uint64_t) time_limit;

  SearchInfo search_info;
  search_info.time_limit = 0;
  search_info.tt = &transpositionTable;
  search_info.other_heuristic = true;
  search_info.search_start = searchStart;

  int ss_score = 0;
  // Initial sorting of moves
  if (!is_sorted && depth > 12) {
    for (int ss_depth = 1; ss_depth <= ROOT_SORT_DEPTHS[depth]; ss_depth++) {
      int ss_move = pvs_best_move(b, e, c, moves, &ss_score, ss_depth, &search_info);
      moves.swap(ss_move, 0);
    }

    #if PRINT_SEARCH_INFO
    time_span = get_time_elapsed(start_time);
    cerr << "Sort search: depth " << ROOT_SORT_DEPTHS[depth] << " | " << time_span << " ms | nodes: "
         << search_info.nodes << " | NPS: " << 1000 * search_info.nodes / time_span << endl;
    cerr << "PV: " << print_move(moves.get(0));
    cerr << " Score: " << ss_score / EVAL_SCALE_FACTOR << endl;
    #endif
    search_info.nodes = 0;
  }

  start_time = Clock::now();

  // Playing with aspiration windows...
  int score;
  int asp_alpha = alpha;
  int asp_beta = beta;
  int best_index = MOVE_FAIL_LOW;
  if (!is_sorted && depth > 12) {
    int est_score = ss_score / EVAL_SCALE_FACTOR;
    asp_alpha = max(est_score - 2, alpha);
    asp_beta = min(est_score + 2, beta);
    // To prevent errors if our sort search score was outside [alpha, beta]
    if (asp_alpha >= beta)
      asp_alpha = beta - 1;
    if (asp_beta <= alpha)
      asp_beta = alpha + 1;
  }
  int window = 2;
  while (true) {
    // Try a search
    #if PRINT_SEARCH_INFO
    cerr << "Aspiration search: [" << asp_alpha << ", " << asp_beta << "]" << endl;
    #endif
    best_index = endgame_aspiration(b, e, c, moves, depth, asp_alpha, asp_beta, score, &search_info);
    // If we got broken out
    if (best_index == MOVE_BROKEN)
      return MOVE_BROKEN;
    if (best_index == MOVE_FAIL_LOW && asp_alpha > alpha) {
      // Fail low
      // We were < than the lower bound, so this is the new upper bound
      asp_beta = score + 1;
      asp_alpha = max(score - window, alpha);
    } else if (score >= asp_beta && asp_beta < beta) {
      // Fail high
      // We were > than the upper bound, so this is the new lower bound
      asp_alpha = score - 1;
      asp_beta = min(score + window, beta);
      // Swap the cut move to the front, it's the best we have right now
      moves.swap(best_index, 0);
    } else {
      // Otherwise we are done
      break;
    }
    // Decrease window as we get closer to the correct value
    window = std::max(1, window - 1);
    search_info.root_age++;
  }
  // Retrieve the best move
  int best_move = MOVE_FAIL_LOW;
  if (best_index != MOVE_FAIL_LOW) {
    best_move = moves.get(best_index);
  } else if (alpha == -64) {
    // If every move loses, return a random move.
    best_move = moves.get(0);
  }

  nodes += search_info.nodes;
  #if PRINT_SEARCH_INFO
  cerr << "Hashfull: PV=" << endgameTable.hash_full() << " | A="
                          << cutTable.hash_full() << " | B="
                          << allTable.hash_full() << " | Sort="
                          << transpositionTable.hash_full() << endl;

  time_span = get_time_elapsed(start_time);
  cerr << "Nodes: " << nodes << " | NPS: " << 1000 * nodes / time_span << endl;

  // cerr << "Hash score cut rate: " << egStats.hashCuts << " / " << egStats.hashHits << endl;
  // cerr << "Hash move cut rate: " << egStats.hashMoveCuts << " / " << egStats.hashMoveAttempts << endl;
  // cerr << "First fail high rate: " << egStats.firstFailHighs << " / " << egStats.failHighs << endl;
  // cerr << "Stability cuts: " << egStats.stability_cuts << " / " << egStats.stability_attempts << endl;

  cerr << "Time spent (ms): " << time_span << endl;
  cerr << "Best move: ";
  // If we failed low on the bounds we were given, that isn't our business
  if (best_move == MOVE_FAIL_LOW)
    cerr << "N/A";
  else
    cerr << print_move(best_move);
  cerr << " Score: " << score << endl << endl;
  #endif

  if (exact_score != nullptr)
    *exact_score = score;
  return best_move;
}

int Endgame::endgame_aspiration(Board &b, Eval* e, Color c, ArrayList &moves, int depth,
  int alpha, int beta, int &exact_score, SearchInfo* search_info) {
  int score, best_score = -INFTY;
  // If this doesn't change, we failed low
  int best_index = MOVE_FAIL_LOW;

  // Sort moves other than the best move
  ArrayList priority;
  priority.add(1 << 25);
  for (int i = 1; i < moves.size(); i++) {
    int m = moves.get(i);
    if (depth > END_MEDIUM) {
      Board copy = b.copy();
      Eval ec = *e;
      uint64_t mask = copy.get_do_move(c, m);
      ec.update(c, m, mask);
      copy.do_move(c, m, mask);
      priority.add(-pvs(copy, &ec, ~c, ENDGAME_SORT_DEPTHS[depth+2], -INFTY, INFTY, false, search_info));
    } else {
      int p = SQ_VAL[m];
      if (!(NEIGHBORS[m] & ~b.occupied()))
        p += 16;
      priority.add(p);
    }
  }

  int i = 0;
  for (int m = next_move(moves, priority, i); m != MOVE_NULL;
           m = next_move(moves, priority, ++i)) {
    Board copy = b.copy();
    Eval ec = *e;
    uint64_t mask = copy.get_do_move(c, m);
    ec.update(c, m, mask);
    copy.do_move(c, m, mask);
    nodes++;

    if (i != 0) {
      score = -dispatch(copy, &ec, ~c, depth-1, -alpha-1, -alpha, search_info);
      if (alpha < score && score < beta)
        score = -dispatch(copy, &ec, ~c, depth-1, -beta, -alpha, search_info);
    }
    else
      score = -dispatch(copy, &ec, ~c, depth-1, -beta, -alpha, search_info);

    if (score == SCORE_TIMEOUT) {
      #if PRINT_SEARCH_INFO
      cerr << "Breaking out of endgame solver." << endl;
      #endif
      // If we have already found a winning move, mind as well take it.
      if (best_index != MOVE_FAIL_LOW && alpha > 0) {
        exact_score = alpha;
        return best_index;
      }
      else
        return MOVE_BROKEN;
    }

    if (score >= beta) {
      exact_score = score;
      return i;
    }
    if (score > best_score) {
      best_score = score;
      if (alpha < score) {
        alpha = score;
        best_index = i;
      }
    }
  }

  exact_score = best_score;
  return best_index;
}

int Endgame::dispatch(Board &b, Eval* e, Color c, int depth, int alpha, int beta, SearchInfo* search_info) {
  switch (depth) {
    case 4:
      return endgame4(b, c, alpha, beta, false);
    case 3:
      return endgame3(b, c, alpha, beta, false);
    case 2: {
      uint64_t empty = ~b.occupied();
      int lm1 = bitscan_forward(empty);
      empty &= empty - 1;
      int lm2 = bitscan_forward(empty);
      return endgame2(b, c, alpha, beta, lm1, lm2);
    }
    default:
      return endgame_deep(b, e, c, depth, alpha, beta, false, search_info);
  }
}

int Endgame::endgame_deep(Board &b, Eval* e, Color c, int depth, int alpha, int beta, bool passed_last, SearchInfo* search_info) {
  if (depth <= END_MEDIUM)
    return endgame_medium(b, c, depth, alpha, beta, passed_last);

  int score, best_score = -INFTY;
  int prev_alpha = alpha;
  bool is_pv_node = (alpha != beta - 1);

  // play best move, if recorded
  EndgameEntry *exact_entry = endgameTable.get(b, c);
  if (exact_entry != nullptr) {
    return exact_entry->score;
  }

  // Stability cutoff: if the current position is hopeless compared to a
  // known lower bound, then we need not waste time searching it.
  #if USE_STABILITY
  if (alpha >= STAB_THRESHOLD[depth]) {
    egStats.stability_attempts++;
    score = 64 - 2*b.count_stability(~c);
    if (score <= alpha) {
      egStats.stability_cuts++;
      return score;
    }
  }
  #endif

  EndgameEntry *all_entry = allTable.get(b, c);
  if (all_entry != nullptr) {
    if (all_entry->score <= alpha)
      return all_entry->score;
    if (beta > all_entry->score)
      beta = all_entry->score;
  }

  // attempt cut node cutoff, using saved alpha
  int hash_move = MOVE_NULL;
  EndgameEntry *cut_entry = cutTable.get(b, c);
  if (cut_entry != nullptr) {
    egStats.hashHits++;
    if (cut_entry->score >= beta) {
      egStats.hashCuts++;
      return cut_entry->score;
    }
    // Fail high is lower bound on score so this is valid
    if (alpha < cut_entry->score)
      alpha = cut_entry->score;
    hash_move = cut_entry->move;

    // Try the move for a cutoff before move generation
    egStats.hashMoveAttempts++;
    Board copy = b.copy();
    Eval ec = *e;
    uint64_t mask = copy.get_do_move(c, hash_move);
    ec.update(c, hash_move, mask);
    copy.do_move(c, hash_move, mask);
    nodes++;

    score = -endgame_deep(copy, &ec, ~c, depth-1, -beta, -alpha, false, search_info);

    // If we received a timeout signal, propagate it upwards
    if (score == SCORE_TIMEOUT)
      return -SCORE_TIMEOUT;

    if (score >= beta) {
      egStats.hashMoveCuts++;
      return score;
    }
    if (score > best_score) {
      best_score = score;
      if (alpha < score) {
        alpha = score;
      }
    }
  }

  ArrayList legal_moves = b.legal_movelist(c);
  if (legal_moves.size() <= 0) {
    if (passed_last) {
      return (2 * b.count(c) - 64 + depth);
    }

    score = -endgame_deep(b, e, ~c, depth, -beta, -alpha, true, search_info);
    // If we received a timeout signal, propagate it upwards
    if (score == SCORE_TIMEOUT)
      return -SCORE_TIMEOUT;

    return score;
  }

  // Get a best move from previous sort searches if available
  int ss_move = MOVE_NULL;
  if (is_pv_node) {
    HashEntry *entry = transpositionTable.get(b, c);
    if (entry != nullptr) {
      if (entry->nodeType == PV_NODE
       && entry->depth >= ENDGAME_SORT_DEPTHS[depth+2]) {
        ss_move = entry->move;
      }
    }
  }

  ArrayList priority;
  // Do better move ordering for PV nodes where alpha != beta - 1
  if (is_pv_node) {
    for (int i = 0; i < legal_moves.size(); i++) {
      int m = legal_moves.get(i);
      Board copy = b.copy();
      uint64_t mask = copy.get_do_move(c, m);
      copy.do_move(c, m, mask);

      if (m == hash_move) {
        priority.add(1 << 25);
      } else if (m == ss_move) {
        priority.add(1 << 24);
      } else {
        Eval ec = *e;
        ec.update(c, m, mask);
        priority.add(-pvs(copy, &ec, ~c, ENDGAME_SORT_DEPTHS[depth+2], -INFTY, INFTY, passed_last, search_info));
      }
    }
  }
  // Otherwise, we focus more on fastest first for a cheaper fail-high
  else {
    for (int i = 0; i < legal_moves.size(); i++) {
      int m = legal_moves.get(i);
      Board copy = b.copy();
      uint64_t mask = copy.get_do_move(c, m);
      copy.do_move(c, m, mask);

      if (m == hash_move) {
        priority.add(1 << 25);
      } else {
        int p = SQ_VAL[m] - 1800 * copy.count_legal_moves(~c) - 256 * stability(copy, ~c);
        Eval ec = *e;
        ec.update(c, m, mask);
        // Slowly phase in the sort search relative to fastest first
        if (ENDGAME_SORT_DEPTHS[depth] == 0)
          p -= pvs(copy, &ec, ~c, ENDGAME_SORT_DEPTHS[depth], -INFTY, INFTY, passed_last, search_info) >> 1;
        else
          p -= pvs(copy, &ec, ~c, ENDGAME_SORT_DEPTHS[depth], -INFTY, INFTY, passed_last, search_info);
        priority.add(p);
      }
    }
  }

  int to_hash = MOVE_NULL;
  int i = 0;
  for (int m = next_move(legal_moves, priority, i); m != MOVE_NULL;
           m = next_move(legal_moves, priority, ++i)) {
    // Check for a timeout
    if (depth >= 15) {
      uint64_t time_span = get_time_elapsed(searchStart);
      if (time_span > timeout)
        return -SCORE_TIMEOUT;
    }
    // We already tried the hash move
    if (m == hash_move)
      continue;
    Board copy = b.copy();
    Eval ec = *e;
    uint64_t mask = copy.get_do_move(c, m);
    ec.update(c, m, mask);
    copy.do_move(c, m, mask);
    nodes++;

    if (i != 0) {
      score = -endgame_deep(copy, &ec, ~c, depth-1, -alpha-1, -alpha, false, search_info);
      if (alpha < score && score < beta)
        score = -endgame_deep(copy, &ec, ~c, depth-1, -beta, -alpha, false, search_info);
    }
    else
      score = -endgame_deep(copy, &ec, ~c, depth-1, -beta, -alpha, false, search_info);

    // If we received a timeout signal, propagate it upwards
    if (score == SCORE_TIMEOUT)
      return -SCORE_TIMEOUT;
    if (score >= beta) {
      egStats.failHighs++;
      if (i == 0)
        egStats.firstFailHighs++;
      cutTable.add(b, c, score, m, depth);
      return score;
    }
    if (score > best_score) {
      best_score = score;
      if (alpha < score) {
        alpha = score;
        to_hash = m;
      }
    }
  }

  // Best move with exact score if alpha < score < beta
  if (to_hash != MOVE_NULL && prev_alpha < alpha && alpha < beta)
    endgameTable.add(b, c, alpha, to_hash, depth);
  else if (alpha <= prev_alpha)
    allTable.add(b, c, best_score, MOVE_NULL, depth);

  return best_score;
}

int Endgame::endgame_medium(Board &b, Color c, int depth, int alpha, int beta, bool passed_last) {
  if (depth <= END_SHALLOW)
    return endgame_shallow(b, c, depth, alpha, beta, passed_last);

  int score, best_score = -INFTY;
  int prev_alpha = alpha;

  // play best move, if recorded
  EndgameEntry *exact_entry = endgameTable.get(b, c);
  if (exact_entry != nullptr) {
    return exact_entry->score;
  }

  // Stability cutoff: if the current position is hopeless compared to a
  // known lower bound, then we need not waste time searching it.
  #if USE_STABILITY
  if (alpha >= STAB_THRESHOLD[depth]) {
    egStats.stability_attempts++;
    score = 64 - 2*b.count_stability(~c);
    if (score <= alpha) {
      egStats.stability_cuts++;
      return score;
    }
  }
  #endif

  EndgameEntry *all_entry = allTable.get(b, c);
  if (all_entry != nullptr) {
    if (all_entry->score <= alpha)
      return all_entry->score;
    if (beta > all_entry->score)
      beta = all_entry->score;
  }

  // attempt cut node cutoff, using saved alpha
  int hash_move = MOVE_NULL;
  EndgameEntry *cut_entry = cutTable.get(b, c);
  if (cut_entry != nullptr) {
    egStats.hashHits++;
    if (cut_entry->score >= beta) {
      egStats.hashCuts++;
      return cut_entry->score;
    }
    // Fail high is lower bound on score so this is valid
    if (alpha < cut_entry->score)
      alpha = cut_entry->score;
    hash_move = cut_entry->move;

    // Try the move for a cutoff before move generation
    egStats.hashMoveAttempts++;
    Board copy = b.copy();
    copy.do_move(c, hash_move);
    nodes++;

    score = -endgame_medium(copy, ~c, depth-1, -beta, -alpha, false);

    if (score >= beta) {
      egStats.hashMoveCuts++;
      return score;
    }
    if (score > best_score) {
      best_score = score;
      if (alpha < score) {
        alpha = score;
      }
    }
  }

  uint64_t legal = b.legal_moves(c);
  if (!legal) {
    if (passed_last) {
      return (2 * b.count(c) - 64 + depth);
    }

    return -endgame_medium(b, ~c, depth, -beta, -alpha, true);
  }

  // Create an array of moves and scores using SIMD
  int moves[END_MEDIUM];
  int n = 0;
  uint64_t empty = ~b.occupied();
  while (legal) {
    int m = bitscan_forward(legal);
    legal &= legal - 1;

    if (m == hash_move) {
      moves[n] = (1 << 25) + m;
    } else {
      Board copy = b.copy();
      copy.do_move(c, m);
      int p = m + 128 * SQ_VAL[m] - 4096 * copy.count_legal_moves(~c);
      if (!(NEIGHBORS[m] & empty))
        p += 2048;
      moves[n] = p;
    }
    n++;
  }

  int to_hash = MOVE_NULL;
  int i = 0;
  for (int move_score = next_move_shallow(moves, n, i); move_score != MOVE_NULL;
           move_score = next_move_shallow(moves, n, ++i)) {
    int m = move_score & 127;
    // We already tried the hash move
    if (m == hash_move)
      continue;
    Board copy = b.copy();
    copy.do_move(c, m);
    nodes++;

    if (i != 0) {
      score = -endgame_medium(copy, ~c, depth-1, -alpha-1, -alpha, false);
      if (alpha < score && score < beta)
        score = -endgame_medium(copy, ~c, depth-1, -beta, -alpha, false);
    }
    else
      score = -endgame_medium(copy, ~c, depth-1, -beta, -alpha, false);

    if (score >= beta) {
      cutTable.add(b, c, score, m, depth);
      return score;
    }
    if (score > best_score) {
      best_score = score;
      if (alpha < score) {
        alpha = score;
        to_hash = m;
      }
    }
  }

  // Best move with exact score if alpha < score < beta
  if (to_hash != MOVE_NULL && prev_alpha < alpha && alpha < beta)
    endgameTable.add(b, c, alpha, to_hash, depth);
  else if (alpha <= prev_alpha)
    allTable.add(b, c, best_score, MOVE_NULL, depth);

  return best_score;
}

int Endgame::endgame_shallow(Board &b, Color c, int depth, int alpha, int beta, bool passed_last) {
  if (depth == 4)
    return endgame4(b, c, alpha, beta, passed_last);

  int score, best_score = -INFTY;

  #if USE_STABILITY
  if (alpha >= STAB_THRESHOLD[depth]) {
    egStats.stability_attempts++;
    score = 64 - 2 * b.count_stability(~c);
    if (score <= alpha) {
      egStats.stability_cuts++;
      return score;
    }
  }
  #endif

  uint64_t legal = b.legal_moves(c);
  if (!legal) {
    if (passed_last)
      return (2 * b.count(c) - 64 + depth);

    return -endgame_shallow(b, ~c, depth, -beta, -alpha, true);
  }

  // Create an array of moves and scores using SIMD
  int moves[END_SHALLOW];
  int n = 0;
  uint64_t empty = ~b.occupied();
  while (legal) {
    int m = bitscan_forward(legal);
    legal &= legal-1;

    // Sort by piece square tables and hole parity
    int p = 128 * SQ_VAL[m] + m;
    if (!(NEIGHBORS[m] & empty))
      p += 8192;
    moves[n] = p;
    n++;
  }

  // search all moves
  int i = 0;
  for (int move = next_move_shallow(moves, n, i); move != MOVE_NULL;
           move = next_move_shallow(moves, n, ++i)) {
    Board copy = b.copy();
    copy.do_move(c, move & 127);
    nodes++;

    score = -endgame_shallow(copy, ~c, depth-1, -beta, -alpha, false);

    if (score >= beta)
      return score;
    if (score > best_score) {
      best_score = score;
      if (alpha < score)
        alpha = score;
    }
  }

  return best_score;
}

int Endgame::endgame4(Board &b, Color c, int alpha, int beta, bool passed_last) {
  int score = -INFTY, best_score = -INFTY;
  int legal_moves[4];
  uint64_t empty = ~b.occupied();

  // At 4 squares remaining, try each individual square as a potential legal move.
  uint64_t mask = empty;
  legal_moves[0] = bitscan_forward(mask);
  mask &= mask-1;
  legal_moves[1] = bitscan_forward(mask);
  mask &= mask-1;
  legal_moves[2] = bitscan_forward(mask);
  mask &= mask-1;
  legal_moves[3] = bitscan_forward(mask);

  // Sort holes by parity
  if (NEIGHBORS[legal_moves[0]] & empty) {
    if (!(NEIGHBORS[legal_moves[1]] & empty)) {
      int temp = legal_moves[0];
      legal_moves[0] = legal_moves[1];
      legal_moves[1] = legal_moves[3];
      legal_moves[3] = temp;
    } else if (!(NEIGHBORS[legal_moves[2]] & empty)) {
      int temp = legal_moves[0];
      legal_moves[0] = legal_moves[2];
      legal_moves[2] = temp;
      temp = legal_moves[1];
      legal_moves[1] = legal_moves[3];
      legal_moves[3] = temp;
    } else {
      int temp = legal_moves[0];
      legal_moves[0] = legal_moves[3];
      legal_moves[3] = temp;
    }
  } else if (NEIGHBORS[legal_moves[1]] & empty) {
    if (!(NEIGHBORS[legal_moves[2]] & empty)) {
      int temp = legal_moves[1];
      legal_moves[1] = legal_moves[2];
      legal_moves[2] = temp;
    } else {
      int temp = legal_moves[1];
      legal_moves[1] = legal_moves[3];
      legal_moves[3] = temp;
    }
  }

  uint64_t opp = b.get_bits(~c);
  for (int i = 0; i < 4; i++) {
    int m = legal_moves[i];
    uint64_t change_mask;
    if ((opp & NEIGHBORS[m]) && (change_mask = b.get_do_move(c, m))) {
      b.do_move(c, m, change_mask);
      nodes++;

      score = -endgame3(b, ~c, -beta, -alpha, false);

      b.undo_move(c, m, change_mask);
      if (score >= beta)
        return score;
      if (score > best_score) {
        best_score = score;
        if (alpha < score)
          alpha = score;
      }
    }
  }

  if (score == -INFTY) {
    if (passed_last)
      return (2 * b.count(c) - 60);

    return -endgame4(b, ~c, -beta, -alpha, true);
  }

  return best_score;
}

int Endgame::endgame3(Board &b, Color c, int alpha, int beta, bool passed_last) {
  int score = -INFTY, best_score = -INFTY;
  int legal_moves[3];
  uint64_t empty = ~b.occupied();

  uint64_t mask = empty;
  legal_moves[0] = bitscan_forward(mask);
  mask &= mask-1;
  legal_moves[1] = bitscan_forward(mask);
  mask &= mask-1;
  legal_moves[2] = bitscan_forward(mask);

  // Sort holes by parity
  if (NEIGHBORS[legal_moves[0]] & empty) {
    if (!(NEIGHBORS[legal_moves[1]] & empty)) {
      int temp = legal_moves[0];
      legal_moves[0] = legal_moves[1];
      legal_moves[1] = temp;
    } else {
      int temp = legal_moves[0];
      legal_moves[0] = legal_moves[2];
      legal_moves[2] = temp;
    }
  }

  uint64_t opp = b.get_bits(~c);

  int m = legal_moves[0];
  uint64_t change_mask;
  if ((opp & NEIGHBORS[m]) && (change_mask = b.get_do_move(c, m))) {
    b.do_move(c, m, change_mask);
    nodes++;
    score = -endgame2(b, ~c, -beta, -alpha, legal_moves[1], legal_moves[2]);
    b.undo_move(c, m, change_mask);

    if (score >= beta)
      return score;
    if (score > best_score) {
      best_score = score;
      if (alpha < score)
        alpha = score;
    }
  }

  m = legal_moves[1];
  if ((opp & NEIGHBORS[m]) && (change_mask = b.get_do_move(c, m))) {
    b.do_move(c, m, change_mask);
    nodes++;
    score = -endgame2(b, ~c, -beta, -alpha, legal_moves[0], legal_moves[2]);
    b.undo_move(c, m, change_mask);

    if (score >= beta)
      return score;
    if (score > best_score) {
      best_score = score;
      if (alpha < score)
        alpha = score;
    }
  }

  m = legal_moves[2];
  if ((opp & NEIGHBORS[m]) && (change_mask = b.get_do_move(c, m))) {
    b.do_move(c, m, change_mask);
    nodes++;
    score = -endgame2(b, ~c, -beta, -alpha, legal_moves[0], legal_moves[1]);
    b.undo_move(c, m, change_mask);

    if (score >= beta)
      return score;
    if (score > best_score) {
      best_score = score;
      if (alpha < score)
        alpha = score;
    }
  }

  if (score == -INFTY) {
    if (passed_last)
      return (2 * b.count(c) - 61);

    return -endgame3(b, ~c, -beta, -alpha, true);
  }

  return best_score;
}

int Endgame::endgame2(Board &b, Color c, int alpha, int beta, int lm1, int lm2) {
  int score = -INFTY, best_score = -INFTY;
  uint64_t opp = b.get_bits(~c);
  uint64_t change_mask;

  if ((opp & NEIGHBORS[lm1]) && (change_mask = b.get_do_move(c, lm1))) {
    b.do_move(c, lm1, change_mask);
    nodes++;
    score = -endgame1(b, ~c, -beta, lm2);
    b.undo_move(c, lm1, change_mask);

    if (score >= beta)
      return score;
    if (score > best_score)
      best_score = score;
  }

  if ((opp & NEIGHBORS[lm2]) && (change_mask = b.get_do_move(c, lm2))) {
    b.do_move(c, lm2, change_mask);
    nodes++;
    score = -endgame1(b, ~c, -beta, lm1);
    b.undo_move(c, lm2, change_mask);

    if (score >= beta)
      return score;
    if (score > best_score)
      best_score = score;
  }

  // if no legal moves... try other player
  if (score == -INFTY) {
    best_score = INFTY;
    opp = b.get_bits(c);

    if ((opp & NEIGHBORS[lm1]) && (change_mask = b.get_do_move(~c, lm1))) {
      b.do_move(~c, lm1, change_mask);
      nodes++;
      score = endgame1(b, c, alpha, lm2);
      b.undo_move(~c, lm1, change_mask);

      if (alpha >= score)
        return score;
      if (score < best_score)
        best_score = score;
    }

    if ((opp & NEIGHBORS[lm2]) && (change_mask = b.get_do_move(~c, lm2))) {
      b.do_move(~c, lm2, change_mask);
      nodes++;
      score = endgame1(b, c, alpha, lm1);
      b.undo_move(~c, lm2, change_mask);

      if (alpha >= score)
        return score;
      if (score < best_score)
        best_score = score;
    }

    // if both players passed, game over
    if (score == -INFTY)
      return (2 * b.count(c) - 62);
  }

  return best_score;
}

int Endgame::endgame1(Board &b, Color c, int alpha, int legal_move) {
  // Get a stand pat score
  int score = 2 * b.count(c) - 63;

  uint64_t change_mask = b.get_do_move(c, legal_move);
  nodes++;
  // If the player "c" can move, calculate final score
  if (change_mask) {
    score += 2 * count_bits(change_mask) + 1;
  }
  // Otherwise, it is the opponent's move. If the opponent can stand pat,
  // we don't need to calculate the final score.
  else if (score >= alpha) {
    uint64_t other_mask = b.get_do_move(~c, legal_move);
    nodes++;
    if (other_mask) {
      score -= 2 * count_bits(other_mask) + 1;
    }
  }

  return score;
}

//--------------------------------Utilities-------------------------------------

// Retrieves the next move with the highest score, starting from index using a
// partial selection sort. This way, the entire list does not have to be sorted
// if an early cutoff occurs.
int Endgame::next_move_shallow(int *moves, int size, int index) {
  if (index >= size)
    return MOVE_NULL;
  // Find the index of the next best move/score
  int best_index = index;
  for (int i = index + 1; i < size; i++) {
    if (moves[i] > moves[best_index]) {
      best_index = i;
    }
  }
  // swap to the correct position
  int temp_move = moves[best_index];
  moves[best_index] = moves[index];
  moves[index] = temp_move;
  // return the move
  return moves[index];
}
