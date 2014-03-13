#ifndef __BOARD_H__
#define __BOARD_H__

#include <bitset>
#include <vector>
#include "common.h"
using namespace std;

class Board {
   
private:
    bitbrd taken;
    bitbrd black;

    bool occupied(int x, int y);
    bool get(Side side, int x, int y);
    void set(Side side, int x, int y);
    bool onBoard(int x, int y);
    bool checkMove(int X, int Y, Side side);

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
    int numLegalMoves(Side side);
    bitbrd toBits(Side side);

    void setBoard(char data[]);
};

#endif
