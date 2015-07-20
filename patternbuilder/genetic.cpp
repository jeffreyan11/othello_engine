#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include "board.h"
#include "endgame.h"
#include "genetic.h"
using namespace std;

#define NUM_OFFSPRING 36
#define OFFSPRING_PER_COMBO 2
#define DIVS 4
#define IOFFSET 10
#define TURNSPERDIV 9

struct thor_game {
    int final;
    int moves[60];
};

thor_game **games;
unsigned int totalSize;

int ***p24Table;
int ***edgeTable;
int ***pE2XTable;
int ***p33Table;
int ***used;
int ***eused;
int ***exused;
int ***p33used;
int64_t p24Errors[NUM_OFFSPRING];
int64_t edgeErrors[NUM_OFFSPRING];
int64_t pE2XErrors[NUM_OFFSPRING];
int64_t p33Errors[NUM_OFFSPRING];

// Random number generator
mt19937 rng(time(0));

void write();
void readEdgeTable();
void readPattern24Table();
void readPatternE2XTable();
void readPattern33Table();
void freemem(int index);
void initializeArrays();

void readThorGame(string file);
void checkGames();
void replaceEnd();
void searchFeatures();
void initializeOffspring();
void createOffspring(int index);

bitbrd reflectVertical(bitbrd i);
bitbrd reflectHorizontal(bitbrd x);
bitbrd reflectDiag(bitbrd x);
void boardToEPV(Board *b, int offspringIndex, int score, int turn);
void boardTo24PV(Board *b, int offspringIndex, int score, int turn);
void boardToE2XPV(Board *b, int offspringIndex, int score, int turn);
void boardTo33PV(Board *b, int offspringIndex, int score, int turn);
int bitsToPI(int b, int w);

int main(int argc, char **argv) {
    initializeArrays();

    readEdgeTable();
    readPattern24Table();
    readPatternE2XTable();
    readPattern33Table();

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

    checkGames();
    replaceEnd();

    initializeOffspring();

    for (int i = 0; i < 30; i++) {
        cerr << "Finding least square errors" << endl;
        searchFeatures();

        cerr << "Lowest 2 least sq errors:" << endl << "(" << p24Errors[0] << ", " << edgeErrors[0] << ", " << pE2XErrors[0] << ", " << p33Errors[0] << ")" << endl << "(" << p24Errors[1] << ", " << edgeErrors[1] << ", " << pE2XErrors[1] << ", " << p33Errors[1] << ")" << endl;

        cerr << "Killing weak candidates." << endl;
        freemem(6);
        cerr << "Creating new offspring." << endl;
        createOffspring(6);
    }

    write();

    return 0;
}

void searchFeatures() {
    for (int i = 0; i < NUM_OFFSPRING; i++) {
        p24Errors[i] = 0;
        edgeErrors[i] = 0;
        pE2XErrors[i] = 0;
        p33Errors[i] = 0;
    }

    for (int a = 0; a < NUM_OFFSPRING; a++) {
        if (a % 10 == 0)
            cerr << "Analyzing offspring " << a+1 << endl;
        for(unsigned int i = 0; i < totalSize; i++) {
            for(int n = 0; n < DIVS; n++) {
                for(int j = 0; j < 6561; j++) {
                    used[a][n][j] = 0;
                    eused[a][n][j] = 0;
                }
                for(int j = 0; j < 59049; j++) {
                    exused[a][n][j] = 0;
                }
                for(int j = 0; j < 19683; j++) {
                    p33used[a][n][j] = 0;
                }
            }
            thor_game *game = games[i];
            if(game == NULL)
                continue;

            int score = 2*game->final - 64;
            score *= 10000;

            Board tracker;
            int side = CBLACK;
            // play opening moves
            for(int j = 0; j < 10; j++) {
                // If one side must pass it is not indicated in the database?
                if(!tracker.checkMove(game->moves[j], side)) {
                    side = -side;
                }
                tracker.doMove(game->moves[j], side);
                side = -side;
            }

            // starting recording statistics
            for(int j = 10; j < 46; j++) {
                if(!tracker.checkMove(game->moves[j], side)) {
                    side = -side;
                }
                tracker.doMove(game->moves[j], side);
                boardTo24PV(&tracker, a, score, j);
                boardToEPV(&tracker, a, score, j);
                boardToE2XPV(&tracker, a, score, j);
                boardTo33PV(&tracker, a, score, j);
                side = -side;
            }
        }
    }

    // sort by least square errors
    for(int i = 1; i < NUM_OFFSPRING; i++) {
        int j = i;
        int **temp1 = p24Table[i];
        int64_t temp2 = p24Errors[i];
        while(j > 0 && p24Errors[j-1] > p24Errors[j]) {
            p24Errors[j] = p24Errors[j-1];
            p24Table[j] = p24Table[j-1];
            j--;
        }
        p24Table[j] = temp1;
        p24Errors[j] = temp2;
    }
    for(int i = 1; i < NUM_OFFSPRING; i++) {
        int j = i;
        int **temp1 = edgeTable[i];
        int64_t temp2 = edgeErrors[i];
        while(j > 0 && edgeErrors[j-1] > edgeErrors[j]) {
            edgeErrors[j] = edgeErrors[j-1];
            edgeTable[j] = edgeTable[j-1];
            j--;
        }
        edgeTable[j] = temp1;
        edgeErrors[j] = temp2;
    }
    for(int i = 1; i < NUM_OFFSPRING; i++) {
        int j = i;
        int **temp1 = pE2XTable[i];
        int64_t temp2 = pE2XErrors[i];
        while(j > 0 && pE2XErrors[j-1] > pE2XErrors[j]) {
            pE2XErrors[j] = pE2XErrors[j-1];
            pE2XTable[j] = pE2XTable[j-1];
            j--;
        }
        pE2XTable[j] = temp1;
        pE2XErrors[j] = temp2;
    }
    for(int i = 1; i < NUM_OFFSPRING; i++) {
        int j = i;
        int **temp1 = p33Table[i];
        int64_t temp2 = p33Errors[i];
        while(j > 0 && p33Errors[j-1] > p33Errors[j]) {
            p33Errors[j] = p33Errors[j-1];
            p33Table[j] = p33Table[j-1];
            j--;
        }
        p33Table[j] = temp1;
        p33Errors[j] = temp2;
    }
}

void createOffspring(int index) {
    for (int i = index; i < NUM_OFFSPRING; i++) {
        edgeTable[i] = new int *[DIVS];
        p24Table[i] = new int *[DIVS];
        pE2XTable[i] = new int *[DIVS];
        p33Table[i] = new int *[DIVS];
    }
    for (int i = index; i < NUM_OFFSPRING; i++) {
        for (int j = 0; j < DIVS; j++) {
            edgeTable[i][j] = new int[6561];
            p24Table[i][j] = new int[6561];
            pE2XTable[i][j] = new int[59049];
            p33Table[i][j] = new int[19683];
        }
    }

    int combos = index * (index - 1) / 2;
    int ***edgeTableC = new int **[combos];
    int ***p24TableC = new int **[combos];
    int ***pE2XTableC = new int **[combos];
    int ***p33TableC = new int **[combos];
    for (int i = 0; i < combos; i++) {
        edgeTableC[i] = new int *[DIVS];
        p24TableC[i] = new int *[DIVS];
        pE2XTableC[i] = new int *[DIVS];
        p33TableC[i] = new int *[DIVS];
    }
    for (int i = 0; i < combos; i++) {
        for (int j = 0; j < DIVS; j++) {
            edgeTableC[i][j] = new int[6561];
            p24TableC[i][j] = new int[6561];
            pE2XTableC[i][j] = new int[59049];
            p33TableC[i][j] = new int[19683];
        }
    }

    int l = 0;
    for (int i = 0; i < index-1; i++) {
        for (int j = i+1; j < index; j++) {
            for (int k = 0; k < DIVS; k++) {
                for (int m = 0; m < 6561; m++)
                    edgeTableC[l][k][m] = (edgeTable[i][k][m] + edgeTable[j][k][m]) / 2;
                for (int m = 0; m < 6561; m++)
                    p24TableC[l][k][m] = (p24Table[i][k][m] + p24Table[j][k][m]) / 2;
                for (int m = 0; m < 59049; m++)
                    pE2XTableC[l][k][m] = (pE2XTable[i][k][m] + pE2XTable[j][k][m]) / 2;
                for (int m = 0; m < 19683; m++)
                    p33TableC[l][k][m] = (p33Table[i][k][m] + p33Table[j][k][m]) / 2;
            }
            l++;
        }
    }

    auto smallRand = bind(uniform_int_distribution<int>(-500, 500), rng);
    auto largeRand = bind(uniform_int_distribution<int>(-5000, 5000), rng);

    for (int i = 0; i < combos; i++) {
        for (int l = 0; l < 1; l++) {
            for (int j = 0; j < DIVS; j++) {
                for (int k = 0; k < 6561; k++)
                    if (edgeTableC[i][j][k])
                        edgeTable[index+OFFSPRING_PER_COMBO*i+l][j][k] = edgeTableC[i][j][k] + smallRand();
                for (int k = 0; k < 6561; k++)
                    if (p24TableC[i][j][k])
                        p24Table[index+OFFSPRING_PER_COMBO*i+l][j][k] = p24TableC[i][j][k] + smallRand();
                for (int k = 0; k < 59049; k++)
                    if (pE2XTableC[i][j][k])
                        pE2XTable[index+OFFSPRING_PER_COMBO*i+l][j][k] = pE2XTableC[i][j][k] + smallRand();
                for (int k = 0; k < 19683; k++)
                    if (p33TableC[i][j][k])
                        p33Table[index+OFFSPRING_PER_COMBO*i+l][j][k] = p33TableC[i][j][k] + smallRand();
            }
        }
        for (int l = 1; l < 2; l++) {
            for (int j = 0; j < DIVS; j++) {
                for (int k = 0; k < 6561; k++)
                    if (edgeTableC[i][j][k])
                        edgeTable[index+OFFSPRING_PER_COMBO*i+l][j][k] = edgeTableC[i][j][k] + largeRand();
                for (int k = 0; k < 6561; k++)
                    if (p24TableC[i][j][k])
                        p24Table[index+OFFSPRING_PER_COMBO*i+l][j][k] = p24TableC[i][j][k] + largeRand();
                for (int k = 0; k < 59049; k++)
                    if (pE2XTableC[i][j][k])
                        pE2XTable[index+OFFSPRING_PER_COMBO*i+l][j][k] = pE2XTableC[i][j][k] + largeRand();
                for (int k = 0; k < 19683; k++)
                    if (p33TableC[i][j][k])
                        p33Table[index+OFFSPRING_PER_COMBO*i+l][j][k] = p33TableC[i][j][k] + largeRand();
            }
        }
    }

    // Free memory
    for (int i = 0; i < combos; i++) {
        for (int j = 0; j < DIVS; j++) {
            delete[] edgeTableC[i][j];
            delete[] p24TableC[i][j];
            delete[] pE2XTableC[i][j];
            delete[] p33TableC[i][j];
        }
    }
    for (int i = 0; i < combos; i++) {
        delete[] edgeTableC[i];
        delete[] p24TableC[i];
        delete[] pE2XTableC[i];
        delete[] p33TableC[i];
    }
    delete[] edgeTableC;
    delete[] p24TableC;
    delete[] pE2XTableC;
    delete[] p33TableC;
}

// Creates the initial set of offspring. These are simply randomly generated
// with no specific rules.
void initializeOffspring() {
    auto uniformRand = bind(uniform_int_distribution<int>(-5000, 5000), rng);

    for (int i = 1; i < NUM_OFFSPRING; i++) {
        for (int j = 0; j < DIVS; j++) {
            for (int k = 0; k < 6561; k++)
                if (edgeTable[0][j][k])
                    edgeTable[i][j][k] = edgeTable[0][j][k] + uniformRand();
            for (int k = 0; k < 6561; k++)
                if (p24Table[0][j][k])
                    p24Table[i][j][k] = p24Table[0][j][k] + uniformRand();
            for (int k = 0; k < 59049; k++)
                if (pE2XTable[0][j][k])
                    pE2XTable[i][j][k] = pE2XTable[0][j][k] + uniformRand();
            for (int k = 0; k < 19683; k++)
                if (p33Table[0][j][k])
                    p33Table[i][j][k] = p33Table[0][j][k] + uniformRand();
        }
    }
}

void checkGames() {
    cout << "Checking game validity: " << totalSize << " games." << endl;
    int errors = 0;
    for(unsigned int i = 0; i < totalSize; i++) {
        thor_game *game = games[i];

        Board tracker;
        int side = CBLACK;

        for(int j = 0; j < 46; j++) {
            if(!tracker.checkMove(game->moves[j], side)) {
                // If one side must pass it is not indicated in the database?
                side = -side;
                if(!tracker.checkMove(game->moves[j], side)) {
                    if (tracker.getLegalMoves(CBLACK).size != 0
                     || tracker.getLegalMoves(CWHITE).size != 0) {
                        errors++;
                        games[i] = NULL;
                        cerr << "error at " << i << " " << j << endl;
                        cerr << game->moves[j-1] << " " << game->moves[j] << " " << game->moves[j+1] << endl;
                        break;
                    }
                }
            }
            tracker.doMove(game->moves[j], side);
            side = -side;
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
        for(int j = 0; j < 46; j++) {
            // If one side must pass it is not indicated in the database?
            if(!tracker.checkMove(game->moves[j], side)) {
                side = -side;
            }
            tracker.doMove(game->moves[j], side);
            side = -side;
        }

        Endgame e;
        if(tracker.countEmpty() > 20) {
            games[i] = NULL;
            continue;
        }

        e.mySide = side;
        MoveList lm = tracker.getLegalMoves(side);
        int score = e.endgame_score(
                tracker, lm, tracker.countEmpty());
        // We want everything from black's POV
        if (side == CWHITE)
            score = -score;
        game->final = (score + 64) / 2;
    }
}

void write() {
    ofstream out;
    out.open("p24table.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 6561; i++) {
            out << ((double) p24Table[0][n][i]) / 10000.0 << endl;
        }
    }
    out.close();

    out.open("edgetable.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 6561; i++) {
            out << ((double) edgeTable[0][n][i]) / 10000.0 << " ";

            if(i%9 == 8) out << endl;
        }
    }
    out.close();

    out.open("pE2Xtable.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 59049; i++) {
            out << ((double) pE2XTable[0][n][i]) / 10000.0 << " ";

            if(i%9 == 8) out << endl;
        }
    }
    out.close();

    out.open("p33table.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 19683; i++) {
            out << ((double) p33Table[0][n][i]) / 10000.0 << " ";

            if(i%9 == 8) out << endl;
        }
    }
    out.close();
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

void boardToEPV(Board *b, int offspringIndex, int score, int turn) {
    int index = (turn - IOFFSET) / TURNSPERDIV;
    bitbrd black = b->toBits(BLACK);
    bitbrd white = b->toBits(WHITE);
    int r2 = bitsToPI( (int)((black >> 8) & 0xFF), (int)((white >> 8) & 0xFF) );
    int r7 = bitsToPI( (int)((black >> 48) & 0xFF), (int)((white >> 48) & 0xFF) );
    int c2 = bitsToPI(
      (int)((((black>>1) & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56),
      (int)((((white>>1) & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56) );
    int c7 = bitsToPI(
      (int)((((black<<1) & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56),
      (int)((((white<<1) & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56) );

    if(!eused[offspringIndex][index][r2]) {
        int64_t error = edgeTable[offspringIndex][index][r2] - score;
        error *= error;
        edgeErrors[offspringIndex] += error;
    }
    if(!eused[offspringIndex][index][r7]) {
        int64_t error = edgeTable[offspringIndex][index][r7] - score;
        error *= error;
        edgeErrors[offspringIndex] += error;
    }
    if(!eused[offspringIndex][index][c2]) {
        int64_t error = edgeTable[offspringIndex][index][c2] - score;
        error *= error;
        edgeErrors[offspringIndex] += error;
    }
    if(!eused[offspringIndex][index][c7]) {
        int64_t error = edgeTable[offspringIndex][index][c7] - score;
        error *= error;
        edgeErrors[offspringIndex] += error;
    }

    eused[offspringIndex][index][r2] = 1;
    eused[offspringIndex][index][r7] = 1;
    eused[offspringIndex][index][c2] = 1;
    eused[offspringIndex][index][c7] = 1;
}

void boardTo24PV(Board *b, int offspringIndex, int score, int turn) {
    int index = (turn - IOFFSET) / TURNSPERDIV;
    bitbrd black = b->toBits(BLACK);
    bitbrd white = b->toBits(WHITE);
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

    if(!used[offspringIndex][index][ul]) {
        int64_t error = p24Table[offspringIndex][index][ul] - score;
        error *= error;
        p24Errors[offspringIndex] += error;
    }
    if(!used[offspringIndex][index][ll]) {
        int64_t error = p24Table[offspringIndex][index][ll] - score;
        error *= error;
        p24Errors[offspringIndex] += error;
    }
    if(!used[offspringIndex][index][ur]) {
        int64_t error = p24Table[offspringIndex][index][ur] - score;
        error *= error;
        p24Errors[offspringIndex] += error;
    }
    if(!used[offspringIndex][index][lr]) {
        int64_t error = p24Table[offspringIndex][index][lr] - score;
        error *= error;
        p24Errors[offspringIndex] += error;
    }
    if(!used[offspringIndex][index][rul]) {
        int64_t error = p24Table[offspringIndex][index][rul] - score;
        error *= error;
        p24Errors[offspringIndex] += error;
    }
    if(!used[offspringIndex][index][rll]) {
        int64_t error = p24Table[offspringIndex][index][rll] - score;
        error *= error;
        p24Errors[offspringIndex] += error;
    }
    if(!used[offspringIndex][index][rur]) {
        int64_t error = p24Table[offspringIndex][index][rur] - score;
        error *= error;
        p24Errors[offspringIndex] += error;
    }
    if(!used[offspringIndex][index][rlr]) {
        int64_t error = p24Table[offspringIndex][index][rlr] - score;
        error *= error;
        p24Errors[offspringIndex] += error;
    }

    used[offspringIndex][index][ul] = 1;
    used[offspringIndex][index][ll] = 1;
    used[offspringIndex][index][ur] = 1;
    used[offspringIndex][index][lr] = 1;
    used[offspringIndex][index][rul] = 1;
    used[offspringIndex][index][rll] = 1;
    used[offspringIndex][index][rur] = 1;
    used[offspringIndex][index][rlr] = 1;
}

void boardToE2XPV(Board *b, int offspringIndex, int score, int turn) {
    int index = (turn - IOFFSET) / TURNSPERDIV;
    bitbrd black = b->toBits(BLACK);
    bitbrd white = b->toBits(WHITE);
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

    if(!exused[offspringIndex][index][r1]) {
        int64_t error = pE2XTable[offspringIndex][index][r1] - score;
        error *= error;
        pE2XErrors[offspringIndex] += error;
    }
    if(!exused[offspringIndex][index][r8]) {
        int64_t error = pE2XTable[offspringIndex][index][r8] - score;
        error *= error;
        pE2XErrors[offspringIndex] += error;
    }
    if(!exused[offspringIndex][index][c1]) {
        int64_t error = pE2XTable[offspringIndex][index][c1] - score;
        error *= error;
        pE2XErrors[offspringIndex] += error;
    }
    if(!exused[offspringIndex][index][c8]) {
        int64_t error = pE2XTable[offspringIndex][index][c8] - score;
        error *= error;
        pE2XErrors[offspringIndex] += error;
    }

    exused[offspringIndex][index][r1] = 1;
    exused[offspringIndex][index][r8] = 1;
    exused[offspringIndex][index][c1] = 1;
    exused[offspringIndex][index][c8] = 1;
}

void boardTo33PV(Board *b, int offspringIndex, int score, int turn) {
    int index = (turn - IOFFSET) / TURNSPERDIV;
    bitbrd black = b->toBits(BLACK);
    bitbrd white = b->toBits(WHITE);
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

    if(!p33used[offspringIndex][index][ul]) {
        int64_t error = p33Table[offspringIndex][index][ul] - score;
        error *= error;
        p33Errors[offspringIndex] += error;
    }
    if(!p33used[offspringIndex][index][ur]) {
        int64_t error = p33Table[offspringIndex][index][ur] - score;
        error *= error;
        p33Errors[offspringIndex] += error;
    }
    if(!p33used[offspringIndex][index][ll]) {
        int64_t error = p33Table[offspringIndex][index][ll] - score;
        error *= error;
        p33Errors[offspringIndex] += error;
    }
    if(!p33used[offspringIndex][index][lr]) {
        int64_t error = p33Table[offspringIndex][index][lr] - score;
        error *= error;
        p33Errors[offspringIndex] += error;
    }

    p33used[offspringIndex][index][ul] = 1;
    p33used[offspringIndex][index][ur] = 1;
    p33used[offspringIndex][index][ll] = 1;
    p33used[offspringIndex][index][lr] = 1;
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

void readEdgeTable() {
    std::string line;
    std::string file;
        file = "edgetable.txt";
    std::ifstream edgetable(file);

    if(edgetable.is_open()) {
        for(int n = 0; n < DIVS; n++) {
            for(int i = 0; i < 729; i++) {
                getline(edgetable, line);
                for(int j = 0; j < 9; j++) {
                    std::string::size_type sz = 0;
                    edgeTable[0][n][9*i+j] = (int) (10000.0 * std::stod(line, &sz));
                    line = line.substr(sz);
                }
            }
        }

        edgetable.close();
    }
}

void readPattern24Table() {
    std::string line;
    std::string file;
        file = "p24table.txt";
    std::ifstream p24table(file);

    if(p24table.is_open()) {
        for(int n = 0; n < DIVS; n++) {
            for(int i = 0; i < 6561; i++) {
                getline(p24table, line);
                std::string::size_type sz = 0;
                p24Table[0][n][i] = (int) (10000.0 * std::stod(line, &sz));
            }
        }
        p24table.close();
    }
}

void readPatternE2XTable() {
    std::string line;
    std::string file;
        file = "pE2Xtable.txt";
    std::ifstream pE2Xtable(file);

    if(pE2Xtable.is_open()) {
        for(int n = 0; n < DIVS; n++) {
            for(int i = 0; i < 6561; i++) {
                getline(pE2Xtable, line);
                for(int j = 0; j < 9; j++) {
                    std::string::size_type sz = 0;
                    pE2XTable[0][n][9*i+j] = (int) (10000.0 * std::stod(line, &sz));
                    line = line.substr(sz);
                }
            }
        }
        pE2Xtable.close();
    }
}

void readPattern33Table() {
    std::string line;
    std::string file;
        file = "p33table.txt";
    std::ifstream p33table(file);

    if(p33table.is_open()) {
        for(int n = 0; n < DIVS; n++) {
            for(int i = 0; i < 2187; i++) {
                getline(p33table, line);
                for(int j = 0; j < 9; j++) {
                    std::string::size_type sz = 0;
                    p33Table[0][n][9*i+j] = (int) (10000.0 * std::stod(line, &sz));
                    line = line.substr(sz);
                }
            }
        }
        p33table.close();
    }
}

void initializeArrays() {
    edgeTable = new int **[NUM_OFFSPRING];
    p24Table = new int **[NUM_OFFSPRING];
    pE2XTable = new int **[NUM_OFFSPRING];
    p33Table = new int **[NUM_OFFSPRING];
    used = new int **[NUM_OFFSPRING];
    eused = new int **[NUM_OFFSPRING];
    exused = new int **[NUM_OFFSPRING];
    p33used = new int **[NUM_OFFSPRING];

    for (int i = 0; i < NUM_OFFSPRING; i++) {
        edgeTable[i] = new int *[DIVS];
        p24Table[i] = new int *[DIVS];
        pE2XTable[i] = new int *[DIVS];
        p33Table[i] = new int *[DIVS];
        used[i] = new int *[DIVS];
        eused[i] = new int *[DIVS];
        exused[i] = new int *[DIVS];
        p33used[i] = new int *[DIVS];
    }
    for (int i = 0; i < NUM_OFFSPRING; i++) {
        for (int j = 0; j < DIVS; j++) {
            edgeTable[i][j] = new int[6561];
            p24Table[i][j] = new int[6561];
            pE2XTable[i][j] = new int[59049];
            p33Table[i][j] = new int[19683];
            used[i][j] = new int[6561];
            eused[i][j] = new int[6561];
            exused[i][j] = new int[59049];
            p33used[i][j] = new int[19683];
        }
    }
}

// Delete all entries including and after index
void freemem(int index) {
    for (int i = index; i < NUM_OFFSPRING; i++) {
        for (int j = 0; j < DIVS; j++) {
            delete[] edgeTable[i][j];
            delete[] p24Table[i][j];
            delete[] pE2XTable[i][j];
            delete[] p33Table[i][j];
            //delete[] used[i][j];
            //delete[] eused[i][j];
            //delete[] exused[i][j];
            //delete[] p33used[i][j];
        }
    }

    for (int i = index; i < NUM_OFFSPRING; i++) {
        delete[] edgeTable[i];
        delete[] p24Table[i];
        delete[] pE2XTable[i];
        delete[] p33Table[i];
        //delete[] used[i];
        //delete[] eused[i];
        //delete[] exused[i];
        //delete[] p33used[i];
    }
}
