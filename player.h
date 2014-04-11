#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <chrono>
#include <fstream>
#include <iostream>
#include "common.h"
#include "board.h"
#include "openings.h"
#include "endgame.h"
#include "hash.h"
using namespace std;

#define USE_EDGE_TABLE true
#define USE_OPENING_BOOK false

const bitbrd CORNERS = 0x8100000000000081;
const bitbrd EDGES = 0x3C0081818181003C;
const bitbrd ADJ_CORNERS = 0x4281000000008142;
const bitbrd X_CORNERS = 0x0042000000004200;

const int PIECES_TO_INDEX[256] = {
0, 1, 3, 4, 9, 10, 12, 13, 27, 28, 30, 31, 36, 37, 39, 40, 
81, 82, 84, 85, 90, 91, 93, 94, 108, 109, 111, 112, 117, 118, 120, 121, 
243, 244, 246, 247, 252, 253, 255, 256, 270, 271, 273, 274, 279, 280, 282, 283, 
324, 325, 327, 328, 333, 334, 336, 337, 351, 352, 354, 355, 360, 361, 363, 364, 
729, 730, 732, 733, 738, 739, 741, 742, 756, 757, 759, 760, 765, 766, 768, 769, 
810, 811, 813, 814, 819, 820, 822, 823, 837, 838, 840, 841, 846, 847, 849, 850, 
972, 973, 975, 976, 981, 982, 984, 985, 
999, 1000, 1002, 1003, 1008, 1009, 1011, 1012, 
1053, 1054, 1056, 1057, 1062, 1063, 1065, 1066, 
1080, 1081, 1083, 1084, 1089, 1090, 1092, 1093, 
2187, 2188, 2190, 2191, 2196, 2197, 2199, 2200, 
2214, 2215, 2217, 2218, 2223, 2224, 2226, 2227, 
2268, 2269, 2271, 2272, 2277, 2278, 2280, 2281, 
2295, 2296, 2298, 2299, 2304, 2305, 2307, 2308, 
2430, 2431, 2433, 2434, 2439, 2440, 2442, 2443, 
2457, 2458, 2460, 2461, 2466, 2467, 2469, 2470, 
2511, 2512, 2514, 2515, 2520, 2521, 2523, 2524, 
2538, 2539, 2541, 2542, 2547, 2548, 2550, 2551, 
2916, 2917, 2919, 2920, 2925, 2926, 2928, 2929, 
2943, 2944, 2946, 2947, 2952, 2953, 2955, 2956, 
2997, 2998, 3000, 3001, 3006, 3007, 3009, 3010, 
3024, 3025, 3027, 3028, 3033, 3034, 3036, 3037, 
3159, 3160, 3162, 3163, 3168, 3169, 3171, 3172, 
3186, 3187, 3189, 3190, 3195, 3196, 3198, 3199, 
3240, 3241, 3243, 3244, 3249, 3250, 3252, 3253, 
3267, 3268, 3270, 3271, 3276, 3277, 3279, 3280
};

class Player {

private:
    int maxDepth;
    int minDepth;
    int sortDepth;
    int endgameDepth;
    int attemptingDepth;
    bool endgameSwitch;
    Endgame endgameSolver;

    #if USE_OPENING_BOOK
    Openings openingBook;
    #endif

    int turn;
    int totalTimePM;
    int endgameTimeMS;

    Move* indexToMove[64];

    int edgeTable[6561];

    int heuristic(Board *b);
    int pvs(Board *b, MoveList &moves, MoveList &scores,
        int side, int depth, int alpha, int beta);
    int pvs_h(Board *b, int &topScore, int side, int depth,
        int alpha, int beta);

    int countSetBits(bitbrd b);
    int boardToPV(Board *b);
    int bitsToPI(int b, int w);

    void sort(MoveList &moves, MoveList &scores, int left, int right);
    void swap(MoveList &moves, MoveList &scores, int i, int j);
    int partition(MoveList &moves, MoveList &scores, int left, int
        right, int pindex);
    void readEdgeTable();

public:
    Board game;
    int mySide;
    int oppSide;

    Hash killer_table;

    Player(Side side);
    ~Player();
    
    Move *doMove(Move *opponentsMove, int msLeft);
};

#endif
