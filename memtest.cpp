
#include <chrono>
#include "common.h"
#include "player.h"
#include "board.h"
#include "endgame.h"

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

// g++ -std=c++0x -O3 -o memtest memtest.cpp player.cpp board.cpp openings.cpp endgame.cpp hash.cpp
int main(int argc, char **argv) {
    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    //Board b;
    //cerr << perft(b, 11, CBLACK, false) << endl;;

    Player p(BLACK);
    Player p2(WHITE);

    Move *m = p.doMove(NULL, -1);
    for(int i = 0; i < 18; i++) {
        m = p2.doMove(m, -1);
        m = p.doMove(m, -1);
    }

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
