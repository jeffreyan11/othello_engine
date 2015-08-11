An othello engine, forked from a CS 2 project at Caltech. The Java GUI and
wrapper come in a precompiled JAR from the CS 2 class. Run make in the main
directory and then run
testgame [player1] [player2] [timelimit]
where player1 and player2 are the names of program executables, or "Human".
Time limit is given in milliseconds.

To compile the tools used to create the opening book and pattern evaluations,
run "make evaltools". These are currently still a work in progress and are buggy
and not well documented.
    - evalbuilder: creates midgame evaluation patterns
    - endeval: creates endgame evaluation patterns
    - tuneheuristic: self-plays engine using heuristic and heuristic2 on
        16400 games, white and black on each of the 8200 PERFT 6 positions
    - crtbk: creates an opening book using the engine

The engine uses a principal variation search, bitboards, an opening book, an
endgame solver, hash tables, and pattern evaluations.

The midgame search uses a two bucket hashtable with Zobrist hashing,
and move orders with internal iterative deepening and a piece-square table.

The bitboards are based on the "Classical Approach" to chess bitboards
(https://chessprogramming.wikispaces.com/Classical+Approach) and achieve about
11.5 sec PERFT 11.

Pattern evaluations were trained from the FFO's database of games
(http://www.ffothello.org/informatique/la-base-wthor/).

The endgame solver is highly optimized using internal iterative deepening, an
optimized hashtable, fastest-first move ordering, special functions for solving
1-4 squares left, and aspiration windows.
Current performance on the FFO test suite (A good explanation is available on
http://www.radagast.se/othello/ffotest.html) is 9190 seconds (2.55 hours) and
93.279.942.553 nodes searched.
The test was performed on one core of a i5-2450m, compiled with GCC
version 4.6.3 on Ubuntu 12.04.

Known bugs:
none

TODO:
Code cleanup
Fix stability estimater
Faster doMove
Tune evaluation
Improve patterns
