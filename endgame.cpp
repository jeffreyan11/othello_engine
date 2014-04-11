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
            score = -endgame_h(copy, -mySide, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha);
            }
        }
        else {
            score = -endgame_h(copy, -mySide, depth-1, -beta, -alpha);
        }

        if (score > alpha) {
            alpha = score;
            tempMove = moves.get(i);
        }
        if (alpha >= beta)
            break;
    }

    cerr << "Endgame table contains " << endgame_table.keys << " keys." << endl;
    return tempMove;
}

int Endgame::endgame_h(Board &b, int s, int depth, int alpha, int beta) {
    if (depth <= 0)
        return (b.count(s) - b.count(-s));

    int score;

    /*int killer = endgame_table.get(&b);
    if(killer != -1) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(killer, s);
        score = -endgame_h(copy, ((s == WHITE) ? (BLACK) : WHITE), mine,
            depth-1, -beta, -alpha, endgame_table);

        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            return alpha;
    }*/

    MoveList legalMoves = b.getLegalMoves(s);

    if(legalMoves.size <= 0) {
        if(b.isDone())
            return (b.count(s) - b.count(-s));

        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(MOVE_NULL, s);
        score = -endgame_h(copy, -s, depth, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    if(depth > 10) {
        int tempMove = legalMoves.get(0);
        for (unsigned int i = 0; i < legalMoves.size; i++) {
            Board copy = Board(b.taken, b.black, b.legal);
            copy.doMove(legalMoves.get(i), s);

            if (i != 0) {
                score = -endgame_h(copy, -s, depth-1, -alpha-1, -alpha);
                if (alpha < score && score < beta) {
                    score = -endgame_h(copy, -s, depth-1, -beta, -alpha);
                }
            }
            else {
                score = -endgame_h(copy, -s, depth-1, -beta, -alpha);
            }

            if (alpha < score) {
                alpha = score;
                tempMove = legalMoves.get(i);
            }
            if (alpha >= beta)
                break;
        }
        endgame_table.add(&b, tempMove, 65);
    }
    else {
        for (unsigned int i = 0; i < legalMoves.size; i++) {
            Board copy = Board(b.taken, b.black, b.legal);
            copy.doMove(legalMoves.get(i), s);

            if (i != 0) {
                score = -endgame_h(copy, -s, depth-1, -alpha-1, -alpha);
                if (alpha < score && score < beta) {
                    score = -endgame_h(copy, -s, depth-1, -beta, -alpha);
                }
            }
            else {
                score = -endgame_h(copy, -s, depth-1, -beta, -alpha);
            }

            if (alpha < score)
                alpha = score;
            if (alpha >= beta)
                break;
        }
    }
    return alpha;
}
