#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include "common.h"
#include "player.h"
#include "board.h"
#include "endgame.h"
#include "eval.h"

bitbrd cstack[20];
int movestack[20];
int top;

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
        cstack[top] = b.getDoMove(lm.get(i), side);
        movestack[top] = lm.get(i);
        b.makeMove(movestack[top], cstack[top], side);
        top++;

        nodes += perftu(b, depth-1, side^1, false);

        top--;
        b.undoMove(movestack[top], cstack[top], side);
    }

    return nodes;
}

int main(int argc, char **argv) {
    top = 0;
    unsigned long long total_nodes = 0;

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    //Board b;
    //cerr << perft(b, 11, CBLACK, false) << endl;

    //ffo("ffoeasy/end40.pos");
    //ffo("ffoeasy/end41.pos");
    //ffo("ffoeasy/end42.pos");

    total_nodes += ffo("ffotest/end40.pos");       // 18.35, 211713808
    total_nodes += ffo("ffotest/end41.pos");
    total_nodes += ffo("ffotest/end42.pos");
    total_nodes += ffo("ffotest/end43.pos");
    total_nodes += ffo("ffotest/end44.pos");

    //total_nodes += ffo("ffotest/end45.pos");        // 71.07, 972971804
    //total_nodes += ffo("ffotest/end46.pos");        // 15.26, 135054341
    //total_nodes += ffo("ffotest/end47.pos");        // 11.56, 35319655
    //total_nodes += ffo("ffotest/end48.pos");        // 70.25, 606255943
    //total_nodes += ffo("ffotest/end59.pos");        // 29.10, 41573

    /*Player p(BLACK);
    Player p2(WHITE);

    Move *m = p.doMove(NULL, -1);
    for(int i = 0; i < 15; i++) {
        m = p2.doMove(m, -1);
        m = p.doMove(m, -1);
    }*/

    //117802417229721632
    //18328941656479632604

    auto end_time = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(
        end_time-start_time);

    cerr << time_span.count() << endl;
    cerr << total_nodes << endl;
}

unsigned long long ffo(std::string file) {
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
    MoveList lm = b.getLegalMoves(side);
    int empties = b.countEmpty();
    cerr << empties << " empty" << endl;

    Endgame e;
    int result = e.solveEndgame(b, lm, side, empties, 100000000, &evaluater);
    cerr << "Best move: " << result << endl;
    return e.nodes;
}

