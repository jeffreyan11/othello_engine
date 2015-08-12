#include <iostream>
#include "player.h"

// Only try an endgame solve if more than this amount of time (in milliseconds)
// if left. Indexed by empties.
const int endgameTime[31] = { 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1-10
10, 20, 40, 80, 150, // 11-15
250, 400, 750, 1500, 3000, // 16-20
6000, 12000, 25000, 50000, 120000, // 21-25
250000, 500000, 1000000, 2500000, 8000000
};

// Internal iterative deepening depths for PV nodes
const int PV_SORT_DEPTHS[21] = { 0,
0, 0, 0, 0, 2, 2, 2, 2, 2, 4, // 1-10
4, 4, 4, 4, 6, 6, 6, 6, 8, 8  // 11-20
};

// Internal iterative deepening depths for non-PV nodes
const int NON_PV_SORT_DEPTHS[21] = { 0,
0, 0, 0, 0, 0, 0, 0, 2, 2, 2, // 1-10
2, 4, 4, 4, 4, 6, 6, 6, 6, 8  // 11-20
};

const int TIMEOUT = (1 << 21);

using namespace std;

/**
 * @brief Constructor for the player.
 * 
 * This constructor initializes the depths, timing variables, and the array
 * used to convert move indices to move objects.
 * 
 * @param side The side the AI is playing as.
 */
Player::Player(Side side) {
    maxDepth = 18;
    minDepth = 6;
    sortDepth = 4;
    endgameDepth = 27;
    depthLimit = maxDepth;

    mySide = (side == BLACK) ? CBLACK : CWHITE;

    // initialize the evaluation functions
    evaluater = new Eval();
    otherHeuristic = false;

    // Initialize transposition table with 2^20 = 1 million array slots and
    // 2 * 2^20 = 2 million entries
    transpositionTable = new Hash(20);

    // Set to false to turn on book
    bookExhausted = false;
}

/**
 * @brief Destructor for the player.
 */
Player::~Player() {
    delete evaluater;
    delete transpositionTable;
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
    #if PRINT_SEARCH_INFO
    cerr << endl;
    #endif
    // register opponent's move
    if(opponentsMove != NULL)
        game.doMove(opponentsMove->getX() + 8*opponentsMove->getY(), mySide^1);
    else {
        // TODO a temporary hack to prevent opening book from crashing
        bookExhausted = true;
    }

    // We can easily count how many moves have been made from the number of
    // empty squares
    int empties = game.countEmpty();
    turn = 64 - empties;

    // timing
    if(msLeft != -1) {
        int movesLeft = 64 - turn;
        timeLimit = 2.5 * msLeft / movesLeft;
        #if PRINT_SEARCH_INFO
        cerr << "Time limit: " << timeLimit / 1000.0 << " s" << endl;
        #endif
    }
    else {
        // 1 min per move for "infinite" time
        timeLimit = 60000;
    }

    // check opening book
    if (!bookExhausted) {
        int openMove = openingBook.get(game.getTaken(), game.getBits(CBLACK));
        if(openMove != OPENING_NOT_FOUND) {
            #if PRINT_SEARCH_INFO
            cerr << "Opening book used! Played ";
            printMove(openMove);
            cerr << endl;
            #endif
            game.doMove(openMove, mySide);
            return indexToMove(openMove);
        }
        else
            bookExhausted = true;
    }

    // find and test all legal moves
    MoveList legalMoves = game.getLegalMoves(mySide);
    if (legalMoves.size <= 0) {
        // TODO a temporary hack to prevent opening book from crashing
        bookExhausted = true;
        #if PRINT_SEARCH_INFO
        cerr << "No legal moves. Passing." << endl;
        #endif
        return NULL;
    }

    int myMove = MOVE_BROKEN;

    // Endgame solver: if we are within sight of the end and we have enough
    // time to do a perfect solve
    if(empties <= endgameDepth &&
            (msLeft >= endgameTime[empties] || msLeft == -1)) {
        // Timing: use a quarter of remaining time for the endgame solve attempt
        int endgameLimit = (msLeft == -1) ? 100000000
                                          : msLeft / 3;
        #if PRINT_SEARCH_INFO
        cerr << "Endgame solver: depth " << empties << endl;
        #endif

        myMove = endgameSolver.solveEndgame(game, legalMoves, false, mySide,
            empties, endgameLimit, evaluater);

        if(myMove != MOVE_BROKEN) {
            game.doMove(myMove, mySide);
            return indexToMove(myMove);
        }
        // Otherwise, we broke out of the endgame solver.
        endgameDepth -= 2;
    }

    // Reset node count. Nodes are counted in the most conservative way (only
    // when doMove() is called), so that pruning does not inflate node counts.
    nodes = 0;
    // Start timers
    auto startTime = OthelloClock::now();
    timeElapsed = OthelloClock::now();
    double timeSpan = 0.0;

    // Root move ordering: sort search
    #if PRINT_SEARCH_INFO
    cerr << "Sort search: depth " << sortDepth << endl;
    #endif
    attemptingDepth = sortDepth;
    MoveList scores;
    sortSearch(game, legalMoves, scores, mySide, sortDepth);
    sort(legalMoves, scores, 0, legalMoves.size-1);

    // Iterative deepening
    attemptingDepth = minDepth;
    int chosenScore = 0;
    int newBest;
    do {
        #if PRINT_SEARCH_INFO
        cerr << "Depth " << attemptingDepth << ": ";
        #endif

        newBest = getBestMoveIndex(game, legalMoves, chosenScore, mySide,
            attemptingDepth);
        if(newBest == MOVE_BROKEN) {
            #if PRINT_SEARCH_INFO
            cerr << " Broken out of search!" << endl;
            #endif
            timeSpan = getTimeElapsed(startTime);
            break;
        }
        // Switch new PV to be searched first
        legalMoves.swap(0, newBest);
        attemptingDepth += 2;

        #if PRINT_SEARCH_INFO
        cerr << "bestmove ";
        printMove(legalMoves.get(0));
        cerr << " score " << ((double) (chosenScore)) / 2000.0 << endl;
        #endif

        timeSpan = getTimeElapsed(startTime);
    // Continue while we think we can finish the next depth within our
    // allotted time for this move. Based on a crude estimate of branch factor.
    } while((timeLimit > timeSpan * 1000.0 * legalMoves.size)
          && attemptingDepth <= depthLimit);

    if (newBest == MOVE_BROKEN) {
        // If we broke at this depth, we only want to search 2 less next time,
        // to prevent an infinite search and break loop.
        depthLimit = attemptingDepth - 2;
    }
    else
        depthLimit = maxDepth;

    myMove = legalMoves.get(0);
    #if PRINT_SEARCH_INFO
    cerr << "Time spent: " << timeSpan << " s" << endl;
    cerr << "Nodes searched: " << nodes << " | NPS: " << (int) ((double) nodes / timeSpan) << endl;
    cerr << "Table contains " << transpositionTable->keys << " entries." << endl;
    cerr << "Playing ";
    printMove(legalMoves.get(0));
    cerr << ". Score: " << ((double) (chosenScore)) / 2000.0 << endl;
    #endif

    game.doMove(myMove, mySide);

    return indexToMove(myMove);
}

/**
 * @brief Performs a principal variation null-window search.
 * Returns the index of the best move.
 */
int Player::getBestMoveIndex(Board &b, MoveList &moves, int &bestScore, int s,
    int depth) {
    int score;
    int bestMove = MOVE_BROKEN;
    int alpha = -INFTY;
    int beta = INFTY;

    for (unsigned int i = 0; i < moves.size; i++) {
        Board copy = b.copy();
        copy.doMove(moves.get(i), s);
        nodes++;
        if (i != 0) {
            score = -pvs(copy, s^1, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -pvs(copy, s^1, depth-1, -beta, -alpha);
        }
        else
            score = -pvs(copy, s^1, depth-1, -beta, -alpha);
        // Handle timeouts
        if (score == TIMEOUT)
            return MOVE_BROKEN;

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
 * stores moves from at least depth 4, and internal iterative deepening,
 * fastest first, and a piece-square table for move ordering.
 */
int Player::pvs(Board &b, int s, int depth, int alpha, int beta) {
    if (depth <= 0) {
        if (otherHeuristic)
            return evaluater->heuristic2(b, turn+attemptingDepth, s);
        else
            return evaluater->heuristic(b, turn+attemptingDepth, s);
    }

    int score;
    int prevAlpha = alpha;
    int hashed = MOVE_NULL;
    int toHash = MOVE_NULL;

    // Probe transposition table for a score or move
    // Do this only at depth 3 and above for efficiency
    if (depth >= 3) {
        BoardData *entry = transpositionTable->get(b, s);
        if (entry != NULL) {
            // For all-nodes, we only have an upper bound score
            if (entry->nodeType == ALL_NODE) {
                if (entry->depth >= depth && entry->score <= alpha)
                    return alpha;
            }
            else {
                if (entry->depth >= depth) {
                    // For cut-nodes, we have a lower bound score
                    if (entry->nodeType == CUT_NODE && entry->score >= beta)
                        return beta;
                    // For PV-nodes, we have an exact score we can return
                    else if (entry->nodeType == PV_NODE)
                        return entry->score;
                }
                // Try the hash move first
                hashed = entry->move;
                Board copy = b.copy();
                copy.doMove(hashed, s);
                nodes++;
                score = -pvs(copy, s^1, depth-1, -beta, -alpha);

                // If we received a timeout signal, propagate it upwards
                if (score == TIMEOUT)
                    return -TIMEOUT;
                if (alpha < score)
                    alpha = score;
                if (alpha >= beta)
                    return beta;
            }
        }
    }

    MoveList legalMoves = b.getLegalMoves(s);
    if (legalMoves.size <= 0) {
        score = -pvs(b, s^1, depth-1, -beta, -alpha);

        // If we received a timeout signal, propagate it upwards
        if (score == TIMEOUT)
            return -TIMEOUT;
        if (alpha < score)
            alpha = score;
        return alpha;
    }

    // Move ordering
    // Don't waste time sorting at depth 1.
    if (depth >= 2) {
        // We want to do better move ordering at PV nodes where alpha != beta - 1
        bool isPVNode = (alpha != beta - 1);
        sortMoves(b, legalMoves, s, depth, isPVNode);
    }

    for (unsigned int i = 0; i < legalMoves.size; i++) {
        // Check for a timeout
        if (getTimeElapsed(timeElapsed) * 1000 > timeLimit)
            return -TIMEOUT;

        if (legalMoves.get(i) == hashed)
            continue;
        Board copy = b.copy();
        copy.doMove(legalMoves.get(i), s);
        nodes++;

        if (depth > 2 && i != 0) {
            score = -pvs(copy, s^1, depth-1, -alpha-1, -alpha);
            if (alpha < score && score < beta)
                score = -pvs(copy, s^1, depth-1, -beta, -alpha);
        }
        else
            score = -pvs(copy, s^1, depth-1, -beta, -alpha);

        // If we received a timeout signal, propagate it upwards
        if (score == TIMEOUT)
            return -TIMEOUT;
        if (alpha < score) {
            alpha = score;
            toHash = legalMoves.get(i);
        }
        if (alpha >= beta) {
            if(depth >= 4)
                transpositionTable->add(b, beta, legalMoves.get(i), s,
                    turn, depth, CUT_NODE);
            return beta;
        }
    }

    if (depth >= 4 && toHash != MOVE_NULL && prevAlpha < alpha && alpha < beta)
        transpositionTable->add(b, alpha, toHash, s, turn, depth, PV_NODE);
    else if (depth >= 4 && alpha <= prevAlpha)
        transpositionTable->add(b, alpha, MOVE_NULL, s, turn, depth, ALL_NODE);

    return alpha;
}

/**
 * @brief Sorts moves based on depth and whether the moves came from a PV node.
 * A shallow sort search (a form of internal iterative deepening) is used along
 * with fastest first (restricting opponent's mobility to reduce branch factor
 * and get the cheapest possible cutoff).
 */
void Player::sortMoves(Board &b, MoveList &legalMoves, int s, int depth,
    bool isPVNode) {
    // internal iterative deepening
    MoveList scores;
    if(depth >= 4 && isPVNode)
        sortSearch(b, legalMoves, scores, s, PV_SORT_DEPTHS[depth]);
    else if(depth >= 4) {
        sortSearch(b, legalMoves, scores, s, NON_PV_SORT_DEPTHS[depth]);
        // Fastest first
        for (unsigned int i = 0; i < legalMoves.size; i++) {
            Board copy = b.copy();
            copy.doMove(legalMoves.get(i), s);
            scores.set(i, scores.get(i) - 1024*copy.numLegalMoves(s^1));
        }
    }
    else {
        for (unsigned int i = 0; i < legalMoves.size; i++)
            scores.add(SQ_VAL[legalMoves.get(i)]);
    }
    sort(legalMoves, scores, 0, legalMoves.size-1);
}

void Player::sortSearch(Board &b, MoveList &moves, MoveList &scores, int side,
    int depth) {

    for (unsigned int i = 0; i < moves.size; i++) {
        Board copy = b.copy();
        copy.doMove(moves.get(i), side);
        nodes++;
        scores.add(-pvs(copy, side^1, depth-1, -INFTY, INFTY));
    }
}

// Converts an integer index of move to a Move object understandable by the
// Java wrapper
Move *Player::indexToMove(int index) {
    return new Move(index % 8, index / 8);
}

void Player::setDepths(int sort, int min, int max, int end) {
    maxDepth = max;
    minDepth = min;
    sortDepth = sort;
    endgameDepth = end;
    depthLimit = maxDepth;
}
