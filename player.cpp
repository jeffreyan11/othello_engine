#include "player.h"

/*
 * Constructor for the player; initialize everything here. The side your AI is
 * on (BLACK or WHITE) is passed in as "side". The constructor must finish 
 * within 30 seconds.
 */
Player::Player(Side side) {
    // Will be set to true in test_minimax.cpp.
    testingMinimax = false;
    if(testingMinimax)
        maxDepth = 2;
    else
        maxDepth = 4;

    mySide = side;
    oppSide = (side == WHITE) ? (BLACK) : (WHITE);

    // set up bitmasks
    /*CORNERS = 0x8100000000000081;
    EDGES = 0x3C0081818181003C;
    ADJ_CORNERS = 0x42C300000000C342;*/
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
    
    int score = -64;
    Move *myMove = NULL;
    // find and test all legal moves
    vector<Move *> legalMoves = game.getLegalMoves(mySide);
    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Board *copy = game.copy();
        copy->doMove(legalMoves[i], mySide);
        // run the recursion to find scores
        int tempScore =
            minimax(copy, ((mySide == WHITE) ? (BLACK) : WHITE), maxDepth);
        if (tempScore >= score) {
            score = tempScore;
            myMove = legalMoves[i];
        }
        delete copy;
    }
    myMove = new Move(myMove->getX(), myMove->getY());
    deleteMoveVector(legalMoves);

    game.doMove(myMove, mySide);
    return myMove;
}

/*
 * Helper function for implementing minimax. The base case is depth <= 2 because
 * one depth is performed in doMove()
*/
int Player::minimax(Board * b, Side side, int depth) {
    if (depth <= 2) { // base case
        // score: maximizing from negative if your turn,
        // minimizing from positive if opponents turn
        int score;
        if (side == mySide)
            score = -64;
        else
            score = 64;
        // get all legal moves, create boards to test with, find best move
        vector <Move *> legalMoves = b->getLegalMoves(side);
        for (unsigned int i = 0; i < legalMoves.size(); i++) {
            int tempScore = heuristic(b, legalMoves[i]);
            if (side == mySide && tempScore >= score)
                score = tempScore;
            else if (side != mySide && tempScore <= score)
                score = tempScore;
        }
        deleteMoveVector(legalMoves);
        return score;
    }
    else { // recursive step
        int score;
        if (side == mySide)
            score = -64;
        else
            score = 64;
        // recurse for each legal move from current board position
        vector <Move *> legalMoves = b->getLegalMoves(side);
        for (unsigned int i = 0; i < legalMoves.size(); i++) {
            Board *copy = b->copy();
            copy->doMove(legalMoves[i], side);
            int tempScore = minimax(copy, ((side == WHITE) ? (BLACK) :
                WHITE), depth-1);
            delete copy;
            if (side == mySide && tempScore >= score)
                score = tempScore;
            else if (side != mySide && tempScore <= score)
                score = tempScore;
        }
        deleteMoveVector(legalMoves);
        return score;
    }
}

int Player::heuristic (Board *b, Move * nextMove) {
    Board * copy = b->copy();
    copy->doMove(nextMove, mySide);
    int score = copy->count(mySide) - copy->count(oppSide);
    /*bits movemask = moveToBit(nextMove);

    if(movemask & CORNERS)
        score += 5;
    else if(movemask & ADJ_CORNERS)
        score -= 3;*/

    if (nextMove->getX() == 0) {
        if (nextMove->getY() == 0 || nextMove->getY() == 7)
            score += 5;
        else if (nextMove->getY() == 1 || nextMove->getY() == 6)
            score -= 3;
    }
    else if (nextMove->getX() == 7) {
        if (nextMove->getY() == 0 || nextMove->getY() == 7)
            score += 5;
        else if (nextMove->getY() == 1 || nextMove->getY() == 6)
            score -= 3;
    }
    else if (nextMove->getX() == 1) {
        if (nextMove->getY() == 0 || nextMove->getY() == 1 ||
                nextMove->getY() == 6 || nextMove->getY() == 7)
            score -= 3;
    }
    else if (nextMove->getX() == 6) {
        if (nextMove->getY() == 0 || nextMove->getY() == 1 ||
                nextMove->getY() == 6 || nextMove->getY() == 7)
            score -= 3;
    }
    delete copy;
    return score;
}

/*
 * Converts a move into a bitmask (which will be all zeros except a single one)
*/
/*bits Player::moveToBit(Move *m) {
    bits result = 1;
    for(int i = 0; i < (m->getX() + 8 * m->getY()); i++) {
        result <<= 1;
    }
    return result;
}*/

void Player::deleteMoveVector(vector<Move *> v) {
    while(v.size() > 0) {
        Move *m = v.back();
        v.pop_back();
        delete m;
    }
}

// g++ -o memtest player.cpp board.cpp
/*int main(int argc, char **argv) {
    Player p(BLACK);
    Move m (3,5);
    p.doMove(&m, -1);
    Move m2 (2,6);
    p.doMove(&m2, -1);
}*/
