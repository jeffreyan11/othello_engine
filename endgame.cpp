#include <chrono>
#include <iostream>
#include "endgame.h"

Endgame::Endgame() {
    #if USE_BESTMOVE_TABLE
    endgame_table = new Hash(4000);
    #endif
}

Endgame::~Endgame() {
    #if USE_BESTMOVE_TABLE
    delete endgame_table;
    #endif
}

/**
 * @brief Solves the endgame for perfect play.
 */
int Endgame::endgame(Board &b, MoveList &moves, int depth, Eval *eval) {
    #if USE_BESTMOVE_TABLE
    // if best move for this position has already been found and stored
    BoardData *entry = endgame_table->get(b, mySide);
    if(entry != NULL) {
        cerr << "Endgame hashtable hit. Score: " << entry->score << endl;
        return entry->move;
    }
    #endif

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    #if COUNT_NODES
    nodes = 0;
    #endif
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
    int tempMove = moves.get(0);

    // Initial sorting of moves
    MoveList scores;
    if (depth > 24)
        sortSearch(b, moves, scores, mySide, 12);
    else if (depth > 22)
        sortSearch(b, moves, scores, mySide, 10);
    else if (depth > 20)
        sortSearch(b, moves, scores, mySide, 8);
    else if (depth > 18)
        sortSearch(b, moves, scores, mySide, 6);
    else if (depth > 15)
        sortSearch(b, moves, scores, mySide, 4);
    else if (depth > 11)
        sortSearch(b, moves, scores, mySide, 2);
    sort(moves, scores, 0, moves.size-1);

    for (unsigned int i = 0; i < moves.size; i++) {
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(
            end_time-start_time);

        if(time_span.count() * 1000 * moves.size > endgameTimeMS * (i+1))
            return MOVE_BROKEN;

        Board copy = Board(b.taken, b.black);
        copy.doMove(moves.get(i), mySide);
        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[moves.get(i)];
        #endif

        if (i != 0) {
            score = -dispatch(copy, -mySide, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -dispatch(copy, -mySide, depth-1, -beta, -alpha);
        }
        else
            score = -dispatch(copy, -mySide, depth-1, -beta, -alpha);

        cerr << "Searched move: " << moves.get(i) << " | alpha: " << score << endl;
        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[moves.get(i)];
        #endif
        if (alpha < score) {
            alpha = score;
            tempMove = moves.get(i);
        }
        if (alpha >= beta)
            break;
    }

    #if USE_BESTMOVE_TABLE
    cerr << "Endgame table has: " << endgame_table->keys << " keys." << endl;
    #endif
    cerr << "Killer table has: " << killer_table.keys << " keys." << endl;
    //endgame_table.test();
    cerr << "Score: " << alpha << endl;
    #if COUNT_NODES
    auto end_time = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(
        end_time-start_time);
    cerr << "Nodes searched: " << nodes << " | NPS: " <<
        (int)((double)nodes / time_span.count()) << endl;
    #endif

    return tempMove;
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
            score = endgame_h(b, s, depth, alpha, beta, false);
            break;
    }
    return score;
}

/**
 * @brief Function for endgame solver. Used when many empty squares remain.
 * A best move table, stability cutoff, killer heuristic cutoff, sort search,
 * and fastest first are used to reduce nodes searched.
 */
int Endgame::endgame_h(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    if(depth <= END_SHLLW)
        return endgame_shallow(b, s, depth, alpha, beta, passedLast);

    int score;
    int prevAlpha = alpha;

    // play best move, if recorded
    #if USE_BESTMOVE_TABLE
    BoardData *exactEntry = endgame_table->get(b, s);
    if(exactEntry != NULL) {
        return exactEntry->score;
    }
    #endif

    // Stability cutoff: if the current position is hopeless compared to a
    // known lower bound, then we need not waste time searching it.
    #if USE_STABILITY
    if(alpha >= STAB_THRESHOLD[depth]) {
        score = 64 - 2*b.getStability(-s);
        if(score <= alpha) {
            return score;
        }
    }
    #endif

    // attempt killer heuristic cutoff, using saved alpha
    int killer = -1;
    BoardData *killerEntry = killer_table.get(b, s);
    if(killerEntry != NULL) {
        //if (killerEntry->nodeType == CUT_NODE) {
            if (killerEntry->score >= beta)
                return beta;
            // Fail high is lower bound on score so this is valid
            if (alpha < killerEntry->score)
                alpha = killerEntry->score;
            killer = killerEntry->move;
        /*}
        else {
            if (killerEntry->score <= alpha)
                return alpha;
            if (beta > killerEntry->score)
                beta = killerEntry->score;
        }*/
    }

    // additionally, place killer move first in ordering
    MoveList priority;
    MoveList legalMoves = b.getLegalMovesOrdered(s, priority, killer);

    if(legalMoves.size <= 0) {
        if(passedLast) {
            return (2 * b.count(s) - 64 + depth);
        }

        #if COUNT_NODES
        nodes++;
        #endif
        score = -endgame_h(b, -s, depth, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    sort(legalMoves, priority, 0, legalMoves.size-1);

    // Use a shallow search for move ordering
    MoveList scores;

    if(depth < 15)
        sortSearch(b, legalMoves, scores, s, 0);
    else if(depth < 18)
        sortSearch(b, legalMoves, scores, s, 2);
    else if(depth < 20)
        sortSearch(b, legalMoves, scores, s, 4);
    else if(depth < 22)
        sortSearch(b, legalMoves, scores, s, 6);
    else if(depth < 24)
        sortSearch(b, legalMoves, scores, s, 8);
    else
        sortSearch(b, legalMoves, scores, s, 10);

    // Restrict opponent's mobility and potential mobility
    for(unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(legalMoves.get(i), s);

        priority.set(i, scores.get(i) - 1024*copy.numLegalMoves(-s)
                - 64*copy.potentialMobility(-s) + 8*priority.get(i));
    }
    sort(legalMoves, priority, 0, legalMoves.size-1);

    #if USE_BESTMOVE_TABLE
    int tempMove = -1;
    #endif
    for(unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(legalMoves.get(i), s);
        #if COUNT_NODES
        nodes++;
        #endif
        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[legalMoves.get(i)];
        #endif

        if (i != 0) {
            score = -endgame_h(copy, -s, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta)
                score = -endgame_h(copy, -s, depth-1, -beta, -alpha, false);
        }
        else
            score = -endgame_h(copy, -s, depth-1, -beta, -alpha, false);

        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[legalMoves.get(i)];
        #endif
        if (alpha < score) {
            alpha = score;
            #if USE_BESTMOVE_TABLE
            tempMove = legalMoves.get(i);
            #endif
        }
        if (alpha >= beta) {
            killer_table.add(b, beta, legalMoves.get(i), s, 0, depth, CUT_NODE);
            return alpha;
        }
    }
    #if USE_BESTMOVE_TABLE
    // Best move with exact score if alpha < score < beta
    if (tempMove != -1 && prevAlpha < alpha && alpha < beta)
        endgame_table->add(b, alpha, tempMove, s, 0, depth, PV_NODE);
    #endif
    //if (alpha <= prevAlpha)
    //    killer_table.add(b, alpha, MOVE_NULL, s, 0, depth, ALL_NODE);

    return alpha;
}

/**
 * @brief Endgame solver, to be used with about 10 or less empty squares.
 * Here, it is no longer efficient to use a hash table or heavy sort searching.
 * Fastest first is used above depth 7, otherwise, moves are just sorted by
 * hole parity.
 */
int Endgame::endgame_shallow(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    if(depth == 4)
        return endgame4(b, s, alpha, beta, passedLast);

    int score;

    #if USE_STABILITY
    if(alpha >= STAB_THRESHOLD[depth]) {
        score = 64 - 2*b.getStability(-s);
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

        #if COUNT_NODES
        nodes++;
        #endif
        score = -endgame_shallow(b, -s, depth, -beta, -alpha, true);

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
    if(region_parity) {
        do {
            moves[n] = bitScanForward(legal);

            int p = 8 * SQ_VAL[moves[n]];
            if(!(NEIGHBORS[moves[n]] & empty))
                p += 128;
            if(QUADRANT_ID[moves[n]] & region_parity)
                p += 256;
            priority[n] = p;

            legal &= legal-1; n++;
        } while(legal);
    }
    else {
    #endif
        do {
            moves[n] = bitScanForward(legal);
            priority[n] = 8 * SQ_VAL[moves[n]];

            if(!(NEIGHBORS[moves[n]] & empty))
                priority[n] += 128;

            legal &= legal-1; n++;
        } while(legal);
    #if USE_REGION_PAR
    }
    #endif

    if(depth > 7) {
        for(int i = 0; i < n; i++) {
            Board copy = Board(b.taken, b.black);
            copy.doMove(moves[i], s);

            priority[i] += -256*copy.numLegalMoves(-s) +
                32*(evaluater->stability(copy, s) - evaluater->stability(copy, -s));
        }
    }

    // search all moves
    int i = 0;
    for (int move = nextMoveShallow(moves, priority, n, i); move != MOVE_NULL;
             move = nextMoveShallow(moves, priority, n, ++i)) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(move, s);
        #if COUNT_NODES
        nodes++;
        #endif
        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[move];
        #endif

        if (i != 0) {
            score = -endgame_shallow(copy, -s, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta) {
                score = -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);
            }
        }
        else {
            score = -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);
        }

        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[move];
        #endif
        if (score >= beta)
            return score;
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
    int legalMove1 = MOVE_NULL;
    int legalMove2 = MOVE_NULL;
    int legalMove3 = MOVE_NULL;
    int legalMove4 = b.getLegalMoves4(s, legalMove1, legalMove2, legalMove3);

    if(legalMove1 == MOVE_NULL) {
        if(passedLast) {
            return (2 * b.count(s) - 60);
        }

        #if COUNT_NODES
        nodes++;
        #endif
        score = -endgame4(b, -s, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    Board copy = Board(b.taken, b.black);
    copy.doMove(legalMove1, s);
    #if COUNT_NODES
    nodes++;
    #endif

    score = -endgame3(copy, -s, -beta, -alpha, false);

    if (score >= beta)
        return score;
    if (alpha < score)
        alpha = score;

    if(legalMove2 != MOVE_NULL) {
        copy = Board(b.taken, b.black);
        copy.doMove(legalMove2, s);
        #if COUNT_NODES
        nodes++;
        #endif

        score = -endgame3(copy, -s, -alpha-1, -alpha, false);
        if (alpha < score && score < beta)
            score = -endgame3(copy, -s, -beta, -alpha, false);

        if (score >= beta)
            return score;
        if (alpha < score)
            alpha = score;

        if(legalMove3 != MOVE_NULL) {
            copy = Board(b.taken, b.black);
            copy.doMove(legalMove3, s);
            #if COUNT_NODES
            nodes++;
            #endif

            score = -endgame3(copy, -s, -alpha-1, -alpha, false);
            if (alpha < score && score < beta)
                score = -endgame3(copy, -s, -beta, -alpha, false);

            if (score >= beta)
                return score;
            if (alpha < score)
                alpha = score;

            if(legalMove4 != MOVE_NULL) {
                copy = Board(b.taken, b.black);
                copy.doMove(legalMove3, s);
                #if COUNT_NODES
                nodes++;
                #endif

                score = -endgame3(copy, -s, -alpha-1, -alpha, false);
                if (alpha < score && score < beta)
                    score = -endgame3(copy, -s, -beta, -alpha, false);

                if (alpha < score)
                    alpha = score;
            }
        }
    }

    return alpha;
}

/**
 * @brief Endgame solver, to be used with exactly 3 empty squares.
 * Starting with 3 moves left, null-window search is no longer used.
 */
int Endgame::endgame3(Board &b, int s, int alpha, int beta, bool passedLast) {
    int score;
    int legalMove1 = MOVE_NULL;
    int legalMove2 = MOVE_NULL;
    int legalMove3 = b.getLegalMoves3(s, legalMove1, legalMove2);

    if(legalMove1 == MOVE_NULL) {
        if(passedLast) {
            return (2 * b.count(s) - 61);
        }

        #if COUNT_NODES
        nodes++;
        #endif
        score = -endgame3(b, -s, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    Board copy = Board(b.taken, b.black);
    copy.doMove(legalMove1, s);
    #if COUNT_NODES
    nodes++;
    #endif

    score = -endgame2(copy, -s, -beta, -alpha);

    if (score >= beta)
        return score;
    if (alpha < score)
        alpha = score;

    if(legalMove2 != MOVE_NULL) {
        copy = Board(b.taken, b.black);
        copy.doMove(legalMove2, s);
        #if COUNT_NODES
        nodes++;
        #endif

        score = -endgame2(copy, -s, -beta, -alpha);

        if (score >= beta)
            return score;
        if (alpha < score)
            alpha = score;

        if(legalMove3 != MOVE_NULL) {
            copy = Board(b.taken, b.black);
            copy.doMove(legalMove3, s);
            #if COUNT_NODES
            nodes++;
            #endif

            score = -endgame2(copy, -s, -beta, -alpha);

            if (alpha < score)
                alpha = score;
        }
    }

    return alpha;
}

/**
 * @brief Endgame solver, to be used with exactly 2 empty squares.
 */
int Endgame::endgame2(Board &b, int s, int alpha, int beta) {
    int score = NEG_INFTY;
    bitbrd empty = ~b.getTaken();
    bitbrd opp = b.toBits(-s);

    // At 2 squares left, it is more efficient to simply try moves on both
    // squares. This approach was based on Richard Delorme's Edax-Reversi.
    int lm1 = bitScanForward(empty);
    empty &= empty-1;
    int lm2 = bitScanForward(empty);

    bitbrd changeMask;

    if( (opp & NEIGHBORS[lm1]) && (changeMask = b.getDoMove(lm1, s)) ) {
        b.makeMove(lm1, changeMask, s);
        #if COUNT_NODES
        nodes++;
        #endif
        score = -endgame1(b, -s, -beta, lm2);
        b.undoMove(lm1, changeMask, s);

        if (score >= beta)
            return score;
        if (alpha < score)
            alpha = score;
    }

    if( (opp & NEIGHBORS[lm2]) && (changeMask = b.getDoMove(lm2, s)) ) {
        b.makeMove(lm2, changeMask, s);
        #if COUNT_NODES
        nodes++;
        #endif
        score = -endgame1(b, -s, -beta, lm1);
        b.undoMove(lm2, changeMask, s);

        if (alpha < score)
            alpha = score;
    }

    if(score == NEG_INFTY) {
        opp = b.toBits(s);

        // if no legal moves... try other player
        if( (opp & NEIGHBORS[lm1]) && (changeMask = b.getDoMove(lm1, -s)) ) {
            b.makeMove(lm1, changeMask, -s);
            #if COUNT_NODES
            nodes++;
            #endif
            score = endgame1(b, s, alpha, lm2);
            b.undoMove(lm1, changeMask, -s);

            if (alpha >= score)
                return score;
            if (beta > score)
                beta = score;
        }

        if( (opp & NEIGHBORS[lm2]) && (changeMask = b.getDoMove(lm2, -s)) ) {
            b.makeMove(lm2, changeMask, -s);
            #if COUNT_NODES
            nodes++;
            #endif
            score = endgame1(b, s, alpha, lm1);
            b.undoMove(lm2, changeMask, -s);

            if (beta > score)
                beta = score;
        }

        // if both players passed, game over
        if(score == NEG_INFTY) {
            return (2 * b.count(s) - 62);
        }

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
    #if COUNT_NODES
    nodes++;
    #endif

    bitbrd changeMask = b.getDoMove(legalMove, s);
    // If the player "s" can move, calculate final score
    if(changeMask) {
        score += 2 * countSetBits(changeMask) + 2;
    }
    // Otherwise, it is the opponent's move. If the opponent can stand pat,
    // we don't need to calculate the final score.
    // Technically the condition is score + 1 >= alpha but the cost of adding 1
    // is about equal to the cost of the extra work we do when alpha = score...
    else if(score >= alpha) {
        bitbrd otherMask = b.getDoMove(legalMove, -s);
        if(otherMask)
            score -= 2 * countSetBits(otherMask);
    }

    return score;
}

//--------------------------------Sort Search-----------------------------------

// Performs an alpha-beta search on each legal move to get a score.
void Endgame::sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth) {
    for (unsigned int i = 0; i < moves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(moves.get(i), side);
        scores.add(-pvs(copy, -side, depth-1, NEG_INFTY, INFTY));
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
    if(legalMoves.size <= 0) {
        score = -pvs(b, -s, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;

        return alpha;
    }

    if (depth >= 2) {
        MoveList scores;
        if(depth >= 9)
            sortSearch(b, legalMoves, scores, s, 4);
        else if(depth >= 5)
            sortSearch(b, legalMoves, scores, s, 2);
        else if(depth >= 3)
            sortSearch(b, legalMoves, scores, s, 0);
        else {
            for (unsigned int i = 0; i < legalMoves.size; i++)
                scores.add(SQ_VAL[legalMoves.get(i)]);
        }
        sort(legalMoves, scores, 0, legalMoves.size-1);
    }

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(legalMoves.get(i), s);

        if (depth > 2 && i != 0) {
            score = -pvs(copy, -s, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -pvs(copy, -s, depth-1, -beta, -alpha);
        }
        else
            score = -pvs(copy, -s, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            break;
    }
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
