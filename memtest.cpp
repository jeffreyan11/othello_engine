
#include <chrono>
#include "common.h"
#include "player.h"
#include "board.h"
#include "endgame.h"
#include "openings.h"

// g++ -std=c++0x -O3 -o memtest memtest.cpp player.cpp board.cpp openings.cpp endgame.cpp
int main(int argc, char **argv) {
    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();
    Player p(BLACK);

    Move m (3,5);
    p.doMove(&m, -1);
    Move m2 (2,6);
    p.doMove(&m2, -1);
    //vector<int> legalMoves = p.game.getLegalMoves(BLACK);
    //int r = endgame(p.game, legalMoves, BLACK, 16, NEG_INFTY,
    //        INFTY, 1000000, p.endgame_table);
    //cerr << r << endl;
    //Board b;
    //for(int i = 0; i < 1000000000; i++)
    //    b.count(WHITE);

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
