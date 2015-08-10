#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include "common.h"
#include "player.h"
#include "board.h"
#include "endgame.h"
#include "eval.h"

using namespace std;

Eval evaluater;

unsigned long long ffo(std::string file);

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

        nodes += perft(b, depth-1, side^1, true);
        return nodes;
    }

    for(unsigned int i = 0; i < lm.size; i++) {
        Board copy = b.copy();
        copy.doMove(lm.get(i), side);
        nodes += perft(copy, depth-1, side^1, false);
    }

    return nodes;
}
unsigned long long perftu(Board &b, int depth, int side, bool passed) {
    if(depth == 0)
        return 1;

    unsigned long long nodes = 0;
    MoveList lm = b.getLegalMoves(side);

    if(lm.size == 0) {
        if(passed)
            return 1;

        nodes += perftu(b, depth-1, side^1, true);
        return nodes;
    }

    for(unsigned int i = 0; i < lm.size; i++) {
        bitbrd changeMask = b.getDoMove(lm.get(i), side);
        b.makeMove(lm.get(i), changeMask, side);

        nodes += perftu(b, depth-1, side^1, false);

        b.undoMove(lm.get(i), changeMask, side);
    }

    return nodes;
}

int main(int argc, char **argv) {
    uint64_t total_nodes = 0;

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    //Board b;
    //cerr << perft(b, 11, CBLACK, false) << endl;
    //cerr << perftu(b, 11, CBLACK, false) << endl;

    // This is a very difficult depth 23 position that
    // appeared in a test game...
    //total_nodes += ffo("ffotest/end39.pos");

    total_nodes += ffo("ffotest/end40.pos");       // 16.61, 189520557
    total_nodes += ffo("ffotest/end41.pos");
    total_nodes += ffo("ffotest/end42.pos");
    total_nodes += ffo("ffotest/end43.pos");
    total_nodes += ffo("ffotest/end44.pos");

    total_nodes += ffo("ffotest/end45.pos");        // 88.97, 1219082258
    total_nodes += ffo("ffotest/end46.pos");        // 9.28, 75989062
    total_nodes += ffo("ffotest/end47.pos");        // 7.40, 43155904
    total_nodes += ffo("ffotest/end48.pos");        // 58.55, 769328924
    total_nodes += ffo("ffotest/end49.pos");

    total_nodes += ffo("ffotest/end50.pos");
    total_nodes += ffo("ffotest/end51.pos");
    total_nodes += ffo("ffotest/end52.pos");
    total_nodes += ffo("ffotest/end53.pos");
    total_nodes += ffo("ffotest/end54.pos");
    total_nodes += ffo("ffotest/end55.pos");
    total_nodes += ffo("ffotest/end56.pos");
    total_nodes += ffo("ffotest/end57.pos");
    total_nodes += ffo("ffotest/end58.pos");

    total_nodes += ffo("ffotest/end59.pos");        // 22.54, 23692

    /*Player p(BLACK);
    Player p2(WHITE);

    Move *m = p.doMove(NULL, -1);
    for(int i = 0; i < 15; i++) {
        m = p2.doMove(m, -1);
        m = p.doMove(m, -1);
    }*/

    auto end_time = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(
        end_time-start_time);

    cerr << time_span.count() << endl;
    cerr << total_nodes << endl;
}

unsigned long long ffo(std::string file) {
    std::string line;
    std::string ffostring;
    std::ifstream cfile(file);
    char board[64];

    if(cfile.is_open()) {
        getline(cfile, line);
        const char *read = line.c_str();
        for(int i = 0; i < 64; i++)
            board[i] = read[i];

        getline(cfile, line);
        getline(cfile, ffostring);
        cerr << ffostring << endl;
    }

    const char *read_color = line.c_str();
    int side = 0;
    if(read_color[0] == 'B') {
        cerr << "Solving for black: ";
        side = CBLACK;
    }
    else {
        cerr << "Solving for white: ";
        side = CWHITE;
    }

    Board b;
    b.setBoard(board);
    // TODO debug stability using this?
/*    b.doMove(62, side);
    b.doMove(49, side^1);
    b.doMove(22, side);
    b.doMove(31, side^1);
    b.doMove(15, side);
    b.doMove(14, side^1);
    b.doMove(5, side);
    b.doMove(6, side^1);
    b.doMove(7, side);
    b.doMove(13, side);
    b.doMove(33, side);
    b.doMove(25, side^1);
    b.doMove(57, side);
    b.doMove(32, side);
    b.doMove(48, side^1);
    b.doMove(56, side);
    b.doMove(24, side^1);
    b.doMove(16, side);
    b.doMove(17, side^1);
    b.doMove(21, side);
    b.doMove(20, side^1);
    b.doMove(9, side);
    b.doMove(8, side^1);
    b.doMove(11, side);
    b.doMove(1, side^1);
    b.doMove(60, side);
    b.doMove(19, side);
    b.doMove(18, side^1);
    b.doMove(0, side);
    b.doMove(2, side);*/
    /*char *pos = b.toString();
    for (int i = 0; i < 64; i++) {
        cerr << pos[i];
        if (i % 8 == 7)
            cerr << endl;
    }
    cerr << endl;*/
    //side ^= 1;

    MoveList lm = b.getLegalMoves(side);
    int empties = b.countEmpty();
    cerr << empties << " empty" << endl;

    Endgame e;
    e.solveEndgame(b, lm, false, side, empties, 100000000, &evaluater);
    cerr << endl;
    return e.nodes;
}

