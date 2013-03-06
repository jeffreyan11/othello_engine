#include "exampleplayer.h"

/*
 * Constructor for the player; initialize everything here. The side your AI is
 * on (BLACK or WHITE) is passed in as "side". The constructor must finish 
 * within 30 seconds.
 */
ExamplePlayer::ExamplePlayer(Side side) {
    /* 
     * TODO: Do any initialization you need to do here (setting up the board,
     * precalculating things, etc.) However, remember that you will only have
     * 30 seconds.
     */
}

/*
 * Destructor for the player.
 */
ExamplePlayer::~ExamplePlayer() {
}


/*
 * Compute the next move given the opponent's last move. Each AI is
 * expected to keep track of the board on its own. If this is the first move,
 * or if the opponent passed on the last move, then opponentsMove will be NULL.
 *
 * If there are no valid moves for your side, doMove must return NULL.
 *
 * Important: doMove must take no longer than the timeout passed in 
 * msLeft, or your AI will lose! The move returned must also be legal.
 */
Move *ExamplePlayer::doMove(Move *opponentsMove, int msLeft) {
    /* 
     * TODO: Implement how moves your AI should play here. You should first
     * process the opponent's opponents move before calculating your own move
     */ 
    return NULL;
}
    
