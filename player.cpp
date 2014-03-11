#include "player.h"

/*
 * Constructor for the player; initialize everything here. The side your AI is
 * on (BLACK or WHITE) is passed in as "side". The constructor must finish 
 * within 30 seconds.
 */
Player::Player(Side side) {
    // Will be set to true in test_minimax.cpp.
    testingMinimax = false;
    maxDepth = 6;
    minDepth = 6;

    mySide = side;
    oppSide = (side == WHITE) ? (BLACK) : (WHITE);

    // set up bitmasks
    CORNERS = 0x8100000000000081;
    EDGES = 0x3C0081818181003C;
    ADJ_CORNERS = 0x42C300000000C342;
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
    
    int score = -99999;
    //int score = -64;
    Move *myMove = NULL;
    // find and test all legal moves
    vector<Move *> legalMoves = game.getLegalMoves(mySide);
    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Board *copy = game.copy();
        copy->doMove(legalMoves[i], mySide);
        // run the recursion to find scores
        int tempScore = negascout(copy, mySide, minDepth, -99999, 99999);
        if (tempScore >= score) {
            score = tempScore;
            myMove = legalMoves[i];
        }
        delete copy;
    }

    if (myMove == NULL)
        return myMove;

    // sort legalMoves, delete bad branches
    // change if statement below

    if (msLeft > 0) {
        for (unsigned int i = 0; i < legalMoves.size(); i++) {
            Board *copy = game.copy();
            copy->doMove(legalMoves[i], mySide);
            // run the recursion to find scores
            int tempScore = negascout(copy, mySide, maxDepth, -99999, 99999);
            if (tempScore >= score) {
                score = tempScore;
                myMove = legalMoves[i];
            }
            delete copy;
        }
    }

    myMove = new Move(myMove->getX(), myMove->getY());
    deleteMoveVector(legalMoves);

    game.doMove(myMove, mySide);
    return myMove;
}

int Player::negascout(Board *b, Side s, int depth, int alpha, int beta) {
    int side;
    if (s == mySide)
        side = 1;
    else
        side = -1;

    if (depth <= 1) {
        return side * heuristic(b);
    }

    int score;
    vector <Move *> legalMoves = b->getLegalMoves(s);
    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Board *copy = b->copy();
        copy->doMove(legalMoves[i], s);
        if (i != 0) {
            score = -negascout(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -negascout(copy, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -score);
            }
        }
        else {
            score = -negascout(copy, ((s == WHITE) ? (BLACK) :
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


int Player::heuristic (Board *b) {
    int score = b->count(mySide) - b->count(oppSide);
    bitbrd bm = b->toBits(mySide);

    score += 5 * countSetBits(bm & CORNERS);
    //score += 3 * countSetBits(bm & EDGES);
    score -= 3 * countSetBits(bm & ADJ_CORNERS);
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

// g++ -std=c++0x -o memtest player.cpp board.cpp
/*int main(int argc, char **argv) {
    Player p(BLACK);
    Move m (3,5);
    p.doMove(&m, -1);
    Move m2 (2,6);
    p.doMove(&m2, -1);
}*/
