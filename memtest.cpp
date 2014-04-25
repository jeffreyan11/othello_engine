#include <chrono>
#include <fstream>
#include "common.h"
#include "player.h"
#include "board.h"
#include "endgame.h"

/*
 DEPTH  #LEAF NODES   #FULL-DEPTH  #HIGHER
==========================================
   1            4
   2           12
   3           56
   4          244
   5         1396
   6         8200
   7        55092
   8       390216
   9      3005288
  10     24571284
  11    212258800  =    212258572  +    228
  12   1939886636  =   1939886052  +    584
  13  18429641748  =  18429634780  +   6968 
  14 184042084512  = 184042061172  +  23340
*/
unsigned long long perft(Board &b, int depth, int side, bool passed) {
    if(depth == 0)
        return 1;

    unsigned long long nodes = 0;
    MoveList lm = b.getLegalMoves(side);

    if(lm.size == 0) {
        if(passed)
            return 1;
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(MOVE_NULL, side);
        nodes += perft(copy, depth-1, -side, true);
    }

    for(unsigned int i = 0; i < lm.size; i++) {
        Board copy = Board(b.taken, b.black, b.legal);
        copy.doMove(lm.get(i), side);
        nodes += perft(copy, depth-1, -side, false);
    }

    return nodes;
}

void ffo(std::string file) {
    std::string line;
    std::ifstream cfile(file);
    char board[64];

    if(cfile.is_open()) {
        getline(cfile, line);
        const char *read = line.c_str();
        for(int i = 0; i < 64; i++)
            board[i] = read[i];

        getline(cfile, line);
    }

    Board b;
    b.setBoard(board);
    MoveList lm = b.getLegalMoves(CBLACK);

    Endgame e;
    e.endgameTimeMS = 100000000;
    e.mySide = CBLACK;
    e.endgame(b, lm, 20);
}

// g++ -std=c++0x -O3 -o memtest memtest.cpp player.cpp board.cpp openings.cpp endgame.cpp hash.cpp
int main(int argc, char **argv) {
    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    Board b;
    cerr << perft(b, 11, CBLACK, false) << endl;

    //ffo("ffotest/end40.pos");

    /*Player p(BLACK);
    Player p2(WHITE);

    Move *m = p.doMove(NULL, -1);
    for(int i = 0; i < 18; i++) {
        m = p2.doMove(m, -1);
        m = p.doMove(m, -1);
    }*/

    auto end_time = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(
        end_time-start_time);

    cerr << time_span.count() << endl;
}

    /*Board b;
    char boardData[64] = {
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
        ' ', ' ', 'b', 'b', 'b', ' ', ' ', ' ', 
        ' ', ' ', ' ', 'b', 'w', 'b', 'w', ' ', 
        ' ', ' ', ' ', ' ', ' ', 'w', 'w', 'w', 
        ' ', ' ', ' ', ' ', ' ', ' ', 'w', 'w', 
        ' ', ' ', ' ', ' ', ' ', ' ', 'w', 'w'
    };
    b.setBoard(boardData);
    Move m (4,5);
    for(int i = 0; i < 200000000; i++) {
        b.checkMove(&m, BLACK);
    }
    cout << "done" << endl;*/
