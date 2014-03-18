Jeffrey - set up bitbucket, Player() constructor, bitboards, minimax, negamax,
    negascout, heuristic, opening book, endgame solver, iterative deepening

Kimberly - wrote doMove() in player.cpp to play random moves, heuristic,
    minimax, negascout, iterative deepening



    We created a new board implementation that uses bitboards to represent the
game board. This was done to improve efficiency as bit operations take very
little time.

    Instead of minimax with alpha beta pruning, we used negascout because it is
slightly more efficient.

    We created an opening book to increase efficiency and have near perfect play
for the first few moves of the game.

    The endgame solver allows for perfect play at the end of the game. While the
solver takes longer, this is because we are searching deeper than in negascout.
This is possible because there are fewer possible lines of play.

    Iterative deepening was implemented to take advantage of all the time alloted
and searches deeper than the current depth if time remains.

    We made changes to the heuristic for better play. Instead of just considering
the number of stones on the board, we also considered mobility, potential
mobility, and position. Of the positions, we gave more weight to corners and
tried to avoid those squares adjacent to corners.

    We tried implementing transposition tables but found that they slowed the
AI down because the program had to deal with overhead for converting and hashing
the boards and moves into a form useable by the hashtable.
