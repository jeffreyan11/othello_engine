#include "endgame.h"

Endgame::Endgame() {
}

Endgame::~Endgame() {
}

/**
 * @brief Solves the endgame for perfect play.
*/
int Endgame::endgame(Board &b, MoveList &moves, int depth) {
    // if best move for this position has already been found and stored
    int temp = endgame_table.get(&b);
    if(temp != -1) {
        return temp;
    }

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    int score;
    int alpha = NEG_INFTY;
    int beta = INFTY;
    int tempMove = moves.get(0);

    for (unsigned int i = 0; i < moves.size; i++) {
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(
            end_time-start_time);

        if(time_span.count() * moves.size * 2000 > endgameTimeMS * (i+1))
            return MOVE_BROKEN;

        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(moves.get(i), mySide);

        if (i != 0) {
            score = -endgame_h(copy, -mySide, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta)
                score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha, false);
        }
        else
            score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha, false);

        if (score > alpha) {
            alpha = score;
            tempMove = moves.get(i);
        }
        if (alpha >= beta)
            break;
    }

    cerr << "Endgame table has: " << endgame_table.keys << " keys." << endl;

    return tempMove;
}

/**
 * @brief Function for endgame solver. Used when many empty squares remain.
*/
int Endgame::endgame_h(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    int score;

    // attempt hashtable move cutoff
    int killer = endgame_table.get(&b);
    if(killer != -1) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(killer, s);
        score = (depth > 10) ?
            -endgame_h(copy, -s, depth-1, -beta, -alpha, false) :
            -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            return alpha;
    }

    MoveList legalMoves = b.getLegalMoves(s);

    if(legalMoves.size <= 0) {
        if(passedLast)
            return (b.count(s) - b.count(-s));

        score = -endgame_h(b, -s, depth, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    int tempMove = -1;
    for (unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(legalMoves.get(i), s);

        if (i != 0) {
            score = (depth > 10) ?
                -endgame_h(copy, -s, depth-1, -alpha-1, -alpha, false) :
                -endgame_shallow(copy, -s, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta) {
                score = (depth > 10) ?
                    -endgame_h(copy, -s, depth-1, -beta, -alpha, false) :
                    -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);
            }
        }
        else {
            score = (depth > 10) ?
                -endgame_h(copy, -s, depth-1, -beta, -alpha, false) :
                -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);
        }

        if (alpha < score) {
            alpha = score;
            tempMove = legalMoves.get(i);
        }
        if (alpha >= beta)
            break;
    }
    if(tempMove != -1)
        endgame_table.add(&b, tempMove, 65);

    return alpha;
}

/**
 * @brief Endgame solver, to be used with 10 or less empty squares.
*/
int Endgame::endgame_shallow(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    int score;
    bitbrd legal = b.getLegalExt(s);
    int moves[10];
    int n = 0;

    if(!legal) {
        if(passedLast)
            return (b.count(s) - b.count(-s));

        score = -endgame_shallow(b, -s, depth, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    // create array of legal moves
    bitbrd corner = legal & 0x8100000000000081;
    bitbrd csq = legal & 0x2400810000810024;
    bitbrd adj = legal & 0x42C300000000C342;
    legal &= 0x183C7EFFFF7E3C18;
    if(corner) {
        moves[n] = bitScanForward(corner); n++;
        corner &= corner-1;
      if(corner) {
          moves[n] = bitScanForward(corner); n++;
          corner &= corner-1;
        if(corner) {
            moves[n] = bitScanForward(corner); n++;
            corner &= corner-1;
          if(corner) {
              moves[n] = bitScanForward(corner); n++;
          }
        }
      }
    }
    while(csq) {
        moves[n] = bitScanForward(csq); n++;
        csq &= csq-1;
    }
    while(legal) {
        moves[n] = bitScanForward(legal); n++;
        legal &= legal-1;
    }
    while(adj) {
        moves[n] = bitScanForward(adj); n++;
        adj &= adj-1;
    }

    for (int i = 0; i < n; i++) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(moves[i], s);

        if (i != 0) {
            if(depth > 4)
            score = -endgame_shallow(copy, -s, depth-1, -alpha-1, -alpha, false);
            else score = -endgame4(copy, -s, -alpha-1, -alpha, false);
            if (alpha < score && score < beta) {
                if(depth > 4)
                score = -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);
                else score = -endgame4(copy, -s, -beta, -alpha, false);
            }
        }
        else {
            if(depth > 4)
            score = -endgame_shallow(copy, -s, depth-1, -beta, -alpha, false);
            else score = -endgame4(copy, -s, -beta, -alpha, false);
        }

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
        if (alpha < score && score < beta)
            score = -endgame2(copy, -s, -beta, -alpha, false);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            return alpha;

        if(legalMove3 != MOVE_NULL) {
            copy = Board(b.taken, b.black, b.legal);
            copy.doMove(legalMove3, s);

            score = -endgame2(copy, -s, -alpha-1, -alpha, false);
            if (alpha < score && score < beta)
                score = -endgame2(copy, -s, -beta, -alpha, false);

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
    int score;
    int legalMove1 = MOVE_NULL;
    int legalMove2 = b.getLegalMoves2(s, legalMove1);

    if(legalMove1 == MOVE_NULL) {
        if(passedLast)
            return (b.count(s) - b.count(-s));

        score = -endgame2(b, -s, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    if(legalMove2 != MOVE_NULL) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(legalMove2, s);
        score = -endgame1(copy, -s, -beta);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            return alpha;
    }

    Board copy = Board(b.taken, b.black, b.legal);
    copy.doMove(legalMove1, s);
    score = -endgame1(copy, -s, -beta);

    if (alpha < score)
        alpha = score;

    return alpha;
}

/**
 * @brief Endgame solver, to be used with exactly 1 empty square.
*/
int Endgame::endgame1(Board &b, int s, int alpha) {
    int score;
    int legalMove = b.getLegalMove1(s);

    if(legalMove == MOVE_NULL) {
        int otherMove = b.getLegalMove1(-s);
        if(otherMove == MOVE_NULL)
            return (b.count(s) - b.count(-s));

        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(otherMove, -s);

        score = 2*copy.count(s) - 64;
        if (alpha < score)
            alpha = score;
        return alpha;
    }

    Board copy = Board(b.taken, b.black, b.legal);
    copy.doMove(legalMove, s);

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
