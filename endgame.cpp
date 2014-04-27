#include "endgame.h"

Endgame::Endgame() {
}

Endgame::~Endgame() {
}

int Endgame::endgame(Board &b, MoveList &moves, int depth) {
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
            if (alpha < score && score < beta) {
                score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha, false);
            }
        }
        else {
            score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha, false);
        }

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

int Endgame::endgame_h(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    int score;

    int killer = endgame_table.get(&b);
    if(killer != -1) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(killer, s);
        score = (depth > 10) ?
            -endgame_h(copy, -s, depth-1, -beta, -alpha, false) :
            -endgame_no_tt(copy, -s, depth-1, -beta, -alpha, false);

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
                -endgame_no_tt(copy, -s, depth-1, -alpha-1, -alpha, false);
            if (alpha < score && score < beta) {
                score = (depth > 10) ?
                    -endgame_h(copy, -s, depth-1, -beta, -alpha, false) :
                    -endgame_no_tt(copy, -s, depth-1, -beta, -alpha, false);
            }
        }
        else {
            score = (depth > 10) ?
                -endgame_h(copy, -s, depth-1, -beta, -alpha, false) :
                -endgame_no_tt(copy, -s, depth-1, -beta, -alpha, false);
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
int Endgame::endgame_no_tt(Board &b, int s, int depth, int alpha, int beta,
        bool passedLast) {
    int score;
    MoveList legalMoves = b.getLegalMoves(s);

    if(legalMoves.size <= 0) {
        if(passedLast)
            return (b.count(s) - b.count(-s));

        score = -endgame_no_tt(b, -s, depth, -beta, -alpha, true);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(legalMoves.get(i), s);

        if (i != 0) {
            if(depth > 4)
            score = -endgame_no_tt(copy, -s, depth-1, -alpha-1, -alpha, false);
            else score = -endgame4(copy, -s, -alpha-1, -alpha, false);
            if (alpha < score && score < beta) {
                if(depth > 4)
                score = -endgame_no_tt(copy, -s, depth-1, -beta, -alpha, false);
                else score = -endgame4(copy, -s, -beta, -alpha, false);
            }
        }
        else {
            if(depth > 4)
                score = -endgame_no_tt(copy, -s, depth-1, -beta, -alpha, false);
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

// -----------------------------Result solver-----------------------------------

int Endgame::result_solve(Board &b, MoveList &moves, int depth) {
    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    int score;
    int alpha = -1;
    int beta = 1;
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
            score = -rs_h(copy, -mySide, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -rs_h(copy, -mySide, depth-1, -beta, -alpha);
        }
        else
            score = -rs_h(copy, -mySide, depth-1, -beta, -alpha);

        if (score > alpha) {
            alpha = score;
            tempMove = moves.get(i);
        }
        if (alpha >= beta)
            break;
    }

    return tempMove;
}

int Endgame::rs_h(Board &b, int s, int depth, int alpha, int beta) {
    if (depth <= 0) {
        int result = b.count(s) - b.count(-s);
        if(result > 0)
            return 1;
        else if(result < 0)
            return -1;
        else return 0;
    }

    int score;
    MoveList legalMoves = b.getLegalMoves(s);

    if(legalMoves.size <= 0) {
        if(b.isDone()) {
            int result = b.count(s) - b.count(-s);
            if(result > 0)
                return 1;
            else if(result < 0)
                return -1;
            else return 0;
        }

        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(MOVE_NULL, s);
        score = -rs_h(copy, -s, depth, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(legalMoves.get(i), s);

        if (i != 0) {
            score = -rs_h(copy, -s, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -rs_h(copy, -s, depth-1, -beta, -alpha);
        }
        else
            score = -rs_h(copy, -s, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            break;
    }
    return alpha;
}
