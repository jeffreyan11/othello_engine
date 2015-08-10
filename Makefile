CC          = g++
CFLAGS      = -Wall -ansi -pedantic -ggdb -std=c++0x -g -O3
OBJS        = common.o player.o board.o openings.o endgame.o hash.o eval.o endhash.o
PLAYERNAME  = ShallowKwok

all: $(PLAYERNAME) testgame testsuites
evaltools: evalbuilder endeval blur tuneheuristic crtbk
	
$(PLAYERNAME): $(OBJS) wrapper.o
	$(CC) -o $@ $^

testgame: testgame.o
	$(CC) -o $@ $^

testsuites: $(OBJS) testsuites.o
	$(CC) -o $@ $^

tuneheuristic: $(OBJS) tuneheuristic.o
	$(CC) -o $@ $^

evalbuilder: common.o board.o endgame.o eval.o endhash.o hash.o patternbuilder.o evalbuilder.o
	$(CC) -o $@ $^

endeval: common.o board.o endgame.o eval.o endhash.o hash.o patternbuilder.o endeval.o
	$(CC) -o $@ $^

blur:
	$(CC) $(CFLAGS) -o blur blur.cpp

crtbk: $(OBJS) crtbk.o
	$(CC) -o $@ $^

%.o: %.cpp
	$(CC) -c $(CFLAGS) -x c++ $< -o $@
	
java:
	make -C java/

cleanjava:
	make -C java/ clean

clean:
	rm -f *.o *.exe $(PLAYERNAME) testgame testsuites tuneheuristic evalbuilder endeval blur crtbk
	
.PHONY: java
