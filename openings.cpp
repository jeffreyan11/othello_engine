#include "openings.h"

OpeningBook::OpeningBook() {
    readFile();
}

OpeningBook::~OpeningBook() {
    for(int i = 0; i < OPENING_BOOK_SIZE; i++) {
        delete openings[i];
    }
}

Move *OpeningBook::get(bitbrd pos) {
    return openings[hash(pos)];
}

unsigned int OpeningBook::hash(bitbrd pos) {
    
}

bool OpeningBook::readFile() {
    string line;
    ifstream openingbk("openings.txt");

    if(openingbk.is_open()) {
        while(getline(openingbk, line)) {
            //TODO parse
            
        }
        openingbk.close();
    }
    else return false;

    return true;
}
