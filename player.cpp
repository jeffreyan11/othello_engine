#include "player.h"

/*
 * Constructor for the player; initialize everything here. The side your AI is
 * on (BLACK or WHITE) is passed in as "side". The constructor must finish 
 * within 30 seconds.
 */
Player::Player(Side side) {
    // Will be set to true in test_minimax.cpp.
    testingMinimax = false;

    mySide = side;
    oppSide = (side == WHITE) ? (BLACK) : (WHITE);

    // set up bitmasks
    CORNERS = bitset<64> (0x8100000000000081);
    EDGES = bitset<64> (0x3C0081818181003C);
    ADJ_CORNERS = bitset<64> (0x42C300000000C342);
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
    vector<Move *> legalMoves = game.getLegalMoves(mySide);
    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Move *tempMove = legalMoves[i];
        if (heuristic(tempMove) > score) {
            score = heuristic(tempMove);
            myMove = tempMove;
        }
    }

    game.doMove(myMove, mySide);
    return myMove;
}

int Player::heuristic (Move * nextMove) {
    Board * copy = game.copy();
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
    return score;
}

/*
 * Converts a move into a bitmask (which will be all zeros except a single one)
*/
bitset<64> Player::moveToBit(Move *m) {
    bitset<64> result;
    result.set(m->getX() + 8 * m->getY());
    return result;
}
