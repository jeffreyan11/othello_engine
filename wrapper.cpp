#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "exampleplayer.h"
using namespace std;

int main(int argc, char *argv[]) {    
    // Read in side the player is on.
    if (argc != 2)  {
        cerr << "usage: " << argv[0] << " side" << endl;
        exit(-1);
    }
    Side side = (!strcmp(argv[1], "Black")) ? BLACK : WHITE;

    // Initialize player.
    ExamplePlayer *player = new ExamplePlayer(side);

    // Tell java wrapper that we are done initializing.
    cout << "Init done" << endl;
    cout.flush();    
    
    while (true) {
        // Get opponent's move and time left for player.
        int moveX, moveY, msLeft;
        cin >> moveX >> moveY >> msLeft;
        Move *opponentsMove = NULL;
        if (moveX >= 0 && moveY >= 0) {
            opponentsMove = new Move(moveX, moveY);
        }
        
        // Get player's move and output to java wrapper.
        Move *playersMove = player->doMove(opponentsMove, msLeft);                        
        if (playersMove != NULL) {                  
            cout << playersMove->x << " " << playersMove->y << endl;
        } else {
            cout << "-1 -1" << endl;
        }
        cout.flush();
        
        // Delete move objects.
        if (opponentsMove != NULL) delete opponentsMove;
        if (playersMove != NULL) delete playersMove; 
    }

    return 0;
}
