#include "board.h"

/**
 * @brief Make a 8x8 othello board and initialize it to the standard setup.
 */
Board::Board() {
    taken = 0x0000001818000000;
    black = 0x0000000810000000;
    legal = 0x0000102004080000;
}

/**
 * @brief A constructor allowing specification of taken, black, legal.
*/
Board::Board(bitbrd t, bitbrd b, bitbrd l) {
    taken = t;
    black = b;
    legal = l;
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
    Board *newBoard = new Board(taken, black, legal);
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
 * For debugging only, not used anywhere in this program.
 */
bool Board::checkMove(Move *m, Side side) {
    // Passing is only legal if you have no moves.
    if (m == NULL) return !hasMoves(side);

    if(legal == 0xFFFF000000000000)
        getLegal(side);

    return legal & MOVEMASK[m->getX() + 8 * m->getY()];
}

/**
 * @brief Overloaded function for internal use with getLegalMoves().
*/
bool Board::checkMove(int index, Side side) {
    if(legal == 0xFFFF000000000000)
        getLegal(side);

    return legal & MOVEMASK[index];
}

/**
 * @brief Modifies the board to reflect the specified move.
 * 
 * This algorithm modifies the bitboards by lookup with precalculated tables in
 * each of the eight directions.
 */
void Board::doMove(int index, Side side) {
    // A NULL move means pass.
    if (index == MOVE_NULL) {
        legal = 0xFFFF000000000000;
        return;
    }

    // Ignore if move is invalid.
    if (!checkMove(index, side)) return;

    bitbrd changeMask = 0;
    bitbrd pos = (side == WHITE) ? ~black : ~(taken^black);
    bitbrd self = (side == BLACK) ? black : taken^black;
    //bitbrd block, result;

    switch(BOARD_REGIONS[index]) {
    case 1:
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 2:
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 3:
        changeMask |= southFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        break;
    case 4:
        changeMask |= northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 6:
        changeMask |= northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        break;
    case 7:
        changeMask |= northFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        break;
    case 8:
        changeMask |= northFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        break;
    case 9:
        changeMask |= northFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        break;
    case 5:
        changeMask |= northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    }

    /*changeMask |= northFill(index, self, pos);
    changeMask |= southFill(index, self, pos);
    changeMask |= eastFill(index, self, pos);
    changeMask |= westFill(index, self, pos);
    changeMask |= neFill(index, self, pos);
    changeMask |= nwFill(index, self, pos);
    changeMask |= swFill(index, self, pos);
    changeMask |= seFill(index, self, pos);*/

    // update taken, black, legal
    taken |= changeMask;
    if(side == BLACK)
        black |= changeMask;
    else
        black &= ~changeMask;

    legal = 0xFFFF000000000000;
}

/**
 * @brief Returns a vector of all legal moves.
*/
vector<int> Board::getLegalMoves(Side side) {
    vector<int> result;
    getLegal(side);
    bitbrd temp = legal;
    bitbrd corner = temp & 0x8100000000000081;
    bitbrd csq = temp & 0x2400810000810024;
    bitbrd adj = temp & 0x42C300000000C342;
    temp &= 0x183C7EFFFF7E3C18;
    if(corner) {
        result.push_back(bitScanForward(corner));
        corner &= corner-1;
      if(corner) {
        result.push_back(bitScanForward(corner));
        corner &= corner-1;
        if(corner) {
          result.push_back(bitScanForward(corner));
          corner &= corner-1;
          if(corner)
            result.push_back(bitScanForward(corner));
        }
      }
    }
    while(csq) {
        result.push_back(bitScanForward(csq));
        csq &= csq-1;
    }
    while(temp) {
        result.push_back(bitScanForward(temp));
        temp &= temp-1;
    }
    while(adj) {
        result.push_back(bitScanForward(adj));
        adj &= adj-1;
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

    return countSetBits(result);
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

    return countSetBits(result);
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

/*
 * Current count of given side's stones.
 */
int Board::count(Side side) {
    bitbrd i = (side == BLACK) ? (black) : (black^taken);

    #if defined(__x86_64__)
        asm ("popcnt %1, %0" : "=r" (i) : "r" (i));
        return (int) i;
    #elif defined(__i386)
        int a = (int) (i & 0xFFFFFFFF);
        int b = (int) ((i>>32) & 0xFFFFFFFF);
        asm ("popcntl %1, %0" : "=r" (a) : "r" (a));
        asm ("popcntl %1, %0" : "=r" (b) : "r" (b));
        return a+b;
    #else
        i = i - ((i >> 1) & 0x5555555555555555);
        i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333);
        i = (((i + (i >> 4)) & 0x0F0F0F0F0F0F0F0F) *
              0x0101010101010101) >> 56;
        return (int) i;
    #endif
}

int Board::bitScanForward(bitbrd bb) {
    return index64[(int)(((bb ^ (bb-1)) * 0x03f79d71b4cb0a89) >> 58)];
}

int Board::bitScanReverse(bitbrd bb) {
    #if defined(__x86_64__)
        asm ("bsr %1, %0" : "=r" (bb) : "r" (bb));
        return (int) bb;
    #elif defined(__i386)
        int b = (int) ((bb>>32) & 0xFFFFFFFF);
        if(b) {
            asm ("bsrl %1, %0" : "=r" (b) : "r" (b));
            return b+32;
        }
        else {
            int a = (int) (bb & 0xFFFFFFFF);
            asm ("bsrl %1, %0" : "=r" (a) : "r" (a));
            return a;
        }
    #else
        const bitbrd debruijn64 = 0x03f79d71b4cb0a89;
        bb |= bb >> 1;
        bb |= bb >> 2;
        bb |= bb >> 4;
        bb |= bb >> 8;
        bb |= bb >> 16;
        bb |= bb >> 32;
        return index64[(int)((bb * debruijn64) >> 58)];
    #endif
}

int Board::countSetBits(bitbrd i) {
    #if defined(__x86_64__)
        asm ("popcnt %1, %0" : "=r" (i) : "r" (i));
        return (int) i;
    #elif defined(__i386)
        int a = (int) (i & 0xFFFFFFFF);
        int b = (int) ((i>>32) & 0xFFFFFFFF);
        asm ("popcntl %1, %0" : "=r" (a) : "r" (a));
        asm ("popcntl %1, %0" : "=r" (b) : "r" (b));
        return a+b;
    #else
        i = i - ((i >> 1) & 0x5555555555555555);
        i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333);
        i = (((i + (i >> 4)) & 0x0F0F0F0F0F0F0F0F) *
              0x0101010101010101) >> 56;
        return (int) i;
    #endif
}

bitbrd Board::northFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = NORTHRAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanReverse(block);
        if(self & MOVEMASK[anchor])
            return (result ^ NORTHRAYI[anchor]) | MOVEMASK[index];
    }
    return 0;
}

bitbrd Board::southFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = SOUTHRAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanForward(block);
        if(self & MOVEMASK[anchor])
            return (result ^ SOUTHRAYI[anchor]) | MOVEMASK[index];
    }
    return 0;
}

bitbrd Board::eastFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = EASTRAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanForward(block);
        if(self & MOVEMASK[anchor])
            return (result ^ EASTRAYI[anchor]) | MOVEMASK[index];
    }
    return 0;
}

bitbrd Board::westFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = WESTRAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanReverse(block);
        if(self & MOVEMASK[anchor])
            return (result ^ WESTRAYI[anchor]) | MOVEMASK[index];
    }
    return 0;
}

bitbrd Board::neFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = NERAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanReverse(block);
        if(self & MOVEMASK[anchor])
            return (result ^ NERAYI[anchor]) | MOVEMASK[index];
    }
    return 0;
}

bitbrd Board::nwFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = NWRAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanReverse(block);
        if(self & MOVEMASK[anchor])
            return (result ^ NWRAYI[anchor]) | MOVEMASK[index];
    }
    return 0;
}

bitbrd Board::swFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = SWRAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanForward(block);
        if(self & MOVEMASK[anchor])
            return (result ^ SWRAYI[anchor]) | MOVEMASK[index];
    }
    return 0;
}

bitbrd Board::seFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = SERAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanForward(block);
        if(self & MOVEMASK[anchor])
            return (result ^ SERAYI[anchor]) | MOVEMASK[index];
    }
    return 0;
}
