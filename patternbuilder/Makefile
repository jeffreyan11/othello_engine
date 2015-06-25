CC          = g++
CFLAGS      = -Wall -ansi -pedantic -ggdb -std=c++0x -g -O3

all:
	$(CC) $(CFLAGS) -o evalbuilder evalbuilder.cpp board.cpp
	$(CC) $(CFLAGS) -o endeval endeval.cpp board.cpp endgame.cpp hash.cpp
	$(CC) $(CFLAGS) -o blur blur.cpp
