#include <iostream>
#include "player.h"

// Only try an endgame solve if more than this amount of time (in milliseconds)
// if left. Indexed by empties.
const int endgameTime[31] = { 0,
25, 30, 30, 40, 40, 40, 40, 50, 50, 50, // 1-10
50, 50, 75, 100, 150, // 11-15
250, 400, 750, 1500, 3000, // 16-20
6000, 15000, 32000, 75000, 200000, // 21-25
500000, 1200000, 2500000, 6000000, 15000000
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

    mySide = (side == BLACK) ? CBLACK : CWHITE;
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
    otherHeuristic = false;
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
        int movesLeft = (turn > 20) ? (64 - turn) : (64 - 20);
        timeLimit = 2 * msLeft / movesLeft;
    }
    else {
        // 1 min per move for "infinite" time
        timeLimit = 60000;
    }

    // register opponent's move
    if(opponentsMove != NULL) {
        game.doMove(opponentsMove->getX() + 8*opponentsMove->getY(), oppSide);
        turn++;
    }
    else {
        game.doMove(MOVE_NULL, oppSide);
    }
    int empties = game.countEmpty();
    #if PRINT_SEARCH_INFO
    cerr << endl;
    #endif

    // check opening book
    #if USE_OPENING_BOOK
    int openMove = openingBook.get(game.getTaken(), game.toBits(CBLACK));
    if(openMove != OPENING_NOT_FOUND) {
        #if PRINT_SEARCH_INFO
        cerr << "Opening book used! Played ";
        printMove(openMove);
        cerr << endl;
        #endif
        turn++;
        game.doMove(openMove, mySide);
        return indexToMove[openMove];
    }
    #endif

    // find and test all legal moves
    MoveList legalMoves = game.getLegalMoves(mySide);
    if (legalMoves.size <= 0) {
        game.doMove(MOVE_NULL, mySide);
        #if PRINT_SEARCH_INFO
        cerr << "No legal moves. Passing." << endl;
        #endif
        return NULL;
    }

    int myMove = -1;

    // endgame solver
    if(empties <= endgameDepth &&
            (msLeft >= endgameTime[empties] || msLeft == -1)) {
        // timing: use a quarter of remaining time for the endgame solve attempt
        int endgameLimit = (msLeft == -1) ? 100000000
                                          : msLeft / 4;
        #if PRINT_SEARCH_INFO
        cerr << "Endgame solver: depth " << empties << endl;
        #endif

        myMove = endgameSolver.solveEndgame(game, legalMoves, false, mySide,
            empties, endgameLimit, evaluater);

        if(myMove != MOVE_BROKEN) {
            game.doMove(myMove, mySide);
            turn++;
            return indexToMove[myMove];
        }

        #if PRINT_SEARCH_INFO
        cerr << "Broken out of endgame solver." << endl;
        #endif
        endgameDepth -= 2;
    }

    // Reset node count. Nodes are counted in the most conservative way (only
    // when doMove() is called), so that pruning does not inflate node counts.
    nodes = 0;
    // Start timers
    using namespace std::chrono;
    auto start_time = high_resolution_clock::now();
    auto end_time = high_resolution_clock::now();
    duration<double> time_span;
    timeElapsed = high_resolution_clock::now();

    // sort search
    #if PRINT_SEARCH_INFO
    cerr << "Sort search: depth " << sortDepth << endl;
    #endif
    attemptingDepth = sortDepth;
    MoveList scores;
    sortSearch(game, legalMoves, scores, mySide, sortDepth);
    sort(legalMoves, scores, 0, legalMoves.size-1);

    // iterative deepening
    attemptingDepth = minDepth;
    int chosenScore = 0;
    do {
        #if PRINT_SEARCH_INFO
        cerr << "Depth " << attemptingDepth << ": ";
        #endif

        int newBest = getBestMoveIndex(game, legalMoves, chosenScore, mySide,
            attemptingDepth);
        if(newBest == MOVE_BROKEN) {
            #if PRINT_SEARCH_INFO
            cerr << " Broken out of search!" << endl;
            #endif
            end_time = high_resolution_clock::now();
            time_span = duration_cast<duration<double>>(end_time-start_time);
            break;
        }
        // Switch new PV to be searched first
        legalMoves.swap(0, newBest);
        attemptingDepth += 2;

        #if PRINT_SEARCH_INFO
        cerr << "bestmove ";
        printMove(legalMoves.get(0));
        cerr << " score " << ((double)(chosenScore)) / 1000.0 << endl;
        #endif

        end_time = high_resolution_clock::now();
        time_span = duration_cast<duration<double>>(end_time-start_time);
    } while((timeLimit > time_span.count() * 1000.0 * legalMoves.size * 2)
          && attemptingDepth <= maxDepth);

    myMove = legalMoves.get(0);
    transpositionTable.clean(turn+2);
    #if PRINT_SEARCH_INFO
    cerr << "Nodes searched: " << nodes << " | NPS: " << (int)((double)nodes / time_span.count()) << endl;
    cerr << "Table contains " << transpositionTable.keys << " keys." << endl;
    cerr << "Playing ";
    printMove(legalMoves.get(0));
    cerr << ". Score: " << ((double)(chosenScore)) / 1000.0 << endl;
    #endif

    game.doMove(myMove, mySide);
    turn++;

    return indexToMove[myMove];
}

/**
 * @brief Performs a principal variation null-window search.
 * Returns the index of the best move.
*/
int Player::getBestMoveIndex(Board &b, MoveList &moves, int &bestScore, int s,
    int depth) {
    int score;
    int bestMove = 0;
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
 * stores moves which previously caused a beta cutoff, and an internal sorting
 * search of depth 2.
*/
int Player::pvs(Board &b, int s, int depth, int alpha, int beta) {
    if (depth <= 0) {
        if (otherHeuristic) {
            return evaluater->heuristic2(b, turn+attemptingDepth, s);
        }
        else
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
            if (entry->depth >= depth) {
                if (entry->nodeType == CUT_NODE && entry->score >= beta)
                    return beta;
                else if (entry->nodeType == PV_NODE)
                    return entry->score;
            }
            if (entry->depth >= 5) {
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

    // internal iterative deepening
    if (depth >= 2) {
        MoveList scores;
        if(depth >= 10)
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
        // Check for a timeout
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span =
            std::chrono::duration_cast<std::chrono::duration<double>>(
            end_time-timeElapsed);
        if (time_span.count() * 1000 > timeLimit)
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
            if(depth >= 5)
                transpositionTable.add(b, beta, legalMoves.get(i), s,
                    turn+attemptingDepth-depth, depth, CUT_NODE);
            return beta;
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
        Board copy = b.copy();
        copy.doMove(moves.get(i), side);
        nodes++;
        scores.add(-pvs(copy, side^1, depth-1, -INFTY, INFTY));
    }
}

void Player::setDepths(int sort, int min, int max, int end) {
    maxDepth = max;
    minDepth = min;
    sortDepth = sort;
    endgameDepth = end;
}
