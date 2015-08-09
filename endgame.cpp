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

// Depths for sort searching. Indexed by depth.
const int ENDGAME_SORT_DEPTHS[36] = { 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 2, 2, 2, 2, 4, 4,
    4, 6, 6, 6, 8, 8, 8, 10, 10, 10,
    10, 10, 12, 12, 12
};

const int END_SHALLOW = 12;
const int MIN_TT_DEPTH = 9;

const int SCORE_TIMEOUT = 65;
const int MOVE_FAIL_LOW = -1;

struct EndgameStatistics {
    uint64_t hashHits;
    uint64_t hashCuts;
    uint64_t hashMoveAttempts;
    uint64_t hashMoveCuts;
    uint64_t firstFailHighs;
    uint64_t failHighs;
    uint64_t searchSpaces;
    uint64_t sortSearchNodes;

    void reset() {
        hashHits = 0;
        hashCuts = 0;
        hashMoveAttempts = 0;
        hashMoveCuts = 0;
        firstFailHighs = 0;
        failHighs = 0;
        searchSpaces = 0;
        sortSearchNodes = 0;
    }
};

Endgame::Endgame() {
    // 16384 entries
    endgame_table = new EndHash(14);
    // 2^23 entries * 28 bytes/entry = 235 MB
    killer_table = new EndHash(23);
    // 2^22 entries * 28 bytes/entry = 117 MB
    all_table = new EndHash(22);
    // 2^20 array slots (2^21 entries) * 64 bytes/slot = 67 MB
    transpositionTable = new Hash(20);

    egStats = new EndgameStatistics();
}

Endgame::~Endgame() {
    delete endgame_table;
    delete killer_table;
    delete all_table;
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
    isWLD = false;
    return solveEndgameWithWindow(b, moves, isSorted, s, depth, -64, 64,
        timeLimit, eval, exactScore);
}

/**
 * @brief Solve the game for final result: win, loss, or draw.
 */
int Endgame::solveWLD(Board &b, MoveList &moves, bool isSorted, int s,
    int depth, int timeLimit, Eval *eval, int *exactScore) {
    cerr << "Starting WLD search" << endl;
    isWLD = true;
    int gameResult = -2;
    int bestMove = solveEndgameWithWindow(b, moves, isSorted, s, depth, -1, 1,
        timeLimit, eval, &gameResult);

    if (bestMove != MOVE_BROKEN) {
        if (gameResult == 0) {
            cerr << "Game is draw" << endl;
        }
        else if (gameResult == -1) {
            cerr << "Game is loss" << endl;
        }
        else {
            cerr << "Game is win" << endl;
        }
    }
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
    EndgameEntry *entry = endgame_table->get(b, s);
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

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();
    auto end_time = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(
        end_time-start_time);

    nodes = 0;
    sortBranch = 0;
    egStats->reset();
    evaluater = eval;
    timeElapsed = high_resolution_clock::now();
    timeout = timeLimit;

    MoveList scores;
    // Initial sorting of moves
    if (!isSorted && depth > 11) {
        if (depth > 23)
            sortSearch(b, moves, scores, s, 12);
        else if (depth > 21)
            sortSearch(b, moves, scores, s, 10);
        else if (depth > 19)
            sortSearch(b, moves, scores, s, 8);
        else if (depth > 17)
            sortSearch(b, moves, scores, s, 6);
        else if (depth > 15)
            sortSearch(b, moves, scores, s, 4);
        else
            sortSearch(b, moves, scores, s, 2);
        sort(moves, scores, 0, moves.size-1);
        end_time = high_resolution_clock::now();
        time_span = duration_cast<duration<double>>(end_time-start_time);
        #if PRINT_SEARCH_INFO
        cerr << "Sort search took: " << time_span.count() << " sec" << endl;
        cerr << "PV: ";
        printMove(moves.get(0));
        cerr << " Score: " << scores.get(0) / 600 << endl;
        #endif
    }

    start_time = high_resolution_clock::now();

    // Playing with aspiration windows...
    int score;
    int aspAlpha = alpha;
    int aspBeta = beta;
    int bestIndex = MOVE_FAIL_LOW;
    if (!isSorted && depth > 11) {
        aspAlpha = max(scores.get(0) / 600 - 4, alpha);
        aspBeta = min(scores.get(0) / 600 + 4, beta);
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
        if (bestIndex == MOVE_FAIL_LOW && score > alpha) {
            // We were < than the lower bound, so this is the new upper bound
            aspBeta = score + 1;
            // Using 8 wide windows for now...
            aspAlpha = max(score - 7, alpha);
        }
        // If we failed high.
        else if (score >= aspBeta && score < beta) {
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
    int bestMove = MOVE_BROKEN;
    if (bestIndex != MOVE_FAIL_LOW)
        bestMove = moves.get(bestIndex);

    #if PRINT_SEARCH_INFO
    cerr << "Endgame table has: " << endgame_table->keys << " keys." << endl;
    cerr << "Killer table has: " << killer_table->keys << " keys." << endl;
    cerr << "All-nodes table has: " << all_table->keys << " keys." << endl;
    cerr << "Sort search table has: " << transpositionTable->keys << " keys." << endl;

    end_time = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(end_time-start_time);
    cerr << "Nodes searched: " << nodes << " | NPS: " <<
        (int)((double)nodes / time_span.count()) << endl;
    cerr << "Sort search nodes: " << egStats->sortSearchNodes << endl;
    cerr << "Hash score cut rate: " << egStats->hashCuts << " / " << egStats->hashHits << endl;
    cerr << "Hash move cut rate: " << egStats->hashMoveCuts << " / " << egStats->hashMoveAttempts << endl;
    cerr << "First fail high rate: " << egStats->firstFailHighs << " / " << egStats->failHighs << " / " << egStats->searchSpaces << endl;
    cerr << "Time spent: " << time_span.count() << endl;
    cerr << "Best move: ";
    // If we failed low on the bounds we were given, that isn't our business
    if (bestMove == MOVE_FAIL_LOW)
        cerr << "N/A";
    else
        printMove(bestMove);
    cerr << " Score: " << score << endl;
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
    int score;
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
        cerr << "Searched move: ";
        printMove(moves.get(i));
        cerr << " | alpha: " << score << endl;
        #endif

        if (alpha < score) {
            alpha = score;
            bestIndex = i;
        }
        if (alpha >= beta)
            break;
    }

    exactScore = alpha;
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

    int score;
    int prevAlpha = alpha;

    // play best move, if recorded
    EndgameEntry *exactEntry = endgame_table->get(b, s);
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

    EndgameEntry *allEntry = all_table->get(b, s);
    if(allEntry != NULL) {
        if (allEntry->score <= alpha)
            return alpha;
        if (beta > allEntry->score)
            beta = allEntry->score;
    }

    // attempt killer heuristic cutoff, using saved alpha
    int killer = MOVE_NULL;
    EndgameEntry *killerEntry = killer_table->get(b, s);
    if(killerEntry != NULL) {
        egStats->hashHits++;
        if (killerEntry->score >= beta) {
            egStats->hashCuts++;
            return beta;
        }
        // Fail high is lower bound on score so this is valid
        if (alpha < killerEntry->score)
            alpha = killerEntry->score;
        killer = killerEntry->move;

        // Try the move for a cutoff before move generation
        egStats->hashMoveAttempts++;
        Board copy = b.copy();
        copy.doMove(killer, s);
        nodes++;

        score = -endgameDeep(copy, s^1, depth-1, -beta, -alpha, false);
        // If we received a timeout signal, propagate it upwards
        if (score == SCORE_TIMEOUT)
            return -SCORE_TIMEOUT;

        if (score >= beta) {
            egStats->hashMoveCuts++;
            return beta;
        }
        if (alpha < score) {
            alpha = score;
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

        if (alpha < score)
            alpha = score;
        return alpha;
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
            if (m == killer)
                p += 1 << 20;

            priority.add(scores.get(i) + 8*p);
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
            if (m == killer)
                p += 1 << 20;

            priority.add(scores.get(i) - 2048*copy.numLegalMoves(s^1)
                    - 64*copy.potentialMobility(s^1) + 32*p);
        }
    }

    int tempMove = MOVE_NULL;
    egStats->searchSpaces++;
    unsigned int i = 0;
    for (int m = nextMove(legalMoves, priority, i); m != MOVE_NULL;
             m = nextMove(legalMoves, priority, ++i)) {
        // Check for a timeout
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span =
            std::chrono::duration_cast<std::chrono::duration<double>>(
            end_time-timeElapsed);
        if (time_span.count() * 1000 > timeout)
            return -SCORE_TIMEOUT;
        // We already tried the hash move
        if (legalMoves.get(i) == killer)
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
            killer_table->add(b, beta, m, s, depth);
            return beta;
        }
        if (alpha < score) {
            alpha = score;
            tempMove = m;
        }
    }

    // Best move with exact score if alpha < score < beta
    if (tempMove != MOVE_NULL && prevAlpha < alpha && alpha < beta)
        endgame_table->add(b, alpha, tempMove, s, depth);
    else if (alpha <= prevAlpha)
        all_table->add(b, alpha, MOVE_NULL, s, depth);

    return alpha;
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
    EndgameEntry *exactEntry = endgame_table->get(b, s);
    if(exactEntry != NULL) {
        return exactEntry->score;
    }

    int score;
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
        EndgameEntry *allEntry = all_table->get(b, s);
        if(allEntry != NULL) {
            if (allEntry->score <= alpha)
                return alpha;
            if (beta > allEntry->score)
                beta = allEntry->score;
        }

        EndgameEntry *killerEntry = killer_table->get(b, s);
        if(killerEntry != NULL) {
            egStats->hashHits++;
            if (killerEntry->score >= beta) {
                egStats->hashCuts++;
                return beta;
            }
            if (alpha < killerEntry->score)
                alpha = killerEntry->score;

            // If no score cutoff, try the move early in move ordering
            egStats->hashMoveAttempts++;
            hashMove = killerEntry->move;
        }
    }

    bitbrd legal = b.getLegal(s);
    if(!legal) {
        if(passedLast) {
            return (2 * b.count(s) - 64 + depth);
        }

        score = -endgameShallow(b, s^1, depth, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    // create array of legal moves
    int moves[END_SHALLOW];
    int priority[END_SHALLOW];
    int n = 0;

    bitbrd empty = ~b.getTaken();
    do {
        moves[n] = bitScanForward(legal);
        priority[n] = SQ_VAL[moves[n]];
        if(moves[n] == hashMove)
            priority[n] += 1 << 16;

        if(!(NEIGHBORS[moves[n]] & empty))
            priority[n] += 64;

        legal &= legal-1; n++;
    } while(legal);

    // Only do fastest first on null window searches (non-PV nodes)
    if (depth >= 7 && (alpha == beta - 1)) {
        for(int i = 0; i < n; i++) {
            Board copy = b.copy();
            copy.doMove(moves[i], s);

            priority[i] += -128*copy.numLegalMoves(s^1);
        }
    }

    // search all moves
    egStats->searchSpaces++;
    int i = 0;
    int tempMove = MOVE_NULL;
    for (int move = nextMoveShallow(moves, priority, n, i); move != MOVE_NULL;
             move = nextMoveShallow(moves, priority, n, ++i)) {
        Board copy = b.copy();
        copy.doMove(move, s);
        nodes++;

        if (i != 0) {
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
            if (move == hashMove)
                egStats->hashMoveCuts++;
            if (depth >= MIN_TT_DEPTH)
                killer_table->add(b, beta, move, s, depth);
            return beta;
        }
        if (alpha < score) {
            alpha = score;
            tempMove = move;
        }
    }

    // Best move with exact score if alpha < score < beta
    if (tempMove != MOVE_NULL && prevAlpha < alpha && alpha < beta)
        endgame_table->add(b, alpha, tempMove, s, depth);
    else if (depth >= MIN_TT_DEPTH && alpha <= prevAlpha)
        all_table->add(b, alpha, MOVE_NULL, s, depth);

    return alpha;
}

/**
 * @brief Endgame solver, to be used with exactly 4 empty squares.
 * Starting with 4 moves left, a special legal moves generator that sorts by
 * hole parity is called.
 */
int Endgame::endgame4(Board &b, int s, int alpha, int beta, bool passedLast) {
    int score;
    int legalMoves[4];
    int n = b.getLegalMoves4(s, legalMoves);

    if (n == 0) {
        if (passedLast)
            return (2 * b.count(s) - 60);

        score = -endgame4(b, s^1, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
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
            return beta;
        if (alpha < score)
            alpha = score;
    }

    return alpha;
}

/**
 * @brief Endgame solver, to be used with exactly 3 empty squares.
 */
int Endgame::endgame3(Board &b, int s, int alpha, int beta, bool passedLast) {
    int score;
    int legalMoves[3];
    int n = b.getLegalMoves3(s, legalMoves);

    if (n == 0) {
        if (passedLast)
            return (2 * b.count(s) - 61);

        score = -endgame3(b, s^1, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
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
            return beta;
        if (alpha < score)
            alpha = score;
    }

    return alpha;
}

/**
 * @brief Endgame solver, to be used with exactly 2 empty squares.
 * Null window searches are no longer performed here.
 */
int Endgame::endgame2(Board &b, int s, int alpha, int beta) {
    int score = -INFTY;
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
            return beta;
        if (alpha < score)
            alpha = score;
    }

    if ((opp & NEIGHBORS[lm2]) && (changeMask = b.getDoMove(lm2, s))) {
        b.makeMove(lm2, changeMask, s);
        nodes++;
        score = -endgame1(b, s^1, -beta, lm1);
        b.undoMove(lm2, changeMask, s);

        if (alpha < score)
            alpha = score;
    }

    if (score == -INFTY) {
        opp = b.getBits(s);

        // if no legal moves... try other player
        if ((opp & NEIGHBORS[lm1]) && (changeMask = b.getDoMove(lm1, s^1))) {
            b.makeMove(lm1, changeMask, s^1);
            nodes++;
            score = endgame1(b, s, alpha, lm2);
            b.undoMove(lm1, changeMask, s^1);

            if (alpha >= score)
                return alpha;
            if (beta > score)
                beta = score;
        }

        if ((opp & NEIGHBORS[lm2]) && (changeMask = b.getDoMove(lm2, s^1))) {
            b.makeMove(lm2, changeMask, s^1);
            nodes++;
            score = endgame1(b, s, alpha, lm1);
            b.undoMove(lm2, changeMask, s^1);

            if (beta > score)
                beta = score;
        }

        // if both players passed, game over
        if (score == -INFTY)
            return (2 * b.count(s) - 62);

        return beta;
    }

    return alpha;
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

    MoveList legalMoves = b.getLegalMoves(s);
    if (legalMoves.size <= 0) {
        score = -pvs(b, s^1, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;

        return alpha;
    }

    // We want to do better move ordering at PV nodes where alpha != beta - 1
    bool isPVNode = (alpha != beta - 1);
    // Do internal iterative deepening
    // TODO make this shorter and less repetitive :(
    if (depth >= 2) {
        MoveList scores;
        if (depth >= 5 && isPVNode)
            sortSearch(b, legalMoves, scores, s, (depth >= 9) ? 4 : 2);
        else if (depth >= 4 && isPVNode)
            sortSearch(b, legalMoves, scores, s, 0);
        else if (depth >= 6) {
            sortSearch(b, legalMoves, scores, s, (depth >= 8) ? 2 : 0);
            // Apparently fastest first works in sort searches too...
            for (unsigned int i = 0; i < legalMoves.size; i++) {
                Board copy = b.copy();
                copy.doMove(legalMoves.get(i), s);
                scores.set(i, scores.get(i) - 2048*copy.numLegalMoves(s^1));
            }
        }
        else if (depth >= 4) {
            for (unsigned int i = 0; i < legalMoves.size; i++) {
                Board copy = b.copy();
                copy.doMove(legalMoves.get(i), s);
                scores.add(SQ_VAL[legalMoves.get(i)]
                    - 16*copy.numLegalMoves(s^1));
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
