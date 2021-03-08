#include <fstream>
#include <iostream>
#include "board.h"
#include "common.h"
#include "patternbuilder.h"

void checkGames(unsigned int totalSize, thor_game **games) {
  std::cout << "Checking game validity: " << totalSize << " games." << std::endl;
  int errors = 0;
  for (unsigned int i = 0; i < totalSize; i++) {
    thor_game *game = games[i];

    Board tracker;
    Color side = BLACK;

    for (int j = 0; j < 55; j++) {
      if (!tracker.is_legal(side, game->moves[j])) {
        // If one side must pass it is not indicated in the database?
        side = ~side;
        if (!tracker.is_legal(side, game->moves[j])) {
          errors++;
          games[i] = NULL;
          break;
        }
      }
      tracker.do_move(side, game->moves[j]);
      side = ~side;
    }
  }
  std::cout << errors << " errors." << std::endl;
}

void readThorGame(std::string file, unsigned int &totalSize, thor_game **games) {
  unsigned int prevSize = totalSize;

  // Thor games are stored as a binary stream
  std::ifstream is(file, std::ifstream::binary);
  if (is) {
    // 16 byte header for each file, then each game record is 68 bytes
    char *header = new char[16];
    char *buffer = new char[68];

    is.read(header, 16);

    //cout << "Century: " << (unsigned)(header[0]) << endl;
    //cout << "Year: " << (unsigned)(header[1]) << endl;

    // We are only interested in the number of games, which is stored as a
    // longint (4 bytes), starting at header[7] or header[1] as an int array
    totalSize = ((unsigned int *)(header))[1];

    std::cout << "Reading " << totalSize << " games." << std::endl;

    totalSize += prevSize;

    for (unsigned int i = prevSize; i < totalSize; i++) {
      is.read(buffer, 68);
      thor_game *temp = new thor_game();
      temp->final = (unsigned char) buffer[6];
      for (int j = 0; j < 60; j++) {
        int movetoparse = (unsigned char) buffer[8+j];
        int x = movetoparse % 10;
        int y = movetoparse / 10;
        temp->moves[j] = (x-1) + 8*(y-1);
        if (temp->moves[j] == -9)
          temp->moves[j] = MOVE_NULL;
      }
      games[i] = temp;
    }

    delete[] header;
    delete[] buffer;
  }
}

void readFlippyGame(std::string file, unsigned int &totalSize, unsigned int n, thor_game **games) {
  std::cout << "Reading " << n << " games." << std::endl;

  std::ifstream db(file);
  std::string line;

  unsigned int prevSize = totalSize;
  totalSize += n;

  for (unsigned int i = prevSize; i < totalSize; i++) {
    std::string::size_type sz = 0;
    thor_game *temp = new thor_game();
    getline(db, line);

    temp->final = std::stoi(line, &sz, 0);
    line = line.substr(sz);

    for (int j = 0; j < 60; j++) {
      temp->moves[j] = std::stoi(line, &sz, 0);
      line = line.substr(sz);
    }

    games[i] = temp;
  }
}
