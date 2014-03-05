#include <cstdio>
#include "common.h"
#include "player.h"
#include "board.h"

// Use this file to test your minimax implementation (2-ply depth, with a
// heuristic of the difference in number of pieces).
int main(int argc, char *argv[]) {

    // Create board with example state. You do not necessarily need to use
    // this, but it's provided for convenience.
    char boardData[64] = {
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
        ' ', 'b', ' ', ' ', ' ', ' ', ' ', ' ', 
        'b', 'w', 'b', 'b', 'b', 'b', ' ', ' ', 
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '
    };
    Board *board = new Board();
    board->setBoard(boardData);

    // Initialize player as the white player, and set testing_minimax flag.
    Player *player = new Player(WHITE);
    player->testingMinimax = true;


    /** 
     * TODO: Write code to set your player's internal board state to the 
     * example state.
     */

    // Get player's move and check if it's right.
    Move *move = player->doMove(NULL, 0);

    if (move != NULL && move->x == 1 && move->y == 1) {
        printf("Correct move: (1, 1)");
    } else {
        printf("Wrong move: got ");
        if (move == NULL) {
            printf("PASS");
        } else {
            printf("(%d, %d)", move->x, move->y);
        }
        printf(", expected (1, 1)\n");
    }

    return 0;
}
