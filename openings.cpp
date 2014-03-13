#include "openings.h"

OpeningBook::OpeningBook() {
    readFile();
}

OpeningBook::~OpeningBook() {
    
}

Move *OpeningBook::get(bitbrd pos) {
    
}

unsigned int OpeningBook::hash(bitbrd pos) {
    
}

bool OpeningBook::readFile() {
    string line;
    ifstream openingbk("openings.txt");

    if(openingbk.is_open()) {
        while(getline(openingbk, line)) {
            //parse
        }
        openingbk.close();
    }
    else return false;

    return true;
}
