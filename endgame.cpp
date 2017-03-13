#include <iostream>
#include "endgame.h"

using namespace std;

const int STAB_THRESHOLD[40] = {
/*
    64, 64, 64, 64, 64,
    12, 14, 16, 18, 20,
    22, 24, 26, 28, 30,
    32, 34, 36, 38, 40,
    42, 44, 46, 48, 50,
    52, 54, 56, 58, 58,
    60, 60, 62, 62, 64,
    64, 64, 64, 64, 64
*/
    64, 64, 64, 64, 64,
    14, 16, 18, 20, 22,
    24, 26, 28, 30, 32,
    34, 36, 38, 40, 42,
    44, 46, 48, 50, 52,
    54, 56, 56, 58, 58,
    60, 60, 62, 62, 64,
    64, 64, 64, 64, 64
};

const int ROOT_SORT_DEPTHS[38] = { 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 2, 2, 4, 4, 6, 6, 8,
    8, 10, 10, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12
};

// Depths for sort searching. Indexed by depth.
const int ENDGAME_SORT_DEPTHS[38] = { 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 2, 2, 2, 2, 4, 4,
    4, 4, 6, 6, 6, 6, 8, 8, 8, 8,
    10, 10, 10, 10, 12, 12, 12
};

const int END_SHALLOW = 12;
const int MIN_TT_DEPTH = 9;

const int SCORE_TIMEOUT = 65;
const int MOVE_FAIL_LOW = -1;
const int EG_SCALE_FACTOR = 600;

struct EndgameStatistics {
    uint64_t hashHits, hashCuts;
    uint64_t hashMoveAttempts, hashMoveCuts;
    uint64_t firstFailHighs, failHighs;
    uint64_t sortSearchNodes;

    void reset() {
        hashHits = 0;
        hashCuts = 0;
        hashMoveAttempts = 0;
        hashMoveCuts = 0;
        firstFailHighs = 0;
        failHighs = 0;
        sortSearchNodes = 0;
    }
};

Endgame::Endgame() {
    // 16384 entries
    endgameTable = new EndHash(14);
    // 2^23 entries * 20 bytes/entry = 168 MB
    killerTable = new EndHash(23);
    // 2^22 entries * 20 bytes/entry = 84 MB
    allTable = new EndHash(22);
    // 2^20 array slots (2^21 entries) * 64 bytes/slot = 67 MB
    transpositionTable = new Hash(20);

    egStats = new EndgameStatistics();
}

Endgame::~Endgame() {
    delete endgameTable;
    delete killerTable;
    delete allTable;
    delete transpositionTable;

    delete egStats;
}
/**
 * @brief Solves the endgame for perfect play.
 * @param b The board to solve
 * @param moves The list of legal moves for the side to move
 * @param isSorted Whether the legal moves list is already sorted or not. If not,
 * it must be sorted within the endgame solver.
 * @param s The side to move
 * @param depth The number of empty squares left on the board
 * @param timeLimit The time limit in milliseconds
 * @param eval A pointer to the evaluater object
 * @param exactScore An optional parameter to also get the exact score
 * @return The index of the best move
 */
int Endgame::solveEndgame(Board &b, MoveList &moves, bool isSorted, int s,
    int depth, int timeLimit, Eval *eval, int *exactScore) {
    return solveEndgameWithWindow(b, moves, isSorted, s, depth, -64, 64,
        timeLimit, eval, exactScore);
}

/**
 * @brief Solve the game for final result: win, loss, or draw.
 */
int Endgame::solveWLD(Board &b, MoveList &moves, bool isSorted, int s,
    int depth, int timeLimit, Eval *eval, int *exactScore) {
    #if PRINT_SEARCH_INFO
    cerr << "WLD solver: depth " << depth << endl;
    #endif
    int gameResult = -2;
    int bestMove = solveEndgameWithWindow(b, moves, isSorted, s, depth, -1, 1,
        timeLimit, eval, &gameResult);

    #if PRINT_SEARCH_INFO
    if (bestMove != MOVE_BROKEN) {
        if (gameResult == 0) {
            cerr << "Game is draw" << endl;
        }
        else if (gameResult <= -1) {
            cerr << "Game is loss" << endl;
        }
        else {
            cerr << "Game is win" << endl;
        }
    }
    #endif
    if (exactScore != NULL)
        *exactScore = gameResult;

    return bestMove;
}

/**
 * @brief "Solve" the game for a result based on the given alpha-beta window.
 * A window of -64, 64 is exact result.
 * A window of -1, 1 is win/loss/draw.
 */
int Endgame::solveEndgameWithWindow(Board &b, MoveList &moves, bool isSorted,
    int s, int depth, int alpha, int beta, int timeLimit, Eval *eval,
    int *exactScore) {
    // if best move for this position has already been found and stored
    EndgameEntry *entry = endgameTable->get(b, s);
    if(entry != NULL) {
        #if PRINT_SEARCH_INFO
        cerr << "Endgame hashtable hit." << endl;
        cerr << "Best move: ";
        printMove(entry->move);
        cerr << " Score: " << (int) (entry->score) << endl;
        #endif
        if (exactScore != NULL)
            *exactScore = entry->score;
        return entry->move;
    }

    auto startTime = OthelloClock::now();
    double timeSpan = 0.0;

    nodes = 0;
    sortBranch = 0;
    egStats->reset();
    evaluater = eval;
    timeElapsed = OthelloClock::now();
    timeout = timeLimit;

    MoveList scores;
    // Initial sorting of moves
    if (!isSorted && depth > 13) {
        sortSearch(b, moves, scores, s, ROOT_SORT_DEPTHS[depth]);

        sort(moves, scores, 0, moves.size-1);

        timeSpan = getTimeElapsed(startTime);
        #if PRINT_SEARCH_INFO
        cerr << "Sort search took: " << timeSpan << " sec" << endl;
        cerr << "PV: ";
        printMove(moves.get(0));
        cerr << " Score: " << scores.get(0) / EG_SCALE_FACTOR << endl;
        #endif
    }

    startTime = OthelloClock::now();

    // Playing with aspiration windows...
    int score;
    int aspAlpha = alpha;
    int aspBeta = beta;
    int bestIndex = MOVE_FAIL_LOW;
    if (!isSorted && depth > 13) {
        aspAlpha = max(scores.get(0) / EG_SCALE_FACTOR - 4, alpha);
        aspBeta = min(scores.get(0) / EG_SCALE_FACTOR + 4, beta);
        // To prevent errors if our sort search score was outside [alpha, beta]
        if (aspAlpha >= beta)
            aspAlpha = beta - 2;
        if (aspBeta <= alpha)
            aspBeta = alpha + 2;
    }
    while (true) {
        // Try a search
        bestIndex = endgameAspiration(b, moves, s, depth, aspAlpha, aspBeta,
            score);
        // If we got broken out
        if (bestIndex == MOVE_BROKEN)
            return MOVE_BROKEN;
        // If we failed low. This is really bad :(
        if (bestIndex == MOVE_FAIL_LOW && aspAlpha > alpha) {
            // We were < than the lower bound, so this is the new upper bound
            aspBeta = score + 1;
            // Using 8 wide windows for now...
            aspAlpha = max(score - 7, alpha);
        }
        // If we failed high.
        else if (score >= aspBeta && aspBeta < beta) {
            // We were > than the upper bound, so this is the new lower bound
            aspAlpha = score - 1;
            // Using 8 wide windows for now...
            aspBeta = min(score + 7, beta);
            // Swap the cut move to the front, it's the best we have right now
            moves.swap(bestIndex, 0);
        }
        // Otherwise we are done
        else {
            break;
        }
    }
    // Retrieve the best move
    int bestMove = MOVE_FAIL_LOW;
    if (bestIndex != MOVE_FAIL_LOW)
        bestMove = moves.get(bestIndex);

    #if PRINT_SEARCH_INFO
    cerr << "Hashtable keys: PV=" << endgameTable->keys << " | A="
                               << killerTable->keys << " | B="
                               << allTable->keys << endl;
    // cerr << "Sort search table has: " << transpositionTable->keys << " keys." << endl;

    timeSpan = getTimeElapsed(startTime);
    cerr << "Nodes searched: " << nodes << " | NPS: " <<
        (int) ((double) nodes / timeSpan) << endl;
    // cerr << "Sort search nodes: " << egStats->sortSearchNodes << endl;

    // cerr << "Hash score cut rate: " << egStats->hashCuts << " / " << egStats->hashHits << endl;
    // cerr << "Hash move cut rate: " << egStats->hashMoveCuts << " / " << egStats->hashMoveAttempts << endl;
    // cerr << "First fail high rate: " << egStats->firstFailHighs << " / " << egStats->failHighs << endl;

    cerr << "Time spent: " << timeSpan << endl;
    cerr << "Best move: ";
    // If we failed low on the bounds we were given, that isn't our business
    if (bestMove == MOVE_FAIL_LOW)
        cerr << "N/A";
    else
        printMove(bestMove);
    cerr << " Score: " << score << endl << endl;
    #endif

    if (exactScore != NULL)
        *exactScore = score;
    return bestMove;
}

// Performs an aspiration search. Returns the index of the best move.
int Endgame::endgameAspiration(Board &b, MoveList &moves, int s, int depth,
    int alpha, int beta, int &exactScore) {
    #if PRINT_SEARCH_INFO
    cerr << "Aspiration search: [" << alpha << ", " << beta << "]" << endl;
    #endif
    int score, bestScore = -INFTY;
    // If this doesn't change, we failed low
    int bestIndex = MOVE_FAIL_LOW;
    for (unsigned int i = 0; i < moves.size; i++) {
        Board copy = b.copy();
        copy.doMove(moves.get(i), s);
        nodes++;
        sortBranch = i;

        if (i != 0) {
            score = -dispatch(copy, s^1, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -dispatch(copy, s^1, depth-1, -beta, -alpha);
        }
        else
            score = -dispatch(copy, s^1, depth-1, -beta, -alpha);

        if (score == SCORE_TIMEOUT) {
            #if PRINT_SEARCH_INFO
            cerr << "Breaking out of endgame solver." << endl;
            #endif
            // If we have already found a winning move, mind as well take it.
            if (bestIndex != MOVE_FAIL_LOW && alpha > 0) {
                exactScore = alpha;
                return bestIndex;
            }
            else
                return MOVE_BROKEN;
        }

        #if PRINT_SEARCH_INFO
        if (depth >= 20) {
            cerr << "Searched move: ";
            printMove(moves.get(i));
            cerr << " | best score: " << score << endl;
        }
        #endif

        if (score >= beta) {
            exactScore = score;
            return i;
        }
        if (score > bestScore) {
            bestScore = score;
            if (alpha < score) {
                alpha = score;
                bestIndex = i;
            }
        }
    }

    exactScore = bestScore;
    return bestIndex;
}

// From root, this function chooses the correct helper to call.
int Endgame::dispatch(Board &b, int s, int depth, int alpha, int beta) {
    int score;
    switch(depth) {
        case 4:
            score = endgame4(b, s, alpha, beta, false);
            break;
        case 3:
            score = endgame3(b, s, alpha, beta, false);
            break;
        case 2:
            score = endgame2(b, s, alpha, beta);
            break;
        default:
            score = endgameDeep(b, s, depth, alpha, beta, false);
            break;
    }
    return score;
}

/**
 * @brief Function for endgame solver. Used when many empty squares remain.
 * A best move table, stability cutoff, killer heuristic cutoff, sort search,
 * and fastest first are used to reduce nodes searched.
 */
int Endgame::endgameDeep(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    if(depth <= END_SHALLOW)
        return endgameShallow(b, s, depth, alpha, beta, passedLast);

    int score, bestScore = -INFTY;
    int prevAlpha = alpha;

    // play best move, if recorded
    EndgameEntry *exactEntry = endgameTable->get(b, s);
    if(exactEntry != NULL) {
        return exactEntry->score;
    }

    // Stability cutoff: if the current position is hopeless compared to a
    // known lower bound, then we need not waste time searching it.
    #if USE_STABILITY
    if(alpha >= STAB_THRESHOLD[depth]) {
        score = 64 - 2*b.getStability(s^1);
        if(score <= alpha) {
            return score;
        }
    }
    #endif

    EndgameEntry *allEntry = allTable->get(b, s);
    if(allEntry != NULL) {
        if (allEntry->score <= alpha)
            return allEntry->score;
        if (beta > allEntry->score)
            beta = allEntry->score;
    }

    // attempt killer heuristic cutoff, using saved alpha
    int hashMove = MOVE_NULL;
    EndgameEntry *killerEntry = killerTable->get(b, s);
    if(killerEntry != NULL) {
        egStats->hashHits++;
        if (killerEntry->score >= beta) {
            egStats->hashCuts++;
            return killerEntry->score;
        }
        // Fail high is lower bound on score so this is valid
        if (alpha < killerEntry->score)
            alpha = killerEntry->score;
        hashMove = killerEntry->move;

        // Try the move for a cutoff before move generation
        egStats->hashMoveAttempts++;
        Board copy = b.copy();
        copy.doMove(hashMove, s);
        nodes++;

        score = -endgameDeep(copy, s^1, depth-1, -beta, -alpha, false);
        // If we received a timeout signal, propagate it upwards
        if (score == SCORE_TIMEOUT)
            return -SCORE_TIMEOUT;

        if (score >= beta) {
            egStats->hashMoveCuts++;
            return score;
        }
        if (score > bestScore) {
            bestScore = score;
            if (alpha < score) {
                alpha = score;
            }
        }
    }

    MoveList legalMoves = b.getLegalMoves(s);
    if(legalMoves.size <= 0) {
        if(passedLast) {
            return (2 * b.count(s) - 64 + depth);
        }

        score = -endgameDeep(b, s^1, depth, -beta, -alpha, true);
        // If we received a timeout signal, propagate it upwards
        if (score == SCORE_TIMEOUT)
            return -SCORE_TIMEOUT;

        return score;
    }

    MoveList priority;
    // Do better move ordering for PV nodes where alpha != beta - 1
    if (alpha != beta - 1) {
        // Use a shallow search for move ordering
        MoveList scores;
        sortSearch(b, legalMoves, scores, s, ENDGAME_SORT_DEPTHS[depth+2]);
        for(unsigned int i = 0; i < legalMoves.size; i++) {
            int m = legalMoves.get(i);
            Board copy = b.copy();
            copy.doMove(m, s);

            int p = SQ_VAL[m];
            if (m == hashMove)
                p += 1 << 20;

            priority.add(scores.get(i) - 128*copy.numLegalMoves(s^1) + 8*p);
        }
    }
    // Otherwise, we focus more on fastest first for a cheaper fail-high
    else {
        MoveList scores;
        sortSearch(b, legalMoves, scores, s, ENDGAME_SORT_DEPTHS[depth]);
        // Restrict opponent's mobility and potential mobility
        for(unsigned int i = 0; i < legalMoves.size; i++) {
            int m = legalMoves.get(i);
            Board copy = b.copy();
            copy.doMove(m, s);

            int p = 8 * SQ_VAL[m];
            if (m == hashMove)
                p += 1 << 20;

            priority.add(scores.get(i) - 2304*copy.numLegalMoves(s^1) + 32*p);
        }
    }

    int tempMove = MOVE_NULL;
    unsigned int i = 0;
    for (int m = nextMove(legalMoves, priority, i); m != MOVE_NULL;
             m = nextMove(legalMoves, priority, ++i)) {
        // Check for a timeout
        double timeSpan = getTimeElapsed(timeElapsed);
        if (timeSpan * 1000 > timeout)
            return -SCORE_TIMEOUT;
        // We already tried the hash move
        if (legalMoves.get(i) == hashMove)
            continue;
        Board copy = b.copy();
        copy.doMove(m, s);
        nodes++;

        if (i != 0) {
            score = -endgameDeep(copy, s^1, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta)
                score = -endgameDeep(copy, s^1, depth-1, -beta, -alpha, false);
        }
        else
            score = -endgameDeep(copy, s^1, depth-1, -beta, -alpha, false);

        // If we received a timeout signal, propagate it upwards
        if (score == SCORE_TIMEOUT)
            return -SCORE_TIMEOUT;
        if (score >= beta) {
            egStats->failHighs++;
            if (i == 0)
                egStats->firstFailHighs++;
            killerTable->add(b, score, m, s, depth);
            return score;
        }
        if (score > bestScore) {
            bestScore = score;
            if (alpha < score) {
                alpha = score;
                tempMove = m;
            }
        }
    }

    // Best move with exact score if alpha < score < beta
    if (tempMove != MOVE_NULL && prevAlpha < alpha && alpha < beta)
        endgameTable->add(b, alpha, tempMove, s, depth);
    else if (alpha <= prevAlpha)
        allTable->add(b, bestScore, MOVE_NULL, s, depth);

    return bestScore;
}

/**
 * @brief Endgame solver, to be used with about 12 or less empty squares.
 * Here, it is no longer efficient to use heavy sorting.
 * The MoveList is dropped in favor of a faster array on the stack.
 * The hash table is used until about depth 9.
 * Fastest first is used until depth 7, otherwise, moves are just sorted by
 * hole parity and piece square tables.
 */
int Endgame::endgameShallow(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    if(depth == 4)
        return endgame4(b, s, alpha, beta, passedLast);

    // Immediately return an exact score, if available
    EndgameEntry *exactEntry = endgameTable->get(b, s);
    if(exactEntry != NULL) {
        return exactEntry->score;
    }

    int score, bestScore = -INFTY;
    int prevAlpha = alpha;

    #if USE_STABILITY
    if(alpha >= STAB_THRESHOLD[depth]) {
        score = 64 - 2*b.getStability(s^1);
        if(score <= alpha) {
            return score;
        }
    }
    #endif

    int hashMove = MOVE_NULL;
    // Probe hash tables
    if (depth >= MIN_TT_DEPTH) {
        EndgameEntry *allEntry = allTable->get(b, s);
        if(allEntry != NULL) {
            if (allEntry->score <= alpha)
                return allEntry->score;
            if (beta > allEntry->score)
                beta = allEntry->score;
        }

        EndgameEntry *killerEntry = killerTable->get(b, s);
        if(killerEntry != NULL) {
            egStats->hashHits++;
            if (killerEntry->score >= beta) {
                egStats->hashCuts++;
                return killerEntry->score;
            }
            if (alpha < killerEntry->score)
                alpha = killerEntry->score;

            // If no score cutoff, try the hash move before move generation
            egStats->hashMoveAttempts++;
            hashMove = killerEntry->move;
            Board copy = b.copy();
            copy.doMove(hashMove, s);
            nodes++;

            score = -endgameShallow(copy, s^1, depth-1, -beta, -alpha, false);

            if (score >= beta) {
                egStats->hashMoveCuts++;
                egStats->failHighs++;
                egStats->firstFailHighs++;
                return score;
            }
            if (score > bestScore) {
                bestScore = score;
                if (alpha < score) {
                    alpha = score;
                }
            }
        }
    }

    bitbrd legal = b.getLegal(s);
    if(!legal) {
        if(passedLast)
            return (2 * b.count(s) - 64 + depth);

        return -endgameShallow(b, s^1, depth, -beta, -alpha, true);
    }

    if (hashMove != MOVE_NULL)
        legal ^= MOVEMASK[hashMove];

    // create array of legal moves
    int moves[END_SHALLOW];
    int priority[END_SHALLOW];
    int n = 0;

    bitbrd empty = ~b.getTaken();
    while (legal) {
        int m = bitScanForward(legal);
        legal &= legal-1;

        priority[n] = SQ_VAL[m];
        if(!(NEIGHBORS[m] & empty))
            priority[n] += 64;
        moves[n] = m;
        n++;
    }

    // Only do fastest first on null window searches (non-PV nodes)
    if (depth >= 7 && (alpha == beta - 1)) {
        for(int i = 0; i < n; i++) {
            Board copy = b.copy();
            copy.doMove(moves[i], s);

            priority[i] += -128*copy.numLegalMoves(s^1);
        }
    }

    // search all moves
    int i = 0;
    int tempMove = MOVE_NULL;
    for (int move = nextMoveShallow(moves, priority, n, i); move != MOVE_NULL;
             move = nextMoveShallow(moves, priority, n, ++i)) {
        Board copy = b.copy();
        copy.doMove(move, s);
        nodes++;

        if (i != 0 || hashMove != MOVE_NULL) {
            score = -endgameShallow(copy, s^1, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta)
                score = -endgameShallow(copy, s^1, depth-1, -beta, -alpha, false);
        }
        else
            score = -endgameShallow(copy, s^1, depth-1, -beta, -alpha, false);

        if (score >= beta) {
            egStats->failHighs++;
            if (i == 0)
                egStats->firstFailHighs++;
            if (depth >= MIN_TT_DEPTH)
                killerTable->add(b, score, move, s, depth);
            return score;
        }
        if (score > bestScore) {
            bestScore = score;
            if (alpha < score) {
                alpha = score;
                tempMove = move;
            }
        }
    }

    // Best move with exact score if alpha < score < beta
    if (tempMove != MOVE_NULL && prevAlpha < alpha && alpha < beta)
        endgameTable->add(b, alpha, tempMove, s, depth);
    else if (depth >= MIN_TT_DEPTH && prevAlpha == alpha)
        allTable->add(b, bestScore, MOVE_NULL, s, depth);

    return bestScore;
}

/**
 * @brief Endgame solver, to be used with exactly 4 empty squares.
 * Starting with 4 moves left, a special legal moves generator that sorts by
 * hole parity is called.
 */
int Endgame::endgame4(Board &b, int s, int alpha, int beta, bool passedLast) {
    int score, bestScore = -INFTY;
    int legalMoves[4];
    int n = b.getLegalMoves4(s, legalMoves);

    if (n == 0) {
        if (passedLast)
            return (2 * b.count(s) - 60);

        return -endgame4(b, s^1, -beta, -alpha, true);
    }

    for (int i = 0; i < n; i++) {
        Board copy = b.copy();
        copy.doMove(legalMoves[i], s);
        nodes++;

        if (i != 0) {
            score = -endgame3(copy, s^1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta)
                score = -endgame3(copy, s^1, -beta, -alpha, false);
        }
        else
            score = -endgame3(copy, s^1, -beta, -alpha, false);

        if (score >= beta)
            return score;
        if (score > bestScore) {
            bestScore = score;
            if (alpha < score)
                alpha = score;
        }
    }

    return bestScore;
}

/**
 * @brief Endgame solver, to be used with exactly 3 empty squares.
 */
int Endgame::endgame3(Board &b, int s, int alpha, int beta, bool passedLast) {
    int score, bestScore = -INFTY;
    int legalMoves[3];
    int n = b.getLegalMoves3(s, legalMoves);

    if (n == 0) {
        if (passedLast)
            return (2 * b.count(s) - 61);

        return -endgame3(b, s^1, -beta, -alpha, true);
    }

    for (int i = 0; i < n; i++) {
        Board copy = b.copy();
        copy.doMove(legalMoves[i], s);
        nodes++;

        if (i != 0) {
            score = -endgame2(copy, s^1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -endgame2(copy, s^1, -beta, -alpha);
        }
        else
            score = -endgame2(copy, s^1, -beta, -alpha);

        if (score >= beta)
            return score;
        if (score > bestScore) {
            bestScore = score;
            if (alpha < score)
                alpha = score;
        }
    }

    return bestScore;
}

/**
 * @brief Endgame solver, to be used with exactly 2 empty squares.
 * Null window searches are no longer performed here.
 */
int Endgame::endgame2(Board &b, int s, int alpha, int beta) {
    int score = -INFTY, bestScore = -INFTY;
    bitbrd empty = ~b.getTaken();
    bitbrd opp = b.getBits(s^1);

    // At 2 squares left, it is more efficient to simply try moves on both
    // squares. This approach was based on Richard Delorme's Edax-Reversi.
    int lm1 = bitScanForward(empty);
    empty &= empty-1;
    int lm2 = bitScanForward(empty);

    bitbrd changeMask;

    if ((opp & NEIGHBORS[lm1]) && (changeMask = b.getDoMove(lm1, s))) {
        b.makeMove(lm1, changeMask, s);
        nodes++;
        score = -endgame1(b, s^1, -beta, lm2);
        b.undoMove(lm1, changeMask, s);

        if (score >= beta)
            return score;
        if (score > bestScore)
            bestScore = score;
    }

    if ((opp & NEIGHBORS[lm2]) && (changeMask = b.getDoMove(lm2, s))) {
        b.makeMove(lm2, changeMask, s);
        nodes++;
        score = -endgame1(b, s^1, -beta, lm1);
        b.undoMove(lm2, changeMask, s);

        if (score >= beta)
            return score;
        if (score > bestScore)
            bestScore = score;
    }

    // if no legal moves... try other player
    if (score == -INFTY) {
        bestScore = INFTY;
        opp = b.getBits(s);

        if ((opp & NEIGHBORS[lm1]) && (changeMask = b.getDoMove(lm1, s^1))) {
            b.makeMove(lm1, changeMask, s^1);
            nodes++;
            score = endgame1(b, s, alpha, lm2);
            b.undoMove(lm1, changeMask, s^1);

            if (alpha >= score)
                return score;
            if (score < bestScore)
                bestScore = score;
        }

        if ((opp & NEIGHBORS[lm2]) && (changeMask = b.getDoMove(lm2, s^1))) {
            b.makeMove(lm2, changeMask, s^1);
            nodes++;
            score = endgame1(b, s, alpha, lm1);
            b.undoMove(lm2, changeMask, s^1);

            if (alpha >= score)
                return score;
            if (score < bestScore)
                bestScore = score;
        }

        // if both players passed, game over
        if (score == -INFTY)
            return (2 * b.count(s) - 62);
    }

    return bestScore;
}

/**
 * @brief Endgame solver, to be used with exactly 1 empty square.
 */
int Endgame::endgame1(Board &b, int s, int alpha, int legalMove) {
    // Get a stand pat score
    int score = 2 * b.count(s) - 63;

    bitbrd changeMask = b.getDoMove(legalMove, s);
    nodes++;
    // If the player "s" can move, calculate final score
    if (changeMask) {
        score += 2 * countSetBits(changeMask) + 1;
    }
    // Otherwise, it is the opponent's move. If the opponent can stand pat,
    // we don't need to calculate the final score.
    else if (score >= alpha) {
        bitbrd otherMask = b.getDoMove(legalMove, s^1);
        nodes++;
        if (otherMask)
            score -= 2 * countSetBits(otherMask) + 1;
    }
    // If both players passed, the empty square should not count against us
    //else
    //    score++;

    return score;
}

//--------------------------------Sort Search-----------------------------------

// Performs an alpha-beta search on each legal move to get a score.
void Endgame::sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth) {
    for (unsigned int i = 0; i < moves.size; i++) {
        Board copy = b.copy();
        copy.doMove(moves.get(i), side);
        egStats->sortSearchNodes++;
        scores.add(-pvs(copy, side^1, depth-1, -INFTY, INFTY));
    }
}

// Helper function for the alpha-beta search.
int Endgame::pvs(Board &b, int s, int depth, int alpha, int beta) {
    if (depth <= 0) {
        return (s == CBLACK) ? evaluater->end_heuristic(b)
                             : -evaluater->end_heuristic(b);
    }

    int score;
    int prevAlpha = alpha;
    int hashed = MOVE_NULL;
    int toHash = MOVE_NULL;

    // Probe transposition table for a score or move
    // Only do this at and above depth 3 for speed
    if (depth >= 3) {
        BoardData *entry = transpositionTable->get(b, s);
        if (entry != NULL) {
            if (entry->nodeType == ALL_NODE) {
                if (entry->depth >= depth && entry->score <= alpha)
                    return alpha;
            }
            else {
                if (entry->depth >= depth) {
                    if (entry->nodeType == CUT_NODE && entry->score >= beta)
                        return beta;
                    else if (entry->nodeType == PV_NODE)
                        return entry->score;
                }

                hashed = entry->move;
                Board copy = b.copy();
                copy.doMove(hashed, s);
                egStats->sortSearchNodes++;

                score = -pvs(copy, s^1, depth-1, -beta, -alpha);

                if (alpha < score)
                    alpha = score;
                if (alpha >= beta)
                    return beta;
            }
        }
    }

    // We want to do better move ordering at PV nodes where alpha != beta - 1
    bool isPVNode = (alpha != beta - 1);

    MoveList legalMoves = b.getLegalMoves(s);
    if (legalMoves.size <= 0) {
        score = -pvs(b, s^1, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;

        return alpha;
    }

    // Do internal iterative deepening
    if (depth >= 2) {
        MoveList scores;
        if (depth >= 4 && isPVNode)
            sortSearch(b, legalMoves, scores, s, (depth-1)/2);
        else if (depth >= 5) {
            sortSearch(b, legalMoves, scores, s, (depth-1)/3);
            // Apparently fastest first works in sort searches too...
            for (unsigned int i = 0; i < legalMoves.size; i++) {
                Board copy = b.copy();
                copy.doMove(legalMoves.get(i), s);
                scores.set(i, scores.get(i) - 1024*copy.numLegalMoves(s^1));
            }
        }
        else if (depth >= 3) {
            for (unsigned int i = 0; i < legalMoves.size; i++) {
                Board copy = b.copy();
                copy.doMove(legalMoves.get(i), s);
                scores.add(SQ_VAL[legalMoves.get(i)] - 16*copy.numLegalMoves(s^1));
            }
        }
        else {
            for (unsigned int i = 0; i < legalMoves.size; i++)
                scores.add(SQ_VAL[legalMoves.get(i)]);
        }
        sort(legalMoves, scores, 0, legalMoves.size-1);
    }

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        int m = legalMoves.get(i);
    /*unsigned int i = 0;
    for (int m = nextMove(legalMoves, scores, i); m != MOVE_NULL;
             m = nextMove(legalMoves, scores, ++i)) {*/
        if (hashed == m)
            continue;
        Board copy = b.copy();
        copy.doMove(m, s);
        egStats->sortSearchNodes++;

        if (depth > 2 && i != 0) {
            score = -pvs(copy, s^1, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -pvs(copy, s^1, depth-1, -beta, -alpha);
        }
        else
            score = -pvs(copy, s^1, depth-1, -beta, -alpha);

        if (alpha < score) {
            alpha = score;
            toHash = m;
        }
        if (alpha >= beta) {
            if(depth >= 4)
                transpositionTable->add(b, beta, m, s, sortBranch, depth,
                    CUT_NODE);
            return beta;
        }
    }

    if (depth >= 4 && toHash != MOVE_NULL && prevAlpha < alpha && alpha < beta)
        transpositionTable->add(b, alpha, toHash, s, sortBranch, depth, PV_NODE);
    else if (depth >= 4 && alpha <= prevAlpha)
        transpositionTable->add(b, alpha, MOVE_NULL, s, sortBranch, depth, ALL_NODE);

    return alpha;
}

//--------------------------------Utilities-------------------------------------

// Retrieves the next move with the highest score, starting from index using a
// partial selection sort. This way, the entire list does not have to be sorted
// if an early cutoff occurs.
int Endgame::nextMoveShallow(int *moves, int *scores, int size, int index) {
    if (index >= size)
        return MOVE_NULL;
    // Find the index of the next best move/score
    int bestIndex = index;
    for (int i = index + 1; i < size; i++) {
        if (scores[i] > scores[bestIndex]) {
            bestIndex = i;
        }
    }
    // swap to the correct position
    int tempMove = moves[bestIndex];
    moves[bestIndex] = moves[index];
    moves[index] = tempMove;
    int tempScore = scores[bestIndex];
    scores[bestIndex] = scores[index];
    scores[index] = tempScore;
    // return the move
    return moves[index];
}
