CC          = g++
CFLAGS      = -Wall -Wshadow -ansi -pedantic -ggdb -std=c++11 -g -O3 -flto
LDFLAGS     = -static -static-libgcc -static-libstdc++
OBJS        = common.o board.o endgame.o endhash.o eval.o hash.o openings.o player.o search.o
PLAYERNAME  = Flippy

all: $(PLAYERNAME)$(EXT) $(PLAYERNAME)$(EXT)T testgame testsuites
evaltools: evalbuilder tuneheuristic crtbk
	
$(PLAYERNAME)$(EXT): $(OBJS) wrapper.o
	$(CC) -O3 -flto -o $@ $^

$(PLAYERNAME)$(EXT)T: $(OBJS) protocol.o
	$(CC) -O3 -flto -o $@ $^ $(LDFLAGS)

testgame: testgame.o
	$(CC) -o $@ $^

testsuites: $(OBJS) testsuites.o
	$(CC) -O3 -flto -o $@ $^

tuneheuristic: $(OBJS) patternbuilder.o tuneheuristic.o
	$(CC) -o $@ $^

evalbuilder: $(OBJS) patternbuilder.o evalbuilder.o
	$(CC) -O3 -flto -o $@ $^

crtbk: $(OBJS) crtbk.o
	$(CC) -o $@ $^

%.o: %.cpp
	$(CC) -c $(CFLAGS) -x c++ $< -o $@
	
java:
	make -C java/

cleanjava:
	make -C java/ clean

clean:
	rm -f *.o $(PLAYERNAME)$(EXT).exe $(PLAYERNAME)$(EXT)T.exe $(PLAYERNAME)$(EXT) $(PLAYERNAME)$(EXT)T testgame testsuites tuneheuristic evalbuilder crtbk tuneheuristic.exe evalbuilder.exe crtbk.exe testgame.exe testsuites.exe
	
.PHONY: java
