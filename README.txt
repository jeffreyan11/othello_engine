An othello engine, forked from a CS2 project at Caltech.

The engine uses a principal variation search, bitboards, an opening book, an endgame solver, hash tables, and pattern evaluations.

The midgame search uses a standard linked-list hashtable with Zobrist hashing,
and move orders with internal iterative deepening and a piece-square table.

The bitboards are based on the "Classical Approach" to chess bitboards (https://chessprogramming.wikispaces.com/Classical+Approach) and achieve about 11.5 sec
PERFT 11.

Pattern evaluations were trained from the FFO's database of games (http://www.ffothello.org/informatique/la-base-wthor/).

The endgame solver is highly optimized using internal iterative deepening, an optimized hashtable, fastest-first move ordering, special functions for solving
1-4 squares left, and aspiration windows.
Current performance on the FFO test suite (A good explanation is available on http://www.radagast.se/othello/ffotest.html) is 9460 seconds (2.63 hours) and 95.243.311.468 nodes searched.
The test was performed on one core of a i5-2450m, compiled with GCC version 4.6.3 on Ubuntu 12.04.

Known bugs:
none

TODO:
Code cleanup
Fix stability estimater
Faster doMove
Tune evaluation
Improve patterns
Improve midgame hashtable
