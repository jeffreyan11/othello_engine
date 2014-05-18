#include "eval.h"

Eval::Eval(int s) {
    mySide = s;
    oppSide = -s;

    edgeTable = new int *[TSPLITS];
    p24Table = new int *[TSPLITS];
    pE2XTable = new int *[TSPLITS];

    for(int i = 0; i < TSPLITS; i++) {
        edgeTable[i] = new int[6561];
        p24Table[i] = new int[6561];
        pE2XTable[i] = new int[59049];
    }

    s33Table = new int[19683];

    readEdgeTable();
    readStability33Table();
    readPattern24Table();
    readPatternE2XTable();
}

Eval::~Eval() {
    for(int i = 0; i < TSPLITS; i++) {
        delete[] edgeTable[i];
        delete[] p24Table[i];
        delete[] pE2XTable[i];
    }

    delete[] edgeTable;
    delete[] s33Table;
    delete[] p24Table;
    delete[] pE2XTable;
}

int Eval::heuristic(Board *b, int turn) {
    int score;
    int myCoins = b->count(mySide);
    if(myCoins == 0)
        return -9001;

    if(turn < 20)
        score = 2*(b->count(oppSide) - myCoins);
    else
        score = myCoins - b->count(oppSide);

    #if USE_EDGE_TABLE
    int patterns = 3*boardTo24PV(b, turn) + 2*boardToEPV(b, turn)
            + 2*boardToE2XPV(b, turn) + 3*boardTo33PV(b);
    if(mySide == CBLACK)
        score += patterns;
    else
        score -= patterns;
    #else
    bitbrd bm = b->toBits(mySide);
    bitbrd bo = b->toBits(oppSide);
    score += 50 * (countSetBits(bm&CORNERS) - countSetBits(bo&CORNERS));
    //if(turn > 35)
    //    score += 3 * (countSetBits(bm&EDGES) - countSetBits(bo&EDGES));
    score -= 12 * (countSetBits(bm&X_CORNERS) - countSetBits(bo&X_CORNERS));
    score -= 10 * (countSetBits(bm&ADJ_CORNERS) - countSetBits(bo&ADJ_CORNERS));
    #endif

    //score += 9 * (b->numLegalMoves(mySide) - b->numLegalMoves(oppSide));
    int myLM = b->numLegalMoves(mySide);
    int oppLM = b->numLegalMoves(oppSide);
    score += 80 * (myLM - oppLM) / (myLM + oppLM + 1);
    score += 4 * (b->potentialMobility(mySide) - b->potentialMobility(oppSide));

    return score;
}

int Eval::end_heuristic(Board *b) {
    int score;
    int myCoins = b->count(mySide);
    if(myCoins == 0)
        return -9001;

    score = 2*(myCoins - b->count(oppSide));

    //score += (mySide == BLACK) ? 3*boardTo24PV(b) : -3*boardTo24PV(b);
    score += (mySide == BLACK) ? boardToEPV(b, 50) : -boardToEPV(b, 50);
    //score += (mySide == BLACK) ? 2*boardToE2XPV(b) : -2*boardToE2XPV(b);
    score += (mySide == BLACK) ? 6*boardTo33PV(b) : -6*boardTo33PV(b);

    score += 5 * (b->numLegalMoves(mySide) - b->numLegalMoves(oppSide));
    //score += 10 * (15 - b->numLegalMoves(oppSide));
    score += 6 * (b->potentialMobility(mySide) - b->potentialMobility(oppSide));

    return score;
}

int Eval::countSetBits(bitbrd i) {
    #if defined(__x86_64__)
        asm ("popcnt %1, %0" : "=r" (i) : "r" (i));
        return (int) i;
    #elif defined(__i386)
        int a = (int) (i & 0xFFFFFFFF);
        int b = (int) ((i>>32) & 0xFFFFFFFF);
        asm ("popcntl %1, %0" : "=r" (a) : "r" (a));
        asm ("popcntl %1, %0" : "=r" (b) : "r" (b));
        return a+b;
    #else
        i = i - ((i >> 1) & 0x5555555555555555);
        i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333);
        i = (((i + (i >> 4)) & 0x0F0F0F0F0F0F0F0F) *
              0x0101010101010101) >> 56;
        return (int) i;
    #endif
}

bitbrd Eval::reflectVertical(bitbrd i) {
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

bitbrd Eval::reflectHorizontal(bitbrd x) {
    const bitbrd k1 = 0x5555555555555555;
    const bitbrd k2 = 0x3333333333333333;
    const bitbrd k4 = 0x0f0f0f0f0f0f0f0f;
    x = ((x >> 1) & k1) | ((x & k1) << 1);
    x = ((x >> 2) & k2) | ((x & k2) << 2);
    x = ((x >> 4) & k4) | ((x & k4) << 4);
    return x;
}

bitbrd Eval::reflectDiag(bitbrd x) {
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

int Eval::boardToEPV(Board *b, int turn) {
    int index = (turn - IOFFSET) / TURNSPERDIV;
    bitbrd black = b->toBits(BLACK);
    bitbrd white = b->toBits(WHITE);
    int r1 = bitsToPI( (int)(black & 0xFF), (int)(white & 0xFF) );
    int r8 = bitsToPI( (int)(black>>56), (int)(white>>56) );
    int c1 = bitsToPI(
      (int)(((black & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56),
      (int)(((white & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56) );
    int c8 = bitsToPI(
      (int)(((black & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56),
      (int)(((white & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56) );
    int result = edgeTable[index][r1] + edgeTable[index][r8] +
            edgeTable[index][c1] + edgeTable[index][c8];
    return result;
}

int Eval::boardTo33PV(Board *b) {
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

    int result = s33Table[ul] + s33Table[ll] + s33Table[ur] + s33Table[lr];
    return result;
}

int Eval::boardTo24PV(Board *b, int turn) {
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

    return p24Table[index][ul] + p24Table[index][ll] + p24Table[index][ur] +
        p24Table[index][lr] + p24Table[index][rul] + p24Table[index][rll] +
        p24Table[index][rur] + p24Table[index][rlr];
}

int Eval::boardToE2XPV(Board *b, int turn) {
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

    int result = pE2XTable[index][r1] + pE2XTable[index][r8] +
            pE2XTable[index][c1] + pE2XTable[index][c8];
    return result;
}

int Eval::bitsToPI(int b, int w) {
    return PIECES_TO_INDEX[b] + 2*PIECES_TO_INDEX[w];
}

void Eval::readEdgeTable() {
    std::string line;
    std::string file;
        file = "patterns/edgetable.txt";
    std::ifstream edgetable(file);

    if(edgetable.is_open()) {
        for(int n = 0; n < TSPLITS; n++) {
            for(int i = 0; i < 729; i++) {
                getline(edgetable, line);
                for(int j = 0; j < 9; j++) {
                    std::string::size_type sz = 0;
                    edgeTable[n][9*i+j] = std::stoi(line, &sz, 0);
                    line = line.substr(sz);
                }
            }
        }

        edgetable.close();
    }
}

void Eval::readStability33Table() {
    std::string line;
    std::string file;
        file = "patterns/s33table.txt";
    std::ifstream s33table(file);

    if(s33table.is_open()) {
        for(int i = 0; i < 729; i++) {
            getline(s33table, line);
            for(int j = 0; j < 27; j++) {
                std::string::size_type sz = 0;
                s33Table[27*i+j] = std::stoi(line, &sz, 0);
                line = line.substr(sz);
            }
        }
        s33table.close();
    }
}

void Eval::readPattern24Table() {
    std::string line;
    std::string file;
        file = "patterns/p24table.txt";
    std::ifstream p24table(file);

    if(p24table.is_open()) {
        for(int n = 0; n < TSPLITS; n++) {
            for(int i = 0; i < 6561; i++) {
                getline(p24table, line);
                std::string::size_type sz = 0;
                p24Table[n][i] = std::stoi(line, &sz, 0);
            }
        }
        p24table.close();
    }
}

void Eval::readPatternE2XTable() {
    std::string line;
    std::string file;
        file = "patterns/pE2Xtable.txt";
    std::ifstream pE2Xtable(file);

    if(pE2Xtable.is_open()) {
        for(int n = 0; n < TSPLITS; n++) {
            for(int i = 0; i < 6561; i++) {
                getline(pE2Xtable, line);
                for(int j = 0; j < 9; j++) {
                    std::string::size_type sz = 0;
                    pE2XTable[n][9*i+j] = std::stoi(line, &sz, 0);
                    line = line.substr(sz);
                }
            }
        }
        pE2Xtable.close();
    }
}
