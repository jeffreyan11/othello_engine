#include "player.h"

/*
 * Constructor for the player; initialize everything here. The side your AI is
 * on (BLACK or WHITE) is passed in as "side". The constructor must finish 
 * within 30 seconds.
 */
Player::Player(Side side) {
    // Will be set to true in test_minimax.cpp.
    testingMinimax = false;
    maxDepth = 14;
    minDepth = 6;
    sortDepth = 4;
    endgameDepth = 20;
    if(side == WHITE)
        endgameDepth--;
    endgameSwitch = false;

    mySide = side;
    oppSide = (side == WHITE) ? (BLACK) : (WHITE);
    turn = 4;
    totalTimePM = -2;
    endgameTimeMS = 0;

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            indexToMove[i+8*j] = new Move(i,j);
        }
    }
}

/*
 * Destructor for the player.
 */
Player::~Player() {
    for(int i = 0; i < 64; i++) {
        delete indexToMove[i];
    }
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
    if(totalTimePM == -2) {
        totalTimePM = msLeft;
        endgameTimeMS = msLeft / 3;
        if(totalTimePM != -1) {
            if(totalTimePM > 500000)
                totalTimePM = (totalTimePM - endgameTimeMS) / 22;
            else {
                totalTimePM = (totalTimePM - endgameTimeMS) / 25;
                endgameDepth = 14;
            }
        }
        else {
            totalTimePM = 1000000;
            endgameTimeMS = 1000000;
        }
    }

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    if(opponentsMove != NULL) {
        game.doMove(opponentsMove->getX() + 8*opponentsMove->getY(), oppSide);
        turn++;
    }
    else {
        game.doMove(MOVE_NULL, oppSide);
        if(endgameSwitch)
            endgameDepth++;
    }

    // check opening book
    int openMove = openingBook.get(game.getTaken(), game.getBlack());
    if(openMove != OPENING_NOT_FOUND) {
        cerr << "Opening book used!" << endl;
        turn++;
        game.doMove(openMove, mySide);
        return indexToMove[openMove];
    }

    // find and test all legal moves
    vector<int> legalMoves = game.getLegalMoves(mySide);

    if (legalMoves.size() <= 0) {
        game.doMove(MOVE_NULL, mySide);
        return NULL;
    }

    int myMove = -1;
    vector<int> scores;

    while(endgameSwitch || turn >= (64 - endgameDepth)) {
        if(msLeft < endgameTimeMS && msLeft != -1) {
            endgameSwitch = false;
            endgameDepth -= 2;
            break;
        }
        cerr << "Endgame solver: attempting depth " << endgameDepth << endl;
        endgameSwitch = true;
        myMove = endgame(&game, legalMoves, mySide, endgameDepth, NEG_INFTY,
            INFTY, endgameTimeMS, endgame_table);
        if(myMove == MOVE_BROKEN) {
            cerr << "Broken out of endgame solver." << endl;
            endgameDepth -= 2;
            break;
        }

        endgameDepth -= 2;

        game.doMove(myMove, mySide);
        turn++;
        return indexToMove[myMove];
    }

    cerr << "Performing initial search: depth " << sortDepth << endl;
    /*for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Board *copy = game.copy();
        copy->doMove(legalMoves[i], mySide);
        // run the recursion to find scores
        int tempScore = -minimax(copy, oppSide, sortDepth);
        scores.push_back(tempScore);
        delete copy;
    }*/
    negascout(&game, legalMoves, scores, mySide, sortDepth, NEG_INFTY, INFTY);

    int attemptingDepth = minDepth;
    duration<double> time_span;
    do {
        cerr << "Attempting NWS of depth " << attemptingDepth << endl;
        sort(legalMoves, scores, 0, legalMoves.size()-1);
        scores.clear();

        int newBest = negascout(&game, legalMoves, scores, mySide,
            attemptingDepth, NEG_INFTY, INFTY);
        if(newBest == MOVE_BROKEN) {
            cerr << "Broken out of search" << endl;
            break;
        }
        myMove = newBest;
        attemptingDepth += 2;

        auto end_time = high_resolution_clock::now();
        time_span = duration_cast<duration<double>>(end_time-start_time);
    } while( (
        ((msLeft-endgameTimeMS)/(64-endgameDepth-turn) > time_span.count()*1000.0*20) || msLeft == -1) && attemptingDepth <= maxDepth );

    game.doMove(myMove, mySide);
    turn++;
    return indexToMove[myMove];
}

int Player::negascout(Board *b, vector<int> &moves, vector<int> &scores,
    Side s, int depth, int alpha, int beta) {

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    int score;
    int tempMove = moves[0];

    for (unsigned int i = 0; i < moves.size(); i++) {
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(
            end_time-start_time);

        if(time_span.count() * moves.size() * 1000 > totalTimePM * (i+1))
            return MOVE_BROKEN;

        Board *copy = b->copy();
        copy->doMove(moves[i], s);
        int ttScore = NEG_INFTY;
        if (i != 0) {
            score = -negascout_h(copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -negascout_h(copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -score);
            }
        }
        else {
            score = -negascout_h(copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -alpha);
        }
        scores.push_back(ttScore);
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

int Player::negascout_h(Board *b, int &topScore, Side s, int depth,
    int alpha, int beta) {

    int side, score;
    if (s == mySide)
        side = 1;
    else
        side = -1;

    if (depth <= 0) {
        topScore = side * heuristic(b);
        return topScore;
    }

    vector<int> legalMoves = b->getLegalMoves(s);
    if(legalMoves.size() <= 0) {
        Board *copy = b->copy();
        copy->doMove(MOVE_NULL, s);
        int ttScore = NEG_INFTY;
        score = -negascout_h(copy, ttScore, ((s == WHITE) ? (BLACK) : WHITE),
            depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        delete copy;
        topScore = ttScore;
        return alpha;
    }
    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Board *copy = b->copy();
        copy->doMove(legalMoves[i], s);
        int ttScore = NEG_INFTY;
        if (i != 0) {
            score = -negascout_h(copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -negascout_h(copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -score);
            }
        }
        else {
            score = -negascout_h(copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -alpha);
        }
        if (alpha < score)
            alpha = score;
        if(ttScore > topScore)
            topScore = ttScore;
        if (alpha >= beta) {
            delete copy;
            break;
        }
        delete copy;
    }
    return alpha;
}

int Player::heuristic (Board *b) {
    int score;
    int myCoins = b->count(mySide);
    if(myCoins == 0)
        return -9001;

    if(turn < 30)
        score = myCoins - b->count(oppSide);
    else
        score = 2 * (myCoins - b->count(oppSide));

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

void Player::sort(vector<int> &moves, vector<int> &scores, int left, int right)
{
    int index = left;

    if (left < right)
    {
        index = partition(moves, scores, left, right, index);
        sort(moves, scores, left, index-1);
        sort(moves, scores, index+1, right);
    }
}

void Player::swap(vector<int> &moves, vector<int> &scores, int i, int j)
{
    int less1;
    int less2;

    less1 = moves[j];
    moves[j] = moves[i];
    moves[i] = less1;

    less2 = scores[j];
    scores[j] = scores[i];
    scores[i] = less2;
}

int Player::partition(vector<int> &moves, vector<int> &scores, int left,
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

// g++ -std=c++0x -O3 -o memtest player.cpp board.cpp openings.cpp endgame.cpp
/*int main(int argc, char **argv) {
    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();
    Player p(BLACK);
    //Move m (3,5);
    //p.doMove(&m, -1);
    vector<int> legalMoves = p.game.getLegalMoves(BLACK);
    int r = endgame(&(p.game), legalMoves, BLACK, 16, NEG_INFTY,
            INFTY, 1000000, p.endgame_table);
    cerr << r << endl;
    //Move m2 (2,6);
    //p.doMove(&m2, -1);

    auto end_time = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(
        end_time-start_time);

    cerr << time_span.count() << endl;

    
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
