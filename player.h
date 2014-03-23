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

#define NEG_INFTY -99999
#define INFTY 99999

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

class Player {

private:
    int maxDepth;
    int minDepth;
    int sortDepth;
    int endgameDepth;
    bool endgameSwitch;
    Openings openingBook;

    int turn;
    int totalTimePM;
    int endgameTimeMS;

    Move* indexToMove[65];

    unordered_map<Board, int, BoardHashFunc> endgame_table;

    int heuristic(Board *b);
    int eheuristic(Board *b);
    int negascout(Board *b, vector<int> &moves, vector<int> &scores,
        Side side, int depth, int alpha, int beta);
    int negascout_h(Board *b, int &topScore, Side side, int depth,
        int alpha, int beta);
    int endgame(Board *b, vector<int> &moves, Side s, int depth,
        int alpha, int beta);
    int endgame_h(Board *b, Side s, int depth, int alpha, int beta);
    //int minimax(Board * b, Side side, int depth);

    int countSetBits(bitbrd b);
    void sort(vector<int> &moves, vector<int> &scores, int left, int right);
    void swap(vector<int> &moves, vector<int> &scores, int i, int j);
    int partition(vector<int> &moves, vector<int> &scores, int left, int
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
