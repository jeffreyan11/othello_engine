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

const int PIECES_TO_INDEX[512] = {
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
3267, 3268, 3270, 3271, 3276, 3277, 3279, 3280, 
6561, 6562, 6564, 6565, 6570, 6571, 6573, 6574, 
6588, 6589, 6591, 6592, 6597, 6598, 6600, 6601, 
6642, 6643, 6645, 6646, 6651, 6652, 6654, 6655, 
6669, 6670, 6672, 6673, 6678, 6679, 6681, 6682, 
6804, 6805, 6807, 6808, 6813, 6814, 6816, 6817, 
6831, 6832, 6834, 6835, 6840, 6841, 6843, 6844, 
6885, 6886, 6888, 6889, 6894, 6895, 6897, 6898, 
6912, 6913, 6915, 6916, 6921, 6922, 6924, 6925, 
7290, 7291, 7293, 7294, 7299, 7300, 7302, 7303, 
7317, 7318, 7320, 7321, 7326, 7327, 7329, 7330, 
7371, 7372, 7374, 7375, 7380, 7381, 7383, 7384, 
7398, 7399, 7401, 7402, 7407, 7408, 7410, 7411, 
7533, 7534, 7536, 7537, 7542, 7543, 7545, 7546, 
7560, 7561, 7563, 7564, 7569, 7570, 7572, 7573, 
7614, 7615, 7617, 7618, 7623, 7624, 7626, 7627, 
7641, 7642, 7644, 7645, 7650, 7651, 7653, 7654, 
8748, 8749, 8751, 8752, 8757, 8758, 8760, 8761, 
8775, 8776, 8778, 8779, 8784, 8785, 8787, 8788, 
8829, 8830, 8832, 8833, 8838, 8839, 8841, 8842, 
8856, 8857, 8859, 8860, 8865, 8866, 8868, 8869, 
8991, 8992, 8994, 8995, 9000, 9001, 9003, 9004, 
9018, 9019, 9021, 9022, 9027, 9028, 9030, 9031, 
9072, 9073, 9075, 9076, 9081, 9082, 9084, 9085, 
9099, 9100, 9102, 9103, 9108, 9109, 9111, 9112, 
9477, 9478, 9480, 9481, 9486, 9487, 9489, 9490, 
9504, 9505, 9507, 9508, 9513, 9514, 9516, 9517, 
9558, 9559, 9561, 9562, 9567, 9568, 9570, 9571, 
9585, 9586, 9588, 9589, 9594, 9595, 9597, 9598, 
9720, 9721, 9723, 9724, 9729, 9730, 9732, 9733, 
9747, 9748, 9750, 9751, 9756, 9757, 9759, 9760, 
9801, 9802, 9804, 9805, 9810, 9811, 9813, 9814, 
9828, 9829, 9831, 9832, 9837, 9838, 9840, 9841
};

class Player {

private:
    int maxDepth;
    int minDepth;
    int sortDepth;
    int endgameDepth;
    int attemptingDepth;

    Endgame endgameSolver;

    #if USE_OPENING_BOOK
    Openings openingBook;
    #endif

    int turn;
    int totalTimePM;
    int endgameTimeMS;

    Move* indexToMove[64];

    int edgeTable[6561];
    int s33Table[19683];
    int p24Table[6561];

    int pvs(Board *b, MoveList &moves, MoveList &scores,
        int side, int depth, int alpha, int beta);
    int pvs_h(Board *b, int &topScore, int side, int depth,
        int alpha, int beta);
    int heuristic(Board *b);

    int countSetBits(bitbrd i);
    bitbrd reflectVertical(bitbrd i);
    bitbrd reflectHorizontal(bitbrd x);
    bitbrd reflectDiag(bitbrd x);

    int boardToEPV(Board *b);
    int boardTo33PV(Board *b);
    int boardTo24PV(Board *b);
    int bitsToPI(int b, int w);

    void sort(MoveList &moves, MoveList &scores, int left, int right);
    void swap(MoveList &moves, MoveList &scores, int i, int j);
    int partition(MoveList &moves, MoveList &scores, int left, int
        right, int pindex);
    void readEdgeTable();
    void readStability33Table();
    void readPattern24Table();

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
