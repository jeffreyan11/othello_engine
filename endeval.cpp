#include <cstdlib>
#include <fstream>
#include <iostream>
#include "board.h"
#include "common.h"
#include "endgame.h"
#include "patternbuilder.h"
using namespace std;

#define DIVS 1
#define IOFFSET 1
#define TURNSPERDIV 64

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

Eval evaluater;
const int startIndex = 42;

void readGame(string file, unsigned int n);
void writeFile();
void freemem();
void resetgames();
void boardToEPV(Board *b, int score, int turn);
void boardTo24PV(Board *b, int score, int turn);
void boardToE2XPV(Board *b, int score, int turn);
void boardTo33PV(Board *b, int score, int turn);
int bitsToPI(int b, int w);

void replaceEnd() {
    for(unsigned int i = 0; i < totalSize - 16400; i++) {
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
        if(tracker.countEmpty() > 22) {
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
            int best = e.solveEndgame(tracker, lm, false, side,
                tracker.countEmpty(), 10000000, &evaluater, &score);

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

        int score = 2*game->final - 64;
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
    games = new thor_game*[159000];
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

    readThorGame("WTH_7708/Logistello_book_1999.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2013.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2012.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2011.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2010.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2009.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2008.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2007.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2006.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2005.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2004.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2003.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2002.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2001.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_2000.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1999.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1998.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1997.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1996.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1995.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1994.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1993.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1992.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1991.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1990.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1989.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1988.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1987.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1986.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1985.wtb", totalSize, games);
    readThorGame("WTH_7708/WTH_1984.wtb", totalSize, games);
    readGame("WTH_7708/tuneoutput-8-19-15.txt", 16400);

    checkGames(totalSize, games);

    replaceEnd();

    searchFeatures();

    writeFile();

    freemem();
    return 0;
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
                double to = ((double)(a->sum))/((double)(a->instances));
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
                double to = ((double)(a->sum))/((double)(a->instances));
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
                double to = ((double)(a->sum))/((double)(a->instances));
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
                double to = ((double)(a->sum))/((double)(a->instances));
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
