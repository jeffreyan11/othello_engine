#include "player.h"

const int POW3[9] = {1, 3, 9, 27, 81, 243, 729, 2187, 6561};

/**
 * @brief Constructor for the player.
 * 
 * This constructor initializes the depths, timing variables, and the array
 * used to convert move indices to move objects.
 * 
 * @param side The side the AI is playing as.
 */
Player::Player(Side side) {
    maxDepth = 16;
    minDepth = 6;
    sortDepth = 4;
    endgameDepth = 20;
    if(side == WHITE)
        endgameDepth--;
    endgameSwitch = false;

    mySide = side;
    oppSide = (side == WHITE) ? (BLACK) : (WHITE);
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

    //TODO readMobilities();
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
        }
    }

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

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
    int openMove = -3;//openingBook.get(game.getTaken(), game.getBlack());
    if(openMove != OPENING_NOT_FOUND) {
        cerr << "Opening book used!" << endl;
        turn++;
        game.doMove(openMove, mySide);
        return indexToMove[openMove];
    }

    // find and test all legal moves
    vector<int> legalMoves = game.getLegalMoves(mySide);

    if (legalMoves.size() <= 0) {
        game.doMove(MOVE_NULL, mySide);
        return NULL;
    }

    int myMove = -1;
    vector<int> scores;

    // endgame solver
    while(endgameSwitch || turn >= (64 - endgameDepth)) {
        if(msLeft < endgameTimeMS && msLeft != -1) {
            endgameSwitch = false;
            endgameDepth -= 2;
            break;
        }
        cerr << "Endgame solver: attempting depth " << endgameDepth << endl;
        endgameSwitch = true;
        myMove = endgame(game, legalMoves, mySide, endgameDepth, NEG_INFTY,
            INFTY, endgameTimeMS, endgame_table);
        if(myMove == MOVE_BROKEN) {
            cerr << "Broken out of endgame solver." << endl;
            endgameDepth -= 2;
            break;
        }

        endgameDepth -= 2;

        game.doMove(myMove, mySide);
        turn++;
        return indexToMove[myMove];
    }

    // sort search
    cerr << "Performing initial search: depth " << sortDepth << endl;
    pvs(&game, legalMoves, scores, mySide, sortDepth, NEG_INFTY, INFTY);

    // iterative deepening
    int attemptingDepth = minDepth;
    duration<double> time_span;
    do {
        cerr << "Attempting NWS of depth " << attemptingDepth << endl;
        sort(legalMoves, scores, 0, legalMoves.size()-1);
        scores.clear();

        int newBest = pvs(&game, legalMoves, scores, mySide,
            attemptingDepth, NEG_INFTY, INFTY);
        if(newBest == MOVE_BROKEN) {
            cerr << "Broken out of search" << endl;
            break;
        }
        myMove = newBest;
        attemptingDepth += 2;

        auto end_time = high_resolution_clock::now();
        time_span = duration_cast<duration<double>>(end_time-start_time);
    } while( (
        ((msLeft-endgameTimeMS)/(64-endgameDepth-turn) > time_span.count()*1000.0*20) || msLeft == -1) && attemptingDepth <= maxDepth );

    game.doMove(myMove, mySide);
    turn++;
    return indexToMove[myMove];
}

int Player::pvs(Board *b, vector<int> &moves, vector<int> &scores,
    Side s, int depth, int alpha, int beta) {

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    int score;
    int tempMove = moves[0];

    for (unsigned int i = 0; i < moves.size(); i++) {
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(
            end_time-start_time);

        if(time_span.count() * moves.size() * 1000 > totalTimePM * (i+1))
            return MOVE_BROKEN;

        Board copy = Board(b->taken, b->black, b->legal);
        copy.doMove(moves[i], s);
        int ttScore = NEG_INFTY;
        if (i != 0) {
            score = -pvs_h(&copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -pvs_h(&copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -score);
            }
        }
        else {
            score = -pvs_h(&copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -alpha);
        }
        scores.push_back(ttScore);
        if (score > alpha) {
            alpha = score;
            tempMove = moves[i];
        }
        if (alpha >= beta)
            break;
    }
    return tempMove;
}

int Player::pvs_h(Board *b, int &topScore, Side s, int depth,
    int alpha, int beta) {

    if (depth <= 0) {
        topScore = (s == mySide) ? heuristic(b) : -heuristic(b);
        return topScore;
    }

    int score;
    int ttScore = NEG_INFTY;

    vector<int> legalMoves = b->getLegalMoves(s);
    if(legalMoves.size() <= 0) {
        Board copy = Board(b->taken, b->black, b->legal);
        copy.doMove(MOVE_NULL, s);
        score = -pvs_h(&copy, ttScore, ((s == WHITE) ? (BLACK) : WHITE),
            depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        topScore = ttScore;
        return alpha;
    }

    int killerMove = killer_table[*b];
    if(!killerMove) {
        killerMove--;
        Board copy = Board(b->taken, b->black, b->legal);
        copy.doMove(killerMove, s);
        score = -pvs_h(&copy, ttScore, ((s == WHITE) ? (BLACK) : WHITE),
            depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        if(ttScore > topScore)
            topScore = ttScore;
        if (alpha >= beta)
            return alpha;
    }

    for (unsigned int i = 0; i < legalMoves.size(); i++) {
        Board copy = Board(b->taken, b->black, b->legal);
        copy.doMove(legalMoves[i], s);
        if (i != 0) {
            score = -pvs_h(&copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta) {
                score = -pvs_h(&copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -alpha);
            }
        }
        else {
            score = -pvs_h(&copy, ttScore, ((s == WHITE) ? (BLACK) :
                WHITE), depth-1, -beta, -alpha);
        }
        if (alpha < score)
            alpha = score;
        if(ttScore > topScore)
            topScore = ttScore;
        if (alpha >= beta) {
            killer_table[*b] = legalMoves[i] + 1;
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

    if(turn < 30)
        score = myCoins - b->count(oppSide);
    else
        score = 2 * (myCoins - b->count(oppSide));

    bitbrd bm = b->toBits(mySide);

    score += 50 * countSetBits(bm & CORNERS);
    if(turn > 35)
        score += 4 * countSetBits(bm & EDGES);
    score -= 15 * countSetBits(bm & X_CORNERS);
    score -= 10 * countSetBits(bm & ADJ_CORNERS);

    score += 2 * (b->numLegalMoves(mySide) - b->numLegalMoves(oppSide));
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

int Player::boardToPV(Board *b) {
    // TODO
    return 0;
}

int Player::mobilityEstimate(Board *b) {
    bitbrd black = b->toBits(BLACK);
    bitbrd white = b->toBits(WHITE);
    int r1 = bitsToPI( black & 0xFF, white & 0xFF );
    int r2 = bitsToPI( (black>>8) & 0xFF, (white>>8) & 0xFF );
    int r3 = bitsToPI( (black>>16) & 0xFF, (white>>16) & 0xFF );
    int r4 = bitsToPI( (black>>24) & 0xFF, (white>>24) & 0xFF );
    int r5 = bitsToPI( (black>>32) & 0xFF, (white>>32) & 0xFF );
    int r6 = bitsToPI( (black>>40) & 0xFF, (white>>40) & 0xFF );
    int r7 = bitsToPI( (black>>48) & 0xFF, (white>>48) & 0xFF );
    int r8 = bitsToPI( (black>>56) & 0xFF, (white>>56) & 0xFF );
    int c1 = bitsToPI(
        ((black & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56,
        ((white & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56 );
    int c2 = bitsToPI(
        ((black & 0x0202020202020202ULL) * 0x0081020408102040ULL) >> 56,
        ((white & 0x0202020202020202ULL) * 0x0081020408102040ULL) >> 56 );
    int c3 = bitsToPI(
        ((black & 0x0404040404040404ULL) * 0x0040810204081020ULL) >> 56,
        ((white & 0x0404040404040404ULL) * 0x0040810204081020ULL) >> 56 );
    int c4 = bitsToPI(
        ((black & 0x0808080808080808ULL) * 0x0020408102040810ULL) >> 56,
        ((white & 0x0808080808080808ULL) * 0x0020408102040810ULL) >> 56 );
    int c5 = bitsToPI(
        ((black & 0x1010101010101010ULL) * 0x0010204081020408ULL) >> 56,
        ((white & 0x1010101010101010ULL) * 0x0010204081020408ULL) >> 56 );
    int c6 = bitsToPI(
        ((black & 0x2020202020202020ULL) * 0x0008102040810204ULL) >> 56,
        ((white & 0x2020202020202020ULL) * 0x0008102040810204ULL) >> 56 );
    int c7 = bitsToPI(
        ((black & 0x4040404040404040ULL) * 0x0004081020408102ULL) >> 56,
        ((white & 0x4040404040404040ULL) * 0x0004081020408102ULL) >> 56 );
    int c8 = bitsToPI(
        ((black & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56,
        ((white & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56 );
    int d81 = bitsToPI(
        ((black & 0x8040201008040201ULL) * 0x0101010101010101ULL) >> 56,
        ((white & 0x8040201008040201ULL) * 0x0101010101010101ULL) >> 56 );
    int d82 = bitsToPI(
        ((black & 0x0102040810204080ULL) * 0x0101010101010101ULL) >> 56,
        ((white & 0x0102040810204080ULL) * 0x0101010101010101ULL) >> 56 );
    int d71 = bitsToPI(
        ((black & 0x0080402010080402ULL) * 0x0101010101010101ULL) >> 56,
        ((white & 0x0080402010080402ULL) * 0x0101010101010101ULL) >> 56 );
    int d72 = bitsToPI(
        ((black & 0x0001020408102040ULL) * 0x0101010101010101ULL) >> 56,
        ((white & 0x0001020408102040ULL) * 0x0101010101010101ULL) >> 56 );
    int d73 = bitsToPI(
        ((black & 0x4020100804020100ULL) * 0x0101010101010101ULL) >> 56,
        ((white & 0x4020100804020100ULL) * 0x0101010101010101ULL) >> 56 );
    int d74 = bitsToPI(
        ((black & 0x0204081020408000ULL) * 0x0101010101010101ULL) >> 56,
        ((white & 0x0204081020408000ULL) * 0x0101010101010101ULL) >> 56 );
    // TODO may need fixing
    int d61 = bitsToPI(
        ((black & 0x0000804020110a04ULL) * 0x0101010101010101ULL) >> 56,
        ((white & 0x0000804020110a04ULL) * 0x0101010101010101ULL) >> 56 );
    int d62 = bitsToPI(
        ((black & 0x0000010204885020ULL) * 0x0101010101010101ULL) >> 56,
        ((white & 0x0000010204885020ULL) * 0x0101010101010101ULL) >> 56 );
    int d63 = bitsToPI(
        (((black & 0x2010080402010204ULL) + 0x6070787c7e7f7e7cULL) & 0x8080808080808080ULL) * 0x0002040810204081ULL >> 56,
        (((white & 0x2010080402010204ULL) + 0x6070787c7e7f7e7cULL) & 0x8080808080808080ULL) * 0x0002040810204081ULL >> 56 );
    int d64 = bitsToPI(
        (((black & 0x0408102040804020ULL) + 0x7c78706040004060ULL) & 0x8080808080808080ULL) * 0x0002040810204081ULL >> 56,
        (((white & 0x0408102040804020ULL) + 0x7c78706040004060ULL) & 0x8080808080808080ULL) * 0x0002040810204081ULL >> 56 );
    int d51 = bitsToPI(
        ((black & 0x0000008041221408ULL) * 0x0101010101010101ULL) >> 56,
        ((white & 0x0000008041221408ULL) * 0x0101010101010101ULL) >> 56 );
    int d52 = bitsToPI(
        ((black & 0x0000000182442810ULL) * 0x0101010101010101ULL) >> 56,
        ((white & 0x0000000182442810ULL) * 0x0101010101010101ULL) >> 56 );

    int result =
        mobilities[r1] + mobilities[r2] + mobilities[r3] + mobilities[r4] +
        mobilities[r5] + mobilities[r6] + mobilities[r7] + mobilities[r8] +
        mobilities[c1] + mobilities[c2] + mobilities[c3] + mobilities[c4] +
        mobilities[c5] + mobilities[c6] + mobilities[c7] + mobilities[c8] +
        mobilities[d81] + mobilities[d82] +
        mobilities[d71] + mobilities[d72] + mobilities[d73] + mobilities[d74] +
        mobilities[d61] + mobilities[d62] + mobilities[d63] + mobilities[d64] +
        mobilities[d51] + mobilities[d52];// + mobilities[d53] + mobilities[d54] +
    return result;
}

int Player::bitsToPI(bitbrd b, bitbrd w) {
    int result = 0;
    int i = 0;
    while(b) {
        if(b & 1)
            result += POW3[i];
        b >>= 1;
        i++;
    }
    i = 0;
    while(w) {
        if(w & 1)
            result += POW3[i];
        w >>= 1;
        i++;
    }

    return result;
}

void Player::sort(vector<int> &moves, vector<int> &scores, int left,
    int right) {

    int index = left;

    if (left < right) {
        index = partition(moves, scores, left, right, index);
        sort(moves, scores, left, index-1);
        sort(moves, scores, index+1, right);
    }
}

void Player::swap(vector<int> &moves, vector<int> &scores, int i, int j) {
    int less1;
    int less2;

    less1 = moves[j];
    moves[j] = moves[i];
    moves[i] = less1;

    less2 = scores[j];
    scores[j] = scores[i];
    scores[i] = less2;
}

int Player::partition(vector<int> &moves, vector<int> &scores, int left,
    int right, int pindex) {

    int index = left;
    int pivot = scores[pindex];

    swap(moves, scores, pindex, right);

    for (int i = left; i < right; i++) {
        if (scores[i] >= pivot) {
            swap(moves, scores, i, index);
            index++;
        }
    }
    swap(moves, scores, index, right);

    return index;
}

void Player::readMobilities() {
    std::string line;
    std::ifstream openingbk("mobility.txt");

    if(openingbk.is_open()) {
        int i = 0;
        while(getline(openingbk, line)) {
            for(int j = 0; j < 9; j++) {
                std::string::size_type sz = 0;
                mobilities[9*i+j] = std::stoi(line, &sz, 0);
                line = line.substr(sz);
            }

            i++;
        }
        openingbk.close();
    }
}
