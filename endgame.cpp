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
    int temp = endgame_table->get(&b, mySide);
    if(temp != -1) {
        return temp;
    }
    #endif

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

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

    if(depth > END_SHLLW) {
        MoveList scores;
        pvs(b, moves, scores, mySide, 2, NEG_INFTY, INFTY);
        sort(moves, scores, 0, moves.size-1);
    }

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
if(depth > 5 || depth <= 3)
    score = -endgame_h(copy, -mySide, depth-1, -alpha-1, -alpha, false);
else if(depth == 5)
    score = -endgame4(copy, -mySide, -alpha-1, -alpha, false);
else
    score = -endgame3(copy, -mySide, -alpha-1, -alpha, false);

            if (alpha < score && score < beta) {
if(depth > 5 || depth <= 3)
    score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha, false);
else if(depth == 5)
    score = -endgame4(copy, -mySide, -beta, -alpha, false);
else
    score = -endgame3(copy, -mySide, -beta, -alpha, false);
            }
        }
        else {
if(depth > 5 || depth <= 3)
    score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha, false);
else if(depth == 5)
    score = -endgame4(copy, -mySide, -beta, -alpha, false);
else
    score = -endgame3(copy, -mySide, -beta, -alpha, false);
        }

        cerr << "Searched " << moves.get(i) << " alpha: " << score << endl;
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

    return tempMove;
}

/**
 * @brief Function for endgame solver. Used when many empty squares remain.
 * A best move table, stability cutoff, killer heuristic cutoff, sort search,
 * and fastest first are used to reducing nodes searched.
*/
int Endgame::endgame_h(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    if(depth <= END_SHLLW)
        return endgame_shallow(b, s, depth, alpha, beta, passedLast);

    int score;

    // play best move, if recorded
    #if USE_BESTMOVE_TABLE
    if(endgame_table->get(&b, s, score) != -1) {
        if (alpha < score)
            alpha = score;
        return alpha;
    }
    #endif

    // TODO stability cutoff?
    #if USE_STABILITY
    if(alpha >= 24 && alpha < STAB_UP) {
        int stab_score = evaluater->stability(&b, -s);
        if(64 - 2*stab_score - STAB_ASP <= alpha)
            return alpha;
    }
    #endif

    // attempt killer heuristic cutoff, using saved alpha
    int killer = killer_table.get(&b, s, score);
    if(killer != -1) {
        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            return alpha;
    }

    // additionally, place killer move first in ordering
    MoveList priority;
    MoveList legalMoves = b.getLegalMovesOrdered(s, priority, killer);

    if(legalMoves.size <= 0) {
        if(passedLast)
            return (b.count(s) - b.count(-s));

        score = -endgame_h(b, -s, depth, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    sort(legalMoves, priority, 0, legalMoves.size-1);

    // Use a shallow search for move ordering
    MoveList scores;
    pvs(b, legalMoves, scores, s, 2, NEG_INFTY, INFTY);

    // Restrict opponent's mobility and potential mobility
    for(unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(legalMoves.get(i), s);

        priority.set(i, scores.get(i) - 16*copy.numLegalMoves(-s)
                - 4*copy.potentialMobility(-s) + 4*priority.get(i));
    }
    sort(legalMoves, priority, 0, legalMoves.size-1);

    #if USE_BESTMOVE_TABLE
    int tempMove = -1;
    #endif
    for(unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(legalMoves.get(i), s);
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
            if(legalMoves.get(i) != killer)
                killer_table.add(&b, s, legalMoves.get(i), alpha);
            return alpha;
        }
    }
    #if USE_BESTMOVE_TABLE
    if(tempMove != -1)
        endgame_table->add(&b, s, tempMove, alpha);
    #endif

    return alpha;
}

/**
 * @brief Endgame solver, to be used with about 10 or less empty squares.
*/
int Endgame::endgame_shallow(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    // TODO stability cutoff?
    #if USE_STABILITY
    if(alpha >= 12 && alpha < STAB_UP) {
        int stab_score = evaluater->stability(&b, -s);
        if(64 - 2*stab_score - STAB_ASP <= alpha)
            return alpha;
    }
    #endif

    int score;
    bitbrd legal = b.getLegal(s);

    if(!legal) {
        if(passedLast)
            return (b.count(s) - b.count(-s));

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

            // move ordering
            int p;
            if(!(NEIGHBORS[moves[n]] & empty))
                p = 100 + SQ_VAL[moves[n]];
            else {
                p = SQ_VAL[moves[n]];
                if(QUADRANT_ID[moves[n]] & region_parity)
                    p += 10;
            }
            priority[n] = p;

            legal &= legal-1; n++;
        } while(legal);
    }
    else {
    #endif
        do {
            moves[n] = bitScanForward(legal);

            if(!(NEIGHBORS[moves[n]] & empty))
                priority[n] = 100 + SQ_VAL[moves[n]];
            else priority[n] = SQ_VAL[moves[n]];

            legal &= legal-1; n++;
        } while(legal);
    #if USE_REGION_PAR
    }
    #endif

    // sort
    for(int i = 1; i < n; i++) {
        int j = i;
        int temp1 = moves[i];
        int temp2 = priority[i];
        while(j > 0 && priority[j-1] < priority[j]) {
            priority[j] = priority[j-1];
            moves[j] = moves[j-1];
            j--;
        }
        moves[j] = temp1;
        priority[j] = temp2;
    }

    // search all moves
    for (int i = 0; i < n; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(moves[i], s);
        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[moves[i]];
        #endif

        if (i != 0) {
            if(depth > 5)
            score = -endgame_shallow(copy, -s, depth-1, -alpha-1, -alpha, false);
            else score = -endgame4(copy, -s, -alpha-1, -alpha, false);
            if (alpha < score && score < beta) {
                if(depth > 5)
                score = -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);
                else score = -endgame4(copy, -s, -beta, -alpha, false);
            }
        }
        else {
            if(depth > 5)
            score = -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);
            else score = -endgame4(copy, -s, -beta, -alpha, false);
        }

        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[moves[i]];
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
*/
int Endgame::endgame4(Board &b, int s, int alpha, int beta, bool passedLast) {
    int score;
    int legalMove1 = MOVE_NULL;
    int legalMove2 = MOVE_NULL;
    int legalMove3 = MOVE_NULL;
    int legalMove4 = b.getLegalMoves4(s, legalMove1, legalMove2, legalMove3);

    if(legalMove1 == MOVE_NULL) {
        if(passedLast)
            return (b.count(s) - b.count(-s));

        score = -endgame4(b, -s, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    Board copy = Board(b.taken, b.black);
    copy.doMove(legalMove1, s);

    score = -endgame3(copy, -s, -beta, -alpha, false);

    if (score >= beta)
        return score;
    if (alpha < score)
        alpha = score;

    if(legalMove2 != MOVE_NULL) {
        copy = Board(b.taken, b.black);
        copy.doMove(legalMove2, s);

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
*/
int Endgame::endgame3(Board &b, int s, int alpha, int beta, bool passedLast) {
    int score;
    int legalMove1 = MOVE_NULL;
    int legalMove2 = MOVE_NULL;
    int legalMove3 = b.getLegalMoves3(s, legalMove1, legalMove2);

    if(legalMove1 == MOVE_NULL) {
        if(passedLast)
            return (b.count(s) - b.count(-s));

        score = -endgame3(b, -s, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    Board copy = Board(b.taken, b.black);
    copy.doMove(legalMove1, s);

    score = -endgame2(copy, -s, -beta, -alpha);

    if (score >= beta)
        return score;
    if (alpha < score)
        alpha = score;

    if(legalMove2 != MOVE_NULL) {
        copy = Board(b.taken, b.black);
        copy.doMove(legalMove2, s);

        score = -endgame2(copy, -s, -beta, -alpha);

        if (score >= beta)
            return score;
        if (alpha < score)
            alpha = score;

        if(legalMove3 != MOVE_NULL) {
            copy = Board(b.taken, b.black);
            copy.doMove(legalMove3, s);

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
    int curr_score = b.count(s) - b.count(-s);
    if(curr_score >= beta + 33)
        return beta;
    if(curr_score <= alpha - 33)
        return alpha;

    int score = NEG_INFTY;
    bitbrd empty = ~b.getTaken();
    bitbrd opp = b.toBits(-s);

    int lm1 = bitScanForward(empty);
    empty &= empty-1;
    int lm2 = bitScanForward(empty);

    bitbrd changeMask;

    if( (opp & NEIGHBORS[lm1]) && (changeMask = b.getDoMove(lm1, s)) ) {
        b.makeMove(lm1, changeMask, s);
        score = -endgame1(b, -s, -beta);
        b.undoMove(lm1, changeMask, s);

        if (score >= beta)
            return score;
        if (alpha < score)
            alpha = score;
    }

    if( (opp & NEIGHBORS[lm2]) && (changeMask = b.getDoMove(lm2, s)) ) {
        b.makeMove(lm2, changeMask, s);
        score = -endgame1(b, -s, -beta);
        b.undoMove(lm2, changeMask, s);

        if (alpha < score)
            alpha = score;
    }
    else {
        if(score == NEG_INFTY) {
            opp = b.toBits(s);

            // if no legal moves... try other player
            if( (opp & NEIGHBORS[lm1]) && (changeMask = b.getDoMove(lm1, -s)) ) {
                b.makeMove(lm1, changeMask, -s);
                score = endgame1(b, s, alpha);
                b.undoMove(lm1, changeMask, -s);

                if (alpha >= score)
                    return score;
                if (beta > score)
                    beta = score;
            }

            if( (opp & NEIGHBORS[lm2]) && (changeMask = b.getDoMove(lm2, -s)) ) {
                b.makeMove(lm2, changeMask, -s);
                score = endgame1(b, s, alpha);
                b.undoMove(lm2, changeMask, -s);

                if (beta > score)
                    beta = score;
            }
            else {
                // if both players passed, game over
                if(score == NEG_INFTY)
                    return b.count(s) - b.count(-s);
            }

            return beta;
        }
    }

    return alpha;
}

/**
 * @brief Endgame solver, to be used with exactly 1 empty square.
*/
int Endgame::endgame1(Board &b, int s, int alpha) {
    int score = b.count(s) - b.count(-s);
    if(score <= alpha - 18)
        return alpha;

    int legalMove = bitScanForward(~b.getTaken());
    bitbrd changeMask = b.getDoMove(legalMove, s);

    if(!changeMask) {
        if(score >= alpha) {
            bitbrd otherMask = b.getDoMove(legalMove, -s);
            if(otherMask)
                score -= 2*countSetBitsLow(otherMask) + 1;
        }
    }
    else {
        score += 2*countSetBitsLow(changeMask) + 1;
    }

    return score;
}

//--------------------------------PVS Search------------------------------------

/**
 * @brief Performs an alpha-beta search.
*/
void Endgame::pvs(Board &b, MoveList &moves, MoveList &scores, int s,
        int depth, int alpha, int beta) {
    int score;

    for (unsigned int i = 0; i < moves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(moves.get(i), s);
        int ttScore = NEG_INFTY;

        score = -pvs_h(copy, ttScore, -s, depth-1, -beta, -alpha);

        scores.add(ttScore);
        if (score > alpha)
            alpha = score;
    }
}

/**
 * @brief Helper function for the alpha-beta search.
*/
int Endgame::pvs_h(Board &b, int &topScore, int s, int depth,
        int alpha, int beta) {
    if (depth <= 0) {
        topScore = (s == CBLACK) ? evaluater->end_heuristic(&b) :
                -evaluater->end_heuristic(&b);
        return topScore;
    }

    int score;
    int ttScore = NEG_INFTY;

    MoveList legalMoves = b.getLegalMoves(s);
    if(legalMoves.size <= 0) {
        score = -pvs_h(b, ttScore, -s, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        topScore = ttScore;

        return alpha;
    }

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(legalMoves.get(i), s);

        score = -pvs_h(copy, ttScore, -s, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        if(ttScore > topScore)
            topScore = ttScore;
        if (alpha >= beta)
            break;
    }
    return alpha;
}

//--------------------------------Utilities-------------------------------------

int Endgame::bitScanForward(bitbrd bb) {
    #if defined(__x86_64__)
        asm ("bsf %1, %0" : "=r" (bb) : "r" (bb));
        return (int) bb;
    #else
        return index64[(int)(((bb ^ (bb-1)) * 0x03f79d71b4cb0a89) >> 58)];
    #endif
}

int Endgame::countSetBitsLow(bitbrd i) {
    #if defined(__x86_64__)
        asm ("popcnt %1, %0" : "=r" (i) : "r" (i));
        return (int) i;
    #elif defined(__i386)
        int a = (int) (i & 0xFFFFFFFF);
        int b = (int) (i >> 32);
        asm ("popcntl %1, %0" : "=r" (a) : "r" (a));
        asm ("popcntl %1, %0" : "=r" (b) : "r" (b));
        return a+b;
    #else
        int result = 0;
        do {
            result++;
            i &= i - 1;
        } while(i);
    #endif
}

void Endgame::sort(MoveList &moves, MoveList &scores, int left, int right) {
    int pivot = (left + right) / 2;

    if (left < right) {
        pivot = partition(moves, scores, left, right, pivot);
        sort(moves, scores, left, pivot-1);
        sort(moves, scores, pivot+1, right);
    }
    /*for(unsigned int i = 1; i < moves.size; i++) {
        unsigned int j = i;
        int temp1 = moves.get(i);
        int temp2 = scores.get(i);
        while(j > 0 && scores.get(j-1) < scores.get(j)) {
            moves.set(j, moves.get(j-1));
            scores.set(j, scores.get(j-1));
            j--;
        }
        moves.set(j, temp1);
        scores.set(j, temp2);
    }*/
}

void Endgame::swap(MoveList &moves, MoveList &scores, int i, int j) {
    int less1 = moves.get(j);
    moves.set(j, moves.get(i));
    moves.set(i, less1);

    int less2 = scores.get(j);
    scores.set(j, scores.get(i));
    scores.set(i, less2);
}

int Endgame::partition(MoveList &moves, MoveList &scores, int left, int right,
    int pindex) {

    int index = left;
    int pivot = scores.get(pindex);

    swap(moves, scores, pindex, right);

    for (int i = left; i < right; i++) {
        if (scores.get(i) > pivot) {
            swap(moves, scores, i, index);
            index++;
        }
    }
    swap(moves, scores, index, right);

    return index;
}
