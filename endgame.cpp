#include "endgame.h"

int endgame(Board &b, vector<int> &moves, Side s, int pieces, int alpha,
    int beta, int endgameTimeMS,
    unordered_map<Board, int, BoardHashFunc> &endgame_table) {

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    int temp = endgame_table[b];
    if(temp != 0) {
        temp--;
        return temp;
    }

    int score;
    int tempMove = moves[0];

    for (unsigned int i = 0; i < moves.size(); i++) {
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(
            end_time-start_time);

        if(time_span.count() * moves.size() * 2000 > endgameTimeMS * (i+1))
            return MOVE_BROKEN;

        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(moves[i], s);

        if (i != 0) {
            score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE), s,
                pieces-1, -alpha-1, -alpha, endgame_table);
            if (alpha < score && score < beta) {
                score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE), s,
                    pieces-1, -beta, -score, endgame_table);
            }
        }
        else {
            score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE), s,
                pieces-1, -beta, -alpha, endgame_table);
        }

        if (score > alpha) {
            alpha = score;
            tempMove = moves[i];
        }
        if (alpha >= beta)
            break;
    }

    return tempMove;
}

int endgame_h(Board &b, Side s, Side mine, int depth, int alpha, int beta,
    unordered_map<Board, int, BoardHashFunc> &endgame_table) {

    int score;

    if (depth <= 0) {
        return (b.count(s) - b.count((s == WHITE) ? BLACK : WHITE));
    }

    vector<int> legalMoves = b.getLegalMoves(s);

    if(legalMoves.size() <= 0) {
        if(b.isDone())
            return (b.count(s) - b.count((s == WHITE) ? BLACK : WHITE));
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(MOVE_NULL, s);
        score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE), mine,
            depth, -beta, -alpha, endgame_table);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    if(depth > 12) {
        int tempMove = legalMoves[0];
        for (unsigned int i = 0; i < legalMoves.size(); i++) {
            Board copy = Board(b.taken, b.black, b.legal);
            copy.doMove(legalMoves[i], s);

            if (i != 0) {
                score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                    mine, depth-1, -alpha-1, -alpha, endgame_table);
                if (alpha < score && score < beta) {
                    score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                        mine, depth-1, -beta, -score, endgame_table);
                }
            }
            else {
                score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                    mine, depth-1, -beta, -alpha, endgame_table);
            }

            if (alpha < score) {
                alpha = score;
                tempMove = legalMoves[i];
            }
            if (alpha >= beta)
                break;
        }
        endgame_table[b] = tempMove + 1;
    }
    else {
        for (unsigned int i = 0; i < legalMoves.size(); i++) {
            Board copy = Board(b.taken, b.black, b.legal);
            copy.doMove(legalMoves[i], s);

            if (i != 0) {
                score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                    mine, depth-1, -alpha-1, -alpha, endgame_table);
                if (alpha < score && score < beta) {
                    score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                        mine, depth-1, -beta, -score, endgame_table);
                }
            }
            else {
                score = -endgame_h(copy, ((s == WHITE) ? BLACK : WHITE),
                    mine, depth-1, -beta, -alpha, endgame_table);
            }

            if (alpha < score)
                alpha = score;
            if (alpha >= beta)
                break;
        }
    }
    return alpha;
}
