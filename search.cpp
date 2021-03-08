#include "search.h"
#include <iostream>
#include "bbinit.h"
using namespace std;

constexpr int BETA_BOUND = 0;
constexpr int ALPHA_BOUND = 1;
constexpr int STATIC_EVAL_MARGIN[2][4] = {
{0,
  55 * EVAL_SCALE_FACTOR / 10,
  41 * EVAL_SCALE_FACTOR / 10,
  72 * EVAL_SCALE_FACTOR / 10
},
{0,
  38 * EVAL_SCALE_FACTOR / 10,
  71 * EVAL_SCALE_FACTOR / 10,
  56 * EVAL_SCALE_FACTOR / 10
}
};

constexpr int SELECTIVITY_FACTOR[6] = {0,
  1, 2, 4, 7, 11
};

int static_eval_margin(int bound_type, int depth, int selectivity) {
  return STATIC_EVAL_MARGIN[bound_type][depth] * (3 + selectivity) / 4;
}

int pvs_0(Board& b, Eval* e, Color c, SearchInfo* search_info) {
  return heuristic(b, e, c);
}

int pvs_1(Board &b, Eval* e, Color c, int alpha, int beta, bool passed_last, SearchInfo* search_info) {
  // Static eval pruning
  if (alpha == beta - 1
   && search_info->selectivity < NO_SELECTIVITY) {
    int static_eval = pvs_0(b, e, c, search_info);
    if (static_eval >= beta + static_eval_margin(BETA_BOUND, 1, SELECTIVITY_FACTOR[search_info->selectivity]))
      return beta;
    if (static_eval < alpha - static_eval_margin(ALPHA_BOUND, 1, SELECTIVITY_FACTOR[search_info->selectivity]))
      return alpha;
  }

  ArrayList legalMoves = b.legal_movelist(c);
  if (legalMoves.size() <= 0) {
    if (passed_last)
      return score_game_end(b, c);

    return -pvs_1(b, e, ~c, -beta, -alpha, true, search_info);
  }

  int bestScore = -INFTY;
  for (int i = 0; i < legalMoves.size(); i++) {
    int m = legalMoves.get(i);
    Board copy = b.copy();
    Eval ec = *e;
    uint64_t mask = copy.get_do_move(c, m);
    ec.update(c, m, mask);
    copy.do_move(c, m, mask);
    search_info->nodes++;

    int score = -pvs_0(copy, &ec, ~c, search_info);

    if (score >= beta)
      return score;
    if (score > bestScore)
      bestScore = score;
  }

  return bestScore;
}

int pvs_2(Board &b, Eval* e, Color c, int alpha, int beta, bool passed_last, SearchInfo* search_info) {
  // Static eval pruning
  if (alpha == beta - 1
   && search_info->selectivity < NO_SELECTIVITY) {
    int static_eval = pvs_0(b, e, c, search_info);
    if (static_eval >= beta + static_eval_margin(BETA_BOUND, 2, SELECTIVITY_FACTOR[search_info->selectivity]))
      return beta;
    if (static_eval < alpha - static_eval_margin(ALPHA_BOUND, 2, SELECTIVITY_FACTOR[search_info->selectivity]))
      return alpha;
  }

  ArrayList legalMoves = b.legal_movelist(c);
  if (legalMoves.size() <= 0) {
    if (passed_last)
      return score_game_end(b, c);

    return -pvs_2(b, e, ~c, -beta, -alpha, true, search_info);
  }

  // Move ordering with piece square table
  ArrayList scores;
  for (int i = 0; i < legalMoves.size(); i++)
    scores.add(SQ_VAL[legalMoves.get(i)]);

  int bestScore = -INFTY;
  int i = 0;
  for (int m = next_move(legalMoves, scores, i); m != MOVE_NULL;
           m = next_move(legalMoves, scores, ++i)) {
    Board copy = b.copy();
    Eval ec = *e;
    uint64_t mask = copy.get_do_move(c, m);
    ec.update(c, m, mask);
    copy.do_move(c, m, mask);
    search_info->nodes++;

    int score = -pvs_1(copy, &ec, ~c, -beta, -alpha, false, search_info);

    if (score >= beta)
      return score;
    if (score > bestScore) {
      bestScore = score;
      if (alpha < score) {
        alpha = score;
      }
    }
  }

  return bestScore;
}

int pvs_deep(Board &b, Eval* e, Color c, int depth, int alpha, int beta, bool passed_last, SearchInfo* search_info) {
  if (b.count_empty() == 0)
    return score_game_end(b, c);
  if (depth <= 2) {
    return pvs_2(b, e, c, alpha, beta, passed_last, search_info);
  }

  int score, bestScore = -INFTY;
  int prevAlpha = alpha;
  int hashed = MOVE_NULL;
  int toHash = MOVE_NULL;

  // We want to do better move ordering at PV nodes where alpha != beta - 1
  bool isPVNode = (alpha != beta - 1);
  int sel_factor = SELECTIVITY_FACTOR[search_info->selectivity];

  // Probe transposition table for a score or move
  // Do this only at depth 4 and above for efficiency
  if (depth >= 4) {
    HashEntry *entry = search_info->tt->get(b, c);
    if (entry != nullptr) {
      // For all-nodes, we only have an upper bound score
      if (entry->nodeType == ALL_NODE) {
        if (entry->depth >= depth && entry->selectivity >= search_info->selectivity && entry->score <= alpha)
          return entry->score;
      }
      else {
        if (entry->depth >= depth && entry->selectivity >= search_info->selectivity) {
          // For cut-nodes, we have a lower bound score
          if (entry->nodeType == CUT_NODE && entry->score >= beta)
            return entry->score;
          // For PV-nodes, we have an exact score we can return
          else if (entry->nodeType == PV_NODE && !isPVNode)
            return entry->score;
        }
        // Try the hash move first
        hashed = entry->move;
        Board copy = b.copy();
        Eval ec = *e;
        uint64_t mask = copy.get_do_move(c, hashed);
        ec.update(c, hashed, mask);
        copy.do_move(c, hashed, mask);
        search_info->nodes++;
        score = -pvs_deep(copy, &ec, ~c, depth-1, -beta, -alpha, false, search_info);

        // If we received a timeout signal, propagate it upwards
        if (score == TIMEOUT)
          return -TIMEOUT;
        if (score >= beta)
          return score;
        if (score > bestScore) {
          bestScore = score;
          if (alpha < score)
            alpha = score;
        }
      }
    }
  }

  // Static eval pruning
  if (!isPVNode && search_info->selectivity < NO_SELECTIVITY) {
    int static_eval = pvs_0(b, e, c, search_info);
    if (depth <= 3
     && static_eval >= beta + static_eval_margin(BETA_BOUND, depth, sel_factor)) {
      return beta;
    }
    if (depth <= 3
     && static_eval < alpha - static_eval_margin(ALPHA_BOUND, depth, sel_factor)) {
      return alpha;
    }

    // Prob-cut
    if (depth >= 4) {
      int mpc_depth = (2 * (depth / 4)) | (depth & 1);
      int static_error = (-70 + 23 * depth - (sel_factor - 1) * (29 + 6 * depth)) * EVAL_SCALE_FACTOR / 100;
      int mpc_error = (240 + 20 * depth - 35 * mpc_depth) * EVAL_SCALE_FACTOR * (3 + sel_factor) / 400;

      // if (static_eval >= beta - static_error) {
      //   int mpc_beta = beta + mpc_error + abs(beta) / 16;
      //   int mpc_score = pvs(b, e, c, mpc_depth, mpc_beta-1, mpc_beta, passed_last, search_info);
      //   if (mpc_score >= mpc_beta)
      //     return beta;
      // }

      if (static_eval < alpha + static_error) {
        int mpc_alpha = alpha - mpc_error - abs(alpha) / 16;
        int mpc_score = pvs(b, e, c, mpc_depth, mpc_alpha, mpc_alpha+1, passed_last, search_info);
        if (mpc_score <= mpc_alpha)
          return alpha;
      }
    }
  }

  ArrayList legalMoves = b.legal_movelist(c);
  if (legalMoves.size() <= 0) {
    if (passed_last)
      return score_game_end(b, c);

    score = -pvs_deep(b, e, ~c, depth, -beta, -alpha, true, search_info);

    // If we received a timeout signal, propagate it upwards
    if (score == TIMEOUT)
      return -TIMEOUT;

    return score;
  }
  // Remove the hash move since it has already been searched
  if (hashed != MOVE_NULL) {
    for (int i = 0; i < legalMoves.size(); i++) {
      if (legalMoves.get(i) == hashed) {
        legalMoves.remove(i);
        break;
      }
    }
  }

  // Move ordering
  ArrayList scores;
  if (depth >= 4) {
    // PV nodes: sort search with higher depth
    for (int i = 0; i < legalMoves.size(); i++) {
      int m = legalMoves.get(i);
      Board copy = b.copy();
      Eval ec = *e;
      uint64_t mask = copy.get_do_move(c, m);
      ec.update(c, m, mask);
      copy.do_move(c, m, mask);
      search_info->nodes++;
      int ss_depth = isPVNode ? std::max(0, (depth - 6) / 3)
                              : std::max(0, (depth - 7) / 4);
      int p = -pvs(copy, &ec, ~c, ss_depth, -INFTY, INFTY, false, search_info);
      if (!isPVNode) {
        p -= 1500*copy.count_legal_moves(~c);
      }
      scores.add(p);
    }
  } else {
    // Low depths: piece square table and fastest first
    uint64_t empty = ~b.occupied();
    for (int i = 0; i < legalMoves.size(); i++) {
      int m = legalMoves.get(i);
      Board copy = b.copy();
      copy.do_move(c, m);
      int p = SQ_VAL[m] - 32*copy.count_legal_moves(~c);
      if (!(NEIGHBORS[m] & empty))
        p += 16;
      scores.add(p);
    }
  }

  int i = 0;
  for (int m = next_move(legalMoves, scores, i); m != MOVE_NULL;
           m = next_move(legalMoves, scores, ++i)) {
    // Check for a timeout
    if (search_info->time_limit != 0 && (search_info->nodes & 127) == 127) {
      if (get_time_elapsed(search_info->search_start) > search_info->time_limit)
        return -TIMEOUT;
    }

    Board copy = b.copy();
    Eval ec = *e;
    uint64_t mask = copy.get_do_move(c, m);
    ec.update(c, m, mask);
    copy.do_move(c, m, mask);
    search_info->nodes++;

    int reduction = 0;
    if (!isPVNode && search_info->selectivity == 1
     && depth >= 4 && i > 2 + (depth == 4)) {
      reduction = 1 + (13 * (depth-4) + 16 * (i-3)) / 128;
      // Reduction must be even
      reduction *= 2;
    }

    if (i > 0 || hashed != MOVE_NULL) {
      score = -pvs(copy, &ec, ~c, depth-1-reduction, -alpha-1, -alpha, false, search_info);
      if (reduction > 0 && score > alpha)
        score = -pvs_deep(copy, &ec, ~c, depth-1, -alpha-1, -alpha, false, search_info);
      if (alpha < score && score < beta)
        score = -pvs_deep(copy, &ec, ~c, depth-1, -beta, -alpha, false, search_info);
    }
    else
      score = -pvs_deep(copy, &ec, ~c, depth-1, -beta, -alpha, false, search_info);

    // If we received a timeout signal, propagate it upwards
    if (score == TIMEOUT)
      return -TIMEOUT;
    if (score >= beta) {
      if (depth >= 4)
        search_info->tt->add(b, c, score, search_info->selectivity, m, search_info->root_age, depth, CUT_NODE);
      return score;
    }
    if (score > bestScore) {
      bestScore = score;
      if (alpha < score) {
        alpha = score;
        toHash = m;
      }
    }
  }

  if (depth >= 4 && toHash != MOVE_NULL && prevAlpha < alpha && alpha < beta)
    search_info->tt->add(b, c, alpha, search_info->selectivity, toHash, search_info->root_age, depth, PV_NODE);
  else if (depth >= 4 && alpha <= prevAlpha)
    search_info->tt->add(b, c, bestScore, search_info->selectivity, MOVE_NULL, search_info->root_age, depth, ALL_NODE);

  return bestScore;
}

int pvs(Board &b, Eval* e, Color c, int depth, int alpha, int beta, bool passed_last, SearchInfo* search_info) {
  switch (depth) {
    case 0:
      return pvs_0(b, e, c, search_info);
    case 1:
      return pvs_1(b, e, c, alpha, beta, passed_last, search_info);
    case 2:
      return pvs_2(b, e, c, alpha, beta, passed_last, search_info);
    default:
      return pvs_deep(b, e, c, depth, alpha, beta, passed_last, search_info);
  }
}

int pvs_best_move(Board &b, Eval* e, Color c, ArrayList &moves, int* best_score, int depth, SearchInfo* search_info) {
  int score;
  int best_move = MOVE_BROKEN;
  int alpha = -INFTY;
  int beta = INFTY;

  for (int i = 0; i < moves.size(); i++) {
    int m = moves.get(i);
    Board copy = b.copy();
    Eval ec = *e;
    uint64_t mask = copy.get_do_move(c, m);
    ec.update(c, m, mask);
    copy.do_move(c, m, mask);
    search_info->nodes++;
    if (i != 0) {
      score = -pvs(copy, &ec, ~c, depth-1, -alpha-1, -alpha, false, search_info);
      if (alpha < score && score < beta)
        score = -pvs(copy, &ec, ~c, depth-1, -beta, -alpha, false, search_info);
    }
    else
      score = -pvs(copy, &ec, ~c, depth-1, -beta, -alpha, false, search_info);
    // Handle timeouts
    if (score == TIMEOUT)
      return MOVE_BROKEN;

    if (score > alpha) {
      alpha = score;
      *best_score = score;
      best_move = i;
    }
  }

  return best_move;
}
