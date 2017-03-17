#ifndef __EVAL_H__
#define __EVAL_H__

#include <string>
#include "board.h"

const int TSPLITS = 4;
const int IOFFSET = 5;
const int TURNSPERDIV = 15;

class Eval {
private:
    int **edgeTable;
    int **p24Table;
    int **pE2XTable;
    int **p33Table;
    int **line3Table;
    int **line4Table;
    int **diag8Table;
    int *s44Table;

    int boardToEPV(Board &b, int turn);
    int boardTo24PV(Board &b, int turn);
    int boardToE2XPV(Board &b, int turn);
    int boardTo33PV(Board &b, int turn);
    int boardToLine3PV(Board &b, int turn);
    int boardToLine4PV(Board &b, int turn);
    int boardToDiag8PV(Board &b, int turn);
    int boardTo44SV(Board &b, int s);
    int bitsToPI(int b, int w);

    void readTable(std::string fileName, int lines, int **tableArray);
    void readStability44Table();

public:
    Eval();
    ~Eval();

    int heuristic(Board &b, int s);
    int heuristic2(Board &b, int s);
    int end_heuristic(Board &b);
    int stability(Board &b, int s);
};

#endif
