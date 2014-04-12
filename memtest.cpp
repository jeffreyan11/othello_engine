
#include <chrono>
#include "common.h"
#include "player.h"
#include "board.h"
#include "endgame.h"

// g++ -std=c++0x -O3 -o memtest memtest.cpp player.cpp board.cpp openings.cpp endgame.cpp hash.cpp
int main(int argc, char **argv) {
    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();
    Player p(BLACK);
    Player p2(WHITE);

    Move *m = p.doMove(NULL, -1);
    for(int i = 0; i < 18; i++) {
        m = p2.doMove(m, -1);
        m = p.doMove(m, -1);
    }
    //vector<int> legalMoves = p.game.getLegalMoves(BLACK);
    //int r = endgame(p.game, legalMoves, BLACK, 16, NEG_INFTY,
    //        INFTY, 1000000, p.endgame_table);

    //Board b;
    //for(int i = 0; i < 1000000000; i++)

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
