#include <fstream>
using namespace std;

#define DIVS 4
#define IOFFSET 10
#define TURNSPERDIV 9

double **edgeTable;
double **p24Table;
double **pE2XTable;
double **p33Table;
double **edgeTableb;
double **p24Tableb;
double **pE2XTableb;
double **p33Tableb;

void readEdgeTable();
void readPattern24Table();
void readPatternE2XTable();
void readPattern33Table();
void freemem();

void blur() {
    for(int n = 0; n < DIVS; n++) {
        if(n == 0) {
            for(int i = 0; i < 6561; i++) {
                edgeTableb[n][i] = (2*edgeTable[n][i] + edgeTable[n+1][i])/3;
                p24Tableb[n][i] = (2*p24Table[n][i] + p24Table[n+1][i])/3;
            }
            for(int i = 0; i < 59049; i++) {
                pE2XTableb[n][i] = (2*pE2XTable[n][i] + pE2XTable[n+1][i])/3;
            }
            for(int i = 0; i < 19683; i++) {
                p33Tableb[n][i] = (2*p33Table[n][i] + p33Table[n+1][i])/3;
            }
        }
        else if(n == DIVS-1) {
            for(int i = 0; i < 6561; i++) {
                edgeTableb[n][i] = (2*edgeTable[n][i] + edgeTable[n-1][i])/3;
                p24Tableb[n][i] = (2*p24Table[n][i] + p24Table[n-1][i])/3;
            }
            for(int i = 0; i < 59049; i++) {
                pE2XTableb[n][i] = (2*pE2XTable[n][i] + pE2XTable[n-1][i])/3;
            }
            for(int i = 0; i < 19683; i++) {
                p33Tableb[n][i] = (2*p33Table[n][i] + p33Table[n-1][i])/3;
            }
        }
        else {
            for(int i = 0; i < 6561; i++) {
                edgeTableb[n][i] = (edgeTable[n-1][i] + 2*edgeTable[n][i] + edgeTable[n+1][i])/4;
                p24Tableb[n][i] = (p24Table[n-1][i] + 2*p24Table[n][i] + p24Table[n+1][i])/4;
            }
            for(int i = 0; i < 59049; i++) {
                pE2XTableb[n][i] = (pE2XTable[n-1][i] + 2*pE2XTable[n][i] + pE2XTable[n+1][i])/4;
            }
            for(int i = 0; i < 19683; i++) {
                p33Tableb[n][i] = (p33Table[n-1][i] + 2*p33Table[n][i] + p33Table[n+1][i])/4;
            }
        }
    }
}

void write() {
    ofstream out;
    out.open("p24table.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 6561; i++) {
            out << (int) (p24Tableb[n][i] * 100.0) << endl;
        }
    }
    out.close();

    out.open("edgetable.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 6561; i++) {
            out << (int) (edgeTableb[n][i] * 100.0) << " ";

            if(i%9 == 8) out << endl;
        }
    }
    out.close();

    out.open("pE2Xtable.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 59049; i++) {
            out << (int) (pE2XTableb[n][i] * 100.0) << " ";

            if(i%9 == 8) out << endl;
        }
    }
    out.close();

    out.open("p33table.txt");
    for(int n = 0; n < DIVS; n++) {
        for(unsigned int i = 0; i < 19683; i++) {
            out << (int) (p33Tableb[n][i] * 100.0) << " ";

            if(i%9 == 8) out << endl;
        }
    }
    out.close();
}

int main(int argc, char **argv) {
    edgeTable = new double *[DIVS];
    p24Table = new double *[DIVS];
    pE2XTable = new double *[DIVS];
    p33Table = new double *[DIVS];

    for(int i = 0; i < DIVS; i++) {
        edgeTable[i] = new double[6561];
        p24Table[i] = new double[6561];
        pE2XTable[i] = new double[59049];
        p33Table[i] = new double[19683];
    }

    edgeTableb = new double *[DIVS];
    p24Tableb = new double *[DIVS];
    pE2XTableb = new double *[DIVS];
    p33Tableb = new double *[DIVS];

    for(int i = 0; i < DIVS; i++) {
        edgeTableb[i] = new double[6561];
        p24Tableb[i] = new double[6561];
        pE2XTableb[i] = new double[59049];
        p33Tableb[i] = new double[19683];
    }

    readEdgeTable();
    readPattern24Table();
    readPatternE2XTable();
    readPattern33Table();

    blur();
    write();

    freemem();

    return 0;
}

void freemem() {
    for(int i = 0; i < DIVS; i++) {
        delete[] edgeTable[i];
        delete[] p24Table[i];
        delete[] pE2XTable[i];
    }
    delete[] edgeTable;
    delete[] p24Table;
    delete[] pE2XTable;

    for(int i = 0; i < DIVS; i++) {
        delete[] edgeTableb[i];
        delete[] p24Tableb[i];
        delete[] pE2XTableb[i];
    }
    delete[] edgeTableb;
    delete[] p24Tableb;
    delete[] pE2XTableb;
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
                    edgeTable[n][9*i+j] = std::stod(line, &sz);
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
                p24Table[n][i] = std::stod(line, &sz);
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
                    pE2XTable[n][9*i+j] = std::stod(line, &sz);
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
                    p33Table[n][9*i+j] = std::stod(line, &sz);
                    line = line.substr(sz);
                }
            }
        }
        p33table.close();
    }
}
