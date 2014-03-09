#include "board.h"
#include <iostream>

/*
 * Make a standard 8x8 othello board and initialize it to the standard setup.
 */
Board::Board() {
    taken = 0x0000001818000000;
    black = 0x0000000810000000;

    // initialize movemasks
    for(int i = 0; i < 64; i++) {
        MOVEMASK[i] = 1;
        for(int j = 0; j < i; j++) {
            MOVEMASK[i] <<= 1;
        }
    }
}

/*
 * Destructor for the board.
 */
Board::~Board() {
}

/*
 * Returns a copy of this board.
 */
Board *Board::copy() {
    Board *newBoard = new Board();
    newBoard->black = black;
    newBoard->taken = taken;
    return newBoard;
}

bool Board::occupied(int x, int y) {
    return (MOVEMASK[x + 8*y] & taken);
}

bool Board::get(Side side, int x, int y) {
    if(side == BLACK)
        return (MOVEMASK[x + 8*y] & black);
    else
        return (occupied(x,y) && (MOVEMASK[x+8*y] & (black ^ taken)));
}

void Board::set(Side side, int x, int y) {
    taken = MOVEMASK[x + 8*y] | taken;
    if(side == BLACK) {
        black = MOVEMASK[x + 8*y] | black;
    }
    else
        black = ((~MOVEMASK[x + 8*y]) & black);
}

bool Board::onBoard(int x, int y) {
    return(0 <= x && x < 8 && 0 <= y && y < 8);
}

 
/*
 * Returns true if the game is finished; false otherwise. The game is finished 
 * if neither side has a legal move.
 */
bool Board::isDone() {
    return !(hasMoves(BLACK) || hasMoves(WHITE));
}

/*
 * Returns true if there are legal moves for the given side.
 */
bool Board::hasMoves(Side side) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Move move(i, j);
            if (checkMove(&move, side)) return true;
        }
    }
    return false;
}

/*
 * Returns true if a move is legal for the given side; false otherwise.
 */
bool Board::checkMove(Move *m, Side side) {
    // Passing is only legal if you have no moves.
    if (m == NULL) return !hasMoves(side);

    int X = m->getX();
    int Y = m->getY();

    // Make sure the square hasn't already been taken.
    if (occupied(X, Y)) return false;

    Side other = (side == BLACK) ? WHITE : BLACK;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dy == 0 && dx == 0) continue;

            // Is there a capture in that direction?
            int x = X + dx;
            int y = Y + dy;
            if (onBoard(x, y) && get(other, x, y)) {
                do {
                    x += dx;
                    y += dy;
                } while (onBoard(x, y) && get(other, x, y));

                if (onBoard(x, y) && get(side, x, y)) return true;
            }
        }
    }
    return false;
}

/*
 * Modifies the board to reflect the specified move.
 */
void Board::doMove(Move *m, Side side) {
    // A NULL move means pass.
    if (m == NULL) return;

    // Ignore if move is invalid.
    if (!checkMove(m, side)) return;

    int X = m->getX();
    int Y = m->getY();
    Side other = (side == BLACK) ? WHITE : BLACK;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dy == 0 && dx == 0) continue;

            int x = X;
            int y = Y;
            do {
                x += dx;
                y += dy;
            } while (onBoard(x, y) && get(other, x, y));

            if (onBoard(x, y) && get(side, x, y)) {
                x = X;
                y = Y;
                x += dx;
                y += dy;
                while (onBoard(x, y) && get(other, x, y)) {
                    set(side, x, y);
                    x += dx;
                    y += dy;
                }
            }
        }
    }
    set(side, X, Y);
}

/*
 * Current count of given side's stones.
 */
int Board::count(Side side) {
    return (side == BLACK) ? countBlack() : countWhite();
}

/*
 * Current count of black stones.
 */
int Board::countBlack() {
    int n = 0;
    bitbrd b = black;
    // while there are 1s
    while(b) {
        n++;
        b &= b - 1; // reset least significant 1
    }
    return n;
}

/*
 * Current count of white stones.
 */
int Board::countWhite() {
    int n = 0;
    bitbrd b = black ^ taken;
    // while there are 1s
    while(b) {
        n++;
        b &= b - 1; // reset least significant 1
    }
    return n;
}

/*
 * Returns a vector of all legal moves.
*/
vector<Move *> Board::getLegalMoves(Side side) {
    vector<Move *> result;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Move * myMove = new Move(i, j);
            if (checkMove(myMove, side))
                result.push_back(myMove);
            else delete myMove;
        }
    }
    return result;
}

bitbrd Board::toBits(Side side) {
    if(side == BLACK)
        return black;
    else
        return (taken ^ black);
}

/*
 * Sets the board state given an 8x8 char array where 'w' indicates a white
 * piece and 'b' indicates a black piece. Mainly for testing purposes.
 */
void Board::setBoard(char data[]) {
    taken = 0;
    black = 0;
    for (int i = 0; i < 64; i++) {
        if (data[i] == 'b') {
            taken = taken | MOVEMASK[i];
            black = black | MOVEMASK[i];
        } if (data[i] == 'w') {
            taken = taken | MOVEMASK[i];
        }
    }
}
