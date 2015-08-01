#include <chrono>
#include <iostream>
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
    maxDepth = 16;
    minDepth = 6;
    sortDepth = 4;
    endgameDepth = 25;

    mySide = (side == BLACK) ? CBLACK : CWHITE;
    endgameSolver.mySide = mySide;
    oppSide = (side == WHITE) ? CBLACK : CWHITE;
    turn = 4;
    timeLimit = -2;

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            indexToMove[i+8*j] = new Move(i,j);
        }
    }

    // initialize the evaluation functions
    evaluater = new Eval();

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
    if(msLeft != -1) {
        timeLimit = 2 * msLeft / (64 - turn);
    }
    else {
        // 1 min per move for "infinite" time
        timeLimit = 60000;
    }

    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();
    auto end_time = high_resolution_clock::now();
    duration<double> time_span;

    // register opponent's move
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
    int openMove = openingBook.get(game.getTaken(), game.toBits(CBLACK));
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

    // endgame solver
    if(empties <= endgameDepth &&
            (msLeft >= endgameTime[empties] || msLeft == -1)) {
        // timing
        endgameSolver.endgameTimeMS = (msLeft + endgameTime[empties]) / 2;
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
            cerr << "Playing: " << myMove << endl;
            cerr << endl;

            return indexToMove[myMove];
        }

        cerr << "Broken out of endgame solver." << endl;
        endgameDepth -= 2;
        start_time = high_resolution_clock::now();
    }

    // Reset node count. Nodes are counted in the most conservative way (only
    // when doMove() is called), so that pruning does not inflate node counts.
    nodes = 0;

    // sort search
    cerr << "Performing sort search: depth " << sortDepth << endl;
    attemptingDepth = sortDepth;
    MoveList scores;
    sortSearch(game, legalMoves, scores, mySide, sortDepth);
    sort(legalMoves, scores, 0, legalMoves.size-1);
    scores.clear();

    // iterative deepening
    attemptingDepth = minDepth;
    int chosenScore = 0;
    do {
        cerr << "Searching depth " << attemptingDepth << ":";

        int newBest = pvs(game, legalMoves, chosenScore, mySide, attemptingDepth);
        if(newBest == MOVE_BROKEN) {
            cerr << " Broken out of search!" << endl;
            end_time = high_resolution_clock::now();
            time_span = duration_cast<duration<double>>(end_time-start_time);
            break;
        }
        int temp = legalMoves.get(newBest);
        legalMoves.set(newBest, legalMoves.get(0));
        legalMoves.set(0, temp);
        attemptingDepth += 2;

        cerr << " (" << legalMoves.get(0) << ", " << ((double)(chosenScore)) / 100.0 << ")" << endl;

        end_time = high_resolution_clock::now();
        time_span = duration_cast<duration<double>>(end_time-start_time);
    } while(((timeLimit > time_span.count() * 1000.0 * legalMoves.size * 3 / 2)
                || msLeft == -1)
            && attemptingDepth <= maxDepth);

    myMove = legalMoves.get(0);
    cerr << "Playing " << myMove << ". Score: " << ((double)(chosenScore)) / 100.0 << endl;
    cerr << "Nodes searched: " << nodes << " | NPS: " << (int)((double)nodes / time_span.count()) << endl;
    transpositionTable.clean(turn+2);
    cerr << "Table contains " << transpositionTable.keys << " keys." << endl;
    cerr << endl;

    game.doMove(myMove, mySide);
    turn++;

    return indexToMove[myMove];
}

/**
 * @brief Performs a principal variation null-window search.
 * Returns the index of the best move.
*/
int Player::pvs(Board &b, MoveList &moves, int &bestScore, int s, int depth) {
    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();

    int score;
    int bestMove = 0;
    int alpha = NEG_INFTY;
    int beta = INFTY;

    for (unsigned int i = 0; i < moves.size; i++) {
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(
            end_time-start_time);

        if (time_span.count() * moves.size * 1000 > 2 * timeLimit * (i+1))
            return MOVE_BROKEN;

        Board copy = Board(b.taken, b.black);
        copy.doMove(moves.get(i), s);
        nodes++;
        if (i != 0) {
            score = -pvs_h(copy, -s, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -pvs_h(copy, -s, depth-1, -beta, -alpha);
        }
        else
            score = -pvs_h(copy, -s, depth-1, -beta, -alpha);

        if (score > alpha) {
            alpha = score;
            bestMove = i;
        }
    }
    bestScore = alpha;
    return bestMove;
}

/**
 * @brief Helper function for the principal variation search.
 * 
 * Uses alpha-beta pruning with a null-window search, a transposition table that
 * stores moves which previously caused a beta cutoff, and an internal sorting
 * search of depth 2.
*/
int Player::pvs_h(Board &b, int s, int depth, int alpha, int beta) {
    if (depth <= 0) {
        return evaluater->heuristic(b, turn+attemptingDepth, s);
    }

    int score;
    int prevAlpha = alpha;
    int hashed = MOVE_NULL;
    int toHash = MOVE_NULL;

    // Hash table using the killer heuristic
    BoardData *entry = transpositionTable.get(b, s);
    if(entry != NULL) {
        if (entry->nodeType == ALL_NODE) {
            if (entry->depth >= depth && entry->score <= alpha)
                return alpha;
        }
        else {
            hashed = entry->move;
            if (entry->depth >= depth) {
                if (entry->nodeType == CUT_NODE && entry->score >= beta)
                    return beta;
                else if (entry->nodeType == PV_NODE)
                    return entry->score;
            }
            Board copy = Board(b.taken, b.black);
            copy.doMove(hashed, s);
            nodes++;
            score = -pvs_h(copy, -s, depth-1, -beta, -alpha);

            if (alpha < score)
                alpha = score;
            if (alpha >= beta)
                return beta;
        }
    }

    MoveList legalMoves = b.getLegalMoves(s);
    if (legalMoves.size <= 0) {
        score = -pvs_h(b, -s, depth-1, -beta, -alpha);

        if (alpha < score)
            alpha = score;
        return alpha;
    }

    // internal iterative deepening
    if (depth >= 2) {
        MoveList scores;
        if(depth >= 9)
            sortSearch(b, legalMoves, scores, s, 4);
        else if(depth >= 5)
            sortSearch(b, legalMoves, scores, s, 2);
        else if(depth >= 3)
            sortSearch(b, legalMoves, scores, s, 0);
        else {
            for (unsigned int i = 0; i < legalMoves.size; i++)
                scores.add(SQ_VAL[legalMoves.get(i)]);
        }
        sort(legalMoves, scores, 0, legalMoves.size-1);
    }

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        if (legalMoves.get(i) == hashed)
            continue;
        Board copy = Board(b.taken, b.black);
        copy.doMove(legalMoves.get(i), s);
        nodes++;

        if (depth > 2 && i != 0) {
            score = -pvs_h(copy, -s, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -pvs_h(copy, -s, depth-1, -beta, -alpha);
        }
        else
            score = -pvs_h(copy, -s, depth-1, -beta, -alpha);

        if (alpha < score) {
            alpha = score;
            toHash = legalMoves.get(i);
        }
        if (alpha >= beta) {
            if(depth >= 5 && depth <= maxDepth-2)
                transpositionTable.add(b, beta, legalMoves.get(i), s,
                    turn+attemptingDepth-depth, depth, CUT_NODE);
            break;
        }
    }

    if (depth >= 5 && toHash != MOVE_NULL && prevAlpha < alpha && alpha < beta)
        transpositionTable.add(b, alpha, toHash, s,
                    turn+attemptingDepth-depth, depth, PV_NODE);
    else if (depth >= 5 && alpha <= prevAlpha)
        transpositionTable.add(b, alpha, MOVE_NULL, s,
                    turn+attemptingDepth-depth, depth, ALL_NODE);

    return alpha;
}

void Player::sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
    int depth) {

    for (unsigned int i = 0; i < moves.size; i++) {
        Board copy = Board(b.taken, b.black);
        copy.doMove(moves.get(i), side);
        nodes++;

        scores.add(-pvs_h(copy, -side, depth-1, NEG_INFTY, INFTY));
    }
}
