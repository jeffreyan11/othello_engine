#ifndef __OPENINGS_H__
#define __OPENINGS_H__

#include "common.h"

const int OPENING_BOOK_SIZE = 5071;

struct Node {
    bitbrd taken, black;
    int move;
};

class Openings {

private:
    Node *openings[OPENING_BOOK_SIZE];
    int binarySearch(bitbrd taken, bitbrd black);

    bool readFile();

public:
    Openings();
    ~Openings();

    int get(bitbrd taken, bitbrd black);
};

#endif
