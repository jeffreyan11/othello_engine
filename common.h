#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdint>
#include <cstdlib>

#define bitbrd uint64_t
#define NEG_INFTY -65536
#define INFTY 65536
#define MOVE_NULL 64
#define MOVE_BROKEN -1
#define OPENING_NOT_FOUND -3
#define CBLACK 1
#define CWHITE -1

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
