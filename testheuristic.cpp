#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "player.h"
#include "board.h"
#include "endgame.h"
#include "common.h"

const int PERFT6 = 8200;

vector<string*> positions;
int wins;
int losses;
int draws;

bool readFile();
void freemem();

void play(string position) {
    std::string::size_type sz = 0;
    uint64_t taken = std::stoull(position, &sz, 0);
    position = position.substr(sz);
    uint64_t black = std::stoull(position, &sz, 0);
    Board b(taken^black, black);

    // Run game on one side
    Player black(BLACK);
    Player white(WHITE);
    black.setDepths(4, 6, 10, 18);
    white.setDepths(4, 6, 10, 18);
    black.otherHeuristic = true;

    Move *m = NULL;
    bool passed = false;
    while(true) {
        m = black.doMove(m, -1);
        // two passes in a row is end of game
        if (m == NULL) {
            if (passed)
                break;
            passed = true;
        }
        m = white.doMove(m, -1);
        if (m == NULL) {
            if (passed)
                break;
            passed = true;
        }
    }

    int bf = b.count(CBLACK);
    int wf = b.count(CWHITE);
    if(bf > wf)
        wins++;
    else if (wf > bf)
        losses++;
    else
        draws++;
    cout << wins << "-" << losses << "-" << draws << endl;

    // Run game on the other side
    Player black(BLACK);
    Player white(WHITE);
    black.setDepths(4, 6, 10, 18);
    white.setDepths(4, 6, 10, 18);
    white.otherHeuristic = true;

    Move *m = NULL;
    bool passed = false;
    while(true) {
        m = black.doMove(m, -1);
        // two passes in a row is end of game
        if (m == NULL) {
            if (passed)
                break;
            passed = true;
        }
        m = white.doMove(m, -1);
        if (m == NULL) {
            if (passed)
                break;
            passed = true;
        }
    }

    int bf = b.count(CBLACK);
    int wf = b.count(CWHITE);
    if(bf > wf)
        losses++;
    else if (wf > bf)
        wins++;
    else
        draws++;
    cout << wins << "-" << losses << "-" << draws << endl;
}

int main(int argc, char **argv) {
    readFile();
    cout << "Files read" << endl;

    wins = 0;
    losses = 0;
    draws = 0;
    cout << "Score from other heuristic POV" << endl;

    for(int i = 0; i < PERFT6; i++) {
        string *pos = positions[0];
        play(*pos);
    }

    cout << "Final result: " << wins << "-" << losses << "-" << draws << endl;

    freemem();

    return 0;
}

bool readFile() {
    std::string line;
    std::ifstream cfile("perft6.txt");

    if(cfile.is_open()) {
        while(getline(cfile, line)) {
            positions.push_back(new string(line));        
        }
        cfile.close();
    }
    else return false;

    return true;
}

void freemem() {
    while(positions.size() > 0) {
        string *temp = positions[0];
        positions.erase(positions.begin());
        delete temp;
    }
}
