#ifndef __BOARD_H__
#define __BOARD_H__

#include <bitset>
#include <vector>
#include "common.h"
using namespace std;

class Board {
   
private:
    bool occupied(int x, int y);
    bool get(Side side, int x, int y);
    void set(Side side, int x, int y);
    bool onBoard(int x, int y);
    bool checkMove(int X, int Y, Side side);
    void getLegal(Side side);

    bool bitCheck(bitbrd move, bitbrd pos, bitbrd self);

    // performing move
    bitbrd northFill(bitbrd move, bitbrd pos);
    bitbrd southFill(bitbrd move, bitbrd pos);
    bitbrd eastFill(bitbrd move, bitbrd pos);
    bitbrd westFill(bitbrd move, bitbrd pos);
    bitbrd neFill(bitbrd move, bitbrd pos);
    bitbrd nwFill(bitbrd move, bitbrd pos);
    bitbrd swFill(bitbrd move, bitbrd pos);
    bitbrd seFill(bitbrd move, bitbrd pos);
      
public:
    bitbrd taken;
    bitbrd black;
    bitbrd legal;

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
    int potentialMobility(Side side);
    bitbrd toBits(Side side);

    void setBoard(char data[]);
    bitbrd getTaken();
    bitbrd getBlack();

    inline bool operator==(const Board &other) const {
        return (taken == other.taken) && (black == other.black) &&
            (legal == other.legal);
    }
};

#endif
