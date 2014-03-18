#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <chrono>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include "common.h"
#include "board.h"
#include "openings.h"
using namespace std;

const bitbrd CORNERS = 0x8100000000000081;
const bitbrd EDGES = 0x3C0081818181003C;
const bitbrd ADJ_CORNERS = 0x4281000000008142;
const bitbrd X_CORNERS = 0x0042000000004200;

struct BoardHashFunc {
    size_t operator()(const Board &b) const {
        using std::size_t;
        using std::hash;
        using std::string;

        return ( (hash<bitbrd>()(b.taken) << 1)
                 ^ hash<bitbrd>()(b.black)
                 ^ (hash<bitbrd>()(b.legal) >> 1) );
    }
};

/* TODO more info in transposition table?
struct TTInfo {
    int score;
    
};*/

class Player {

private:
    int maxDepth;
    int minDepth;
    int sortDepth;
    int endgameDepth;
    Openings openingBook;

    int added;
    int used;

    unordered_map<Board, int, BoardHashFunc> endgame_table;

    int turn;

    int heuristic(Board *b);
    int eheuristic(Board *b);
    int mmheuristic(Board *b);
    Move *negascout(Board *b, vector<Move *> &moves, Side side, int depth,
        int alpha, int beta);
    int negascout_h(Board *b, Side side, int depth, int alpha, int beta);
    Move *endgame(Board *b, vector<Move *> &moves, Side s, int depth,
        int alpha, int beta);
    int endgame_h(Board *b, Side s, int depth, int alpha, int beta);
    int minimax(Board * b, Side side, int depth);

    int countSetBits(bitbrd b);
    void deleteMoveVector(vector<Move *> v);
    void sort(vector<Move *> &moves, vector<int> &scores, int left, int right);
    void swap(vector<Move *> &moves, vector<int> &scores, int i, int j);
    int partition(vector<Move *> &moves, vector<int> &scores, int left, int
        right, int pindex);

public:
    Board game;
    Side mySide;
    Side oppSide;

    Player(Side side);
    ~Player();
    
    Move *doMove(Move *opponentsMove, int msLeft);

    // Flag to tell if the player is running within the test_minimax context
    bool testingMinimax;
};

#endif
