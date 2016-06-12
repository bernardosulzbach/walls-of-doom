#include "game.h"

#include "about.h"
#include "data.h"
#include "constants.h"
#include "menu.h"
#include "game.h"
#include "io.h"
#include "logger.h"
#include "menu.h"
#include "physics.h"
#include "random.h"
#include "record.h"
#include "rest.h"
#include "version.h"

#include <stdlib.h>
#include <string.h>

/**
 * Creates a new Game object with the provided objects.
 */
Game create_game(Player *player, Platform *platforms, const size_t platform_count, BoundingBox *box) {
    Game game;

    game.player = player;
    game.platforms = platforms;
    game.platform_count = platform_count;
    
    game.frame = 0;
    game.played_frames = 0;

    game.box = box;

    game.perk = PERK_NONE;
    game.perk_x = 0;
    game.perk_y = 0;
    game.perk_end_frame = PERK_DURATION_ON_SCREEN_IN_FRAMES; /* Don't start with a Perk on the screen. */

    return game;
}

/**
 * Returns 0 if the screen size has not changed since the creation of the provided Game.
 */
int check_for_screen_size_change(const Game * const game) {
    BoundingBox current_box = bounding_box_from_screen();
    return !bounding_box_equals(&current_box, game->box);
}

void register_score(const Game * const game) {
    const Player * const player = game->player;
    /* Log that we are registering the score */
    char buffer[MAXIMUM_STRING_SIZE];
    const char *format = "Started registering a score of %d points for %s";
    Record record;
    int scoreboard_index;
    int position;
    sprintf(buffer, format, player->score, player->name);
    log_message(buffer);

    /* The name has already been entered to make the Player object. */
    record = make_record(player->name, player->score);

    /* Write the Record to disk */
    scoreboard_index = save_record(&record);
    position = scoreboard_index + 1;

    sprintf(buffer, "Saved the record successfully");
    log_message(buffer);

    print_game_result(player->name, player->score, position);
    
    rest_for_seconds(2);
}

/**
 * Runs the main loop of the provided game and registers the player score at the end.
 *
 * Returns 0 if successful.
 */
int run_game(Game * const game) {
    unsigned long next_played_frames_score = FPS;
    Command command = COMMAND_NONE;
    /* Checking for any nonpositive player.lives value would be safer but could hide some bugs */
    while (command != COMMAND_QUIT && !check_for_screen_size_change(game) && game->player->lives != 0) {
        /* Game loop */
        /* 1. Update the score */
        if (game->played_frames == next_played_frames_score) {
            game->player->score++;
            next_played_frames_score += FPS;
        }
        /* 2. Update the platforms */
        update_platforms(game);
        /* 3. Update the perk */
        update_perk(game);
        /* 4. Draw everything */
        draw_game(game);
        /* 5. Sleep */
        rest_for_second_fraction(FPS);
        /* 6. Read whatever command we got (if any) */
        command = read_next_command();
        /* 7. Update the player using the command */
        update_player(game, command);
        /* 8. Increment the frame counter */
        game->frame++;
    }
    /* Ignoring how the game ended (quit command, screen resize, or death), register the score */
    register_score(game);
    return 0;
}

int main(void) {
    int result;
    initialize();
    seed_random();
    result = main_menu();
    finalize();
    return result;
}
