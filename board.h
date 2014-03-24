#ifndef __BOARD_H__
#define __BOARD_H__

#include <bitset>
#include <vector>
#include "common.h"
using namespace std;

class Board {
   
private:
    bool checkMove(int index, Side side);
    void getLegal(Side side);

    bool bitCheck(bitbrd move, bitbrd pos, bitbrd self);
      
public:
    bitbrd taken;
    bitbrd black;
    bitbrd legal;

    Board();
    Board(bitbrd t, bitbrd b, bitbrd l);
    ~Board();
    Board *copy();

    bool isDone();
    bool hasMoves(Side side);
    bool checkMove(Move *m, Side side);
    void doMove(int index, Side side);
    int count(Side side);
    int countBlack();
    int countWhite();
    int countHigh(Side side);
    vector<int> getLegalMoves(Side side);
    int numLegalMoves(Side side);
    int potentialMobility(Side side);
    bitbrd toBits(Side side);

    void setBoard(char data[]);
    bitbrd getTaken();
    bitbrd getBlack();

    inline bool operator==(const Board &other) const {
        return (taken == other.taken) && (black == other.black) &&
            (legal == other.legal);
    }

    // performing move
    bitbrd northFill(bitbrd move, bitbrd pos, bitbrd self);
    bitbrd southFill(bitbrd move, bitbrd pos, bitbrd self);
    bitbrd eastFill(bitbrd move, bitbrd pos, bitbrd self);
    bitbrd westFill(bitbrd move, bitbrd pos, bitbrd self);
    bitbrd neFill(bitbrd move, bitbrd pos, bitbrd self);
    bitbrd nwFill(bitbrd move, bitbrd pos, bitbrd self);
    bitbrd swFill(bitbrd move, bitbrd pos, bitbrd self);
    bitbrd seFill(bitbrd move, bitbrd pos, bitbrd self);
};

#endif
