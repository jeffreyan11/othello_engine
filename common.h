#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdint>
#include <cstdlib>

#define bitbrd uint64_t
#define NEG_INFTY -99999
#define INFTY 99999
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
        moves = new int[40];
        size = 0;
    }
    ~MoveList() { delete[] moves; }

    void add(int m) {
        moves[size] = m;
        size++;
    }
    int get(int i) { return moves[i]; }
    void set(int i, int val) { moves[i] = val; }
    void clear() {
        delete[] moves;
        moves = new int[40];
        size = 0;
    }
};

#endif
