#include "player.h"

/**
 * @brief Constructor for the player.
 * 
 * This constructor initializes the depths, timing variables, and the array
 * used to convert move indices to move objects.
 * 
 * @param side The side the AI is playing as.
 */
Player::Player(Side side) {
    maxDepth = 14;
    minDepth = 6;
    sortDepth = 4;
    endgameDepth = 20;
    if(side == WHITE)
        endgameDepth--;
    endgameSwitch = false;

    mySide = (side == BLACK) ? CBLACK : CWHITE;
    endgameSolver.mySide = mySide;
    oppSide = (side == WHITE) ? CBLACK : CWHITE;
    turn = 4;
    totalTimePM = -2;
    endgameTimeMS = 0;

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            indexToMove[i+8*j] = new Move(i,j);
        }
    }

    #if defined(__x86_64__)
        cerr << "x86-64 processor detected." << endl;
    #elif defined(__i386)
        cerr << "x86 processor detected." << endl;
    #else
        cerr << "non-x86 processor detected." << endl;
    #endif

    readEdgeTable();
}

/**
 * @brief Destructor for the player.
 */
Player::~Player() {
    for(int i = 0; i < 64; i++) {
        delete indexToMove[i];
    }
}

/**
 * @brief Processes opponent's last move and selects a best move to play.
 * 
 * This function delegates all necessary tasks to the appropriate helper
 * functions. It first processes the opponent's move. It then checks the opening
 * book for a move, then the endgame solver, and finally begins an iterative
 * deepening principal variation null window search.
 * 
 * @param opponentsMove The last move the opponent made.
 * @param msLeft Total milliseconds left for the game, -1 if untimed.
 * @return The move the AI chose to play.
 */
Move *Player::doMove(Move *opponentsMove, int msLeft) {
    // timing
    if(totalTimePM == -2) {
        totalTimePM = msLeft;
        endgameTimeMS = msLeft / 3;
        endgameSolver.endgameTimeMS = endgameTimeMS;
        if(totalTimePM != -1) {
            if(totalTimePM > 600000)
                totalTimePM = (totalTimePM - endgameTimeMS) / 21;
            else if(totalTimePM > 300000) {
                totalTimePM = (totalTimePM - endgameTimeMS) / 24;
                endgameDepth = 16;
            }
            else {
                totalTimePM = (totalTimePM - endgameTimeMS) / 26;
                endgameDepth = 14;
            }
        }
        else {
            totalTimePM = 1000000;
            endgameTimeMS = 1000000;
            endgameSolver.endgameTimeMS = endgameTimeMS;
        }
    }

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();
    auto end_time = high_resolution_clock::now();
    duration<double> time_span;

    if(opponentsMove != NULL) {
        game.doMove(opponentsMove->getX() + 8*opponentsMove->getY(), oppSide);
        turn++;
    }
    else {
        game.doMove(MOVE_NULL, oppSide);
        if(endgameSwitch)
            endgameDepth++;
    }

    // check opening book
    #if USE_OPENING_BOOK
    int openMove = openingBook.get(game.getTaken(), game.getBlack());
    if(openMove != OPENING_NOT_FOUND) {
        cerr << "Opening book used! Played " << openMove << endl;
        turn++;
        game.doMove(openMove, mySide);
        return indexToMove[openMove];
    }
    #endif

    // find and test all legal moves
    MoveList legalMoves = game.getLegalMoves(mySide);

    if (legalMoves.size <= 0) {
        game.doMove(MOVE_NULL, mySide);
        cerr << "No legal moves. Passing." << endl;
        return NULL;
    }

    int myMove = -1;
    MoveList scores;

    // solve for game result
    /*while(turn >= (64 - endgameDepth - 2) && turn < (64 - endgameDepth)) {
        if(msLeft < endgameTimeMS && msLeft != -1)
            break;

        cerr << "Game result solver: depth " << endgameDepth+2 << endl;

        myMove = endgameSolver.result_solve(game, legalMoves, endgameDepth+2);
        if(myMove == MOVE_BROKEN) {
            cerr << "Broken out of result solver." << endl;
            break;
        }

        game.doMove(myMove, mySide);
        turn++;

        end_time = high_resolution_clock::now();
        time_span = duration_cast<duration<double>>(end_time-start_time);
        cerr << "Result solver took: " << time_span.count() << endl;

        return indexToMove[myMove];
    }*/

    // endgame solver
    while(endgameSwitch || turn >= (64 - endgameDepth)) {
        if(msLeft < endgameTimeMS && msLeft != -1) {
            endgameSwitch = false;
            endgameDepth -= 2;
            break;
        }
        cerr << "Endgame solver: depth " << endgameDepth << endl;
        //pvs(&game, legalMoves, scores, mySide, sortDepth, NEG_INFTY, INFTY);
        //sort(legalMoves, scores, 0, legalMoves.size()-1);
        //scores.clear();

        endgameSwitch = true;
        myMove = endgameSolver.endgame(game, legalMoves, endgameDepth);
        if(myMove == MOVE_BROKEN) {
            cerr << "Broken out of endgame solver." << endl;
            endgameDepth -= 2;
            break;
        }

        endgameDepth -= 2;
        game.doMove(myMove, mySide);
        turn++;

        end_time = high_resolution_clock::now();
        time_span = duration_cast<duration<double>>(end_time-start_time);
        cerr << "Endgame took: " << time_span.count() << endl;

        return indexToMove[myMove];
    }

    // sort search
    cerr << "Performing sort search: depth " << sortDepth << endl;
    pvs(&game, legalMoves, scores, mySide, sortDepth, NEG_INFTY, INFTY);
    sort(legalMoves, scores, 0, legalMoves.size-1);
    scores.clear();

    // iterative deepening
    attemptingDepth = minDepth;
    int chosenScore = 0;
    do {
        cerr << "Searching depth " << attemptingDepth << endl;

        int newBest = pvs(&game, legalMoves, scores, mySide,
            attemptingDepth, NEG_INFTY, INFTY);
        if(newBest == MOVE_BROKEN) {
            cerr << "Broken out of search" << endl;
            break;
        }
        myMove = newBest;
        attemptingDepth += 2;

        sort(legalMoves, scores, 0, legalMoves.size-1);
        chosenScore = scores.get(0);
        scores.clear();

        end_time = high_resolution_clock::now();
        time_span = duration_cast<duration<double>>(end_time-start_time);
    } while( (
        ((msLeft-endgameTimeMS)/(64-endgameDepth-turn) > time_span.count()*1000.0*20) || msLeft == -1) && attemptingDepth <= maxDepth );

    game.doMove(myMove, mySide);
    turn++;

    cerr << "Playing " << myMove << ". Score: " << chosenScore << endl;
    killer_table.clean(turn);
    cerr << "Table contains " << killer_table.keys << " keys." << endl;
    cerr << endl;

    return indexToMove[myMove];
}

/**
 * @brief Performs a principal variation null-window search.
*/
int Player::pvs(Board *b, MoveList &moves, MoveList &scores, int s,
    int depth, int alpha, int beta) {

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    int score;
    int tempMove = moves.get(0);

    for (unsigned int i = 0; i < moves.size; i++) {
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(
            end_time-start_time);

        if(time_span.count() * moves.size * 1000 > totalTimePM * (i+1))
            return MOVE_BROKEN;

        Board copy = Board(b->taken, b->black, b->legal);
        copy.doMove(moves.get(i), s);
        int ttScore = NEG_INFTY;
        if (i != 0) {
            score = -pvs_h(&copy, ttScore, -s, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -pvs_h(&copy, ttScore, -s, depth-1, -beta, -score);
            }
        }
        else {
            score = -pvs_h(&copy, ttScore, -s, depth-1, -beta, -alpha);
        }
        scores.add(ttScore);
        if (score > alpha) {
            alpha = score;
            tempMove = moves.get(i);
        }
        if (alpha >= beta)
            break;
    }
    return tempMove;
}

/**
 * @brief Helper function for the principal variation search.
*/
int Player::pvs_h(Board *b, int &topScore, int s, int depth,
    int alpha, int beta) {

    if (depth <= 0) {
        topScore = (s == mySide) ? heuristic(b) : -heuristic(b);
        return topScore;
    }

    int score;
    int ttScore = NEG_INFTY;

    int killerMove = killer_table.get(b);
    if(killerMove != -1) {
        Board copy = Board(b->taken, b->black, b->legal);
        copy.doMove(killerMove, s);
        score = -pvs_h(&copy, ttScore, -s, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        if(ttScore > topScore)
            topScore = ttScore;
        if (alpha >= beta)
            return alpha;
    }

    MoveList legalMoves = b->getLegalMoves(s);
    if(legalMoves.size <= 0) {
        Board copy = Board(b->taken, b->black, b->legal);
        copy.doMove(MOVE_NULL, s);
        score = -pvs_h(&copy, ttScore, -s, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        if(ttScore > topScore)
            topScore = ttScore;
        return alpha;
    }

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b->taken, b->black, b->legal);
        copy.doMove(legalMoves.get(i), s);
        if (i != 0) {
            score = -pvs_h(&copy, ttScore, -s, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -pvs_h(&copy, ttScore, -s, depth-1, -beta, -alpha);
            }
        }
        else {
            score = -pvs_h(&copy, ttScore, -s, depth-1, -beta, -alpha);
        }
        if (alpha < score)
            alpha = score;
        if(ttScore > topScore)
            topScore = ttScore;
        if (alpha >= beta) {
            if(depth >= 3)
                killer_table.add(b, legalMoves.get(i),
                    turn+attemptingDepth-depth);
            break;
        }
    }
    return alpha;
}

int Player::heuristic (Board *b) {
    int score;
    int myCoins = b->count(mySide);
    if(myCoins == 0)
        return -9001;

    if(turn < 40)
        score = myCoins - b->count(oppSide);
    else
        score = 2 * (myCoins - b->count(oppSide));

    bitbrd bm = b->toBits(mySide);
    bitbrd bo = b->toBits(oppSide);
    #if USE_EDGE_TABLE
        score += (mySide == BLACK) ? 5*boardToPV(b) : -5*boardToPV(b);
        score -= 22 * (countSetBits(bm&X_CORNERS) - countSetBits(bo&X_CORNERS));
    #else
        score += 50 * (countSetBits(bm&CORNERS) - countSetBits(bo&CORNERS));
        //if(turn > 35)
        //    score += 3 * (countSetBits(bm&EDGES) - countSetBits(bo&EDGES));
        score -= 12 * (countSetBits(bm&X_CORNERS) - countSetBits(bo&X_CORNERS));
        score -= 10 * (countSetBits(bm&ADJ_CORNERS) -
            countSetBits(bo&ADJ_CORNERS));
    #endif

    //score += 9 * (b->numLegalMoves(mySide) - b->numLegalMoves(oppSide));
    int myLM = b->numLegalMoves(mySide);
    int oppLM = b->numLegalMoves(oppSide);
    score += 100 * (myLM - oppLM) / (myLM + oppLM + 1);
    score += 6 * (b->potentialMobility(mySide) - b->potentialMobility(oppSide));

    return score;
}

int Player::countSetBits(bitbrd i) {
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

int Player::boardToPV(Board *b) {
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
    int result = edgeTable[r1] + edgeTable[r8] + edgeTable[c1] + edgeTable[c8];
    return result;
}

int Player::bitsToPI(int b, int w) {
    return PIECES_TO_INDEX[(int)b] + 2*PIECES_TO_INDEX[(int)w];
}

void Player::sort(MoveList &moves, MoveList &scores, int left, int right) {
    int index = left;

    if (left < right) {
        index = partition(moves, scores, left, right, index);
        sort(moves, scores, left, index-1);
        sort(moves, scores, index+1, right);
    }
}

void Player::swap(MoveList &moves, MoveList &scores, int i, int j) {
    int less1 = moves.get(j);
    moves.set(j, moves.get(i));
    moves.set(i, less1);

    int less2 = scores.get(j);
    scores.set(j, scores.get(i));
    scores.set(i, less2);
}

int Player::partition(MoveList &moves, MoveList &scores, int left, int right,
    int pindex) {

    int index = left;
    int pivot = scores.get(pindex);

    swap(moves, scores, pindex, right);

    for (int i = left; i < right; i++) {
        if (scores.get(i) >= pivot) {
            swap(moves, scores, i, index);
            index++;
        }
    }
    swap(moves, scores, index, right);

    return index;
}

void Player::readEdgeTable() {
    std::string line;
    std::string file;
        file = "edgetable.txt";
    std::ifstream edgetable(file);

    if(edgetable.is_open()) {
        int i = 0;
        while(getline(edgetable, line)) {
            for(int j = 0; j < 9; j++) {
                std::string::size_type sz = 0;
                edgeTable[9*i+j] = std::stoi(line, &sz, 0);
                line = line.substr(sz);
            }

            i++;
        }
        edgetable.close();
    }
}
