#ifndef __OPENINGS_H__
#define __OPENINGS_H__

#include "common.h"

struct Node {
    bitbrd taken, black;
    int move;
};

class Openings {

private:
    Node **openings;
    int bookSize;
    int binarySearch(bitbrd taken, bitbrd black);

    bool readFile();

public:
    Openings();
    ~Openings();

    int get(bitbrd taken, bitbrd black);
};

#endif
