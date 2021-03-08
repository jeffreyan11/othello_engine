#ifndef __ENDGAME_H__
#define __ENDGAME_H__

#include "common.h"
#include "board.h"
#include "endhash.h"
#include "eval.h"
#include "hash.h"
#include "search.h"

struct EndgameStatistics;

// Resizes all endgame hash tables with:
// 2^pv_bits entries for the PV table
// 2^(pv_bits+9) entries for the cut table
// 2^(pv_bits+8) entries for the all table
// 2^(pv_bits+2) entries for the sort search table
void resize_endhash(uint32_t pv_bits);

// This class contains a large number of functions to help solve the endgame
// for a game result or perfect play.
class Endgame {
 public:
  uint64_t nodes;

  Endgame();
  ~Endgame() = default;

  /**
   * @brief Solves the endgame for perfect play.
   * @param b The board to solve
   * @param moves The list of legal moves for the side to move
   * @param is_sorted Whether the legal moves list is already sorted or not. If not,
   * it must be sorted within the endgame solver.
   * @param s The side to move
   * @param depth The number of empty squares left on the board
   * @param time_limit The time limit in milliseconds
   * @param exact_score An optional parameter to also get the exact score
   * @return The index of the best move
   */
  int solve_endgame(Board &b, Eval* e, Color c, ArrayList &moves, bool is_sorted, int depth,
    int time_limit, int *exact_score = NULL);
  // Solve the game for final result: win, loss, or draw.
  int solve_wld(Board &b, Eval* e, Color c, ArrayList &moves, bool is_sorted, int depth,
    int time_limit, int *exact_score = NULL);
  // Solve the game for a result based on the given alpha-beta window.
  // A window of -64, 64 is exact result.
  // A window of -1, 1 is win/loss/draw.
  int solve_endgame_with_window(Board &b, Eval* e, Color c, ArrayList &moves, bool is_sorted,
    int depth, int alpha, int beta, int time_limit, int *exact_score = NULL);

 private:
  TimePoint searchStart;
  uint64_t timeout;

  // Performs an aspiration search. Returns the index of the best move.
  int endgame_aspiration(Board &b, Eval* e, Color c, ArrayList &moves, int depth,
    int alpha, int beta, int &exact_score, SearchInfo* search_info);
  // From root, this function chooses the correct helper to call.
  int dispatch(Board &b, Eval* e, Color c, int depth, int alpha, int beta, SearchInfo* search_info);
  // Function for endgame solver, used when many empty squares remain.
  // Hash tables, stability cutoff, sort search, and fastest first are used to
  // reduce nodes searched.
  int endgame_deep(Board &b, Eval* e, Color c, int depth, int alpha, int beta, bool passed_last, SearchInfo* search_info);
  // Endgame solver without sort searches.
  int endgame_medium(Board &b, Color c, int depth, int alpha, int beta, bool passed_last);
  // Endgame solver, to be used with about 6 or less empty squares.
  // Here, it is no longer efficient to use heavy sorting: moves are just
  // sorted by hole parity and piece square tables. The ArrayList is dropped
  // in favor of a faster array on the stack with SIMD for moves and scores.
  int endgame_shallow(Board &b, Color c, int depth, int alpha, int beta, bool passed_last);
  // Endgame solvers, to be used with exactly 1-4 moves remaining.
  // At depth 4, only hole parity is used for sorting. Each empty square is tested
  //             directly for legality. Null window searches are no longer done.
  // At depth 2, the move loop is manually unrolled.
  // At depth 1, the flipped stones are counted without making the move on the board.
  int endgame4(Board &b, Color c, int alpha, int beta, bool passed_last);
  int endgame3(Board &b, Color c, int alpha, int beta, bool passed_last);
  int endgame2(Board &b, Color c, int alpha, int beta, int lm1, int lm2);
  int endgame1(Board &b, Color c, int alpha, int legal_move);

  int next_move_shallow(int *moves, int size, int index);
};

#endif
