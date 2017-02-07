#include <iostream>
#include <cstdlib>
#include <cstring>
#include "player.h"
using namespace std;

int main(int argc, char *argv[]) {
    // Read in side the player is on.
    if (argc < 2)  {
        cerr << "usage: " << argv[0] << " side [startpos]" << endl;
        exit(-1);
    }
    Side side = (!strcmp(argv[1], "black")) ? BLACK : WHITE;

    // Initialize player.
    Player *player = new Player(side);

    // Send ready signal
    cout << "ready" << endl;
    cout.flush();
    
    int moveX, moveY, msLeft;

    // Get opponent's move and time left for player each turn.
    while (cin >> moveX >> moveY >> msLeft) {
        Move *opponentsMove = NULL;
        if (moveX >= 0 && moveY >= 0) {
            opponentsMove = new Move(moveX, moveY);
        }
        
        // Get player's move and output to java wrapper.
        Move *playersMove = player->doMove(opponentsMove, msLeft);
        if (playersMove != NULL) {
            cout << playersMove->x << " " << playersMove->y << " "
                 << player->game.count(CBLACK) << " " << player->game.count(CWHITE) << endl;
        } else {
            cout << "-1 -1 " << player->game.count(CBLACK) << " " << player->game.count(CWHITE) << endl;
        }
        cout.flush();
        cerr.flush();
        
        // Delete move objects.
        if (opponentsMove != NULL) delete opponentsMove;
        if (playersMove != NULL) delete playersMove;
    }

    return 0;
}
