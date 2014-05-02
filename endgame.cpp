#include "endgame.h"

Endgame::Endgame() {
    #if USE_BESTMOVE_TABLE
    endgame_table = new Hash(8000);
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
int Endgame::endgame(Board &b, MoveList &moves, int depth) {
    #if USE_BESTMOVE_TABLE
    // if best move for this position has already been found and stored
    int temp = endgame_table->get(&b, mySide);
    if(temp != -1) {
        return temp;
    }
    #endif

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    // initialize region parity
    region_parity = 0;
    bitbrd empty = ~b.getTaken();
    while(empty) {
        region_parity ^= QUADRANT_ID[bitScanForward(empty)];
        empty &= empty-1;
    }

    int score;
    int alpha = -64;
    int beta = 64;
    int tempMove = moves.get(0);

    for (unsigned int i = 0; i < moves.size; i++) {
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(
            end_time-start_time);

        if(time_span.count() * 1500 * moves.size > endgameTimeMS * (i+1))
            return MOVE_BROKEN;

        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(moves.get(i), mySide);
        region_parity ^= QUADRANT_ID[moves.get(i)];

        if (i != 0) {
            score = -endgame_h(copy, -mySide, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta)
                score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha, false);
        }
        else
            score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha, false);

        cerr << "Searched " << moves.get(i) << " alpha: " << score << endl;
        region_parity ^= QUADRANT_ID[moves.get(i)];
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
*/
int Endgame::endgame_h(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    int score;

    #if USE_BESTMOVE_TABLE
    // attempt hashtable move cutoff
    int hashed = endgame_table->get(&b, s, score);
    if(hashed != -1) {
        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            return alpha;
    }
    #endif
    // attempt killer heuristic cutoff
    int killer = killer_table.get(&b, s, score);
    if(killer != -1) {
        //Board copy = Board(b.taken, b.black, b.legal);
        //copy.doMove(killer, s);
        /*region_parity ^= QUADRANT_ID[killer];
        score = (depth > END_SHLLW) ?
            -endgame_h(copy, -s, depth-1, -beta, -alpha, false) :
            -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);

        region_parity ^= QUADRANT_ID[killer];*/
        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            return alpha;
    }

    MoveList priority;
    MoveList legalMoves = b.getLegalMovesOrdered(s, priority);
    sort(legalMoves, priority, 0, legalMoves.size-1);

    if(legalMoves.size <= 0) {
        if(passedLast)
            return (b.count(s) - b.count(-s));

        score = -endgame_h(b, -s, depth, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    #if USE_BESTMOVE_TABLE
    int tempMove = -1;
    #endif
    for(unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(legalMoves.get(i), s);
        region_parity ^= QUADRANT_ID[legalMoves.get(i)];

        if (i != 0) {
            score = (depth > END_SHLLW) ?
                -endgame_h(copy, -s, depth-1, -alpha-1, -alpha, false) :
                -endgame_shallow(copy, -s, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta) {
                score = (depth > END_SHLLW) ?
                    -endgame_h(copy, -s, depth-1, -beta, -alpha, false) :
                    -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);
            }
        }
        else {
            score = (depth > END_SHLLW) ?
                -endgame_h(copy, -s, depth-1, -beta, -alpha, false) :
                -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);
        }

        region_parity ^= QUADRANT_ID[legalMoves.get(i)];
        if (alpha < score) {
            alpha = score;
            #if USE_BESTMOVE_TABLE
            tempMove = legalMoves.get(i);
            #endif
        }
        if (alpha >= beta) {
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
    int score;
    bitbrd legal = b.getLegalExt(s);

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
        do {
            moves[n] = bitScanForward(legal);

            if(!(NEIGHBORS[moves[n]] & empty))
                priority[n] = 100 + SQ_VAL[moves[n]];
            else priority[n] = SQ_VAL[moves[n]];

            legal &= legal-1; n++;
        } while(legal);
    }

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
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(moves[i], s);
        region_parity ^= QUADRANT_ID[moves[i]];

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

        region_parity ^= QUADRANT_ID[moves[i]];
        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            break;
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

    Board copy = Board(b.taken, b.black, b.legal);
    copy.doMove(legalMove1, s);

    score = -endgame3(copy, -s, -beta, -alpha, false);

    if (alpha < score)
        alpha = score;
    if (alpha >= beta)
        return alpha;

    if(legalMove2 != MOVE_NULL) {
        copy = Board(b.taken, b.black, b.legal);
        copy.doMove(legalMove2, s);

        score = -endgame3(copy, -s, -alpha-1, -alpha, false);
        if (alpha < score && score < beta)
            score = -endgame3(copy, -s, -beta, -alpha, false);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            return alpha;

        if(legalMove3 != MOVE_NULL) {
            copy = Board(b.taken, b.black, b.legal);
            copy.doMove(legalMove3, s);

            score = -endgame3(copy, -s, -alpha-1, -alpha, false);
            if (alpha < score && score < beta)
                score = -endgame3(copy, -s, -beta, -alpha, false);

            if (alpha < score)
                alpha = score;
            if (alpha >= beta)
                return alpha;

            if(legalMove4 != MOVE_NULL) {
                copy = Board(b.taken, b.black, b.legal);
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

    Board copy = Board(b.taken, b.black, b.legal);
    copy.doMove(legalMove1, s);

    score = -endgame2(copy, -s, -beta, -alpha, false);

    if (alpha < score)
        alpha = score;
    if (alpha >= beta)
        return alpha;

    if(legalMove2 != MOVE_NULL) {
        copy = Board(b.taken, b.black, b.legal);
        copy.doMove(legalMove2, s);

        score = -endgame2(copy, -s, -alpha-1, -alpha, false);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            return alpha;

        if(legalMove3 != MOVE_NULL) {
            copy = Board(b.taken, b.black, b.legal);
            copy.doMove(legalMove3, s);

            score = -endgame2(copy, -s, -alpha-1, -alpha, false);

            if (alpha < score)
                alpha = score;
        }
    }

    return alpha;
}

/**
 * @brief Endgame solver, to be used with exactly 2 empty squares.
*/
int Endgame::endgame2(Board &b, int s, int alpha, int beta, bool passedLast) {
    int score = NEG_INFTY;
    bitbrd empty = ~b.getTaken();
    int legalMove1 = bitScanForward(empty);
    empty &= empty-1;
    int legalMove2 = bitScanForward(empty);

    bitbrd changeMask = b.getDoMove(legalMove1, s);
    if(changeMask) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.makeMove(legalMove1, changeMask, s);
        score = -endgame1(copy, -s, -beta);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            return alpha;
    }

    changeMask = b.getDoMove(legalMove2, s);
    if(changeMask) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.makeMove(legalMove2, changeMask, s);
        score = -endgame1(copy, -s, -beta);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            return alpha;
    }

    // if no legal moves
    if(score == NEG_INFTY) {
        if(passedLast)
            return (b.count(s) - b.count(-s));

        score = -endgame2(b, -s, -beta, -alpha, true);
        if (alpha < score)
            alpha = score;
        return alpha;
    }

    return alpha;
}

/**
 * @brief Endgame solver, to be used with exactly 1 empty square.
*/
int Endgame::endgame1(Board &b, int s, int alpha) {
    int score;

    int legalMove = bitScanForward(~b.getTaken());
    bitbrd changeMask = b.getDoMove(legalMove, s);

    if(!changeMask) {
        bitbrd otherMask = b.getDoMove(legalMove, -s);
        if(!otherMask)
            return (b.count(s) - b.count(-s));

        Board copy = Board(b.taken, b.black, b.legal);
        copy.makeMove(legalMove, otherMask, -s);

        score = 2*copy.count(s) - 64;
        if (alpha < score)
            alpha = score;
        return alpha;
    }

    Board copy = Board(b.taken, b.black, b.legal);
    copy.makeMove(legalMove, changeMask, s);

    score = 2*copy.count(s) - 64;
    if (alpha < score)
        alpha = score;
    return alpha;
}


int Endgame::bitScanForward(bitbrd bb) {
    #if defined(__x86_64__)
        asm ("bsf %1, %0" : "=r" (bb) : "r" (bb));
        return (int) bb;
    #else
        return index64[(int)(((bb ^ (bb-1)) * 0x03f79d71b4cb0a89) >> 58)];
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
