#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include "eval.h"
#include "player.h"
using namespace std;

int main(int argc, char *argv[]) {
  // Read in side the player is on.
  if (argc < 2)  {
    cerr << "usage: " << argv[0] << " side [startpos]" << endl;
    exit(-1);
  }
  Color side = (!strcmp(argv[1], "black")) ? BLACK : WHITE;

  // Initialize player.
  init_eval();
  Player *player = new Player(side, false, /*tt_bits=*/16);
  resize_endhash(10);

  // If an opening position is given:
  if (argc == 4) {
    uint64_t taken_bits, black_bits;
    std::stringstream ss;
    ss << std::hex << argv[2];
    ss >> taken_bits;
    ss.str("");
    ss.clear();
    ss << std::hex << argv[3];
    ss >> black_bits;
    player->set_position(taken_bits, black_bits);
  }

  // Send ready signal
  std::string rstr;
  while (true) {
    cin >> rstr;
    if (rstr.compare("isready") == 0) {
      cout << "ready" << endl;
      cout.flush();
      break;
    }
  }
  
  int moveX, moveY, msLeft;

  // Get opponent's move and time left for player each turn.
  while (cin >> moveX >> moveY >> msLeft) {
    int opponentsMove = MOVE_NULL;
    if (moveX >= 0 && moveY >= 0) {
      opponentsMove = move_xy(moveX, moveY);
    }
    
    // Get player's move and output to python wrapper.
    int playersMove = player->do_move(opponentsMove, msLeft);
    if (playersMove != MOVE_NULL) {
      cout << move_col(playersMove) << " " << move_row(playersMove) << " "
         << player->game.count(BLACK) << " " << player->game.count(WHITE) << endl;
    } else {
      cout << "-1 -1 " << player->game.count(BLACK) << " " << player->game.count(WHITE) << endl;
    }
    cout.flush();
    cerr.flush();
  }

  return 0;
}
