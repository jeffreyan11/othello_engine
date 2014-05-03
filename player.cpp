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

    evaluater = new Eval(mySide);

    #if defined(__x86_64__)
        cerr << "x86-64 processor detected." << endl;
    #elif defined(__i386)
        cerr << "x86 processor detected." << endl;
    #else
        cerr << "non-x86 processor detected." << endl;
    #endif
}

/**
 * @brief Destructor for the player.
 */
Player::~Player() {
    for(int i = 0; i < 64; i++) {
        delete indexToMove[i];
    }

    delete evaluater;
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
    int empties = game.countEmpty();
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

        myMove = endgameSolver.endgame(game, legalMoves, empties, evaluater);

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
        topScore = (s == mySide) ? evaluater->heuristic(b, turn) :
                -evaluater->heuristic(b, turn);
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
