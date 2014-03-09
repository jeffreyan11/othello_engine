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

    int w[] =
{0,0,5,0,1,-10,0,6,-10,0,7,5,1,0,-10,1,1,-10,1,6,-10,1,7,-10,2,0,3,2,7,3,3,0,3,3,7,3,
4,0,3,4,7,3,5,0,3,5,7,3,6,0,-10,6,1,-10,6,6,-10,6,7,-10,7,0,5,7,1,-10,7,6,-10,7,6,5};
    for (int i = 0; i < 72; i++)
        weights[i] = w[i];
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
    Move *myMove = NULL;
    // find and test all legal moves
    vector<Move *> legalMoves = game.getLegalMoves(mySide);
    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Board *copy = game.copy();
        copy->doMove(legalMoves[i], mySide);
        // run the recursion to find scores
        int tempScore = negascout(copy, ((mySide == WHITE) ? (BLACK) : WHITE), maxDepth, -99999, 99999);
        if (tempScore >= score) {
            score = tempScore;
            myMove = legalMoves[i];
        }
        delete copy;
    }
    if(myMove != NULL) {
        myMove = new Move(myMove->getX(), myMove->getY());
        deleteMoveVector(legalMoves);
    }
    else {
        deleteMoveVector(legalMoves);
        myMove = NULL;
    }

    game.doMove(myMove, mySide);
    return myMove;
}


int Player::negascout(Board *b, Side s, int depth, int alpha, int beta) {
    int side;
    if (s == mySide)
        side = 1;
    else
        side = -1;

    if (depth == 0) {
        return side * heuristic2(b);
    }

    int score;
    vector <Move *> legalMoves = b->getLegalMoves(s);
    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Board *copy = b->copy();
        copy->doMove(legalMoves[i], s);
        if (i == 0) {
            score = -negascout(copy, ((side == WHITE) ? (BLACK) :
                WHITE), depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -negascout(copy, ((side == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -score);
            }
        }
        else {
            score = -negascout(copy, ((side == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -alpha);
            }
        if (alpha < score)
            alpha = score;
        if (alpha >= beta)
            break;
        delete copy;
    }
    return alpha;
}

int Player::heuristic (Board *b, Move * nextMove) {
    Board * copy = b->copy();
    copy->doMove(nextMove, mySide);
    int score = copy->count(mySide) - copy->count(oppSide);

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


int Player::heuristic2 (Board *b) {
    int score = b->count(mySide) - b->count(oppSide);
    for (int i = 0; i < 24; i++) {
        int x = weights[i*3];
        int y = weights[i*3+1];
        int w = weights[i*3+2];
        if (b->get(mySide, x, y)) {
            score += w;
        }
    }
    return score;
}


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
