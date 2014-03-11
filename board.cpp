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
    }
    for(int i = 1; i < 64; i++) {
        MOVEMASK[i] = MOVEMASK[i-1] << 1;
    }

    NORTHLINE = 0x00000000000000FF;
    SOUTHLINE = 0xFF00000000000000;
    EASTLINE = 0x8080808080808080;
    WESTLINE = 0x0101010101010101;
    NELINE = 0x80808080808080FF;
    NWLINE = 0x01010101010101FF;
    SWLINE = 0xFF01010101010101;
    SELINE = 0xFF80808080808080;
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
    taken |= MOVEMASK[x + 8*y];
    if(side == BLACK) {
        black |= MOVEMASK[x + 8*y];
    }
    else
        black &= ~MOVEMASK[x + 8*y];
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

    bitbrd mv = MOVEMASK[m->getX() + 8 * m->getY()];

    if(side == BLACK) {
        bitbrd pos_other = taken ^ black;

        bitbrd filled = northFill(mv, pos_other);
        filled |= southFill(mv, pos_other);
        filled |= eastFill(mv, pos_other);
        filled |= westFill(mv, pos_other);
        filled |= neFill(mv, pos_other);
        filled |= nwFill(mv, pos_other);
        filled |= swFill(mv, pos_other);
        filled |= seFill(mv, pos_other);

        taken |= filled;
        black |= filled;
    }
    else {
        bitbrd filled = northFill(mv, black);
        filled |= southFill(mv, black);
        filled |= eastFill(mv, black);
        filled |= westFill(mv, black);
        filled |= neFill(mv, black);
        filled |= nwFill(mv, black);
        filled |= swFill(mv, black);
        filled |= seFill(mv, black);

        taken |= filled;
        black &= ~filled;
    }
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
        b &= b - 1; // flip least significant 1
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
        b &= b - 1; // flip least significant 1
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
            taken |= MOVEMASK[i];
            black |= MOVEMASK[i];
        } if (data[i] == 'w') {
            taken |= MOVEMASK[i];
        }
    }
}

// -------------Helper functions to check if a move is legal-------------
bool Board::northCheck(bitbrd move, bitbrd pos, bitbrd self) {
    bitbrd captures = 0;
    bitbrd mtemp = move;
    while(move) {
        captures |= move;
        move = (move >> 8) & pos;
    }

    bitbrd anchor = 0;
    self = ~self;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp >> 8) & self;
    }
    return ((anchor == captures) && ((anchor >> 8) == anchor));
}
bool Board::southCheck(bitbrd move, bitbrd pos, bitbrd self) {
    bitbrd captures = 0;
    bitbrd mtemp = move;
    while(move) {
        captures |= move;
        move = (move << 8) & pos;
    }

    bitbrd anchor = 0;
    self = ~self;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp << 8) & self;
    }

    return ((anchor == captures) && ((anchor << 8) == anchor));
}
bool Board::eastCheck(bitbrd move, bitbrd pos, bitbrd self) {
    bitbrd wrapperA = 0xFEFEFEFEFEFEFEFE;
    pos &= wrapperA;
    bitbrd captures = 0;
    bitbrd mtemp = move;
    while(move) {
        captures |= move;
        move = (move << 1) & pos;
    }

    bitbrd anchor = 0;
    self = ~self;
    self &= wrapperA;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp << 1) & self;
    }

    return ((anchor == captures) && ((anchor << 1) == anchor));
}
bool Board::westCheck(bitbrd move, bitbrd pos, bitbrd self) {
    bitbrd wrapperH = 0x7F7F7F7F7F7F7F7F;
    pos &= wrapperH;
    bitbrd captures = 0;
    bitbrd mtemp = move;
    while(move) {
        captures |= move;
        move = (move >> 1) & pos;
    }

    bitbrd anchor = 0;
    self = ~self;
    self &= wrapperH;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp >> 1) & self;
    }

    return ((anchor == captures) && ((anchor >> 1) == anchor));
}
bool Board::neCheck(bitbrd move, bitbrd pos, bitbrd self) {
    bitbrd wrapperA = 0xFEFEFEFEFEFEFEFE;
    pos &= wrapperA;
    bitbrd captures = 0;
    bitbrd mtemp = move;
    while(move) {
        captures |= move;
        move = (move >> 7) & pos;
    }

    bitbrd anchor = 0;
    self = ~self;
    self &= wrapperA;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp >> 7) & self;
    }

    return ((anchor == captures) && ((anchor >> 7) == anchor));
}
bool Board::nwCheck(bitbrd move, bitbrd pos, bitbrd self) {
    bitbrd wrapperH = 0x7F7F7F7F7F7F7F7F;
    pos &= wrapperH;
    bitbrd captures = 0;
    bitbrd mtemp = move;
    while(move) {
        captures |= move;
        move = (move >> 9) & pos;
    }

    bitbrd anchor = 0;
    self = ~self;
    self &= wrapperH;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp >> 9) & self;
    }

    return ((anchor == captures) && ((anchor >> 9) == anchor));
}
bool Board::swCheck(bitbrd move, bitbrd pos, bitbrd self) {
    bitbrd wrapperH = 0x7F7F7F7F7F7F7F7F;
    pos &= wrapperH;
    bitbrd captures = 0;
    bitbrd mtemp = move;
    while(move) {
        captures |= move;
        move = (move << 7) & pos;
    }

    bitbrd anchor = 0;
    self = ~self;
    self &= wrapperH;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp << 7) & self;
    }

    return ((anchor == captures) && ((anchor << 7) == anchor));
}
bool Board::seCheck(bitbrd move, bitbrd pos, bitbrd self) {
    bitbrd wrapperA = 0xFEFEFEFEFEFEFEFE;
    pos &= wrapperA;
    bitbrd captures = 0;
    bitbrd mtemp = move;
    while(move) {
        captures |= move;
        move = (move << 9) & pos;
    }

    bitbrd anchor = 0;
    self = ~self;
    self &= wrapperA;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp << 9) & self;
    }

    return ((anchor == captures) && ((anchor << 9) == anchor));
}

// -------------Helper functions to perform a move on the bitboard-------------
bitbrd Board::northFill(bitbrd move, bitbrd pos) {
    bitbrd result = 0;
    bitbrd mtemp = move;
    while(move) {
        result |= move;
        move = (move >> 8) & pos;
    }

    bitbrd anchor = 0;
    bitbrd self = ~(taken ^ pos);
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp >> 8) & self;
    }

    if( (result == anchor) && ((result ^ NORTHLINE) == (result | NORTHLINE)) )
        return result;
    else
        return 0;
}
bitbrd Board::southFill(bitbrd move, bitbrd pos) {
    bitbrd result = 0;
    bitbrd mtemp = move;
    while(move) {
        result |= move;
        move = (move << 8) & pos;
    }

    bitbrd anchor = 0;
    bitbrd self = ~(taken ^ pos);
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp << 8) & self;
    }

    if( (result == anchor) && ((result ^ SOUTHLINE) == (result | SOUTHLINE)) )
        return result;
    else
        return 0;
}
bitbrd Board::eastFill(bitbrd move, bitbrd pos) {
    bitbrd wrapperA = 0xFEFEFEFEFEFEFEFE;
    pos &= wrapperA;
    bitbrd result = 0;
    bitbrd mtemp = move;
    while(move) {
        result |= move;
        move = (move << 1) & pos;
    }

    bitbrd anchor = 0;
    bitbrd self = ~(taken ^ pos);
    self &= wrapperA;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp << 1) & self;
    }

    if( (result == anchor) && ((result ^ EASTLINE) == (result | EASTLINE)) )
        return result;
    else
        return 0;
}
bitbrd Board::westFill(bitbrd move, bitbrd pos) {
    bitbrd wrapperH = 0x7F7F7F7F7F7F7F7F;
    pos &= wrapperH;
    bitbrd result = 0;
    bitbrd mtemp = move;
    while(move) {
        result |= move;
        move = (move >> 1) & pos;
    }

    bitbrd anchor = 0;
    bitbrd self = ~(taken ^ pos);
    self &= wrapperH;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp >> 1) & self;
    }

    if( (result == anchor) && ((result ^ WESTLINE) == (result | WESTLINE)) )
        return result;
    else
        return 0;
}
bitbrd Board::neFill(bitbrd move, bitbrd pos) {
    bitbrd wrapperA = 0xFEFEFEFEFEFEFEFE;
    pos &= wrapperA;
    bitbrd result = 0;
    bitbrd mtemp = move;
    while(move) {
        result |= move;
        move = (move >> 7) & pos;
    }

    bitbrd anchor = 0;
    bitbrd self = ~(taken ^ pos);
    self &= wrapperA;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp >> 7) & self;
    }

    if( (result == anchor) && ((result ^ NELINE) == (result | NELINE)) )
        return result;
    else
        return 0;
}
bitbrd Board::nwFill(bitbrd move, bitbrd pos) {
    bitbrd wrapperH = 0x7F7F7F7F7F7F7F7F;
    pos &= wrapperH;
    bitbrd result = 0;
    bitbrd mtemp = move;
    while(move) {
        result |= move;
        move = (move >> 9) & pos;
    }

    bitbrd anchor = 0;
    bitbrd self = ~(taken ^ pos);
    self &= wrapperH;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp >> 9) & self;
    }

    if( (result == anchor) && ((result ^ NWLINE) == (result | NWLINE)) )
        return result;
    else
        return 0;
}
bitbrd Board::swFill(bitbrd move, bitbrd pos) {
    bitbrd wrapperH = 0x7F7F7F7F7F7F7F7F;
    pos &= wrapperH;
    bitbrd result = 0;
    bitbrd mtemp = move;
    while(move) {
        result |= move;
        move = (move << 7) & pos;
    }

    bitbrd anchor = 0;
    bitbrd self = ~(taken ^ pos);
    self &= wrapperH;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp << 7) & self;
    }

    if( (result == anchor) && ((result ^ SWLINE) == (result | SWLINE)) )
        return result;
    else
        return 0;
}
bitbrd Board::seFill(bitbrd move, bitbrd pos) {
    bitbrd wrapperA = 0xFEFEFEFEFEFEFEFE;
    pos &= wrapperA;
    bitbrd result = 0;
    bitbrd mtemp = move;
    while(move) {
        result |= move;
        move = (move << 9) & pos;
    }

    bitbrd anchor = 0;
    bitbrd self = ~(taken ^ pos);
    self &= wrapperA;
    while(mtemp) {
        anchor |= mtemp;
        mtemp = (mtemp << 9) & self;
    }

    if( (result == anchor) && ((result ^ SELINE) == (result | SELINE)) )
        return result;
    else
        return 0;
}
