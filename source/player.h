#ifndef PLAYER_H
#define PLAYER_H

#include "vector.h"

#define PLAYER_NAME_MAXIMUM_SIZE 64

typedef struct Player {
    char *name;
    
    Vector position;
    Vector velocity;
    
    // Whether or not the player is being affected by physics.
    int physics;
   
    // How many jumps they player has left.
    unsigned int jumps;
  
    unsigned int lives;
    unsigned int score;
} Player;

/**
 * Returns a Player object with the provided name.
 */
Player make_player(char *name);

#endif
