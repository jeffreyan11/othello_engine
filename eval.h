#ifndef __EVAL_H__
#define __EVAL_H__

#include <string>
#include "board.h"

#define USE_EDGE_TABLE true

const int TSPLITS = 4;
const int IOFFSET = 6;
const int TURNSPERDIV = 13;

const bitbrd CORNERS = 0x8100000000000081;
const bitbrd EDGES = 0x3C0081818181003C;
const bitbrd ADJ_CORNERS = 0x4281000000008142;
const bitbrd X_CORNERS = 0x0042000000004200;

class Eval {
private:
    int **edgeTable;
    int **p24Table;
    int **pE2XTable;
    int **p33Table;
    int *s44Table;

    bitbrd reflectVertical(bitbrd i);
    bitbrd reflectHorizontal(bitbrd x);
    bitbrd reflectDiag(bitbrd x);

    int boardToEPV(Board &b, int turn);
    int boardTo24PV(Board &b, int turn);
    int boardToE2XPV(Board &b, int turn);
    int boardTo33PV(Board &b, int turn);
    int boardTo44SV(Board &b, int s);
    int bitsToPI(int b, int w);

    void readTable(std::string fileName, int lines, int **tableArray);
    void readStability44Table();
    void readEndTable(std::string fileName, int lines, int **tableArray);

public:
    Eval();
    ~Eval();

    int heuristic(Board &b, int turn, int s);
    int heuristic2(Board &b, int turn, int s);
    int end_heuristic(Board &b);
    int stability(Board &b, int s);
};

#endif
