#include "endgame.h"

Endgame::Endgame() {
}

Endgame::~Endgame() {
}

int Endgame::endgame(Board &b, vector<int> &moves, int depth) {
    int temp = endgame_table.get(&b);
    if(temp != -1) {
        return temp;
    }

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    int score;
    int alpha = NEG_INFTY;
    int beta = INFTY;
    int tempMove = moves[0];

    for (unsigned int i = 0; i < moves.size(); i++) {
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(
            end_time-start_time);

        if(time_span.count() * moves.size() * 2000 > endgameTimeMS * (i+1))
            return MOVE_BROKEN;

        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(moves[i], mySide);

        if (i != 0) {
            score = -endgame_h(copy, ((mySide == WHITE) ? BLACK : WHITE),
                depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -endgame_h(copy, ((mySide == WHITE) ? BLACK : WHITE),
                    depth-1, -beta, -alpha);
            }
        }
        else {
            score = -endgame_h(copy, ((mySide == WHITE) ? BLACK : WHITE),
                depth-1, -beta, -alpha);
        }

        if (score > alpha) {
            alpha = score;
            tempMove = moves[i];
        }
        if (alpha >= beta)
            break;
    }

    cerr << "Endgame table contains " << endgame_table.keys << " keys." << endl;
    return tempMove;
}

int Endgame::endgame_h(Board &b, Side s, int depth, int alpha, int beta) {
    if (depth <= 0)
        return (b.count(s) - b.count((s == WHITE) ? BLACK : WHITE));

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

    vector<int> legalMoves = b.getLegalMoves(s);

    if(legalMoves.size() <= 0) {
        if(b.isDone())
            return (b.count(s) - b.count((s == WHITE) ? BLACK : WHITE));
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(MOVE_NULL, s);
        score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
            depth, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    if(depth > 10) {
        int tempMove = legalMoves[0];
        for (unsigned int i = 0; i < legalMoves.size(); i++) {
            Board copy = Board(b.taken, b.black, b.legal);
            copy.doMove(legalMoves[i], s);

            if (i != 0) {
                score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                    depth-1, -alpha-1, -alpha);
                if (alpha < score && score < beta) {
                    score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                        depth-1, -beta, -alpha);
                }
            }
            else {
                score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                    depth-1, -beta, -alpha);
            }

            if (alpha < score) {
                alpha = score;
                tempMove = legalMoves[i];
            }
            if (alpha >= beta)
                break;
        }
        endgame_table.add(&b, tempMove, 65);
    }
    else {
        for (unsigned int i = 0; i < legalMoves.size(); i++) {
            Board copy = Board(b.taken, b.black, b.legal);
            copy.doMove(legalMoves[i], s);

            if (i != 0) {
                score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                    depth-1, -alpha-1, -alpha);
                if (alpha < score && score < beta) {
                    score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                        depth-1, -beta, -alpha);
                }
            }
            else {
                score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                    depth-1, -beta, -alpha);
            }

            if (alpha < score)
                alpha = score;
            if (alpha >= beta)
                break;
        }
    }
    return alpha;
}
