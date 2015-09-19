#include <algorithm>
#include <iostream>
#include "player.h"

// Internal iterative deepening depths for PV nodes
const int PV_SORT_DEPTHS[21] = { 0,
0, 0, 0, 0, 2, 2, 2, 2, 2, 4, // 1-10
4, 4, 4, 4, 6, 6, 6, 6, 8, 8  // 11-20
};

// Internal iterative deepening depths for non-PV nodes
const int NON_PV_SORT_DEPTHS[21] = { 0,
0, 0, 0, 0, 0, 0, 2, 2, 2, 2, // 1-10
2, 4, 4, 4, 4, 4, 6, 6, 6, 6  // 11-20
};

const int TIMEOUT = (1 << 21);

const int EVAL_SCALE_FACTOR = 2000;

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
    maxDepth = 22;
    minDepth = 4;
    sortDepth = 2;
    endgameDepth = 34;
    lastMaxDepth = 0;

    mySide = (side == BLACK) ? CBLACK : CWHITE;
    turn = 4;

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
    // If opponent is passing and it isn't the start of the game
    else if (turn != 4) {
        // TODO a temporary hack to prevent opening book from crashing
        bookExhausted = true;
    }

    // We can easily count how many moves have been made from the number of
    // empty squares
    int empties = game.countEmpty();
    turn = 64 - empties;
    // Reset node count. Nodes are counted in the most conservative way (only
    // when doMove() is called), so that pruning does not inflate node counts.
    nodes = 0;

    // TODO reset MPC statistics
    #if COLLECT_MPC_STATS
    for (unsigned int i = 0; i < 21; i++) {
        MPCdone[i] = 0;
        MPCfail[i] = 0;
    }
    #endif

    // Timing
    if(msLeft != -1) {
        // Time odds, if desired
        //msLeft -= 600000;
        // Buffer time: to prevent losses on time at short time controls
        if (empties > 14)
            msLeft -= 800;

        // Use up to 2.5x "fair" time
        int movesLeft = max(1, empties / 2);
        timeLimit = 5 * msLeft / (2 * movesLeft);
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
    // time to do a perfect solve (estimated by lastMaxDepth) or have unlimited
    // time. Always use endgame solver for the last 14 plies since it is faster
    // and for more accurate results.
    if(empties <= endgameDepth
    && (lastMaxDepth + 10 >= empties || msLeft == -1 || empties <= 14)) {
        // Timing: use a quarter of remaining time for the endgame solve attempt
        int endgameLimit = (msLeft == -1) ? 100000000
                                          : msLeft / 4;
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
        timeLimit = 2 * timeLimit / 3;
    }

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
    scores.clear();

    // Iterative deepening
    attemptingDepth = minDepth;
    int newBest;
    do {
        #if PRINT_SEARCH_INFO
        cerr << "Depth " << attemptingDepth << ": ";
        #endif

        newBest = getBestMoveIndex(game, legalMoves, scores, mySide,
            attemptingDepth);
        if(newBest == MOVE_BROKEN) {
            #if PRINT_SEARCH_INFO
            cerr << " Broken out of search!" << endl;
            #endif
            timeSpan = getTimeElapsed(startTime);
            break;
        }
        lastMaxDepth = attemptingDepth;

        // Switch new PV to be searched first
        legalMoves.swap(0, newBest);
        scores.swap(0, newBest);
        attemptingDepth += 2;
        myMove = legalMoves.get(0);

        #if PRINT_SEARCH_INFO
        cerr << "bestmove ";
        printMove(myMove);
        cerr << " score " << ((double) scores.get(0)) / EVAL_SCALE_FACTOR << endl;
        #endif

        timeSpan = getTimeElapsed(startTime);
    // Continue while we think we can finish the next depth within our
    // allotted time for this move. Based on a crude estimate of branch factor.
    } while((timeLimit > timeSpan * 1000.0 * legalMoves.size)
          && attemptingDepth <= maxDepth);

    // WLD confirmation at high depths
    if(empties <= endgameDepth + 2
    && (lastMaxDepth + 12 >= empties || msLeft == -1)
    && empties > 14
    && timeSpan < timeLimit) {
        // Timing: use 1/6 of remaining time for the WLD solve
        int endgameLimit = (msLeft == -1) ? 100000000
                                          : msLeft / 6;
        #if PRINT_SEARCH_INFO
        cerr << "WLD solver: depth " << empties << endl;
        #endif

        int WLDMove = endgameSolver.solveWLD(game, legalMoves, true, mySide,
            empties, endgameLimit, evaluater);

        if(WLDMove != MOVE_BROKEN) {
            if (WLDMove != -1 && myMove != WLDMove) {
                #if PRINT_SEARCH_INFO
                cerr << "Move changed to " << endl;
                printMove(WLDMove);
                #endif
                myMove = WLDMove;
            }
        }
    }

    // Heh. Heh. Heh.
    if (scores.get(0) > 60 * EVAL_SCALE_FACTOR)
        lastMaxDepth += 6;

    timeSpan = getTimeElapsed(startTime);
    #if PRINT_SEARCH_INFO
    cerr << "Time spent: " << timeSpan << " s" << endl;
    cerr << "Nodes searched: " << nodes << " | NPS: " << (int) ((double) nodes / timeSpan) << endl;
    cerr << "Table contains " << transpositionTable->keys << " entries." << endl;
    cerr << "Playing ";
    printMove(myMove);
    cerr << ". Score: " << ((double) scores.get(0)) / EVAL_SCALE_FACTOR << endl;
    #endif

    #if COLLECT_MPC_STATS
    for (unsigned int i = 7; i < 21; i++) {
        cerr << "MPC depth " << i << ": " << MPCdone[i] << " done " << MPCfail[i] << " failed" << endl;
    }
    #endif

    game.doMove(myMove, mySide);

    return indexToMove(myMove);
}

/**
 * @brief Performs a principal variation null-window search.
 * Returns the index of the best move.
 */
int Player::getBestMoveIndex(Board &b, MoveList &moves, MoveList &scores, int s,
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

        scores.set(i, score);
        if (score > alpha) {
            alpha = score;
            bestMove = i;
        }
    }

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

    int score, bestScore = -INFTY;
    int prevAlpha = alpha;
    int hashed = MOVE_NULL;
    int toHash = MOVE_NULL;

    // We want to do better move ordering at PV nodes where alpha != beta - 1
    bool isPVNode = (alpha != beta - 1);

    // Probe transposition table for a score or move
    // Do this only at depth 3 and above for efficiency
    if (depth >= 3) {
        BoardData *entry = transpositionTable->get(b, s);
        if (entry != NULL) {
            // For all-nodes, we only have an upper bound score
            if (entry->nodeType == ALL_NODE) {
                if (entry->depth >= depth && entry->score <= alpha)
                    return entry->score;
            }
            else {
                if (entry->depth >= depth) {
                    // For cut-nodes, we have a lower bound score
                    if (entry->nodeType == CUT_NODE && entry->score >= beta)
                        return entry->score;
                    // For PV-nodes, we have an exact score we can return
                    else if (entry->nodeType == PV_NODE && !isPVNode)
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
                if (score >= beta)
                    return score;
                if (score > bestScore) {
                    bestScore = score;
                    if (alpha < score)
                        alpha = score;
                }
            }
        }
    }

    MoveList legalMoves = b.getLegalMoves(s);
    if (legalMoves.size <= 0) {
        score = -pvs(b, s^1, depth-1, -beta, -alpha);

        // If we received a timeout signal, propagate it upwards
        if (score == TIMEOUT)
            return -TIMEOUT;

        return score;
    }

    // The new list size after chopping off the end of the list for MPC.
    unsigned int prunedSize = legalMoves.size;
    // Move ordering
    // Don't waste time sorting at depth 1.
    if (depth >= 2) {
        prunedSize = sortMoves(b, legalMoves, s, depth, alpha, isPVNode);
    }

#if COLLECT_MPC_STATS
    for (unsigned int i = 0; i < legalMoves.size; i++) {
#else
    for (unsigned int i = 0; i < prunedSize; i++) {
#endif
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
        if (score >= beta) {
            #if COLLECT_MPC_STATS
            if (i >= prunedSize)
                MPCfail[depth]++;
            #endif
            if(depth >= 4)
                transpositionTable->add(b, score, legalMoves.get(i), s,
                    turn, depth, CUT_NODE);
            return score;
        }
        if (score > bestScore) {
            bestScore = score;
            if (alpha < score) {
                #if COLLECT_MPC_STATS
                if (i >= prunedSize)
                    MPCfail[depth]++;
                #endif
                alpha = score;
                toHash = legalMoves.get(i);
            }
        }
    }

    // If all moves were pruned as a fail-low
    if (bestScore == -INFTY) {
        bestScore = alpha - (int) ((depth / 2.0 - 1) * EVAL_SCALE_FACTOR)
                          - abs(alpha);
    }

    if (depth >= 4 && toHash != MOVE_NULL && prevAlpha < alpha && alpha < beta)
        transpositionTable->add(b, alpha, toHash, s, turn, depth, PV_NODE);
    else if (depth >= 4 && alpha <= prevAlpha)
        transpositionTable->add(b, bestScore, MOVE_NULL, s, turn, depth, ALL_NODE);

    return bestScore;
}

/**
 * @brief Sorts moves based on depth and whether the moves came from a PV node.
 * A shallow sort search (a form of internal iterative deepening) is used along
 * with fastest first (restricting opponent's mobility to reduce branch factor
 * and get the cheapest possible cutoff).
 * This function also uses the shallow sort search to perform Multi-ProbCut.
 */
unsigned int Player::sortMoves(Board &b, MoveList &legalMoves, int s, int depth,
    int alpha, bool isPVNode) {
    // Detect strongly expected all-nodes
    if (!isPVNode && depth >= 5) {
        int staticEval = otherHeuristic ? evaluater->heuristic2(b, turn+attemptingDepth, s)
                                        : evaluater->heuristic(b, turn+attemptingDepth, s);

        // Do sorting as if depth was 2 lower
        if (staticEval < alpha - 2 * EVAL_SCALE_FACTOR)
            depth -= 2;
    }

    // internal iterative deepening
    MoveList scores;
    // Number of values below the MPC threshold
    unsigned int belowThreshold = 0;

    if (depth >= 4 && isPVNode)
        sortSearch(b, legalMoves, scores, s, PV_SORT_DEPTHS[depth]);
    else if (depth >= 5) {
        sortSearch(b, legalMoves, scores, s, NON_PV_SORT_DEPTHS[depth]);
        // Detect very poor moves for MPC
        if (depth >= 7) {
            #if COLLECT_MPC_STATS
            MPCdone[depth]++;
            #endif
            for (unsigned int i = 0; i < scores.size; i++) {
                if (scores.get(i) < alpha
                            - (int) ((depth / 2.0 - 1) * EVAL_SCALE_FACTOR)
                            - abs(alpha))
                    belowThreshold++;
            }
        }
        // Fastest first
        for (unsigned int i = 0; i < legalMoves.size; i++) {
            Board copy = b.copy();
            copy.doMove(legalMoves.get(i), s);
            scores.set(i, scores.get(i) - 1024*copy.numLegalMoves(s^1));
        }
    }
    else if (depth >= 3) {
        for (unsigned int i = 0; i < legalMoves.size; i++) {
            Board copy = b.copy();
            copy.doMove(legalMoves.get(i), s);
            scores.add(SQ_VAL[legalMoves.get(i)] - 16*copy.numLegalMoves(s^1));
        }
    }
    else {
        for (unsigned int i = 0; i < legalMoves.size; i++)
            scores.add(SQ_VAL[legalMoves.get(i)]);
    }

    sort(legalMoves, scores, 0, legalMoves.size-1);
    return legalMoves.size - belowThreshold;
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
}

uint64_t Player::getNodes() {
    return nodes;
}
