#ifndef __COMMON_H__
#define __COMMON_H__

#define bitbrd uint64_t
#define NEG_INFTY -99999
#define INFTY 99999
#define MOVE_NULL 64
#define MOVE_BROKEN -1
#define OPENING_NOT_FOUND -3

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

#endif
