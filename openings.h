#ifndef __OPENINGS_H__
#define __OPENINGS_H__

#include <fstream>
#include <string>
#include "common.h"

#define OPENING_BOOK_SIZE 5071

struct Node {
    bitbrd taken, black;
    int move;
};

class Openings {

private:
    Node *openings[OPENING_BOOK_SIZE];
    int binarySearch(bitbrd pos, bitbrd black);

    bool readFile();

public:
    Openings();
    ~Openings();

    int get(bitbrd pos, bitbrd black);
};

#endif
