CC          = g++
CFLAGS      = -Wall -ansi -pedantic -ggdb -std=c++0x -g -O2
OBJS        = player.o board.o openings.o
PLAYERNAME  = the_riders_of_rohan

all: $(PLAYERNAME) testgame
	
$(PLAYERNAME): $(OBJS) wrapper.o
	$(CC) -o $@ $^

testgame: testgame.o
	$(CC) -o $@ $^

%.o: %.cpp
	$(CC) -c $(CFLAGS) -x c++ $< -o $@
	
java:
	make -C java/

cleanjava:
	make -C java/ clean

clean:
	rm -f *.o $(PLAYERNAME) testgame testminimax
	
.PHONY: java testminimax
