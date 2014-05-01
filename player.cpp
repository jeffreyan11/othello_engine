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
    maxDepth = 12;
    minDepth = 6;
    sortDepth = 4;
    endgameDepth = 20;

    mySide = (side == BLACK) ? CBLACK : CWHITE;
    endgameSolver.mySide = mySide;
    oppSide = (side == WHITE) ? CBLACK : CWHITE;
    turn = 4;
    totalTimePM = -2;

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
    readStability33Table();
    readPattern24Table();
    readPatternE2XTable();
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
        if(totalTimePM != -1) {
            totalTimePM /= 24;
        }
        else {
            totalTimePM = 10000000;
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
    }
    int empties = 64 - countSetBits(game.getTaken());
    cerr << endl;

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

    // endgame solver
    if(empties <= endgameDepth &&
            (msLeft >= endgameTime[empties] || msLeft == -1)) {
        // timing
        endgameSolver.endgameTimeMS = (msLeft + endgameTime[empties]) / 4;
        if(msLeft == -1)
            endgameSolver.endgameTimeMS = 100000000;
        cerr << "Endgame solver: depth " << empties << endl;

        myMove = endgameSolver.endgame(game, legalMoves, empties);

        if(myMove != MOVE_BROKEN) {
            game.doMove(myMove, mySide);
            turn++;

            end_time = high_resolution_clock::now();
            time_span = duration_cast<duration<double>>(end_time-start_time);
            cerr << "Endgame took: " << time_span.count() << endl;

            return indexToMove[myMove];
        }
        cerr << "Broken out of endgame solver." << endl;
        endgameDepth -= 2;
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
        (msLeft/empties > time_span.count()*1000.0*25) || msLeft == -1) && attemptingDepth <= maxDepth );

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
 * 
 * Uses alpha-beta pruning with a null-window search, a transposition table that
 * stores moves which previously caused a beta cutoff, and an internal sorting
 * search of depth 2.
*/
int Player::pvs_h(Board *b, int &topScore, int s, int depth,
    int alpha, int beta) {

    if (depth <= 0) {
        topScore = (s == mySide) ? heuristic(b) : -heuristic(b);
        return topScore;
    }

    int score;
    int ttScore = NEG_INFTY;

    int killerMove = killer_table.get(b, s);
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

    // internal sort
    if(depth >= 5) {
        MoveList scores;
        pvs(b, legalMoves, scores, s, 2, NEG_INFTY, INFTY);
        sort(legalMoves, scores, 0, legalMoves.size-1);
    }

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        Board copy = Board(b->taken, b->black, b->legal);
        copy.doMove(legalMoves.get(i), s);

        if (i != 0) {
            score = -pvs_h(&copy, ttScore, -s, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -pvs_h(&copy, ttScore, -s, depth-1, -beta, -alpha);
        }
        else
            score = -pvs_h(&copy, ttScore, -s, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        if(ttScore > topScore)
            topScore = ttScore;
        if (alpha >= beta) {
            if(depth >= 4 && depth <= maxDepth-3)
                killer_table.add(b, s, legalMoves.get(i),
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

    if(turn < 25)
        score = 2*(b->count(oppSide) - myCoins);
    else if(turn < 50)
        score = myCoins - b->count(oppSide);
    else
        score = 2*(myCoins - b->count(oppSide));

    #if USE_EDGE_TABLE
    score += (mySide == BLACK) ? 3*boardTo24PV(b) : -3*boardTo24PV(b);
    score += (mySide == BLACK) ? 2*boardToEPV(b) : -2*boardToEPV(b);
    score += (mySide == BLACK) ? 2*boardToE2XPV(b) : -2*boardToE2XPV(b);
    score += (mySide == BLACK) ? 2*boardTo33PV(b) : -2*boardTo33PV(b);
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

bitbrd Player::reflectVertical(bitbrd i) {
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

bitbrd Player::reflectHorizontal(bitbrd x) {
    const bitbrd k1 = 0x5555555555555555;
    const bitbrd k2 = 0x3333333333333333;
    const bitbrd k4 = 0x0f0f0f0f0f0f0f0f;
    x = ((x >> 1) & k1) | ((x & k1) << 1);
    x = ((x >> 2) & k2) | ((x & k2) << 2);
    x = ((x >> 4) & k4) | ((x & k4) << 4);
    return x;
}

bitbrd Player::reflectDiag(bitbrd x) {
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

int Player::boardToEPV(Board *b) {
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

int Player::boardTo33PV(Board *b) {
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

int Player::boardTo24PV(Board *b) {
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

    return p24Table[ul] + p24Table[ll] + p24Table[ur] + p24Table[lr] +
        p24Table[rul] + p24Table[rll] + p24Table[rur] + p24Table[rlr];
}

int Player::boardToE2XPV(Board *b) {
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

    int result = pE2XTable[r1] + pE2XTable[r8] + pE2XTable[c1] + pE2XTable[c8];
    return result;
}

int Player::bitsToPI(int b, int w) {
    return PIECES_TO_INDEX[b] + 2*PIECES_TO_INDEX[w];
}

void Player::sort(MoveList &moves, MoveList &scores, int left, int right) {
    int pivot = (left + right) / 2;

    if (left < right) {
        pivot = partition(moves, scores, left, right, pivot);
        sort(moves, scores, left, pivot-1);
        sort(moves, scores, pivot+1, right);
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
        if (scores.get(i) > pivot) {
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

void Player::readStability33Table() {
    std::string line;
    std::string file;
        file = "s33table.txt";
    std::ifstream s33table(file);

    if(s33table.is_open()) {
        int i = 0;
        while(getline(s33table, line)) {
            for(int j = 0; j < 27; j++) {
                std::string::size_type sz = 0;
                s33Table[27*i+j] = std::stoi(line, &sz, 0);
                line = line.substr(sz);
            }

            i++;
        }
        s33table.close();
    }
}

void Player::readPattern24Table() {
    std::string line;
    std::string file;
        file = "p24table.txt";
    std::ifstream p24table(file);

    if(p24table.is_open()) {
        int i = 0;
        while(getline(p24table, line)) {
            std::string::size_type sz = 0;
            p24Table[i] = std::stoi(line, &sz, 0);

            i++;
        }
        p24table.close();
    }
}

void Player::readPatternE2XTable() {
    std::string line;
    std::string file;
        file = "pE2Xtable.txt";
    std::ifstream pE2Xtable(file);

    if(pE2Xtable.is_open()) {
        int i = 0;
        while(getline(pE2Xtable, line)) {
            for(int j = 0; j < 9; j++) {
                std::string::size_type sz = 0;
                pE2XTable[9*i+j] = std::stoi(line, &sz, 0);
                line = line.substr(sz);
            }

            i++;
        }
    }
}
