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

    if (depth > 20) {
        MoveList scores;
        sortSearch(b, moves, scores, mySide, 8);
        sort(moves, scores, 0, moves.size-1);
    }
    else if (depth > 16) {
        MoveList scores;
        sortSearch(b, moves, scores, mySide, 6);
        sort(moves, scores, 0, moves.size-1);
    }
    else if (depth > 12) {
        MoveList scores;
        sortSearch(b, moves, scores, mySide, 4);
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
            switch(depth) {
                case 5:
                    score = -endgame4(copy, -mySide, -alpha-1, -alpha, false);
                    break;
                case 4:
                    score = -endgame3(copy, -mySide, -alpha-1, -alpha, false);
                    break;
                case 3:
                    score = -endgame2(copy, -mySide, -alpha-1, -alpha);
                    break;
                default:
                    score = -endgame_h(copy, -mySide, depth-1, -alpha-1, -alpha, false);
                    break;
            }

            if (alpha < score && score < beta) {
                switch(depth) {
                    case 5:
                        score = -endgame4(copy, -mySide, -beta, -alpha, false);
                        break;
                    case 4:
                        score = -endgame3(copy, -mySide, -beta, -alpha, false);
                        break;
                    case 3:
                        score = -endgame2(copy, -mySide, -beta, -alpha);
                        break;
                    default:
                        score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha, false);
                        break;
                }
            }
        }
        else {
            switch(depth) {
                case 5:
                    score = -endgame4(copy, -mySide, -beta, -alpha, false);
                    break;
                case 4:
                    score = -endgame3(copy, -mySide, -beta, -alpha, false);
                    break;
                case 3:
                    score = -endgame2(copy, -mySide, -beta, -alpha);
                    break;
                default:
                    score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha, false);
                    break;
            }
        }

        cerr << "Searched move: " << moves.get(i) << " | score: " << score << endl;
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

    // TODO stability cutoff?
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
        if (killerEntry->nodeType == CUT_NODE) {
            if (killerEntry->score >= beta)
                return beta;
            // Fail high is lower bound on score so this is valid
            if (alpha < killerEntry->score)
                alpha = killerEntry->score;
            killer = killerEntry->move;
        }
        else {
            if (killerEntry->score <= alpha)
                return alpha;
            if (beta > killerEntry->score)
                beta = killerEntry->score;
        }
    }

    // additionally, place killer move first in ordering
    MoveList priority;
    MoveList legalMoves = b.getLegalMovesOrdered(s, priority, killer);

    if(legalMoves.size <= 0) {
        if(passedLast) {
            return (b.count(s) - b.count(-s));
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
    if(depth > 20)
        sortSearch(b, legalMoves, scores, s, 6);
    else if(depth > 14)
        sortSearch(b, legalMoves, scores, s, 4);
    else
        sortSearch(b, legalMoves, scores, s, 2);

    // Restrict opponent's mobility and potential mobility
    for(unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(legalMoves.get(i), s);

        priority.set(i, 16*scores.get(i) - 256*copy.numLegalMoves(-s)
                - 8*copy.potentialMobility(-s) + priority.get(i));
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
    if (alpha <= prevAlpha)
        killer_table.add(b, alpha, MOVE_NULL, s, 0, depth, ALL_NODE);

    return alpha;
}

/**
 * @brief Endgame solver, to be used with about 10 or less empty squares.
*/
int Endgame::endgame_shallow(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    if(depth <= 4)
        return endgame4(b, s, alpha, beta, passedLast);

    int score;

    // TODO stability cutoff?
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
            return (b.count(s) - b.count(-s));
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

            // move ordering
            int p = 10 * SQ_VAL[moves[n]];
            if(!(NEIGHBORS[moves[n]] & empty))
                p += 100;
            if(QUADRANT_ID[moves[n]] & region_parity)
                p++;
            priority[n] = p;

            legal &= legal-1; n++;
        } while(legal);
    }
    else {
    #endif
        do {
            moves[n] = bitScanForward(legal);
            priority[n] = 10 * SQ_VAL[moves[n]];

            if(!(NEIGHBORS[moves[n]] & empty))
                priority[n] += 100;

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
        #if COUNT_NODES
        nodes++;
        #endif
        #if USE_REGION_PAR
        region_parity ^= QUADRANT_ID[moves[i]];
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
        if(passedLast) {
            return (b.count(s) - b.count(-s));
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
*/
int Endgame::endgame3(Board &b, int s, int alpha, int beta, bool passedLast) {
    int score;
    int legalMove1 = MOVE_NULL;
    int legalMove2 = MOVE_NULL;
    int legalMove3 = b.getLegalMoves3(s, legalMove1, legalMove2);

    if(legalMove1 == MOVE_NULL) {
        if(passedLast) {
            return (b.count(s) - b.count(-s));
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
    // squares.
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
            return b.count(s) - b.count(-s);
        }

        return beta;
    }

    return alpha;
}

/**
 * @brief Endgame solver, to be used with exactly 1 empty square.
*/
int Endgame::endgame1(Board &b, int s, int alpha, int legalMove) {
    int score = b.count(s) - b.count(-s);
    #if COUNT_NODES
    nodes++;
    #endif

    bitbrd changeMask = b.getDoMove(legalMove, s);

    if(!changeMask) {
        if(score >= alpha) {
            bitbrd otherMask = b.getDoMove(legalMove, -s);
            if(otherMask)
                score -= 2*countSetBits(otherMask) + 1;
        }
    }
    else {
        score += 2*countSetBits(changeMask) + 1;
    }

    return score;
}

//--------------------------------Sort Search-----------------------------------

/**
 * @brief Performs an alpha-beta search.
*/
void Endgame::sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
        int depth) {

    for (unsigned int i = 0; i < moves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(moves.get(i), side);
        scores.add(-pvs(copy, -side, depth-1, NEG_INFTY, INFTY));
    }
}

/**
 * @brief Helper function for the alpha-beta search.
*/
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

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(legalMoves.get(i), s);

        score = -pvs(copy, -s, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            break;
    }
    return alpha;
}

//--------------------------------Utilities-------------------------------------

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
