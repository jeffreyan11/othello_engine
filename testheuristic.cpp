#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "player.h"
#include "board.h"
#include "endgame.h"
#include "common.h"

const int PERFT6 = 8200;
const int sortDepth = 2;
const int minDepth = 4;
const int maxDepth = 6;
const int endgameDepth = 16;

vector<string*> positions;
int wins;
int losses;
int draws;

bool readFile();
void freemem();

void play(string position) {
    std::string::size_type sz = 0;
    uint64_t takenBits = std::stoull(position, &sz, 0);
    position = position.substr(sz);
    uint64_t blackBits = std::stoull(position, &sz, 0);
    Board b(takenBits^blackBits, blackBits);

    // Run game on one side
    Player black(BLACK);
    Player white(WHITE);
    black.setDepths(sortDepth, minDepth, maxDepth, endgameDepth);
    white.setDepths(sortDepth, minDepth, maxDepth, endgameDepth);
    black.otherHeuristic = true;
    black.game = b;
    white.game = b;

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
        else passed = false;
        m = white.doMove(m, -1);
        if (m == NULL) {
            if (passed)
                break;
            passed = true;
        }
        else passed = false;
    }

    int bf = black.game.count(CBLACK);
    int wf = black.game.count(CWHITE);
    cout << bf << " " << wf << endl;
    if(bf > wf)
        wins++;
    else if (wf > bf)
        losses++;
    else
        draws++;

    // Run game on the other side
    Player black2(BLACK);
    Player white2(WHITE);
    black2.setDepths(sortDepth, minDepth, maxDepth, endgameDepth);
    white2.setDepths(sortDepth, minDepth, maxDepth, endgameDepth);
    white2.otherHeuristic = true;
    black2.game = b;
    white2.game = b;

    m = NULL;
    passed = false;
    while(true) {
        m = black2.doMove(m, -1);
        // two passes in a row is end of game
        if (m == NULL) {
            if (passed)
                break;
            passed = true;
        }
        else passed = false;
        m = white2.doMove(m, -1);
        if (m == NULL) {
            if (passed)
                break;
            passed = true;
        }
        else passed = false;
    }

    bf = black2.game.count(CBLACK);
    wf = black2.game.count(CWHITE);
    cout << bf << " " << wf << endl;
    if(bf > wf)
        losses++;
    else if (wf > bf)
        wins++;
    else
        draws++;
    cout << "" << wins << "-" << losses << "-" << draws << endl;
}

int main(int argc, char **argv) {
    readFile();
    cout << "Files read" << endl;

    wins = 0;
    losses = 0;
    draws = 0;
    cout << "Score from other heuristic POV" << endl;

    for(int i = 0; i < PERFT6; i++) {
        string *pos = positions[i];
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
