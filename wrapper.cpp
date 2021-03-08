#include <iostream>
#include <cstdlib>
#include <cstring>
#include "eval.h"
#include "player.h"
using namespace std;

int main(int argc, char *argv[]) {
  // Read in side the player is on.
  if (argc != 2)  {
    cerr << "usage: " << argv[0] << " side" << endl;
    exit(-1);
  }
  Color side = (!strcmp(argv[1], "Black")) ? BLACK : WHITE;

  // Initialize player.
  init_eval();
  Player *player = new Player(side, true, /*tt_bits=*/20);
  // The Java GUI does terribly at handling overhead time.
  player->bufferPerMove = 50;
  resize_endhash(14);

  // Tell java wrapper that we are done initializing.
  cout << "Init done" << endl;
  cout.flush();
  
  int moveX, moveY, msLeft;

  // Get opponent's move and time left for player each turn.
  while (cin >> moveX >> moveY >> msLeft) {
    int opponentsMove = MOVE_NULL;
    if (moveX >= 0 && moveY >= 0) {
      opponentsMove = move_xy(moveX, moveY);
    }
    
    // Get player's move and output to java wrapper.
    int playersMove = player->do_move(opponentsMove, msLeft);
    if (playersMove != MOVE_NULL) {
      cout << move_col(playersMove) << " " << move_row(playersMove) << endl;
    } else {
      cout << "-1 -1" << endl;
    }
    cout.flush();
    cerr.flush();
  }

  return 0;
}
