#include "board.h"
#include <iostream>

/**
 * @brief Make a 8x8 othello board and initialize it to the standard setup.
 */
Board::Board() {
    taken = 0x0000001818000000;
    black = 0x0000000810000000;
}

/**
 * @brief A constructor allowing specification of taken, black.
*/
Board::Board(bitbrd t, bitbrd b) {
    taken = t;
    black = b;
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
    Board *newBoard = new Board(taken, black);
    return newBoard;
}
 
/**
 * @brief Returns true if the game is finished; false otherwise. The game is
 * finished if neither side has a legal move.
 */
bool Board::isDone() {
    return !(hasMoves(CBLACK) || hasMoves(CWHITE));
}

/**
 * @brief Returns true if there are legal moves for the given side.
 */
bool Board::hasMoves(int side) {
    return numLegalMoves(side);
}

/**
 * @brief Returns true if a move is legal for the given side; false otherwise.
 * For debugging only, not used anywhere in this program.
 */
bool Board::checkMove(Move *m, Side side) {
    // Passing is only legal if you have no moves.
    if (m == NULL) return !hasMoves((side == BLACK) ? CBLACK : CWHITE);

    bitbrd legal = getLegal(side);

    return legal & MOVEMASK[m->getX() + 8 * m->getY()];
}

/**
 * @brief Overloaded function for internal use with getLegalMoves().
*/
bool Board::checkMove(int index, int side) {
    bitbrd legal = getLegal(side);
    return legal & MOVEMASK[index];
}

/**
 * @brief Modifies the board to reflect the specified move.
 * 
 * This algorithm modifies the bitboards by lookup with precalculated tables in
 * each of the eight directions. A switch with the board region is used to only
 * consider certain directions for efficiency.
 */
void Board::doMove(int index, int side) {
    // A NULL move means pass.
    if (index == MOVE_NULL) {
        return;
    }

    // Ignore if move is invalid.
    //if (!checkMove(index, side)) return;

    bitbrd changeMask = 0;
    bitbrd pos = (side == CWHITE) ? ~black : ~(taken^black);
    bitbrd self = (side == CBLACK) ? black : taken^black;

    switch(BOARD_REGIONS[index]) {
    case 2:
        changeMask = southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 4:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 6:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        break;
    case 8:
        changeMask = northFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        break;
    case 1:
        changeMask = southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 3:
        changeMask = southFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        break;
    case 7:
        changeMask = northFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        break;
    case 9:
        changeMask = northFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        break;
    case 5:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    }

    changeMask |= MOVEMASK[index];

    // update taken, black
    taken |= changeMask;
    if(side == CBLACK)
        black |= changeMask;
    else
        black &= ~changeMask;
}

/**
 * @brief This function returns the mask of changed bits.
 */
bitbrd Board::getDoMove(int index, int side) {
    bitbrd changeMask = 0;
    bitbrd pos = (side == CWHITE) ? ~black : ~(taken^black);
    bitbrd self = (side == CBLACK) ? black : taken^black;

    switch(BOARD_REGIONS[index]) {
    case 2:
        changeMask = southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 4:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 6:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        break;
    case 8:
        changeMask = northFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        break;
    case 1:
        changeMask = southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 3:
        changeMask = southFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        break;
    case 7:
        changeMask = northFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        break;
    case 9:
        changeMask = northFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        break;
    case 5:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    }

    return changeMask;
}

void Board::makeMove(int index, bitbrd changeMask, int side) {
    taken |= MOVEMASK[index];
    black ^= changeMask | ((side == CBLACK) * MOVEMASK[index]);
}

void Board::undoMove(int index, bitbrd changed, int side) {
    taken ^= MOVEMASK[index];
    black ^= changed | ((side == CBLACK) * MOVEMASK[index]);
}

/**
 * @brief Returns a list of all legal moves.
*/
MoveList Board::getLegalMoves(int side) {
    MoveList result;
    bitbrd temp = getLegal(side);
    bitbrd corner = temp & 0x8100000000000081;
    bitbrd csq = temp & 0x2400810000810024;
    bitbrd adj = temp & 0x42C300000000C342;
    temp &= 0x183C7EFFFF7E3C18;
    if(corner) {
        result.add(bitScanForward(corner));
        corner &= corner-1;
      if(corner) {
          result.add(bitScanForward(corner));
          corner &= corner-1;
        if(corner) {
            result.add(bitScanForward(corner));
            corner &= corner-1;
          if(corner)
              result.add(bitScanForward(corner));
        }
      }
    }
    while(csq) {
        result.add(bitScanForward(csq));
        csq &= csq-1;
    }
    while(temp) {
        result.add(bitScanForward(temp));
        temp &= temp-1;
    }
    while(adj) {
        result.add(bitScanForward(adj));
        adj &= adj-1;
    }
    return result;
}

/**
 * @brief Returns a list of all legal moves, with priorities for sorting.
*/
MoveList Board::getLegalMovesOrdered(int side, MoveList &priority) {
    MoveList result;
    bitbrd temp = getLegal(side);

    while(temp) {
        result.add(bitScanForward(temp));
        if(!(NEIGHBORS[result.last()] & ~taken))
            priority.add( 100 + SQ_VAL[result.last()] );
        else priority.add( SQ_VAL[result.last()] );
        temp &= temp-1;
    }

    return result;
}

/**
 * @brief Returns a list of all legal moves, given 4 or less empty squares.
*/
int Board::getLegalMoves4(int side, int &m1, int &m2, int &m3) {
    int m4 = MOVE_NULL;
    bitbrd temp = getLegal(side);
    int n = 0;

    if(temp) {
        m1 = bitScanForward(temp);
        temp &= temp-1; n++;
      if(temp) {
          m2 = bitScanForward(temp);
          temp &= temp-1; n++;
        if(temp) {
            m3 = bitScanForward(temp);
            temp &= temp-1; n++;
          if(temp) {
              m4 = bitScanForward(temp);
              n++;
          }
        }
      }
    }

    // parity sorting
    if(n == 2) {
        if( (NEIGHBORS[m1] & ~taken) && !(NEIGHBORS[m2] & ~taken) ) {
            int temp = m1;
            m1 = m2;
            m2 = temp;
        }
    }
    else if(n == 3) {
        if( (NEIGHBORS[m1] & ~taken) ) {
            if( !(NEIGHBORS[m2] & ~taken) ) {
                int temp = m1;
                m1 = m2;
                m2 = temp;
            }
            else if ( !(NEIGHBORS[m3] & ~taken) ) {
                int temp = m1;
                m1 = m3;
                m3 = temp;
            }
        }
    }
    else if(n == 4) {
        if( (NEIGHBORS[m1] & ~taken) ) {
            if( !(NEIGHBORS[m2] & ~taken) ) {
                int temp = m1;
                m1 = m2;
                m2 = m4;
                m4 = temp;
            }
            else if ( !(NEIGHBORS[m3] & ~taken) ) {
                int temp = m1;
                m1 = m3;
                m3 = temp;
                temp = m2;
                m2 = m4;
                m4 = temp;
            }
            else if ( !(NEIGHBORS[m4] & ~taken) ) {
                int temp = m1;
                m1 = m4;
                m4 = temp;
            }
        }
        else if( (NEIGHBORS[m2] & ~taken) ) {
            if ( !(NEIGHBORS[m3] & ~taken) ) {
                int temp = m2;
                m2 = m3;
                m3 = temp;
            }
            else if ( !(NEIGHBORS[m4] & ~taken) ) {
                int temp = m2;
                m2 = m4;
                m4 = temp;
            }
        }
    }

    return m4;
}

/**
 * @brief Returns a list of all legal moves, given 3 or less empty squares.
*/
int Board::getLegalMoves3(int side, int &m1, int &m2) {
    int result = MOVE_NULL;
    bitbrd temp = getLegal(side);
    int n = 0;

    if(temp) {
        m1 = bitScanForward(temp);
        temp &= temp-1; n++;
      if(temp) {
          m2 = bitScanForward(temp);
          temp &= temp-1; n++;
        if(temp) {
            result = bitScanForward(temp);
            n++;
        }
      }
    }

    // parity sorting
    if(n == 2) {
        if( (NEIGHBORS[m1] & ~taken) && !(NEIGHBORS[m2] & ~taken) ) {
            int temp = m1;
            m1 = m2;
            m2 = temp;
        }
    }
    else if(n == 3) {
        if( (NEIGHBORS[m1] & ~taken) ) {
            if( !(NEIGHBORS[m2] & ~taken) ) {
                int temp = m1;
                m1 = m2;
                m2 = temp;
            }
            else if ( !(NEIGHBORS[result] & ~taken) ) {
                int temp = m1;
                m1 = result;
                result = temp;
            }
        }
    }

    return result;
}

/**
 * @brief Returns a bitmask with a 1 set in every square that is a legal move
 * for the given side.
 * 
 * This method operates by checking in all eight directions, first for the line
 * of pieces of the opposite color, then for the anchor once the line ends.
*/
bitbrd Board::getLegal(int side) {
    bitbrd result = 0;
    bitbrd self = (side == CBLACK) ? (black) : (taken ^ black);
    bitbrd opp = (side == CBLACK) ? (taken ^ black) : (black);

#if USE_SSE && defined(__x86_64__)
    bitbrd mask_7e = 0x7e7e7e7e7e7e7e7eULL;

    asm volatile(
    "movq   %2, %%xmm14\n\t" // opp
    "movq   %3, %%xmm5\n\t" // mask = 0x7E7E7E7E7E7E7E7E
    "movq   %1, %%xmm15\n\t" // self
    "pand   %%xmm14, %%xmm5\n\t" // opp & mask

    /* shift=+1 */                  /* shift=+8 */
    /* shift=+7 */                  /* shift=+9 */
    "movq   %%xmm15, %%xmm8\n\t"    "movq   %%xmm15, %%xmm0\n\t"        //18
    "movq   %%xmm15, %%rax\n\t"     "movq   %%xmm15, %%xmm2\n\t"         //79
    "movq   %%xmm5, %%rbx\n\t"
    "psrlq  $7, %%xmm8\n\t"         "psrlq  $8, %%xmm0\n\t"             //18
    "shrq   $1, %%rax\n\t"          "psrlq  $9, %%xmm2\n\t"              //79
    "pand   %%xmm5, %%xmm8\n\t"     "pand   %%xmm14, %%xmm0\n\t"
    "andq   %%rbx, %%rax\n\t"       "pand   %%xmm5, %%xmm2\n\t"
    "movq   %%xmm8, %%xmm9\n\t"     "movq   %%xmm0, %%xmm1\n\t"
    "movq   %%rax, %%rcx\n\t"       "movq   %%xmm2, %%xmm6\n\t"
    "psrlq  $7, %%xmm8\n\t"         "psrlq  $8, %%xmm0\n\t"
    "shrq   $1, %%rax\n\t"          "psrlq  $9, %%xmm2\n\t"
    "movq   %%xmm5, %%xmm11\n\t"    "movq   %%xmm14, %%xmm3\n\t"
    "movq   %%rbx, %%rdx\n\t"       "movq   %%xmm5, %%xmm13\n\t"
    "pand   %%xmm5, %%xmm8\n\t"     "pand   %%xmm14, %%xmm0\n\t"
    "andq   %%rbx, %%rax\n\t"       "pand   %%xmm5, %%xmm2\n\t"
    "psrlq  $7, %%xmm11\n\t"        "psrlq  $8, %%xmm3\n\t"
    "shrq   $1, %%rdx\n\t"          "psrlq  $9, %%xmm13\n\t"
    "por    %%xmm9, %%xmm8\n\t"     "por    %%xmm1, %%xmm0\n\t"
    "orq    %%rcx, %%rax\n\t"       "por    %%xmm6, %%xmm2\n\t"
    "pand   %%xmm5, %%xmm11\n\t"    "pand   %%xmm14, %%xmm3\n\t"
    "andq   %%rbx, %%rdx\n\t"       "pand   %%xmm5, %%xmm13\n\t"
    "movq   %%xmm8, %%xmm12\n\t"    "movq   %%xmm0, %%xmm4\n\t"
    "movq   %%rax, %%rcx\n\t"       "movq   %%xmm2, %%xmm6\n\t"
    "psrlq  $14, %%xmm8\n\t"        "psrlq  $16, %%xmm0\n\t"
    "shrq   $2, %%rax\n\t"          "psrlq  $18, %%xmm2\n\t"
    "pand   %%xmm11, %%xmm8\n\t"    "pand   %%xmm3, %%xmm0\n\t"
    "andq   %%rdx, %%rax\n\t"       "pand   %%xmm13, %%xmm2\n\t"
    "por    %%xmm8, %%xmm12\n\t"    "por    %%xmm0, %%xmm4\n\t"
    "orq    %%rax, %%rcx\n\t"       "por    %%xmm2, %%xmm6\n\t"
    "psrlq  $14, %%xmm8\n\t"        "psrlq  $16, %%xmm0\n\t"
    "shrq   $2, %%rax\n\t"          "psrlq  $18, %%xmm2\n\t"
    "pand   %%xmm11, %%xmm8\n\t"    "pand   %%xmm3, %%xmm0\n\t"
    "andq   %%rdx, %%rax\n\t"       "pand   %%xmm13, %%xmm2\n\t"
    "por    %%xmm8, %%xmm12\n\t"    "por    %%xmm0, %%xmm4\n\t"
    "orq    %%rcx, %%rax\n\t"       "por    %%xmm6, %%xmm2\n\t"
    "psrlq  $7, %%xmm12\n\t"        "psrlq  $8, %%xmm4\n\t"
    "shrq   $1, %%rax\n\t"          "psrlq  $9, %%xmm2\n\t"
    "movq   %%rax, %%rsi\n\t"       "por    %%xmm2, %%xmm4\n\t"

    /* shift=-1 */                  /* shift=-8 */
    /* shift=-7 */                  /* shift=-9 */
    "movq   %%xmm15, %%xmm8\n\t"    "movq   %%xmm15, %%xmm0\n\t"
    "movq   %%xmm15, %%rax\n\t"     "movq   %%xmm15, %%xmm2\n\t"
    "psllq  $7, %%xmm8\n\t"         "psllq  $8, %%xmm0\n\t"
    "leaq   (,%%rax,2), %%rax\n\t"  "psllq  $9, %%xmm2\n\t"
    "pand   %%xmm5, %%xmm8\n\t"     "pand   %%xmm14, %%xmm0\n\t"
    "andq   %%rbx, %%rax\n\t"       "pand   %%xmm5, %%xmm2\n\t"
    "movq   %%xmm8, %%xmm9\n\t"     "movq   %%xmm0, %%xmm1\n\t"
    "movq   %%rax, %%rcx\n\t"       "movq   %%xmm2, %%xmm6\n\t"
    "psllq  $7, %%xmm8\n\t"         "psllq  $8, %%xmm0\n\t"
    "leaq   (,%%rax,2), %%rax\n\t"  "psllq  $9, %%xmm2\n\t"
    "pand   %%xmm5, %%xmm8\n\t"     "pand   %%xmm14, %%xmm0\n\t"
    "andq   %%rbx, %%rax\n\t"       "pand   %%xmm5, %%xmm2\n\t"
    "por    %%xmm9, %%xmm8\n\t"     "por    %%xmm1, %%xmm0\n\t"
    "orq    %%rcx, %%rax\n\t"       "por    %%xmm6, %%xmm2\n\t"
    "psllq  $7, %%xmm11\n\t"        "psllq  $8, %%xmm3\n\t"
    "leaq   (,%%rdx,2), %%rdx\n\t"  "psllq  $9, %%xmm13\n\t"
    "movq   %%xmm8, %%xmm9\n\t"     "movq   %%xmm0, %%xmm1\n\t"
    "movq   %%rax, %%rcx\n\t"       "movq   %%xmm2, %%xmm6\n\t"
    "psllq  $14, %%xmm8\n\t"        "psllq  $16, %%xmm0\n\t"
    "leaq   (,%%rax,4), %%rax\n\t"  "psllq  $18, %%xmm2\n\t"
    "pand   %%xmm11, %%xmm8\n\t"    "pand   %%xmm3, %%xmm0\n\t"
    "andq   %%rdx, %%rax\n\t"       "pand   %%xmm13, %%xmm2\n\t"
    "por    %%xmm8, %%xmm9\n\t"     "por    %%xmm0, %%xmm1\n\t"
    "orq    %%rax, %%rcx\n\t"       "por    %%xmm2, %%xmm6\n\t"
    "psllq  $14, %%xmm8\n\t"        "psllq  $16, %%xmm0\n\t"
    "shlq   $2, %%rax\n\t"          "psllq  $18, %%xmm2\n\t"
    "pand   %%xmm11, %%xmm8\n\t"    "pand   %%xmm3, %%xmm0\n\t"
    "andq   %%rdx, %%rax\n\t"       "pand   %%xmm13, %%xmm2\n\t"
    "por    %%xmm9, %%xmm8\n\t"     "por    %%xmm1, %%xmm0\n\t"
    "orq    %%rcx, %%rax\n\t"       "por    %%xmm6, %%xmm2\n\t"
    "psllq  $7, %%xmm8\n\t"         "psllq  $8, %%xmm0\n\t"
    "leaq   (,%%rax,2), %%rax\n\t"  "psllq  $9, %%xmm2\n\t"
    "por    %%xmm8, %%xmm12\n\t"    "por    %%xmm0, %%xmm4\n\t"
    "orq    %%rax, %%rsi\n\t"
    "nop\n\t"
    "movq   %%rsi, %%xmm9\n\t"
                                    "por    %%xmm2, %%xmm4\n\t"
    "por    %%xmm9, %%xmm12\n\t"
    /* Combine, & with empty squares. */
    "nop\n\t"
    "por    %%xmm12, %%xmm4\n\t"
    "por    %%xmm14, %%xmm15\n\t"
    "pandn  %%xmm4, %%xmm15\n\t"
    "nop\n\t"
    "movq   %%xmm15, %0\n\t"
    : "=g" (result) : "m" (self), "m" (opp), "m" (mask_7e)
    : "rax", "rbx", "rcx", "rdx", "rsi");

    return result;

#elif USE_SSE && defined(__i386)
    bitbrd mask_7e = 0x7e7e7e7e7e7e7e7eULL;

    asm volatile(
    "movl   %3, %%esi\n\t"          "movq   %1, %%xmm7\n\t"
    "movl   %4, %%edi\n\t"          "movq   %2, %%xmm6\n\t"
    /* shift=+1 */                  /* shift=+8 */
    "movl   %%esi, %%eax\n\t"       "movq   %%xmm7, %%xmm0\n\t"
    "movq   %5, %%xmm5\n\t"
    "shrl   $1, %%eax\n\t"          "psrlq  $8, %%xmm0\n\t"
    "andl   $2122219134, %%edi\n\t" "pand   %%xmm6, %%xmm5\n\t"
    "andl   %%edi, %%eax\n\t"       "pand   %%xmm6, %%xmm0\n\t"       /* 0 m7&o6 m6&o5 .. m1&o0 */
    "movl   %%eax, %%edx\n\t"       "movq   %%xmm0, %%xmm1\n\t"
    "shrl   $1, %%eax\n\t"          "psrlq  $8, %%xmm0\n\t"
    "movl   %%edi, %%ecx\n\t"       "movq   %%xmm6, %%xmm3\n\t"
    "andl   %%edi, %%eax\n\t"       "pand   %%xmm6, %%xmm0\n\t"       /* 0 0 m7&o6&o5 .. m2&o1&o0 */
    "shrl   $1, %%ecx\n\t"          "psrlq  $8, %%xmm3\n\t"
    "orl    %%edx, %%eax\n\t"       "por    %%xmm1, %%xmm0\n\t"       /* 0 m7&o6 (m6&o5)|(m7&o6&o5) .. (m1&o0) */
    "andl   %%edi, %%ecx\n\t"       "pand   %%xmm6, %%xmm3\n\t"       /* 0 o7&o6 o6&o5 o5&o4 o4&o3 .. */
    "movl   %%eax, %%edx\n\t"       "movq   %%xmm0, %%xmm4\n\t"
    "shrl   $2, %%eax\n\t"          "psrlq  $16, %%xmm0\n\t"
    "andl   %%ecx, %%eax\n\t"       "pand   %%xmm3, %%xmm0\n\t"       /* 0 0 0 m7&o6&o5&o4 (m6&o5&o4&o3)|(m7&o6&o5&o4&o3) .. */
    "orl    %%eax, %%edx\n\t"       "por    %%xmm0, %%xmm4\n\t"
    "shrl   $2, %%eax\n\t"          "psrlq  $16, %%xmm0\n\t"
    "andl   %%ecx, %%eax\n\t"       "pand   %%xmm3, %%xmm0\n\t"       /* 0 0 0 0 0 m7&o6&..&o2 (m6&o5&..&o1)|(m7&o6&..&o1) .. */
    "orl    %%edx, %%eax\n\t"       "por    %%xmm0, %%xmm4\n\t"
    "shrl   $1, %%eax\n\t"          "psrlq  $8, %%xmm4\n\t"          /* result of +8 */
    /* shift=-1 */                          /* shift=-8 */
    "movq   %%xmm7, %%xmm0\n\t"
    "addl   %%esi, %%esi\n\t"       "psllq  $8, %%xmm0\n\t"
    "andl   %%edi, %%esi\n\t"       "pand   %%xmm6, %%xmm0\n\t"
    "movl   %%esi, %%edx\n\t"       "movq   %%xmm0, %%xmm1\n\t"
    "leal   (,%%esi,2), %%esi\n\t"  "psllq  $8, %%xmm0\n\t"
    "andl   %%edi, %%esi\n\t"       "pand   %%xmm6, %%xmm0\n\t"
    "orl    %%esi, %%edx\n\t"       "por    %%xmm1, %%xmm0\n\t"
    "leal   (,%%ecx,2), %%ecx\n\t"  "psllq  $8, %%xmm3\n\t"
    "movq   %%xmm0, %%xmm1\n\t"
    "leal   (,%%edx,4), %%esi\n\t"  "psllq  $16, %%xmm0\n\t"
    "andl   %%ecx, %%esi\n\t"       "pand   %%xmm3, %%xmm0\n\t"
    "orl    %%esi, %%edx\n\t"       "por    %%xmm0, %%xmm1\n\t"
    "leal   (,%%esi,4), %%esi\n\t"  "psllq  $16, %%xmm0\n\t"
    "andl   %%ecx, %%esi\n\t"       "pand   %%xmm3, %%xmm0\n\t"
    "orl    %%edx, %%esi\n\t"       "por    %%xmm1, %%xmm0\n\t"
    "leal   (,%%esi,2), %%esi\n\t"  "psllq  $8, %%xmm0\n\t"
    "orl    %%eax, %%esi\n\t"       "por    %%xmm0, %%xmm4\n\t"
    /* Serialize */                         /* shift=+7 */
    "movq   %%xmm7, %%xmm0\n\t"
    "movd   %%esi, %%xmm1\n\t"
    "psrlq  $7, %%xmm0\n\t"
    "psllq  $32, %%xmm1\n\t"
    "pand   %%xmm5, %%xmm0\n\t"
    "por    %%xmm1, %%xmm4\n\t"
    "movq   %%xmm0, %%xmm1\n\t"
    "psrlq  $7, %%xmm0\n\t"
    "pand   %%xmm5, %%xmm0\n\t"
    "movq   %%xmm5, %%xmm3\n\t"
    "por    %%xmm1, %%xmm0\n\t"
    "psrlq  $7, %%xmm3\n\t"
    "movq   %%xmm0, %%xmm1\n\t"
    "pand   %%xmm5, %%xmm3\n\t"
    "psrlq  $14, %%xmm0\n\t"
    "pand   %%xmm3, %%xmm0\n\t"
    "movl   %1, %%esi\n\t"          "por    %%xmm0, %%xmm1\n\t"
    "movl   %2, %%edi\n\t"          "psrlq  $14, %%xmm0\n\t"
    "andl   $2122219134, %%edi\n\t" "pand   %%xmm3, %%xmm0\n\t"
    "movl   %%edi, %%ecx\n\t"       "por    %%xmm1, %%xmm0\n\t"
    "shrl   $1, %%ecx\n\t"          "psrlq  $7, %%xmm0\n\t"
    "andl   %%edi, %%ecx\n\t"       "por    %%xmm0, %%xmm4\n\t"
    /* shift=+1 */                  /* shift=-7 */
    "movl   %%esi, %%eax\n\t"       "movq   %%xmm7, %%xmm0\n\t"
    "shrl   $1, %%eax\n\t"          "psllq  $7, %%xmm0\n\t"
    "andl   %%edi, %%eax\n\t"       "pand   %%xmm5, %%xmm0\n\t"
    "movl   %%eax, %%edx\n\t"       "movq   %%xmm0, %%xmm1\n\t"
    "shrl   $1, %%eax\n\t"          "psllq  $7, %%xmm0\n\t"
    "andl   %%edi, %%eax\n\t"       "pand   %%xmm5, %%xmm0\n\t"
    "orl    %%edx, %%eax\n\t"       "por    %%xmm1, %%xmm0\n\t"
                                    "psllq  $7, %%xmm3\n\t"
    "movl   %%eax, %%edx\n\t"       "movq   %%xmm0, %%xmm1\n\t"
    "shrl   $2, %%eax\n\t"          "psllq  $14, %%xmm0\n\t"
    "andl   %%ecx, %%eax\n\t"       "pand   %%xmm3, %%xmm0\n\t"
    "orl    %%eax, %%edx\n\t"       "por    %%xmm0, %%xmm1\n\t"
    "shrl   $2, %%eax\n\t"          "psllq  $14, %%xmm0\n\t"
    "andl   %%ecx, %%eax\n\t"       "pand   %%xmm3, %%xmm0\n\t"
    "orl    %%edx, %%eax\n\t"       "por    %%xmm1, %%xmm0\n\t"
    "shrl   $1, %%eax\n\t"          "psllq  $7, %%xmm0\n\t"
    "por    %%xmm0, %%xmm4\n\t"
    /* shift=-1 */                  /* shift=+9 */
    "movq   %%xmm7, %%xmm0\n\t"
    "leal   (,%%esi,2), %%esi\n\t"  "psrlq  $9, %%xmm0\n\t"
    "andl   %%edi, %%esi\n\t"       "pand   %%xmm5, %%xmm0\n\t"
    "movl   %%esi, %%edx\n\t"       "movq   %%xmm0, %%xmm1\n\t"
    "leal   (,%%esi,2), %%esi\n\t"  "psrlq  $9, %%xmm0\n\t"
    "andl   %%edi, %%esi\n\t"       "pand   %%xmm5, %%xmm0\n\t"
    "movq   %%xmm5, %%xmm3\n\t"
    "orl    %%esi, %%edx\n\t"       "por    %%xmm1, %%xmm0\n\t"
    "psrlq  $9, %%xmm3\n\t"
    "movq   %%xmm0, %%xmm1\n\t"
    "addl   %%ecx, %%ecx\n\t"       "pand   %%xmm5, %%xmm3\n\t"
    "leal   (,%%edx,4), %%esi\n\t"  "psrlq  $18, %%xmm0\n\t"
    "andl   %%ecx, %%esi\n\t"       "pand   %%xmm3, %%xmm0\n\t"
    "orl    %%esi, %%edx\n\t"       "por    %%xmm0, %%xmm1\n\t"
    "leal   (,%%esi,4), %%esi\n\t"  "psrlq  $18, %%xmm0\n\t"
    "andl   %%ecx, %%esi\n\t"       "pand   %%xmm3, %%xmm0\n\t"
    "orl    %%edx, %%esi\n\t"       "por    %%xmm1, %%xmm0\n\t"
    "leal   (,%%esi,2), %%esi\n\t"  "psrlq  $9, %%xmm0\n\t"
    "orl    %%eax, %%esi\n\t"       "por    %%xmm0, %%xmm4\n\t"
    /* Serialize */                 /* shift=-9 */
    "movq   %%xmm7, %%xmm0\n\t"
    "movd   %%esi, %%xmm1\n\t"
    "psllq  $9, %%xmm0\n\t"
    "por    %%xmm1, %%xmm4\n\t"
    "pand   %%xmm5, %%xmm0\n\t"
    "movq   %%xmm0, %%xmm1\n\t"
    "psllq  $9, %%xmm0\n\t"
    "pand   %%xmm5, %%xmm0\n\t"
    "por    %%xmm1, %%xmm0\n\t"
    "psllq  $9, %%xmm3\n\t"
    "movq   %%xmm0, %%xmm1\n\t"
    "psllq  $18, %%xmm0\n\t"
    "pand   %%xmm3, %%xmm0\n\t"
    "por    %%xmm0, %%xmm1\n\t"
    "psllq  $18, %%xmm0\n\t"
    "pand   %%xmm3, %%xmm0\n\t"
    "por    %%xmm1, %%xmm0\n\t"
    "psllq  $9, %%xmm0\n\t"
    "por    %%xmm0, %%xmm4\n\t"

    "por    %%xmm6, %%xmm7\n\t"
    "pandn  %%xmm4, %%xmm7\n\t"
    "movq   %%xmm7, %0\n\t"
    : "=g" (result) : "m" (self), "m" (opp), "g" (self >> 32), "g" (opp >> 32), "m" (mask_7e) : "eax", "edx", "ecx", "esi", "edi" );

    return result;

#elif KOGGE_STONE
    bitbrd other = opp & 0x00FFFFFFFFFFFF00;
    // north and south
    bitbrd templ = other & (self << 8);
    bitbrd tempr = other & (self >> 8);
    templ |= other & (templ << 8);
    tempr |= other & (tempr >> 8);
    bitbrd maskl = other & (other << 8);
    bitbrd maskr = other & (other >> 8);
    templ |= maskl & (templ << 16);
    tempr |= maskr & (tempr >> 16);
    templ |= maskl & (templ << 16);
    tempr |= maskr & (tempr >> 16);
    result |= (templ << 8) | (tempr >> 8);

    other = opp & 0x7E7E7E7E7E7E7E7E;
    // east and west
    templ = other & (self << 1);
    tempr = other & (self >> 1);
    templ |= other & (templ << 1);
    tempr |= other & (tempr >> 1);
    maskl = other & (other << 1);
    maskr = other & (other >> 1);
    templ |= maskl & (templ << 2);
    tempr |= maskr & (tempr >> 2);
    templ |= maskl & (templ << 2);
    tempr |= maskr & (tempr >> 2);
    result |= (templ << 1) | (tempr >> 1);

    // ne and sw
    templ = other & (self << 7);
    tempr = other & (self >> 7);
    templ |= other & (templ << 7);
    tempr |= other & (tempr >> 7);
    maskl = other & (other << 7);
    maskr = other & (other >> 7);
    templ |= maskl & (templ << 14);
    tempr |= maskr & (tempr >> 14);
    templ |= maskl & (templ << 14);
    tempr |= maskr & (tempr >> 14);
    result |= (templ << 7) | (tempr >> 7);

    // nw and se
    templ = other & (self << 9);
    tempr = other & (self >> 9);
    templ |= other & (templ << 9);
    tempr |= other & (tempr >> 9);
    maskl = other & (other << 9);
    maskr = other & (other >> 9);
    templ |= maskl & (templ << 18);
    tempr |= maskr & (tempr >> 18);
    templ |= maskl & (templ << 18);
    tempr |= maskr & (tempr >> 18);
    result |= (templ << 9) | (tempr >> 9);

    return result & ~taken;

#else
    bitbrd other = opp & 0x00FFFFFFFFFFFF00;
    // north and south
    bitbrd tempM = (((self << 8) | (self >> 8)) & other);
    tempM |= (((tempM << 8) | (tempM >> 8)) & other);
    tempM |= (((tempM << 8) | (tempM >> 8)) & other);
    tempM |= (((tempM << 8) | (tempM >> 8)) & other);
    tempM |= (((tempM << 8) | (tempM >> 8)) & other);
    tempM |= (((tempM << 8) | (tempM >> 8)) & other);
    result = ((tempM << 8) | (tempM >> 8));

    other = opp & 0x7E7E7E7E7E7E7E7E;
    // east and west
    tempM = (((self << 1) | (self >> 1)) & other);
    tempM |= (((tempM << 1) | (tempM >> 1)) & other);
    tempM |= (((tempM << 1) | (tempM >> 1)) & other);
    tempM |= (((tempM << 1) | (tempM >> 1)) & other);
    tempM |= (((tempM << 1) | (tempM >> 1)) & other);
    tempM |= (((tempM << 1) | (tempM >> 1)) & other);
    result |= ((tempM << 1) | (tempM >> 1));

    // ne and sw
    tempM = (((self << 7) | (self >> 7)) & other);
    tempM |= (((tempM << 7) | (tempM >> 7)) & other);
    tempM |= (((tempM << 7) | (tempM >> 7)) & other);
    tempM |= (((tempM << 7) | (tempM >> 7)) & other);
    tempM |= (((tempM << 7) | (tempM >> 7)) & other);
    tempM |= (((tempM << 7) | (tempM >> 7)) & other);
    result |= ((tempM << 7) | (tempM >> 7));

    // nw and se
    tempM = (((self << 9) | (self >> 9)) & other);
    tempM |= (((tempM << 9) | (tempM >> 9)) & other);
    tempM |= (((tempM << 9) | (tempM >> 9)) & other);
    tempM |= (((tempM << 9) | (tempM >> 9)) & other);
    tempM |= (((tempM << 9) | (tempM >> 9)) & other);
    tempM |= (((tempM << 9) | (tempM >> 9)) & other);
    result |= ((tempM << 9) | (tempM >> 9));

    return result & ~taken;
#endif
}

int Board::numLegalMoves(int side) {
    return countSetBits(getLegal(side));
}

int Board::potentialMobility(int side) {
    bitbrd other = (side == CBLACK) ? (taken ^ black) : (black);
    bitbrd empty = ~taken;

    bitbrd result = (other >> 8) | (other << 8);
    result &= empty;

    empty &= 0x7E7E7E7E7E7E7E7E;
    result |= (other << 1) | (other >> 1);
    result |= (other >> 7) | (other << 7);
    result |= (other >> 9) | (other << 9);
    result &= empty;

    return countSetBits(result);
}

bitbrd Board::toBits(int side) {
    return (side == CBLACK) ? black : (taken ^ black);
}

/*
 * Sets the board state given an 8x8 char array where 'w', 'O' indicates a white
 * piece and 'b', 'X' indicates a black piece. Mainly for testing purposes.
 */
void Board::setBoard(char data[]) {
    taken = 0;
    black = 0;
    for (int i = 0; i < 64; i++) {
        if (data[i] == 'b' || data[i] == 'B' || data[i] == 'X') {
            taken |= MOVEMASK[i];
            black |= MOVEMASK[i];
        } if (data[i] == 'w' || data[i] == 'W' || data[i] == 'O') {
            taken |= MOVEMASK[i];
        }
    }
}

char *Board::toString() {
    char *result = new char[64];
    for (int i = 0; i < 64; i++) {
        if (taken & MOVEMASK[i]) {
            if (black & MOVEMASK[i])
                result[i] = 'b';
            else
                result[i] = 'w';
        }
        else
            result[i] = '-';
    }
    return result;
}

bitbrd Board::getTaken() {
    return taken;
}

int Board::countEmpty() {
    return countSetBits(~taken);
}

int Board::bitScanForward(bitbrd bb) {
    #if defined(__x86_64__)
        asm ("bsf %1, %0" : "=r" (bb) : "r" (bb));
        return (int) bb;
    #else
        return index64[(int)(((bb ^ (bb-1)) * 0x03f79d71b4cb0a89) >> 58)];
    #endif
}

int Board::bitScanReverse(bitbrd bb) {
    #if defined(__x86_64__)
        asm ("bsr %1, %0" : "=r" (bb) : "r" (bb));
        return (int) bb;
    #elif defined(__i386)
        int b = (int) (bb >> 32);
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
        bb |= bb >> 1;
        bb |= bb >> 2;
        bb |= bb >> 4;
        bb |= bb >> 8;
        bb |= bb >> 16;
        bb |= bb >> 32;
        return index64[(int)((bb * 0x03f79d71b4cb0a89) >> 58)];
    #endif
}

/*
 * Current count of given side's stones.
 */
int Board::count(int side) {
    bitbrd i = (side == CBLACK) ? (black) : (black^taken);
    return countSetBits(i);
}

int Board::countSetBits(bitbrd i) {
    #if defined(__x86_64__)
        asm ("popcnt %1, %0" : "=r" (i) : "r" (i));
        return (int) i;
    #elif defined(__i386)
        int a = (int) (i & 0xFFFFFFFF);
        int b = (int) (i >> 32);
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
        // use multiplication to reduce branching
        return ((self & MOVEMASK[anchor]) && true) *
                (result ^ NORTHRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::southFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = SOUTHRAY[index];
    bitbrd block = result & pos;
    block &= -block & self;
    if((block >> 8) & ~pos) {
        int anchor = bitScanForward(block);
        return (result ^ SOUTHRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::eastFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = EASTRAY[index];
    bitbrd block = result & pos;
    block &= -block & self;
    if((block >> 1) & ~pos) {
        int anchor = bitScanForward(block);
        return (result ^ EASTRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::westFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = WESTRAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanReverse(block);
        return ((self & MOVEMASK[anchor]) && true) *
                (result ^ WESTRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::neFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = NERAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanReverse(block);
        return ((self & MOVEMASK[anchor]) && true) *
                (result ^ NERAYI[anchor]);
    }
    return 0;
}

bitbrd Board::nwFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = NWRAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanReverse(block);
        return ((self & MOVEMASK[anchor]) && true) *
                (result ^ NWRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::swFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = SWRAY[index];
    bitbrd block = result & pos;
    block &= -block & self;
    if((block >> 7) & ~pos) {
        int anchor = bitScanForward(block);
        return (result ^ SWRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::seFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = SERAY[index];
    bitbrd block = result & pos;
    block &= -block & self;
    if((block >> 9) & ~pos) {
        int anchor = bitScanForward(block);
        return (result ^ SERAYI[anchor]);
    }
    return 0;
}
