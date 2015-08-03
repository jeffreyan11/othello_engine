#include <chrono>
#include <iostream>
#include "endgame.h"

const int QUADRANT_ID[64] = {
1, 1, 1, 1, 2, 2, 2, 2,
1, 1, 1, 1, 2, 2, 2, 2,
1, 1, 1, 1, 2, 2, 2, 2,
1, 1, 1, 1, 2, 2, 2, 2,
4, 4, 4, 4, 8, 8, 8, 8,
4, 4, 4, 4, 8, 8, 8, 8,
4, 4, 4, 4, 8, 8, 8, 8,
4, 4, 4, 4, 8, 8, 8, 8
};

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

const int WLD_SORT_DEPTHS[36] = { 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 2, 2, 2, 2, 4, 4, 4,
    6, 6, 8, 8, 10, 10, 10, 12, 12, 12,
    12, 12, 12, 12, 12
};

const int ENDGAME_SORT_DEPTHS[36] = { 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 2, 2, 2, 2, 4, 4, 4,
    6, 6, 8, 8, 10, 10, 10, 12, 12, 12,
    12, 12, 12, 12, 12
};

const int END_MEDIUM = 12;
const int END_SHLLW = 8;

Endgame::Endgame() {
    endgame_table = new EndHash(16000);
    killer_table = new EndHash(8000000);
    #if USE_ALL_TABLE
    all_table = new EndHash(2000000);
    #endif
}

Endgame::~Endgame() {
    delete endgame_table;
    delete killer_table;
    #if USE_ALL_TABLE
    delete all_table;
    #endif
}

/**
 * @brief Solves the endgame for perfect play.
 */
int Endgame::solveEndgame(Board &b, MoveList &moves, int s, int depth,
    int timeLimit, Eval *eval, int *exactScore) {
    // if best move for this position has already been found and stored
    EndgameEntry *entry = endgame_table->get(b, s);
    if(entry != NULL) {
        cerr << "Endgame hashtable hit. Score: " << (int) (entry->score) << endl;
        return entry->move;
    }

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();
    auto end_time = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(
        end_time-start_time);

    nodes = 0;
    hashHits = 0;
    hashCuts = 0;
    firstFailHigh = 0;
    failHighs = 0;
    searchSpaces = 0;
    evaluater = eval;

    #if USE_REGION_PAR
    // initialize region parity
    region_parity = 0;
    bitbrd empty = ~b.getTaken();
    while(empty) {
        region_parity ^= QUADRANT_ID[bitScanForward(empty)];
        empty &= empty-1;
    }
    #endif

    int score;
    int alpha = -64;
    int beta = 64;
    int bestIndex = 0;

    // Initial sorting of moves
    MoveList scores;
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
    else if (depth > 11)
        sortSearch(b, moves, scores, s, 2);
    sort(moves, scores, 0, moves.size-1);
    end_time = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(end_time-start_time);
    cerr << "Sort search took: " << time_span.count() << " sec" << endl;

/*
    cerr << "Starting WLD search" << endl;
    start_time = high_resolution_clock::now();
    alpha = -1;
    beta = 1;
    isWLD = true;
    for (unsigned int i = 0; i < moves.size; i++) {
        Board copy = b.copy();
        copy.doMove(moves.get(i), s);
        nodes++;
        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[moves.get(i)];
        #endif

        if (i != 0) {
            score = -dispatch(copy, s^1, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -dispatch(copy, s^1, depth-1, -beta, -alpha);
        }
        else
            score = -dispatch(copy, s^1, depth-1, -beta, -alpha);

        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[moves.get(i)];
        #endif
        if (alpha < score) {
            alpha = score;
            bestIndex = i;
        }
        if (alpha >= beta)
            break;
    }
    end_time = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(end_time-start_time);
    cerr << "WLD search took: " << time_span.count() << " sec" << endl;
    if (alpha == 0) {
        cerr << "Game is draw" << endl;
    }
    else { // If game is win or loss, we must find the best move and score
        if (alpha == -1) {
            cerr << "Game is loss" << endl;
            alpha = -64;
            beta = -1;
        }
        else {
            cerr << "Game is win" << endl;
            alpha = 1;
            beta = 64;
        }
        if (bestIndex != 0)
            moves.swap(bestIndex, 0);

        delete endgame_table;
        endgame_table = new EndHash(16000);*/
        isWLD = false;
        start_time = high_resolution_clock::now();

        for (unsigned int i = 0; i < moves.size; i++) {
            end_time = high_resolution_clock::now();
            time_span = duration_cast<duration<double>>(end_time-start_time);

            if(time_span.count() * 1000 * moves.size > timeLimit * (i+1))
                return MOVE_BROKEN;

            Board copy = b.copy();
            copy.doMove(moves.get(i), s);
            nodes++;
            #if USE_REGION_PAR
            region_parity ^= QUADRANT_ID[moves.get(i)];
            #endif

            if (i != 0) {
                score = -dispatch(copy, s^1, depth-1, -alpha-1, -alpha);
                if (alpha < score && score < beta)
                    score = -dispatch(copy, s^1, depth-1, -beta, -alpha);
            }
            else
                score = -dispatch(copy, s^1, depth-1, -beta, -alpha);

            cerr << "Searched move: " << moves.get(i) << " | alpha: " << score << endl;
            #if USE_REGION_PAR
            region_parity ^= QUADRANT_ID[moves.get(i)];
            #endif
            if (alpha < score) {
                alpha = score;
                bestIndex = i;
            }
            if (alpha >= beta)
                break;
        }
    //}

    cerr << "Endgame table has: " << endgame_table->keys << " keys." << endl;
    cerr << "Killer table has: " << killer_table->keys << " keys." << endl;
    #if USE_ALL_TABLE
    cerr << "All-nodes table has: " << all_table->keys << " keys." << endl;
    #endif
    cerr << "Score: " << alpha << endl;

    end_time = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(end_time-start_time);
    cerr << "Nodes searched: " << nodes << " | NPS: " <<
        (int)((double)nodes / time_span.count()) << endl;
    cerr << "Hash hits: " << hashHits << endl;
    cerr << "Hash cuts: " << hashCuts << endl;
    cerr << "First fail high rate: " << firstFailHigh << " / " << failHighs << " / " << searchSpaces << endl;
    cerr << "Time spent: " << time_span.count() << endl;

    if (exactScore != NULL)
        *exactScore = alpha;
    return moves.get(bestIndex);
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
    if(depth <= END_MEDIUM)
        return endgameMedium(b, s, depth, alpha, beta, passedLast);

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

    #if USE_ALL_TABLE
    EndgameEntry *allEntry = all_table->get(b, s);
    if(allEntry != NULL) {
        if (allEntry->score <= alpha)
            return alpha;
        if (beta > allEntry->score)
            beta = allEntry->score;
    }
    #endif

    // attempt killer heuristic cutoff, using saved alpha
    int killer = -1;
    EndgameEntry *killerEntry = killer_table->get(b, s);
    if(killerEntry != NULL) {
        hashHits++;
        if (killerEntry->score >= beta) {
            hashCuts++;
            return beta;
        }
        // Fail high is lower bound on score so this is valid
        if (alpha < killerEntry->score)
            alpha = killerEntry->score;
        killer = killerEntry->move;
    }

    MoveList legalMoves = b.getLegalMoves(s);
    if(legalMoves.size <= 0) {
        if(passedLast) {
            return (2 * b.count(s) - 64 + depth);
        }

        score = -endgameDeep(b, s^1, depth, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    // Use a shallow search for move ordering
    MoveList scores;
    sortSearch(b, legalMoves, scores, s, (isWLD ? WLD_SORT_DEPTHS[depth]
                                                : ENDGAME_SORT_DEPTHS[depth]));

    MoveList priority;
    // Restrict opponent's mobility and potential mobility
    for(unsigned int i = 0; i < legalMoves.size; i++) {
        int m = legalMoves.get(i);
        Board copy = b.copy();
        copy.doMove(m, s);

        int p = 11 * SQ_VAL[m];
        if (m == killer)
            p |= 1 << 16;

        priority.add(scores.get(i) - 1024*copy.numLegalMoves(s^1)
                - 64*copy.potentialMobility(s^1) + 8*p);
    }
    sort(legalMoves, priority, 0, legalMoves.size-1);

    int tempMove = -1;
    searchSpaces++;
    for(unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = b.copy();
        copy.doMove(legalMoves.get(i), s);
        nodes++;
        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[legalMoves.get(i)];
        #endif

        // Enhanced TT cutoff: searches one move ahead for a TT cutoff
        /*#if USE_ALL_TABLE
        EndgameEntry *etcEntry = all_table->get(copy, s^1);
        if(etcEntry != NULL) {
            if (etcEntry->score <= -beta)
                return beta;
        }
        #endif*/

        if (i != 0) {
            score = -endgameDeep(copy, s^1, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta)
                score = -endgameDeep(copy, s^1, depth-1, -beta, -alpha, false);
        }
        else
            score = -endgameDeep(copy, s^1, depth-1, -beta, -alpha, false);

        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[legalMoves.get(i)];
        #endif
        if (score >= beta) {
            failHighs++;
            if (i == 0)
                firstFailHigh++;
            killer_table->add(b, beta, legalMoves.get(i), s, depth - isWLD * 2);
            return beta;
        }
        if (alpha < score) {
            alpha = score;
            tempMove = legalMoves.get(i);
        }
    }

    // Best move with exact score if alpha < score < beta
    if (tempMove != -1 && prevAlpha < alpha && alpha < beta)
        endgame_table->add(b, alpha, tempMove, s, depth);
    #if USE_ALL_TABLE
    if (alpha <= prevAlpha)
        all_table->add(b, alpha, MOVE_NULL, s, depth);
    #endif

    return alpha;
}

/**
 * @brief A function for the endgame solver, used when a medium number of
 * squares remain. Sort searching is no longer used here, and the MoveList is
 * dropped in favor of a faster array on the stack.
 */
int Endgame::endgameMedium(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    if(depth <= END_SHLLW)
        return endgameShallow(b, s, depth, alpha, beta, passedLast);

    int score;
    int prevAlpha = alpha;

    // play best move, if recorded
    EndgameEntry *exactEntry = endgame_table->get(b, s);
    if(exactEntry != NULL) {
        return exactEntry->score;
    }

    #if USE_STABILITY
    if(alpha >= STAB_THRESHOLD[depth]) {
        score = 64 - 2*b.getStability(s^1);
        if(score <= alpha) {
            return score;
        }
    }
    #endif

    // attempt killer heuristic cutoff, using saved alpha
    int killer = -1;
    EndgameEntry *killerEntry = killer_table->get(b, s);
    if(killerEntry != NULL) {
        hashHits++;
        if (killerEntry->score >= beta) {
            hashCuts++;
            return beta;
        }
        // Fail high is lower bound on score so this is valid
        if (alpha < killerEntry->score)
            alpha = killerEntry->score;
        killer = killerEntry->move;
    }

    bitbrd legal = b.getLegal(s);
    if(!legal) {
        if(passedLast) {
            return (2 * b.count(s) - 64 + depth);
        }

        score = -endgameMedium(b, s^1, depth, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    // create array of legal moves
    int moves[END_MEDIUM];
    int priority[END_MEDIUM];
    int n = 0;

    do {
        int m = bitScanForward(legal);
        Board copy = b.copy();
        copy.doMove(m, s);

        priority[n] = SQ_VAL[m];
        if(m == killer)
            priority[n] += 1 << 16;
        priority[n] -= 16 * copy.numLegalMoves(s^1);

        moves[n] = m;
        legal &= legal-1; n++;
    } while(legal);

    searchSpaces++;
    int tempMove = -1;
    // search all moves
    int i = 0;
    for (int move = nextMoveShallow(moves, priority, n, i); move != MOVE_NULL;
             move = nextMoveShallow(moves, priority, n, ++i)) {
        Board copy = b.copy();
        copy.doMove(move, s);
        nodes++;
        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[move];
        #endif

        if (i != 0) {
            score = -endgameMedium(copy, s^1, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta)
                score = -endgameMedium(copy, s^1, depth-1, -beta, -alpha, false);
        }
        else
            score = -endgameMedium(copy, s^1, depth-1, -beta, -alpha, false);

        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[move];
        #endif
        if (score >= beta) {
            failHighs++;
            if (i == 0)
                firstFailHigh++;
            killer_table->add(b, beta, move, s, depth);
            return beta;
        }
        if (alpha < score) {
            alpha = score;
            tempMove = move;
        }
    }

    // Best move with exact score if alpha < score < beta
    if (tempMove != -1 && prevAlpha < alpha && alpha < beta)
        endgame_table->add(b, alpha, tempMove, s, depth);

    return alpha;
}

/**
 * @brief Endgame solver, to be used with about 8 or less empty squares.
 * Here, it is no longer efficient to use a hash table or heavy sorting.
 * Fastest first is used above depth 6, otherwise, moves are just sorted by
 * hole parity.
 */
int Endgame::endgameShallow(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    if(depth == 4)
        return endgame4(b, s, alpha, beta, passedLast);

    int score;

    #if USE_STABILITY
    if(alpha >= STAB_THRESHOLD[depth]) {
        score = 64 - 2*b.getStability(s^1);
        if(score <= alpha) {
            return score;
        }
    }
    #endif

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
    int moves[END_SHLLW];
    int priority[END_SHLLW];
    int n = 0;

    bitbrd empty = ~b.getTaken();
    #if USE_REGION_PAR
    if(depth < 5 && region_parity) {
        do {
            moves[n] = bitScanForward(legal);

            int p = SQ_VAL[moves[n]];
            if(!(NEIGHBORS[moves[n]] & empty))
                p += 64;
            if(QUADRANT_ID[moves[n]] & region_parity)
                p += 1 << 15;
            priority[n] = p;

            legal &= legal-1; n++;
        } while(legal);
    }
    else {
    #endif
        do {
            moves[n] = bitScanForward(legal);
            priority[n] = SQ_VAL[moves[n]];

            if(!(NEIGHBORS[moves[n]] & empty))
                priority[n] += 64;

            legal &= legal-1; n++;
        } while(legal);
    #if USE_REGION_PAR
    }
    #endif

    if(depth > 6) {
        for(int i = 0; i < n; i++) {
            Board copy = b.copy();
            copy.doMove(moves[i], s);

            priority[i] += -128*copy.numLegalMoves(s^1);
        }
    }

    // search all moves
    searchSpaces++;
    int i = 0;
    for (int move = nextMoveShallow(moves, priority, n, i); move != MOVE_NULL;
             move = nextMoveShallow(moves, priority, n, ++i)) {
        Board copy = b.copy();
        copy.doMove(move, s);
        nodes++;

        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[move];
        #endif

        if (i != 0) {
            score = -endgameShallow(copy, s^1, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta) {
                score = -endgameShallow(copy, s^1, depth-1, -beta, -alpha, false);
            }
        }
        else {
            score = -endgameShallow(copy, s^1, depth-1, -beta, -alpha, false);
        }

        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[move];
        #endif
        if (score >= beta) {
            failHighs++;
            if (i == 0)
                firstFailHigh++;
            return score;
        }
        if (alpha < score)
            alpha = score;
    }

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
            return score;
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
            return score;
        if (alpha < score)
            alpha = score;
    }

    return alpha;
}

/**
 * @brief Endgame solver, to be used with exactly 2 empty squares.
 */
int Endgame::endgame2(Board &b, int s, int alpha, int beta) {
    int score = NEG_INFTY;
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

    if (score == NEG_INFTY) {
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
        if (score == NEG_INFTY)
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
    int score = 2 * b.count(s) - 64;

    bitbrd changeMask = b.getDoMove(legalMove, s);
    nodes++;
    // If the player "s" can move, calculate final score
    if (changeMask) {
        score += 2 * countSetBits(changeMask) + 2;
    }
    // Otherwise, it is the opponent's move. If the opponent can stand pat,
    // we don't need to calculate the final score.
    // Technically the condition is score + 1 >= alpha but the cost of adding 1
    // is about equal to the cost of the extra work we do when alpha = score...
    else if (score >= alpha) {
        bitbrd otherMask = b.getDoMove(legalMove, s^1);
        nodes++;
        if (otherMask)
            score -= 2 * countSetBits(otherMask);
    }

    return score;
}

//--------------------------------Sort Search-----------------------------------

// Performs an alpha-beta search on each legal move to get a score.
void Endgame::sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth) {
    for (unsigned int i = 0; i < moves.size; i++) {
        Board copy = b.copy();
        copy.doMove(moves.get(i), side);
        scores.add(-pvs(copy, side^1, depth-1, NEG_INFTY, INFTY));
    }
}

// Helper function for the alpha-beta search.
int Endgame::pvs(Board &b, int s, int depth, int alpha, int beta) {
    if (depth <= 0) {
        return (s == CBLACK) ? evaluater->end_heuristic(b)
                             : -evaluater->end_heuristic(b);
    }

    int score;

    MoveList legalMoves = b.getLegalMoves(s);
    if (legalMoves.size <= 0) {
        score = -pvs(b, s^1, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;

        return alpha;
    }

    if (depth >= 2) {
        MoveList scores;
        if (depth >= 9)
            sortSearch(b, legalMoves, scores, s, 4);
        else if (depth >= 5)
            sortSearch(b, legalMoves, scores, s, 2);
        else if (depth >= 4)
            sortSearch(b, legalMoves, scores, s, 0);
        else {
            for (unsigned int i = 0; i < legalMoves.size; i++)
                scores.add(SQ_VAL[legalMoves.get(i)]);
        }
        sort(legalMoves, scores, 0, legalMoves.size-1);
    }

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = b.copy();
        copy.doMove(legalMoves.get(i), s);

        if (depth > 2 && i != 0) {
            score = -pvs(copy, s^1, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -pvs(copy, s^1, depth-1, -beta, -alpha);
        }
        else
            score = -pvs(copy, s^1, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            break;
    }
    return alpha;
}
/*
// Performs an alpha-beta search on each legal move to get a score.
void Endgame::rootSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth) {
    for (unsigned int i = 0; i < moves.size; i++) {
        Board copy = b.copy();
        copy.doMove(moves.get(i), side);
        scores.add(-rootPVS(copy, side^1, depth-1, NEG_INFTY, INFTY));
    }
}

// Helper function for the alpha-beta search.
int Endgame::rootPVS(Board &b, int s, int depth, int alpha, int beta) {
    if (depth <= 0) {
        return evaluater->heuristic(b, 60, s);
    }

    int score;

    MoveList legalMoves = b.getLegalMoves(s);
    if (legalMoves.size <= 0) {
        score = -rootPVS(b, s^1, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;

        return alpha;
    }

    if (depth >= 2) {
        MoveList scores;
        if (depth >= 9)
            rootSearch(b, legalMoves, scores, s, 4);
        else if (depth >= 5)
            rootSearch(b, legalMoves, scores, s, 2);
        else if (depth >= 4)
            rootSearch(b, legalMoves, scores, s, 0);
        else {
            for (unsigned int i = 0; i < legalMoves.size; i++)
                scores.add(SQ_VAL[legalMoves.get(i)]);
        }
        sort(legalMoves, scores, 0, legalMoves.size-1);
    }

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = b.copy();
        copy.doMove(legalMoves.get(i), s);

        if (depth > 2 && i != 0) {
            score = -rootPVS(copy, s^1, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -rootPVS(copy, s^1, depth-1, -beta, -alpha);
        }
        else
            score = -rootPVS(copy, s^1, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            break;
    }
    return alpha;
}
*/
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
