#include "openings.h"
#include <fstream>
#include <iostream>
#include <string>

Openings::Openings() {
  if (!read_file())
    std::cerr << "Error: opening book not found." << std::endl;
}

Openings::~Openings() {
  delete[] openings;
}

int Openings::get(uint64_t taken, uint64_t black) {
  int index = binary_search(taken, black);
  if (index == -1) {
    return OPENING_NOT_FOUND;
  }
  else return openings[index].move;
}

int Openings::binary_search(uint64_t taken, uint64_t black) {
  int min = 0;
  int max = bookSize - 1;

  while (max >= min) {
    int mid = min + (max - min) / 2;
    uint64_t ttaken = openings[mid].taken;
    uint64_t tblack = openings[mid].black;
    if ((ttaken == taken) && (tblack == black))
      return mid;
    else if ((ttaken < taken) || ((ttaken == taken) && (tblack < black))) {
      min = mid + 1;
    }
    else {
      max = mid - 1;
    }
  }

  return -1;
}

bool Openings::read_file() {
  std::string line;
  std::ifstream openingbk("Flippy_Resources/flippy_book.txt");

  if (openingbk.is_open()) {
    getline(openingbk, line);
    bookSize = std::stoi(line);
    openings = new Node[bookSize];
    #if PRINT_SEARCH_INFO
    std::cerr << "Opening book read. Contains " << bookSize
              << " positions." << std::endl;
    #endif
    int i = 0;
    while (getline(openingbk, line)) {
      std::string::size_type sz = 0;
      Node temp;
      temp.taken = std::stoull(line, &sz, 0);
      line = line.substr(sz);
      temp.black = std::stoull(line, &sz, 0);
      line = line.substr(sz);
      int move = std::stoi(line, &sz, 0);
      temp.move = move;
      openings[i] = temp;
      i++;
    }
    openingbk.close();
    return true;
  }
  return false;
}
