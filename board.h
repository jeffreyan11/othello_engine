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

    int countSetBits(bitbrd i);
      
public:
    bitbrd taken;
    bitbrd black;
    bitbrd legal;

    Board();
    Board(bitbrd b, bitbrd t, bitbrd l);
    ~Board();
    Board *copy();
        
    bool isDone();
    bool hasMoves(Side side);
    bool checkMove(Move *m, Side side);
    void doMove(int index, Side side);
    int count(Side side);
    vector<int> getLegalMoves(Side side);
    int numLegalMoves(Side side);
    int potentialMobility(Side side);
    bitbrd toBits(Side side);

    void setBoard(char data[]);
    bitbrd getTaken();
    bitbrd getBlack();
    int bitScanForward(bitbrd bb);
    int bitScanReverse(bitbrd bb);

    inline bool operator==(const Board &other) const {
        return (taken == other.taken) && (black == other.black) &&
            (legal == other.legal);
    }
};

struct BoardHashFunc {
    size_t operator()(const Board &b) const {
    using std::size_t;
    using std::hash;
    using std::string;

    return ( (hash<bitbrd>()(b.taken) << 1)
        ^ hash<bitbrd>()(b.black)
        ^ (hash<bitbrd>()(b.legal) >> 1) );
    }
};

#endif
