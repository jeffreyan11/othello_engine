#include <fstream>
#include <iostream>
#include "board.h"
#include "common.h"
#include "patternbuilder.h"

using namespace std;

void checkGames(unsigned int totalSize, thor_game **games) {
    cout << "Checking game validity: " << totalSize << " games." << endl;
    int errors = 0;
    for(unsigned int i = 0; i < totalSize; i++) {
        thor_game *game = games[i];

        Board tracker;
        int side = CBLACK;

        for(int j = 0; j < 55; j++) {
            if(!tracker.checkMove(game->moves[j], side)) {
                // If one side must pass it is not indicated in the database?
                side = side^1;
                if(!tracker.checkMove(game->moves[j], side)) {
                    errors++;
                    games[i] = NULL;
                    break;
                }
            }
            tracker.doMove(game->moves[j], side);
            side = side^1;
        }
    }
    cout << errors << " errors." << endl;
}

void readThorGame(string file, unsigned int &totalSize, thor_game **games) {
    unsigned int prevSize = totalSize;

    // Thor games are stored as a binary stream
    std::ifstream is (file, std::ifstream::binary);
    if(is) {
        // 16 byte header for each file, then each game record is 68 bytes
        char *header = new char[16];
        char *buffer = new char[68];

        is.read(header, 16);

        //cout << "Century: " << (unsigned)(header[0]) << endl;
        //cout << "Year: " << (unsigned)(header[1]) << endl;

        // We are only interested in the number of games, which is stored as a
        // longint (4 bytes), starting at header[7] or header[1] as an int array
        totalSize = ((unsigned int *)(header))[1];

        cout << "Reading " << totalSize << " games." << endl;

        totalSize += prevSize;

        for(unsigned int i = prevSize; i < totalSize; i++) {
            is.read(buffer, 68);
            thor_game *temp = new thor_game();
            temp->final = (unsigned char) buffer[6];
            for(int j = 0; j < 60; j++) {
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
