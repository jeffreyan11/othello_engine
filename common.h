#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdint>
#include <cstdlib>

typedef uint64_t bitbrd;

const int NEG_INFTY = -(1 << 20);
const int INFTY = (1 << 20);
const int MOVE_NULL = 64;
const int MOVE_BROKEN = -1;
const int OPENING_NOT_FOUND = -3;
const int CBLACK = 1;
const int CWHITE = -1;

enum Side { 
    WHITE, BLACK
};

class Move {
   
public:
    int x, y;
    Move(int x, int y) {
        this->x = x;
        this->y = y;     
    }
    ~Move() {}

    int getX() { return x; }
    int getY() { return y; }

    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }
};

class MoveList {
public:
    int *moves;
    unsigned int size;

    MoveList() {
        moves = new int[32];
        size = 0;
    }
    ~MoveList() { delete[] moves; }

    void add(int m) {
        moves[size] = m;
        size++;
    }
    int get(int i) { return moves[i]; }
    int last() { return moves[size-1]; }
    void set(int i, int val) { moves[i] = val; }
    void clear() {
        size = 0;
    }
};

#endif
