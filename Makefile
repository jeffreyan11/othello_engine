CC          = g++
CFLAGS      = -Wall -ansi -pedantic -ggdb -std=c++0x -g -O3
OBJS        = common.o player.o board.o openings.o endgame.o hash.o eval.o endhash.o
PLAYERNAME  = ShallowKwok

all: $(PLAYERNAME) testgame memtest
evaltools: evalbuilder endeval blur testheuristic crtbk
	
$(PLAYERNAME): $(OBJS) wrapper.o
	$(CC) -o $@ $^

testgame: testgame.o
	$(CC) -o $@ $^

memtest: $(OBJS) memtest.o
	$(CC) -o $@ $^

testheuristic: $(OBJS) testheuristic.o
	$(CC) -o $@ $^

evalbuilder: common.o board.o endgame.o eval.o endhash.o evalbuilder.o
	$(CC) -o $@ $^

endeval: common.o board.o endgame.o eval.o endhash.o endeval.o
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
	rm -f *.o *.exe $(PLAYERNAME) testgame memtest testheuristic evalbuilder endeval blur crtbk
	
.PHONY: java
