An othello engine, forked from a CS2 project at Caltech.
The engine uses a principal variation search, bitboards, an opening book, an endgame solver, hash tables, and pattern evaluations.

Current performance on the FFO test suite (A good explanation is available on http://radagast.se/othello/index.html) is 12866 seconds (3.57 hours) and 77654840677 nodes searched.
The test was performed on one core of a i5-5200u, compiled with MinGW, GCC version 4.9.2

TODO:
Code cleanup
Fix stability estimater
Faster doMove
Tune evaluation
Improve patterns
Improve midgame hashtable
Better timing/break out of search scheme
