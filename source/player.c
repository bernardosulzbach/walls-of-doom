#include "player.h"

Player make_player(char *name) {
    Player player;
    player.name = name;

    player.position.x = 0.0;
    player.position.y = 0.0;

    player.velocity.x = 0.0;
    player.velocity.y = 0.0;

    player.physics = 0;

    player.jumps = 2;

    player.lives = 3;

    player.score = 0;

    return player;
}
