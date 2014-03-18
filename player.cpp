#include "player.h"

/*
 * Constructor for the player; initialize everything here. The side your AI is
 * on (BLACK or WHITE) is passed in as "side". The constructor must finish 
 * within 30 seconds.
 */
Player::Player(Side side) {
    // Will be set to true in test_minimax.cpp.
    testingMinimax = false;
    maxDepth = 13;
    minDepth = 10;
    sortDepth = 4;
    endgameDepth = 20;

    mySide = side;
    oppSide = (side == WHITE) ? (BLACK) : (WHITE);
    turn = 4;
}

/*
 * Destructor for the player.
 */
Player::~Player() {
}

/*
 * Compute the next move given the opponent's last move. Your AI is
 * expected to keep track of the board on its own. If this is the first move,
 * or if the opponent passed on the last move, then opponentsMove will be NULL.
 *
 * msLeft represents the time your AI has left for the total game, in
 * milliseconds. doMove() must take no longer than msLeft, or your AI will
 * be disqualified! An msLeft value of -1 indicates no time limit.
 *
 * The move returned must be legal; if there are no valid moves for your side,
 * return NULL.
 */
Move *Player::doMove(Move *opponentsMove, int msLeft) {
    game.doMove(opponentsMove, oppSide);
    if(opponentsMove != NULL)
        turn++;

    // check opening book
    Move *myMove = openingBook.get(game.getTaken(), game.getBlack());
    if(myMove != NULL) {
        cerr << "Opening book used!" << endl;
        turn++;
        game.doMove(myMove, mySide);
        return myMove;
    }

    // find and test all legal moves
    vector<Move *> legalMoves = game.getLegalMoves(mySide);
    vector<int> scores;

    if (legalMoves.size() <= 0) {
        game.doMove(NULL, mySide);
        return NULL;
    }

    while(turn > (64 - endgameDepth - 1)) {
        cerr << game.numLegalMoves(mySide) << " " << game.potentialMobility(mySide) << endl;
        myMove = endgame(&game, legalMoves, mySide, endgameDepth+1, -99999,
            99999);
        if(myMove == NULL) {
            cerr << "broken" << endl;
            endgameDepth -= 2;
            break;
        }

        endgameDepth--;
        myMove = new Move(myMove->getX(), myMove->getY());
        deleteMoveVector(legalMoves);

        game.doMove(myMove, mySide);
        turn++;
        return myMove;
    }

    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Board *copy = game.copy();
        copy->doMove(legalMoves[i], mySide);
        // run the recursion to find scores
        int tempScore = -minimax(copy, oppSide, sortDepth);
        scores.push_back(tempScore);
        delete copy;
    }

    sort(legalMoves, scores, 0, legalMoves.size()-1);
    scores.clear();

    myMove = negascout(&game, legalMoves, mySide, minDepth, -99999, 99999);
    if(myMove == NULL)
        myMove = negascout(&game, legalMoves, mySide, minDepth-2, -99999,
            99999);

    //myMove = NULL;
    // if enough time, search deeper
    if ((msLeft-300000)/(64-turn) > 16000) {
        cerr << "here" << endl;
        Move* bestM = myMove;

        for (unsigned int i = 0; i < legalMoves.size(); i++) {
            Board *copy = game.copy();
            copy->doMove(legalMoves[i], mySide);
            // run the recursion to find scores
            int tempScore = -minimax(copy, oppSide, sortDepth+2);
            scores.push_back(tempScore);
            delete copy;
        }
        sort(legalMoves, scores, 0, legalMoves.size()-1);

        vector<Move *> deeperMoves;
        deeperMoves.push_back(bestM);
        for(unsigned int i = 0; i < legalMoves.size() / 2; i++) {
            if(legalMoves[i] != bestM) {
                deeperMoves.push_back(legalMoves[i]);
            }
        }

        myMove = negascout(&game, deeperMoves, mySide, maxDepth, -99999, 99999);
        if(myMove == NULL) {
            cerr << "broken" << endl;
            myMove = bestM;
        }
        if(myMove != bestM)
            cerr << "changed" << endl;
    }

    myMove = new Move(myMove->getX(), myMove->getY());
    deleteMoveVector(legalMoves);

    game.doMove(myMove, mySide);
    turn++;
    return myMove;
}

Move *Player::negascout(Board *b, vector<Move *> &moves, Side s, int depth,
    int alpha, int beta) {

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    int score;
    Move *tempMove = moves[0];

    for (unsigned int i = 0; i < moves.size(); i++) {
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(
            end_time-start_time);

        if(time_span.count() > 30 && i < moves.size() / 2)
            return NULL;

        Board *copy = b->copy();
        copy->doMove(moves[i], s);
        if (i != 0) {
            score = -negascout_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -negascout_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -score);
            }
        }
        else {
            score = -negascout_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -alpha);
        }
        if (score > alpha) {
            alpha = score;
            tempMove = moves[i];
        }
        if (alpha >= beta) {
            delete copy;
            break;
        }
        delete copy;
    }
    return tempMove;
}

int Player::negascout_h(Board *b, Side s, int depth, int alpha, int beta) {
    int side, score;
    if (s == mySide)
        side = 1;
    else
        side = -1;

    if (depth <= 0) {
        return side * heuristic(b);
    }

    vector<Move *> legalMoves = b->getLegalMoves(s);
    if(legalMoves.size() <= 0) {
        Board *copy = b->copy();
        copy->doMove(NULL, s);
        score = -negascout_h(copy, ((s == WHITE) ? (BLACK) : WHITE),
            depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        delete copy;
        return alpha;
    }
    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Board *copy = b->copy();
        copy->doMove(legalMoves[i], s);
        if (i != 0) {
            score = -negascout_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -negascout_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -score);
            }
        }
        else {
            score = -negascout_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -alpha);
        }
        if (alpha < score)
            alpha = score;
        if (alpha >= beta) {
            delete copy;
            break;
        }
        delete copy;
    }
    deleteMoveVector(legalMoves);
    return alpha;
}

Move *Player::endgame(Board *b, vector<Move *> &moves, Side s, int depth,
    int alpha, int beta) {

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    int score;
    Move *tempMove = moves[0];

    for (unsigned int i = 0; i < moves.size(); i++) {
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(
            end_time-start_time);

        if(time_span.count() > 120 && i < moves.size() / 2)
            return NULL;

        Board *copy = b->copy();
        copy->doMove(moves[i], s);

        if (i != 0) {
            score = -endgame_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -endgame_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -score);
            }
        }
        else {
            score = -endgame_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -alpha);
        }

        if (score > alpha) {
            alpha = score;
            tempMove = moves[i];
        }
        if (alpha >= beta) {
            delete copy;
            break;
        }
        delete copy;
    }
    return tempMove;
}

int Player::endgame_h(Board *b, Side s, int depth, int alpha, int beta) {
    int side, score;
    if (s == mySide)
        side = 1;
    else
        side = -1;

    if (depth <= 0) {
        return (side * eheuristic(b));
    }

    vector<Move *> legalMoves = b->getLegalMoves(s);
    if(legalMoves.size() <= 0) {
        Board *copy = b->copy();
        copy->doMove(NULL, s);
        score = -endgame_h(copy, ((s == WHITE) ? (BLACK) : WHITE),
            depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        delete copy;
        return alpha;
    }

    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Board *copy = b->copy();
        copy->doMove(legalMoves[i], s);

        if (i != 0) {
            score = -endgame_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -endgame_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -score);
            }
        }
        else {
            score = -endgame_h(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -alpha);
        }

        if (alpha < score)
            alpha = score;
        if (alpha >= beta) {
            delete copy;
            break;
        }
        delete copy;
    }
    deleteMoveVector(legalMoves);
    return alpha;
}

int Player::minimax(Board * b, Side side, int depth) {
    if (depth <= 2) { // base case
        int score = -9999;

        // get all legal moves, create boards to test with, find best move
        vector <Move *> legalMoves = b->getLegalMoves(side);
        for (unsigned int i = 0; i < legalMoves.size(); i++) {
            Board *copy = b->copy();
            copy->doMove(legalMoves[i], side);
            int tempScore = mmheuristic(copy) * ((side == mySide) ? 1 : -1);
            delete copy;
            if (tempScore > score)
                score = tempScore;
        }
        deleteMoveVector(legalMoves);
        return score;
    }
    else { // recursive step
        int score = -9999;
        // recurse for each legal move from current board position
        vector <Move *> legalMoves = b->getLegalMoves(side);
        for (unsigned int i = 0; i < legalMoves.size(); i++) {
            Board *copy = b->copy();
            copy->doMove(legalMoves[i], side);
            int tempScore = -minimax(copy, ((side == WHITE) ? (BLACK) :
                WHITE), depth-1);
            delete copy;
            if (tempScore > score)
                score = tempScore;
        }
        deleteMoveVector(legalMoves);
        return score;
    }
}

int Player::heuristic (Board *b) {
    int score;
    if(b->count(mySide) == 0)
        return -9001;

    if(turn < 30)
        score = b->count(mySide) - b->count(oppSide);
    else
        score = 2 * (b->count(mySide) - b->count(oppSide));

    bitbrd bm = b->toBits(mySide);

    score += 50 * countSetBits(bm & CORNERS);
    if(turn > 35)
        score += 4 * countSetBits(bm & EDGES);
    score -= 12 * countSetBits(bm & X_CORNERS);
    score -= 6 * countSetBits(bm & ADJ_CORNERS);

    score += 2 * (b->numLegalMoves(mySide) - b->numLegalMoves(oppSide));
    score += 2 * (b->potentialMobility(mySide) - b->potentialMobility(oppSide));

    return score;
}

int Player::eheuristic(Board *b) {
    return (b->count(mySide) - b->count(oppSide));
}

int Player::mmheuristic (Board *b) {
    int score;
    if(turn < 30)
        score = b->count(mySide) - b->count(oppSide);
    else if(turn < 61)
        score = 2 * (b->count(mySide) - b->count(oppSide));
    else {
        score = b->count(mySide) - b->count(oppSide);
        return score;
    }

    bitbrd bm = b->toBits(mySide);

    score += 50 * countSetBits(bm & CORNERS);
    if(turn > 35)
        score += 4 * countSetBits(bm & EDGES);
    score -= 12 * countSetBits(bm & X_CORNERS);
    score -= 6 * countSetBits(bm & ADJ_CORNERS);

    score += 2 * (b->numLegalMoves(mySide) - b->numLegalMoves(oppSide));
    score += 2 * (b->potentialMobility(mySide) - b->potentialMobility(oppSide));
    return score;
}

int Player::countSetBits(bitbrd b) {
    int n = 0;
    // while there are 1s
    while(b) {
        n++;
        b &= b - 1; // reset least significant 1
    }
    return n;
}

void Player::deleteMoveVector(vector<Move *> v) {
    while(v.size() > 0) {
        Move *m = v.back();
        v.pop_back();
        delete m;
    }
}

void Player::sort(vector<Move *> &moves, vector<int> &scores, int left, int right)
{
    int index = left;

    if (left < right)
    {
        index = partition(moves, scores, left, right, index);
        sort(moves, scores, left, index-1);
        sort(moves, scores, index+1, right);
    }
}

void Player::swap(vector<Move *> &moves, vector<int> &scores, int i, int j)
{
    Move * less1;
    int less2;

    less1 = moves[j];
    moves[j] = moves[i];
    moves[i] = less1;

    less2 = scores[j];
    scores[j] = scores[i];
    scores[i] = less2;
}

int Player::partition(vector<Move *> &moves, vector<int> &scores, int left,
        int right, int pindex)
{
    int index = left;
    int pivot = scores[pindex];

    swap(moves, scores, pindex, right);

    for (int i = left; i < right; i++)
    {
        if (scores[i] >= pivot)
        {
            swap(moves, scores, i, index);
            index++;
        }
    }
    swap(moves, scores, index, right);

    return index;
}

// g++ -std=c++0x -o memtest player.cpp board.cpp openings.cpp
/*int main(int argc, char **argv) {
    Player p(BLACK);
    Move m (3,5);
    p.doMove(&m, -1);
    Move m2 (2,6);
    p.doMove(&m2, -1);

    Board b;
    char boardData[64] = {
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
        ' ', ' ', 'b', 'b', 'b', ' ', ' ', ' ', 
        ' ', ' ', ' ', 'b', 'w', 'b', 'w', ' ', 
        ' ', ' ', ' ', ' ', ' ', 'w', 'w', 'w', 
        ' ', ' ', ' ', ' ', ' ', ' ', 'w', 'w', 
        ' ', ' ', ' ', ' ', ' ', ' ', 'w', 'w'
    };
    b.setBoard(boardData);
    Move m (4,5);
    for(int i = 0; i < 200000000; i++) {
        b.checkMove(&m, BLACK);
    }
    cout << "done" << endl;
}*/
