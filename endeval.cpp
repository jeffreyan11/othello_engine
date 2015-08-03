#include <cstdlib>
#include <fstream>
#include <iostream>
#include "board.h"
#include "endgame.h"
using namespace std;

#define DIVS 1
#define IOFFSET 1
#define TURNSPERDIV 64
#define USE_WIN_PROB false

struct thor_game {
    int final;
    int moves[60];
};

struct pv {
    long sum;
    int instances;
    pv() {
        sum = 0;
        instances = 0;
    }
};

thor_game **games;
unsigned int totalSize;

pv* pvTable2x4[DIVS][6561];
pv* edgeTable[DIVS][6561];
pv* e2xTable[DIVS][59049];
pv* p33Table[DIVS][19683];
int used[DIVS][6561];
int eused[DIVS][6561];
int exused[DIVS][59049];
int p33used[DIVS][19683];

const int PIECES_TO_INDEX[1024] = {
0, 1, 3, 4, 9, 10, 12, 13, 27, 28, 30, 31, 36, 37, 39, 40, 81, 82, 84, 85, 90, 
91, 93, 94, 108, 109, 111, 112, 117, 118, 120, 121, 243, 244, 246, 247, 252, 
253, 255, 256, 270, 271, 273, 274, 279, 280, 282, 283, 324, 325, 327, 328, 333, 
334, 336, 337, 351, 352, 354, 355, 360, 361, 363, 364, 729, 730, 732, 733, 738, 
739, 741, 742, 756, 757, 759, 760, 765, 766, 768, 769, 810, 811, 813, 814, 819, 
820, 822, 823, 837, 838, 840, 841, 846, 847, 849, 850, 972, 973, 975, 976, 981, 
982, 984, 985, 999, 1000, 1002, 1003, 1008, 1009, 1011, 1012, 1053, 1054, 1056, 
1057, 1062, 1063, 1065, 1066, 1080, 1081, 1083, 1084, 1089, 1090, 1092, 1093, 
2187, 2188, 2190, 2191, 2196, 2197, 2199, 2200, 2214, 2215, 2217, 2218, 2223, 
2224, 2226, 2227, 2268, 2269, 2271, 2272, 2277, 2278, 2280, 2281, 2295, 2296, 
2298, 2299, 2304, 2305, 2307, 2308, 2430, 2431, 2433, 2434, 2439, 2440, 2442, 
2443, 2457, 2458, 2460, 2461, 2466, 2467, 2469, 2470, 2511, 2512, 2514, 2515, 
2520, 2521, 2523, 2524, 2538, 2539, 2541, 2542, 2547, 2548, 2550, 2551, 2916, 
2917, 2919, 2920, 2925, 2926, 2928, 2929, 2943, 2944, 2946, 2947, 2952, 2953, 
2955, 2956, 2997, 2998, 3000, 3001, 3006, 3007, 3009, 3010, 3024, 3025, 3027, 
3028, 3033, 3034, 3036, 3037, 3159, 3160, 3162, 3163, 3168, 3169, 3171, 3172, 
3186, 3187, 3189, 3190, 3195, 3196, 3198, 3199, 3240, 3241, 3243, 3244, 3249, 
3250, 3252, 3253, 3267, 3268, 3270, 3271, 3276, 3277, 3279, 3280, 6561, 6562, 
6564, 6565, 6570, 6571, 6573, 6574, 6588, 6589, 6591, 6592, 6597, 6598, 6600, 
6601, 6642, 6643, 6645, 6646, 6651, 6652, 6654, 6655, 6669, 6670, 6672, 6673, 
6678, 6679, 6681, 6682, 6804, 6805, 6807, 6808, 6813, 6814, 6816, 6817, 6831, 
6832, 6834, 6835, 6840, 6841, 6843, 6844, 6885, 6886, 6888, 6889, 6894, 6895, 
6897, 6898, 6912, 6913, 6915, 6916, 6921, 6922, 6924, 6925, 7290, 7291, 7293, 
7294, 7299, 7300, 7302, 7303, 7317, 7318, 7320, 7321, 7326, 7327, 7329, 7330, 
7371, 7372, 7374, 7375, 7380, 7381, 7383, 7384, 7398, 7399, 7401, 7402, 7407, 
7408, 7410, 7411, 7533, 7534, 7536, 7537, 7542, 7543, 7545, 7546, 7560, 7561, 
7563, 7564, 7569, 7570, 7572, 7573, 7614, 7615, 7617, 7618, 7623, 7624, 7626, 
7627, 7641, 7642, 7644, 7645, 7650, 7651, 7653, 7654, 8748, 8749, 8751, 8752, 
8757, 8758, 8760, 8761, 8775, 8776, 8778, 8779, 8784, 8785, 8787, 8788, 8829, 
8830, 8832, 8833, 8838, 8839, 8841, 8842, 8856, 8857, 8859, 8860, 8865, 8866, 
8868, 8869, 8991, 8992, 8994, 8995, 9000, 9001, 9003, 9004, 9018, 9019, 9021, 
9022, 9027, 9028, 9030, 9031, 9072, 9073, 9075, 9076, 9081, 9082, 9084, 9085, 
9099, 9100, 9102, 9103, 9108, 9109, 9111, 9112, 9477, 9478, 9480, 9481, 9486, 
9487, 9489, 9490, 9504, 9505, 9507, 9508, 9513, 9514, 9516, 9517, 9558, 9559, 
9561, 9562, 9567, 9568, 9570, 9571, 9585, 9586, 9588, 9589, 9594, 9595, 9597, 
9598, 9720, 9721, 9723, 9724, 9729, 9730, 9732, 9733, 9747, 9748, 9750, 9751, 
9756, 9757, 9759, 9760, 9801, 9802, 9804, 9805, 9810, 9811, 9813, 9814, 9828, 
9829, 9831, 9832, 9837, 9838, 9840, 9841, 
19683, 19684, 19686, 19687, 19692, 19693, 19695, 19696, 
19710, 19711, 19713, 19714, 19719, 19720, 19722, 19723, 
19764, 19765, 19767, 19768, 19773, 19774, 19776, 19777, 
19791, 19792, 19794, 19795, 19800, 19801, 19803, 19804, 
19926, 19927, 19929, 19930, 19935, 19936, 19938, 19939, 
19953, 19954, 19956, 19957, 19962, 19963, 19965, 19966, 
20007, 20008, 20010, 20011, 20016, 20017, 20019, 20020, 
20034, 20035, 20037, 20038, 20043, 20044, 20046, 20047, 
20412, 20413, 20415, 20416, 20421, 20422, 20424, 20425, 
20439, 20440, 20442, 20443, 20448, 20449, 20451, 20452, 
20493, 20494, 20496, 20497, 20502, 20503, 20505, 20506, 
20520, 20521, 20523, 20524, 20529, 20530, 20532, 20533, 
20655, 20656, 20658, 20659, 20664, 20665, 20667, 20668, 
20682, 20683, 20685, 20686, 20691, 20692, 20694, 20695, 
20736, 20737, 20739, 20740, 20745, 20746, 20748, 20749, 
20763, 20764, 20766, 20767, 20772, 20773, 20775, 20776, 
21870, 21871, 21873, 21874, 21879, 21880, 21882, 21883, 
21897, 21898, 21900, 21901, 21906, 21907, 21909, 21910, 
21951, 21952, 21954, 21955, 21960, 21961, 21963, 21964, 
21978, 21979, 21981, 21982, 21987, 21988, 21990, 21991, 
22113, 22114, 22116, 22117, 22122, 22123, 22125, 22126, 
22140, 22141, 22143, 22144, 22149, 22150, 22152, 22153, 
22194, 22195, 22197, 22198, 22203, 22204, 22206, 22207, 
22221, 22222, 22224, 22225, 22230, 22231, 22233, 22234, 
22599, 22600, 22602, 22603, 22608, 22609, 22611, 22612, 
22626, 22627, 22629, 22630, 22635, 22636, 22638, 22639, 
22680, 22681, 22683, 22684, 22689, 22690, 22692, 22693, 
22707, 22708, 22710, 22711, 22716, 22717, 22719, 22720, 
22842, 22843, 22845, 22846, 22851, 22852, 22854, 22855, 
22869, 22870, 22872, 22873, 22878, 22879, 22881, 22882, 
22923, 22924, 22926, 22927, 22932, 22933, 22935, 22936, 
22950, 22951, 22953, 22954, 22959, 22960, 22962, 22963, 
26244, 26245, 26247, 26248, 26253, 26254, 26256, 26257, 
26271, 26272, 26274, 26275, 26280, 26281, 26283, 26284, 
26325, 26326, 26328, 26329, 26334, 26335, 26337, 26338, 
26352, 26353, 26355, 26356, 26361, 26362, 26364, 26365, 
26487, 26488, 26490, 26491, 26496, 26497, 26499, 26500, 
26514, 26515, 26517, 26518, 26523, 26524, 26526, 26527, 
26568, 26569, 26571, 26572, 26577, 26578, 26580, 26581, 
26595, 26596, 26598, 26599, 26604, 26605, 26607, 26608, 
26973, 26974, 26976, 26977, 26982, 26983, 26985, 26986, 
27000, 27001, 27003, 27004, 27009, 27010, 27012, 27013, 
27054, 27055, 27057, 27058, 27063, 27064, 27066, 27067, 
27081, 27082, 27084, 27085, 27090, 27091, 27093, 27094, 
27216, 27217, 27219, 27220, 27225, 27226, 27228, 27229, 
27243, 27244, 27246, 27247, 27252, 27253, 27255, 27256, 
27297, 27298, 27300, 27301, 27306, 27307, 27309, 27310, 
27324, 27325, 27327, 27328, 27333, 27334, 27336, 27337, 
28431, 28432, 28434, 28435, 28440, 28441, 28443, 28444, 
28458, 28459, 28461, 28462, 28467, 28468, 28470, 28471, 
28512, 28513, 28515, 28516, 28521, 28522, 28524, 28525, 
28539, 28540, 28542, 28543, 28548, 28549, 28551, 28552, 
28674, 28675, 28677, 28678, 28683, 28684, 28686, 28687, 
28701, 28702, 28704, 28705, 28710, 28711, 28713, 28714, 
28755, 28756, 28758, 28759, 28764, 28765, 28767, 28768, 
28782, 28783, 28785, 28786, 28791, 28792, 28794, 28795, 
29160, 29161, 29163, 29164, 29169, 29170, 29172, 29173, 
29187, 29188, 29190, 29191, 29196, 29197, 29199, 29200, 
29241, 29242, 29244, 29245, 29250, 29251, 29253, 29254, 
29268, 29269, 29271, 29272, 29277, 29278, 29280, 29281, 
29403, 29404, 29406, 29407, 29412, 29413, 29415, 29416, 
29430, 29431, 29433, 29434, 29439, 29440, 29442, 29443, 
29484, 29485, 29487, 29488, 29493, 29494, 29496, 29497, 
29511, 29512, 29514, 29515, 29520, 29521, 29523, 29524
};

Eval evaluater;
const int startIndex = 46;

void readThorGame(string file);
void readGame(string file, unsigned int n);
void writeFile();
void freemem();
void resetgames();
bitbrd reflectVertical(bitbrd i);
bitbrd reflectHorizontal(bitbrd x);
bitbrd reflectDiag(bitbrd x);
void boardToEPV(Board *b, int score, int turn);
void boardTo24PV(Board *b, int score, int turn);
void boardToE2XPV(Board *b, int score, int turn);
void boardTo33PV(Board *b, int score, int turn);
int bitsToPI(int b, int w);

void checkGames() {
    cout << "Checking game validity: " << totalSize << " games." << endl;
    int errors = 0;
    for(unsigned int i = 0; i < totalSize; i++) {
        thor_game *game = games[i];

        Board tracker;
        int side = CBLACK;

        for(int j = 0; j < 55; j++) {
            if(!tracker.checkMove(game->moves[j], side)) {
                // If one side must pass it is not indicated in the database?
                side = side^1;
                if(!tracker.checkMove(game->moves[j], side)) {
                    errors++;
                    games[i] = NULL;
                    /*cerr << "error at " << i << " " << j << endl;
                    cerr << game->moves[j-1] << " " << game->moves[j] << " " << game->moves[j+1] << endl;*/
                    break;
                }
            }
            tracker.doMove(game->moves[j], side);
            side = side^1;
        }
    }
    cout << errors << " errors." << endl;
}

void replaceEnd() {
    for(unsigned int i = 0; i < totalSize; i++) {
        cerr << "Replacing end: " << i << endl;

        thor_game *game = games[i];
        if(game == NULL)
            continue;

        Board tracker;
        int side = CBLACK;
        // play opening moves
        for(int j = 0; j < startIndex; j++) {
            // If one side must pass it is not indicated in the database?
            if(!tracker.checkMove(game->moves[j], side)) {
                side = side^1;
            }
            tracker.doMove(game->moves[j], side);
            side = side^1;
        }

        Endgame e;
        if(tracker.countEmpty() > 18) {
            games[i] = NULL;
            continue;
        }

        // start filling in moves
        int score = 0;
        for(int j = startIndex; j < 55; j++) {
            if(!tracker.checkMove(game->moves[j], side)) {
                side = side^1;
            }
            MoveList lm = tracker.getLegalMoves(side);
            /*if (lm.size == 0) {
                cerr << "passing" << endl;
                //games[i] = NULL;
                //break;
                side = side^1;
                continue;
                lm = tracker.getLegalMoves(side);
                if (lm.size == 0)
                    break;
            }*/
            int best = e.solveEndgame(tracker, lm, side, tracker.countEmpty(),
                10000000, &evaluater, &score);

            game->moves[j] = best;
            tracker.doMove(best, side);

            if (j == startIndex) {
                // We want everything from black's POV
                if (side == CWHITE)
                    score = -score;
                game->final = (score + 64) / 2;
            }

            side = side^1;
        }
    }
}

void searchFeatures() {
    cout << "Searching features." << endl;
    for(unsigned int i = 0; i < totalSize; i++) {
        for(int n = 0; n < DIVS; n++) {
            for(int j = 0; j < 6561; j++) {
                used[n][j] = 0;
                eused[n][j] = 0;
            }
            for(int j = 0; j < 59049; j++) {
                exused[n][j] = 0;
            }
            for(int j = 0; j < 19683; j++) {
                p33used[n][j] = 0;
            }
        }
        thor_game *game = games[i];
        if(game == NULL)
            continue;

        #if USE_WIN_PROB
        int score = 0;
        if(2*game->final - 64 > 0) score = 1;
        else if(2*game->final - 64 < 0) score = -1;
        #else
        int score = 2*game->final - 64;
        #endif
        Board tracker;
        int side = CBLACK;
        // play opening moves
        for(int j = 0; j < startIndex; j++) {
            // If one side must pass it is not indicated in the database?
            if(!tracker.checkMove(game->moves[j], side)) {
                side = side^1;
            }
            tracker.doMove(game->moves[j], side);
            side = side^1;
        }

        // starting recording statistics
        for(int j = startIndex; j < 55; j++) {
            tracker.doMove(game->moves[j], side);
            boardTo24PV(&tracker, score, j);
            boardToEPV(&tracker, score, j);
            boardToE2XPV(&tracker, score, j);
            boardTo33PV(&tracker, score, j);
            side = side^1;
        }
    }
}

int main(int argc, char **argv) {
    totalSize = 0;
    games = NULL;
    for(int n = 0; n < DIVS; n++) {
        for(int i = 0; i < 6561; i++) {
            pvTable2x4[n][i] = new pv();
            edgeTable[n][i] = new pv();
        }
        for(int i = 0; i < 59049; i++) {
            e2xTable[n][i] = new pv();
        }
        for(int i = 0; i < 19683; i++) {
            p33Table[n][i] = new pv();
        }
    }

    readThorGame("WTH_7708/Logistello_book_1999.wtb");
    readThorGame("WTH_7708/WTH_2013.wtb");
    readThorGame("WTH_7708/WTH_2012.wtb");
    readThorGame("WTH_7708/WTH_2011.wtb");
    readThorGame("WTH_7708/WTH_2010.wtb");
    readThorGame("WTH_7708/WTH_2009.wtb");
    readThorGame("WTH_7708/WTH_2008.wtb");
    readThorGame("WTH_7708/WTH_2007.wtb");
    readThorGame("WTH_7708/WTH_2006.wtb");
    readThorGame("WTH_7708/WTH_2005.wtb");
    readThorGame("WTH_7708/WTH_2004.wtb");
    readThorGame("WTH_7708/WTH_2003.wtb");
    readThorGame("WTH_7708/WTH_2002.wtb");
    readThorGame("WTH_7708/WTH_2001.wtb");
    readThorGame("WTH_7708/WTH_2000.wtb");
    readThorGame("WTH_7708/WTH_1999.wtb");
    readThorGame("WTH_7708/WTH_1998.wtb");
    readThorGame("WTH_7708/WTH_1997.wtb");
    readThorGame("WTH_7708/WTH_1996.wtb");
    readThorGame("WTH_7708/WTH_1995.wtb");
    readThorGame("WTH_7708/WTH_1994.wtb");
    readThorGame("WTH_7708/WTH_1993.wtb");
    readThorGame("WTH_7708/WTH_1992.wtb");
    readThorGame("WTH_7708/WTH_1991.wtb");
    readThorGame("WTH_7708/WTH_1990.wtb");
    readThorGame("WTH_7708/WTH_1989.wtb");
    readThorGame("WTH_7708/WTH_1988.wtb");
    readThorGame("WTH_7708/WTH_1987.wtb");
    readThorGame("WTH_7708/WTH_1986.wtb");
    readThorGame("WTH_7708/WTH_1985.wtb");
    readThorGame("WTH_7708/WTH_1984.wtb");
    //readGame("gamedb042714.txt", 8200);

    checkGames();

    replaceEnd();

    searchFeatures();

    writeFile();

    freemem();
    return 0;
}

void readThorGame(string file) {
    unsigned int prevSize = totalSize;

    std::ifstream is (file, std::ifstream::binary);
    if(is) {
        char *header = new char[16];
        char *buffer = new char[68];

        is.read(header, 16);

        totalSize = (unsigned char)(header[5]);
        totalSize <<= 8;
        totalSize += (unsigned char)(header[4]);

        cout << "Reading " << totalSize << " games." << endl;

        totalSize += prevSize;

        thor_game** e = games;
        games = new thor_game*[totalSize];
        for(unsigned int i = 0; i < prevSize; i++) {
            games[i] = e[i];
        }
        if(prevSize > 0)
            delete[] e;

        for(unsigned int i = prevSize; i < totalSize; i++) {
            is.read(buffer, 68);
            thor_game *temp = new thor_game();
            temp->final = (unsigned char) buffer[6];
            for(int j = 0; j < 60; j++) {
                int movetoparse = (unsigned char) buffer[8+j];
                int x = movetoparse % 10;
                int y = movetoparse / 10;
                temp->moves[j] = (x-1) + 8*(y-1);
            }
            games[i] = temp;
        }

        delete[] header;
        delete[] buffer;
    }
}

void readGame(string file, unsigned int n) {
    cout << "Reading " << n << " games." << endl;

    std::ifstream db(file);
    std::string line;

    unsigned int prevSize = totalSize;
    totalSize += n;

    thor_game** e = games;
    games = new thor_game*[totalSize];
    for(unsigned int i = 0; i < prevSize; i++) {
        games[i] = e[i];
    }
    if(prevSize > 0)
        delete[] e;

    for(unsigned int i = prevSize; i < totalSize; i++) {
        std::string::size_type sz = 0;
        thor_game *temp = new thor_game();
        getline(db, line);

        temp->final = std::stoi(line, &sz, 0);
        line = line.substr(sz);

        for(int j = 0; j < 60; j++) {
            temp->moves[j] = std::stoi(line, &sz, 0);
            line = line.substr(sz);
        }

        games[i] = temp;
    }
}

void writeFile() {
    ofstream out;
    out.open("patterns/new/p24end.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 6561; i++) {
            pv *a = pvTable2x4[n][i];
            if(a->instances != 0) {
            #if USE_WIN_PROB
                int to = 100*(a->sum)/(a->instances);
            #else
                double to = ((double)(a->sum))/((double)(a->instances));
            #endif
                if(a->instances < 2) to /= 6;
                else if(a->instances < 3) to /= 3;
                else if(a->instances < 6) to /= 2;
                out << (int) (to * 100.0) << " ";
            }
            else out << 0 << " ";

            if(i%9 == 8) out << endl;
        }
    }
    out.close();

    out.open("patterns/new/edgeend.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 6561; i++) {
            pv *a = edgeTable[n][i];
            if(a->instances != 0) {
            #if USE_WIN_PROB
                int to = 100*(a->sum)/(a->instances);
            #else
                double to = ((double)(a->sum))/((double)(a->instances));
            #endif
                if(a->instances < 2) to /= 6;
                else if(a->instances < 3) to /= 3;
                else if(a->instances < 6) to /= 2;
                out << (int) (to * 100.0) << " ";
            }
            else out << 0 << " ";

            if(i%9 == 8) out << endl;
        }
    }
    out.close();

    out.open("patterns/new/pE2Xend.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 59049; i++) {
            pv *a = e2xTable[n][i];
            if(a->instances != 0) {
            #if USE_WIN_PROB
                int to = 100*(a->sum)/(a->instances);
            #else
                double to = ((double)(a->sum))/((double)(a->instances));
            #endif
                if(a->instances < 2) to /= 6;
                else if(a->instances < 3) to /= 3;
                else if(a->instances < 6) to /= 2;
                out << (int) (to * 100.0) << " ";
            }
            else out << 0 << " ";

            if(i % 9 == 8) out << endl;
        }
    }
    out.close();

    out.open("patterns/new/p33end.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 19683; i++) {
            pv *a = p33Table[n][i];
            if(a->instances != 0) {
            #if USE_WIN_PROB
                int to = 100*(a->sum)/(a->instances);
            #else
                double to = ((double)(a->sum))/((double)(a->instances));
            #endif
                if(a->instances < 2) to /= 6;
                else if(a->instances < 3) to /= 3;
                else if(a->instances < 6) to /= 2;
                out << (int) (to * 100.0) << " ";
            }
            else out << 0 << " ";

            if(i % 9 == 8) out << endl;
        }
    }
    out.close();
}

void boardToEPV(Board *b, int score, int turn) {
    int index = (turn - IOFFSET) / TURNSPERDIV;
    bitbrd black = b->getBits(CBLACK);
    bitbrd white = b->getBits(CWHITE);
    int r2 = bitsToPI( (int)((black >> 8) & 0xFF), (int)((white >> 8) & 0xFF) );
    int r7 = bitsToPI( (int)((black >> 48) & 0xFF), (int)((white >> 48) & 0xFF) );
    int c2 = bitsToPI(
      (int)((((black>>1) & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56),
      (int)((((white>>1) & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56) );
    int c7 = bitsToPI(
      (int)((((black<<1) & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56),
      (int)((((white<<1) & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56) );

    if(!eused[index][r2]) {
        edgeTable[index][r2]->sum += score;
        edgeTable[index][r2]->instances++;
    }
    if(!eused[index][r7]) {
        edgeTable[index][r7]->sum += score;
        edgeTable[index][r7]->instances++;
    }
    if(!eused[index][c2]) {
        edgeTable[index][c2]->sum += score;
        edgeTable[index][c2]->instances++;
    }
    if(!eused[index][c7]) {
        edgeTable[index][c7]->sum += score;
        edgeTable[index][c7]->instances++;
    }

    eused[index][r2] = 1;
    eused[index][r7] = 1;
    eused[index][c2] = 1;
    eused[index][c7] = 1;
}

void boardTo24PV(Board *b, int score, int turn) {
    int index = (turn - IOFFSET) / TURNSPERDIV;
    bitbrd black = b->getBits(CBLACK);
    bitbrd white = b->getBits(CWHITE);
    int ulb = (int) ((black&0xF) + ((black>>4)&0xF0));
    int ulw = (int) ((white&0xF) + ((white>>4)&0xF0));
    int ul = bitsToPI(ulb, ulw);

    bitbrd rvb = reflectVertical(black);
    bitbrd rvw = reflectVertical(white);
    int llb = (int) ((rvb&0xF) + ((rvb>>4)&0xF0));
    int llw = (int) ((rvw&0xF) + ((rvw>>4)&0xF0));
    int ll = bitsToPI(llb, llw);

    bitbrd rhb = reflectHorizontal(black);
    bitbrd rhw = reflectHorizontal(white);
    int urb = (int) ((rhb&0xF) + ((rhb>>4)&0xF0));
    int urw = (int) ((rhw&0xF) + ((rhw>>4)&0xF0));
    int ur = bitsToPI(urb, urw);

    bitbrd rbb = reflectVertical(rhb);
    bitbrd rbw = reflectVertical(rhw);
    int lrb = (int) ((rbb&0xF) + ((rbb>>4)&0xF0));
    int lrw = (int) ((rbw&0xF) + ((rbw>>4)&0xF0));
    int lr = bitsToPI(lrb, lrw);

    bitbrd rotb = reflectDiag(black);
    bitbrd rotw = reflectDiag(white);
    int rulb = (int) ((rotb&0xF) + ((rotb>>4)&0xF0));
    int rulw = (int) ((rotw&0xF) + ((rotw>>4)&0xF0));
    int rul = bitsToPI(rulb, rulw);

    bitbrd rotvb = reflectVertical(rotb);
    bitbrd rotvw = reflectVertical(rotw);
    int rllb = (int) ((rotvb&0xF) + ((rotvb>>4)&0xF0));
    int rllw = (int) ((rotvw&0xF) + ((rotvw>>4)&0xF0));
    int rll = bitsToPI(rllb, rllw);

    bitbrd rothb = reflectHorizontal(rotb);
    bitbrd rothw = reflectHorizontal(rotw);
    int rurb = (int) ((rothb&0xF) + ((rothb>>4)&0xF0));
    int rurw = (int) ((rothw&0xF) + ((rothw>>4)&0xF0));
    int rur = bitsToPI(rurb, rurw);

    bitbrd rotbb = reflectVertical(rothb);
    bitbrd rotbw = reflectVertical(rothw);
    int rlrb = (int) ((rotbb&0xF) + ((rotbb>>4)&0xF0));
    int rlrw = (int) ((rotbw&0xF) + ((rotbw>>4)&0xF0));
    int rlr = bitsToPI(rlrb, rlrw);

    // TODO record values
    if(!used[index][ul]) {
        pvTable2x4[index][ul]->sum += score;
        pvTable2x4[index][ul]->instances++;
    }
    if(!used[index][ll]) {
        pvTable2x4[index][ll]->sum += score;
        pvTable2x4[index][ll]->instances++;
    }
    if(!used[index][ur]) {
        pvTable2x4[index][ur]->sum += score;
        pvTable2x4[index][ur]->instances++;
    }
    if(!used[index][lr]) {
        pvTable2x4[index][lr]->sum += score;
        pvTable2x4[index][lr]->instances++;
    }
    if(!used[index][rul]) {
        pvTable2x4[index][rul]->sum += score;
        pvTable2x4[index][rul]->instances++;
    }
    if(!used[index][rll]) {
        pvTable2x4[index][rll]->sum += score;
        pvTable2x4[index][rll]->instances++;
    }
    if(!used[index][rur]) {
        pvTable2x4[index][rur]->sum += score;
        pvTable2x4[index][rur]->instances++;
    }
    if(!used[index][rlr]) {
        pvTable2x4[index][rlr]->sum += score;
        pvTable2x4[index][rlr]->instances++;
    }

    used[index][ul] = 1;
    used[index][ll] = 1;
    used[index][ur] = 1;
    used[index][lr] = 1;
    used[index][rul] = 1;
    used[index][rll] = 1;
    used[index][rur] = 1;
    used[index][rlr] = 1;
}

void boardToE2XPV(Board *b, int score, int turn) {
    int index = (turn - IOFFSET) / TURNSPERDIV;
    bitbrd black = b->getBits(CBLACK);
    bitbrd white = b->getBits(CWHITE);
    int r1b = (int) ( (black & 0xFF) +
        ((black & 0x200) >> 1) + ((black & 0x4000) >> 5) );
    int r1w = (int) ( (white & 0xFF) +
        ((white & 0x200) >> 1) + ((white & 0x4000) >> 5) );
    int r1 = bitsToPI(r1b, r1w);

    int r8b = (int) ( (black>>56) + ((black & 0x2000000000000) >> 41) +
        ((black & 0x40000000000000) >> 45) );
    int r8w = (int) ( (white>>56) + ((white & 0x2000000000000) >> 41) +
        ((white & 0x40000000000000) >> 45) );
    int r8 = bitsToPI(r8b, r8w);

    int c1b = (int) (
        (((black & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56) + 
        ((black & 0x200) >> 1) + ((black & 0x2000000000000) >> 40) );
    int c1w = (int) (
        (((white & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56) +
        ((white & 0x200) >> 1) + ((white & 0x2000000000000) >> 40) );
    int c1 = bitsToPI(c1b, c1w);

    int c8b = (int) (
        (((black & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56) +
        ((black & 0x4000) >> 6) + ((black & 0x40000000000000) >> 45) );
    int c8w = (int) (
        (((white & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56) +
        ((white & 0x4000) >> 6) + ((white & 0x40000000000000) >> 45) );
    int c8 = bitsToPI(c8b, c8w);


    if(!exused[index][r1]) {
        e2xTable[index][r1]->sum += score;
        e2xTable[index][r1]->instances++;
    }
    if(!exused[index][r8]) {
        e2xTable[index][r8]->sum += score;
        e2xTable[index][r8]->instances++;
    }
    if(!exused[index][c1]) {
        e2xTable[index][c1]->sum += score;
        e2xTable[index][c1]->instances++;
    }
    if(!exused[index][c8]) {
        e2xTable[index][c8]->sum += score;
        e2xTable[index][c8]->instances++;
    }

    exused[index][r1] = 1;
    exused[index][r8] = 1;
    exused[index][c1] = 1;
    exused[index][c8] = 1;
}

void boardTo33PV(Board *b, int score, int turn) {
    int index = (turn - IOFFSET) / TURNSPERDIV;
    bitbrd black = b->getBits(CBLACK);
    bitbrd white = b->getBits(CWHITE);
    int ulb = (int) ((black&7) + ((black>>5)&0x38) + ((black>>10)&0x1C0));
    int ulw = (int) ((white&7) + ((white>>5)&0x38) + ((white>>10)&0x1C0));
    int ul = bitsToPI(ulb, ulw);

    bitbrd rvb = reflectVertical(black);
    bitbrd rvw = reflectVertical(white);
    int llb = (int) ((rvb&7) + ((rvb>>5)&0x38) + ((rvb>>10)&0x1C0));
    int llw = (int) ((rvw&7) + ((rvw>>5)&0x38) + ((rvw>>10)&0x1C0));
    int ll = bitsToPI(llb, llw);

    bitbrd rhb = reflectHorizontal(black);
    bitbrd rhw = reflectHorizontal(white);
    int urb = (int) ((rhb&7) + ((rhb>>5)&0x38) + ((rhb>>10)&0x1C0));
    int urw = (int) ((rhw&7) + ((rhw>>5)&0x38) + ((rhw>>10)&0x1C0));
    int ur = bitsToPI(urb, urw);

    bitbrd rbb = reflectVertical(rhb);
    bitbrd rbw = reflectVertical(rhw);
    int lrb = (int) ((rbb&7) + ((rbb>>5)&0x38) + ((rbb>>10)&0x1C0));
    int lrw = (int) ((rbw&7) + ((rbw>>5)&0x38) + ((rbw>>10)&0x1C0));
    int lr = bitsToPI(lrb, lrw);

    if(!p33used[index][ul]) {
        p33Table[index][ul]->sum += score;
        p33Table[index][ul]->instances++;
    }
    if(!p33used[index][ur]) {
        p33Table[index][ur]->sum += score;
        p33Table[index][ur]->instances++;
    }
    if(!p33used[index][ll]) {
        p33Table[index][ll]->sum += score;
        p33Table[index][ll]->instances++;
    }
    if(!p33used[index][lr]) {
        p33Table[index][lr]->sum += score;
        p33Table[index][lr]->instances++;
    }

    p33used[index][ul] = 1;
    p33used[index][ur] = 1;
    p33used[index][ll] = 1;
    p33used[index][lr] = 1;
}

int bitsToPI(int b, int w) {
    return PIECES_TO_INDEX[b] + 2*PIECES_TO_INDEX[w];
}

bitbrd reflectVertical(bitbrd i) {
    #if defined(__x86_64__)
        asm ("bswap %0" : "=r" (i) : "0" (i));
        return i;
    #else
        const bitbrd k1 = 0x00FF00FF00FF00FF;
        const bitbrd k2 = 0x0000FFFF0000FFFF;
        i = ((i >>  8) & k1) | ((i & k1) <<  8);
        i = ((i >> 16) & k2) | ((i & k2) << 16);
        i = ( i >> 32)       | ( i       << 32);
        return i;
    #endif
}

bitbrd reflectHorizontal(bitbrd x) {
    const bitbrd k1 = 0x5555555555555555;
    const bitbrd k2 = 0x3333333333333333;
    const bitbrd k4 = 0x0f0f0f0f0f0f0f0f;
    x = ((x >> 1) & k1) | ((x & k1) << 1);
    x = ((x >> 2) & k2) | ((x & k2) << 2);
    x = ((x >> 4) & k4) | ((x & k4) << 4);
    return x;
}

bitbrd reflectDiag(bitbrd x) {
    bitbrd t;
    const bitbrd k1 = 0x5500550055005500;
    const bitbrd k2 = 0x3333000033330000;
    const bitbrd k4 = 0x0f0f0f0f00000000;
    t  = k4 & (x ^ (x << 28));
    x ^=       t ^ (t >> 28) ;
    t  = k2 & (x ^ (x << 14));
    x ^=       t ^ (t >> 14) ;
    t  = k1 & (x ^ (x <<  7));
    x ^=       t ^ (t >>  7) ;
    return x;
}

void resetgames() {
    for(unsigned int i = 0; i < totalSize; i++) {
        delete games[i];
    }
    delete[] games;
}

void freemem() {
    for(unsigned int i = 0; i < totalSize; i++) {
        delete games[i];
    }
    delete[] games;
    for(int n = 0; n < DIVS; n++) {
        for(int i = 0; i < 6561; i++) {
            delete pvTable2x4[n][i];
            delete edgeTable[n][i];
        }
        for(int i = 0; i < 59049; i++) {
            delete e2xTable[n][i];
        }
    }
}
