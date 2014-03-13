#ifndef __OPENINGS_H__
#define __OPENINGS_H__

#include <ifstream>
#include <string>
#include "common.h"

#define OPENING_BOOK_SIZE 1

class OpeningBook {

private:
    (Move *)openings[OPENING_BOOK_SIZE];

    unsigned int hash(bitbrd pos);
    void readFile();

public:
    OpeningBook();
    ~OpeningBook();

    Move *get(bitbrd pos);
};

#endif
