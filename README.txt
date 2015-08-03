An othello engine, forked from a CS2 project at Caltech.
The engine uses a principal variation search, bitboards, an opening book, an endgame solver, hash tables, and pattern evaluations.

Current performance on the FFO test suite (A good explanation is available on http://radagast.se/othello/index.html) is 12866 seconds (3.57 hours) and 77654840677 nodes searched.

TODO:
Code cleanup
Faster doMove
Tune evaluation
Improve patterns
Improve midgame hashtable
Better timing/break out of search scheme