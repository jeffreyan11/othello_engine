#ifndef __BOARD_H__
#define __BOARD_H__

#include <bitset>
#include <vector>
#include "common.h"
using namespace std;

bitbrd MOVEMASK[64] = {
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
bitbrd NORTHLINE = 0x00000000000000FF;
bitbrd SOUTHLINE = 0xFF00000000000000;
bitbrd EASTLINE = 0x8080808080808080;
bitbrd WESTLINE = 0x0101010101010101;
bitbrd NELINE = 0x80808080808080FF;
bitbrd NWLINE = 0x01010101010101FF;
bitbrd SWLINE = 0xFF01010101010101;
bitbrd SELINE = 0xFF80808080808080;

class Board {
   
private:
    bitbrd taken;
    bitbrd black;

    bool occupied(int x, int y);
    bool get(Side side, int x, int y);
    void set(Side side, int x, int y);
    bool onBoard(int x, int y);

    // ------------bitboard operators--------------------
    // checking to see if move is legal
    bool northCheck(bitbrd move, bitbrd pos, bitbrd self);
    bool southCheck(bitbrd move, bitbrd pos, bitbrd self);
    bool eastCheck(bitbrd move, bitbrd pos, bitbrd self);
    bool westCheck(bitbrd move, bitbrd pos, bitbrd self);
    bool neCheck(bitbrd move, bitbrd pos, bitbrd self);
    bool nwCheck(bitbrd move, bitbrd pos, bitbrd self);
    bool swCheck(bitbrd move, bitbrd pos, bitbrd self);
    bool seCheck(bitbrd move, bitbrd pos, bitbrd self);

    // performing move, assuming legal
    bitbrd northFill(bitbrd move, bitbrd pos);
    bitbrd southFill(bitbrd move, bitbrd pos);
    bitbrd eastFill(bitbrd move, bitbrd pos);
    bitbrd westFill(bitbrd move, bitbrd pos);
    bitbrd neFill(bitbrd move, bitbrd pos);
    bitbrd nwFill(bitbrd move, bitbrd pos);
    bitbrd swFill(bitbrd move, bitbrd pos);
    bitbrd seFill(bitbrd move, bitbrd pos);
      
public:
    Board();
    ~Board();
    Board *copy();
        
    bool isDone();
    bool hasMoves(Side side);
    bool checkMove(Move *m, Side side);
    void doMove(Move *m, Side side);
    int count(Side side);
    int countBlack();
    int countWhite();
    vector<Move *> getLegalMoves(Side side);
    bitbrd toBits(Side side);

    void setBoard(char data[]);
};

#endif
