CC          = g++
CFLAGS      = -Wall -ansi -pedantic -ggdb -std=c++0x -g -O3

all:
	$(CC) $(CFLAGS) -o evalbuilder evalbuilder.cpp board.cpp endgame.cpp hash.cpp
	$(CC) $(CFLAGS) -o endeval endeval.cpp board.cpp endgame.cpp hash.cpp
	$(CC) $(CFLAGS) -o blur blur.cpp
	$(CC) $(CFLAGS) -o genetic genetic.cpp board.cpp endgame.cpp hash.cpp

genetic:
	$(CC) $(CFLAGS) -o genetic genetic.cpp board.cpp endgame.cpp hash.cpp

clean:
	rm -f *.o evalbuilder endeval blur genetic
