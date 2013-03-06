CC          = g++
CFLAGS      = -Wall -ansi -pedantic -ggdb
OBJS        = exampleplayer.o wrapper.o board.o
PLAYERNAME  = player

all: $(PLAYERNAME) testgame
	
$(PLAYERNAME): $(OBJS)
	$(CC) -o $@ $^
        
TestGame: testgame.o
	$(CC) -o $@ $^
        
%.o: %.cpp
	$(CC) -c $(CFLAGS) -x c++ $< -o $@
	
java:
	make -C java/

cleanjava:
	make -C java/ clean

clean:
	rm -f *.o $(PLAYERNAME) testgame		
	
.PHONY: java
