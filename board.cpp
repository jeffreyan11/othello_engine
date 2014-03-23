#include "board.h"
#include <iostream>

/**
 * @brief For converting a move number 0-63 to a bitmask.
*/
const bitbrd MOVEMASK[64] = {
0x0000000000000001, 0x0000000000000002, 0x0000000000000004, 0x0000000000000008,
0x0000000000000010, 0x0000000000000020, 0x0000000000000040, 0x0000000000000080,
0x0000000000000100, 0x0000000000000200, 0x0000000000000400, 0x0000000000000800,
0x0000000000001000, 0x0000000000002000, 0x0000000000004000, 0x0000000000008000,
0x0000000000010000, 0x0000000000020000, 0x0000000000040000, 0x0000000000080000,
0x0000000000100000, 0x0000000000200000, 0x0000000000400000, 0x0000000000800000,
0x0000000001000000, 0x0000000002000000, 0x0000000004000000, 0x0000000008000000,
0x0000000010000000, 0x0000000020000000, 0x0000000040000000, 0x0000000080000000,
0x0000000100000000, 0x0000000200000000, 0x0000000400000000, 0x0000000800000000,
0x0000001000000000, 0x0000002000000000, 0x0000004000000000, 0x0000008000000000,
0x0000010000000000, 0x0000020000000000, 0x0000040000000000, 0x0000080000000000,
0x0000100000000000, 0x0000200000000000, 0x0000400000000000, 0x0000800000000000,
0x0001000000000000, 0x0002000000000000, 0x0004000000000000, 0x0008000000000000,
0x0010000000000000, 0x0020000000000000, 0x0040000000000000, 0x0080000000000000,
0x0100000000000000, 0x0200000000000000, 0x0400000000000000, 0x0800000000000000,
0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
};

/**
 * @brief Make a standard 8x8 othello board and initialize it to the standard setup.
 */
Board::Board() {
    taken = 0x0000001818000000;
    black = 0x0000000810000000;
    legal = 0x0000102004080000;
}

/**
 * @brief Destructor for the board.
 */
Board::~Board() {
}

/**
 * @brief Returns a copy of this board.
 */
Board *Board::copy() {
    Board *newBoard = new Board();
    newBoard->black = black;
    newBoard->taken = taken;
    newBoard->legal = legal;
    return newBoard;
}
 
/**
 * @brief Returns true if the game is finished; false otherwise. The game is
 * finished if neither side has a legal move.
 */
bool Board::isDone() {
    return !(hasMoves(BLACK) || hasMoves(WHITE));
}

/**
 * @brief Returns true if there are legal moves for the given side.
 */
bool Board::hasMoves(Side side) {
    return numLegalMoves(side);
}

/**
 * @brief Returns true if a move is legal for the given side; false otherwise.
 */
bool Board::checkMove(Move *m, Side side) {
    // Passing is only legal if you have no moves.
    if (m == NULL) return !hasMoves(side);

    if(legal == 0xFFFF000000000000)
        getLegal(side);

    return legal & MOVEMASK[m->getX() + 8 * m->getY()];
}

/*
 * Overloaded function taking x, y instead of a move object for internal use.
 * Passing is not an option here.
*/
bool Board::checkMove(int index, Side side) {
    if(legal == 0xFFFF000000000000)
        getLegal(side);

    return legal & MOVEMASK[index];
}

/**
 * @brief Modifies the board to reflect the specified move.
 */
void Board::doMove(int index, Side side) {
    // A NULL move means pass.
    if (index == MOVE_NULL) {
        legal = 0xFFFF000000000000;
        return;
    }

    // Ignore if move is invalid.
    if (!checkMove(index, side)) return;

    bitbrd mv = MOVEMASK[index];

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

        legal = 0xFFFF000000000000;
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

        legal = 0xFFFF000000000000;
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

int Board::countHigh(Side side) {
    bitbrd i = (side == BLACK) ? (black) : (black^taken);

    i = i - ((i >> 1) & 0x5555555555555555);
    i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333);
    i = (((i + (i >> 4)) & 0x0F0F0F0F0F0F0F0F) *
          0x0101010101010101) >> 56;
    return (int) i;
}

/**
 * @brief Returns a vector of all legal moves.
*/
vector<int> Board::getLegalMoves(Side side) {
    vector<int> result;
    for (int i = 0; i < 64; i++) {
        if (checkMove(i, side)) {
            result.push_back(i);
        }
    }
    return result;
}

/**
 * @brief Stores all legal moves for a side in the bitbrd object legal, for
 * quick retrieval later by checkMove().
 * 
 * This method operates by checking in all eight directions, first for the line
 * of pieces of the opposite color, then for the anchor once the line ends.
*/
void Board::getLegal(Side side) {
    bitbrd result = 0;
    bitbrd tempM;
    bitbrd self = (side == BLACK) ? (black) : (taken ^ black);
    bitbrd other = (side == BLACK) ? (taken ^ black) : (black);
    bitbrd empty = ~taken;
    // check north captures
    tempM = (self >> 8) & other;
    while(tempM) {
        bitbrd temp = (tempM >> 8);
        result |= temp & empty;
        tempM = temp & other;
    }
    // south
    tempM = (self << 8) & other;
    while(tempM) {
        bitbrd temp = (tempM << 8);
        result |= temp & empty;
        tempM = temp & other;
    }
    // east
    tempM = (self << 1) & other & 0xFEFEFEFEFEFEFEFE;
    while(tempM) {
        bitbrd temp = (tempM << 1) & 0xFEFEFEFEFEFEFEFE;
        result |= temp & empty;
        tempM = temp & other;
    }
    // west
    tempM = (self >> 1) & other & 0x7F7F7F7F7F7F7F7F;
    while(tempM) {
        bitbrd temp = (tempM >> 1) & 0x7F7F7F7F7F7F7F7F;
        result |= temp & empty;
        tempM = temp & other;
    }
    // ne
    tempM = (self >> 7) & other & 0xFEFEFEFEFEFEFEFE;
    while(tempM) {
        bitbrd temp = (tempM >> 7) & 0xFEFEFEFEFEFEFEFE;
        result |= temp & empty;
        tempM = temp & other;
    }
    // nw
    tempM = (self >> 9) & other & 0x7F7F7F7F7F7F7F7F;
    while(tempM) {
        bitbrd temp = (tempM >> 9) & 0x7F7F7F7F7F7F7F7F;
        result |= temp & empty;
        tempM = temp & other;
    }
    // sw
    tempM = (self << 7) & other & 0x7F7F7F7F7F7F7F7F;
    while(tempM) {
        bitbrd temp = (tempM << 7) & 0x7F7F7F7F7F7F7F7F;
        result |= temp & empty;
        tempM = temp & other;
    }
    // se
    tempM = (self << 9) & other & 0xFEFEFEFEFEFEFEFE;
    while(tempM) {
        bitbrd temp = (tempM << 9) & 0xFEFEFEFEFEFEFEFE;
        result |= temp & empty;
        tempM = temp & other;
    }

    legal = result;
}

int Board::numLegalMoves(Side side) {
    bitbrd result = 0;
    bitbrd tempM;
    bitbrd self = (side == BLACK) ? (black) : (taken ^ black);
    bitbrd other = (side == BLACK) ? (taken ^ black) : (black);
    bitbrd empty = ~taken;
    // check north captures
    tempM = (self >> 8) & other;
    while(tempM) {
        bitbrd temp = (tempM >> 8);
        result |= temp & empty;
        tempM = temp & other;
    }
    // south
    tempM = (self << 8) & other;
    while(tempM) {
        bitbrd temp = (tempM << 8);
        result |= temp & empty;
        tempM = temp & other;
    }
    // east
    tempM = (self << 1) & other & 0xFEFEFEFEFEFEFEFE;
    while(tempM) {
        bitbrd temp = (tempM << 1) & 0xFEFEFEFEFEFEFEFE;
        result |= temp & empty;
        tempM = temp & other;
    }
    // west
    tempM = (self >> 1) & other & 0x7F7F7F7F7F7F7F7F;
    while(tempM) {
        bitbrd temp = (tempM >> 1) & 0x7F7F7F7F7F7F7F7F;
        result |= temp & empty;
        tempM = temp & other;
    }
    // ne
    tempM = (self >> 7) & other & 0xFEFEFEFEFEFEFEFE;
    while(tempM) {
        bitbrd temp = (tempM >> 7) & 0xFEFEFEFEFEFEFEFE;
        result |= temp & empty;
        tempM = temp & other;
    }
    // nw
    tempM = (self >> 9) & other & 0x7F7F7F7F7F7F7F7F;
    while(tempM) {
        bitbrd temp = (tempM >> 9) & 0x7F7F7F7F7F7F7F7F;
        result |= temp & empty;
        tempM = temp & other;
    }
    // sw
    tempM = (self << 7) & other & 0x7F7F7F7F7F7F7F7F;
    while(tempM) {
        bitbrd temp = (tempM << 7) & 0x7F7F7F7F7F7F7F7F;
        result |= temp & empty;
        tempM = temp & other;
    }
    // se
    tempM = (self << 9) & other & 0xFEFEFEFEFEFEFEFE;
    while(tempM) {
        bitbrd temp = (tempM << 9) & 0xFEFEFEFEFEFEFEFE;
        result |= temp & empty;
        tempM = temp & other;
    }

    int n = 0;
    // while there are 1s
    while(result) {
        n++;
        result &= result - 1; // flip least significant 1
    }
    return n;
}

int Board::potentialMobility(Side side) {
    bitbrd result = 0;
    bitbrd temp;
    bitbrd other = (side == BLACK) ? (taken ^ black) : (black);
    bitbrd empty = ~taken;
    // check north
    temp = (other >> 8) & empty;
    temp <<= 8;
    result |= temp;
    // south
    temp = (other << 8) & empty;
    temp >>= 8;
    result |= temp;
    // east
    temp = (other << 1) & empty & 0xFEFEFEFEFEFEFEFE;
    temp >>= 1;
    result |= temp;
    // west
    temp = (other >> 1) & empty & 0x7F7F7F7F7F7F7F7F;
    temp <<= 1;
    result |= temp;
    // ne
    temp = (other >> 7) & empty & 0xFEFEFEFEFEFEFEFE;
    temp <<= 7;
    result |= temp;
    // nw
    temp = (other >> 9) & empty & 0x7F7F7F7F7F7F7F7F;
    temp <<= 9;
    result |= temp;
    // sw
    temp = (other << 7) & empty & 0x7F7F7F7F7F7F7F7F;
    temp >>= 7;
    result |= temp;
    // se
    temp = (other << 9) & empty & 0xFEFEFEFEFEFEFEFE;
    temp >>= 9;
    result |= temp;

    int n = 0;
    // while there are 1s
    while(result) {
        n++;
        result &= result - 1; // flip least significant 1
    }
    return n;
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

bitbrd Board::getTaken() {
    return taken;
}
bitbrd Board::getBlack() {
    return black;
}

// -------------Helper functions to check if a move is legal-------------
bool Board::bitCheck(bitbrd move, bitbrd pos, bitbrd self) {
    bool result = false;
    bitbrd mtemp;
    // check north
    // Check for pieces to capture
    mtemp = (move >> 8) & pos;
    while(mtemp & pos) {
        mtemp >>= 8;
    }
    // Check for anchor
    result |= (bool)(mtemp & self);

    // check south
    mtemp = (move << 8) & pos;
    while(mtemp & pos) {
        mtemp <<= 8;
    }
    result |= (bool)(mtemp & self);

    if(result)
        return result;

    // check east
    mtemp = (move << 1) & pos & 0xFEFEFEFEFEFEFEFE;
    while(mtemp & pos) {
        mtemp = (mtemp << 1) & 0xFEFEFEFEFEFEFEFE;
    }
    result |= (bool)(mtemp & self);

    // check west
    mtemp = (move >> 1) & pos & 0x7F7F7F7F7F7F7F7F;
    while(mtemp & pos) {
        mtemp = (mtemp >> 1) & 0x7F7F7F7F7F7F7F7F;
    }
    result |= (bool)(mtemp & self);

    if(result)
        return result;

    // check ne
    mtemp = (move >> 7) & pos & 0xFEFEFEFEFEFEFEFE;
    while(mtemp & pos) {
        mtemp = (mtemp >> 7) & 0xFEFEFEFEFEFEFEFE;
    }
    result |= (bool)(mtemp & self);

    // check nw
    mtemp = (move >> 9) & pos & 0x7F7F7F7F7F7F7F7F;
    while(mtemp & pos) {
        mtemp = (mtemp >> 9) & 0x7F7F7F7F7F7F7F7F;
    }
    result |= (bool)(mtemp & self);

    if(result)
        return result;

    // check sw
    mtemp = (move << 7) & pos & 0x7F7F7F7F7F7F7F7F;
    while(mtemp & pos) {
        mtemp = (mtemp << 7) & 0x7F7F7F7F7F7F7F7F;
    }
    result |= (bool)(mtemp & self);

    // check se
    mtemp = (move << 9) & pos & 0xFEFEFEFEFEFEFEFE;
    while(mtemp & pos) {
        mtemp = (mtemp << 9) & 0xFEFEFEFEFEFEFEFE;
    }
    result |= (bool)(mtemp & self);

    return result;
}

// -------------Helper functions to perform a move on the bitboard-------------
bitbrd Board::northFill(bitbrd move, bitbrd pos) {
    bitbrd result = move;
    bitbrd self = taken ^ pos;
    move >>= 8;
    while(move & pos) {
        result |= move;
        move >>= 8;
    }

    if(move & self)
        return result;
    else return 0;
}
bitbrd Board::southFill(bitbrd move, bitbrd pos) {
    bitbrd result = move;
    bitbrd self = taken ^ pos;
    move <<= 8;
    while(move & pos) {
        result |= move;
        move <<= 8;
    }

    if(move & self)
        return result;
    else return 0;
}
bitbrd Board::eastFill(bitbrd move, bitbrd pos) {
    bitbrd result = move;
    bitbrd self = taken ^ pos;
    move = (move << 1) & 0xFEFEFEFEFEFEFEFE;
    while(move & pos) {
        result |= move;
        move = (move << 1) & 0xFEFEFEFEFEFEFEFE;
    }

    if(move & self)
        return result;
    else return 0;
}
bitbrd Board::westFill(bitbrd move, bitbrd pos) {
    bitbrd result = move;
    bitbrd self = taken ^ pos;
    move = (move >> 1) & 0x7F7F7F7F7F7F7F7F;
    while(move & pos) {
        result |= move;
        move = (move >> 1) & 0x7F7F7F7F7F7F7F7F;
    }

    if(move & self)
        return result;
    else return 0;
}
bitbrd Board::neFill(bitbrd move, bitbrd pos) {
    bitbrd result = move;
    bitbrd self = taken ^ pos;
    move = (move >> 7) & 0xFEFEFEFEFEFEFEFE;
    while(move & pos) {
        result |= move;
        move = (move >> 7) & 0xFEFEFEFEFEFEFEFE;
    }

    if(move & self)
        return result;
    else return 0;
}
bitbrd Board::nwFill(bitbrd move, bitbrd pos) {
    bitbrd result = move;
    bitbrd self = taken ^ pos;
    move = (move >> 9) & 0x7F7F7F7F7F7F7F7F;
    while(move & pos) {
        result |= move;
        move = (move >> 9) & 0x7F7F7F7F7F7F7F7F;
    }

    if(move & self)
        return result;
    else return 0;
}
bitbrd Board::swFill(bitbrd move, bitbrd pos) {
    bitbrd result = move;
    bitbrd self = taken ^ pos;
    move = (move << 7) & 0x7F7F7F7F7F7F7F7F;
    while(move & pos) {
        result |= move;
        move = (move << 7) & 0x7F7F7F7F7F7F7F7F;
    }

    if(move & self)
        return result;
    else return 0;
}
bitbrd Board::seFill(bitbrd move, bitbrd pos) {
    bitbrd result = move;
    bitbrd self = taken ^ pos;
    move = (move << 9) & 0xFEFEFEFEFEFEFEFE;
    while(move & pos) {
        result |= move;
        move = (move << 9) & 0xFEFEFEFEFEFEFEFE;
    }

    if(move & self)
        return result;
    else return 0;
}
