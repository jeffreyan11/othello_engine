#include "openings.h"
#include <fstream>
#include <iostream>
#include <string>

Openings::Openings() {
    readFile();
}

Openings::~Openings() {
    for(int i = 0; i < bookSize; i++) {
        delete openings[i];
    }
    delete openings;
}

int Openings::get(bitbrd taken, bitbrd black) {
    int index = binarySearch(taken, black);
    if(index == -1) {
        return OPENING_NOT_FOUND;
    }
    else return openings[index]->move;
}

int Openings::binarySearch(bitbrd taken, bitbrd black) {
    int min = 0;
    int max = bookSize - 1;

    while(max >= min) {
        int mid = min + (max - min) / 2;
        bitbrd ttaken = openings[mid]->taken;
        bitbrd tblack = openings[mid]->black;
        if((ttaken == taken) && (tblack == black))
            return mid;
        else if( (ttaken < taken) || ((ttaken == taken) && (tblack < black)) ) {
            min = mid + 1;
        }
        else {
            max = mid - 1;
        }
    }

    return -1;
}

bool Openings::readFile() {
    std::string line;
    std::ifstream openingbk("openings.txt");

    if(openingbk.is_open()) {
        getline(openingbk, line);
        bookSize = std::stoi(line);
        openings = new Node *[bookSize];
        std::cerr << "Opening book read. Contains " << bookSize
                  << " positions." << std::endl;
        int i = 0;
        while(getline(openingbk, line)) {
            std::string::size_type sz = 0;
            Node *temp = new Node();
            temp->taken = std::stoull(line, &sz, 0);
            line = line.substr(sz);
            temp->black = std::stoull(line, &sz, 0);
            line = line.substr(sz);
            int move = std::stoi(line, &sz, 0);
            temp->move = move;
            openings[i] = temp;
            i++;
        }
        openingbk.close();
    }
    else return false;

    return true;
}
