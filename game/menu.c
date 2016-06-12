#include "menu.h"

#include "about.h"
#include "constants.h"
#include "data.h"
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

#include <curses.h>

#define MAXIMUM_RECORD_ARRAY_SIZE 128

typedef struct Menu {
    char *title;
    char **options;
    size_t option_count;
    size_t selected_option;
} Menu;

/**
 * Returns the number of characters used to represent the provided number on
 * base 10.
 */
int count_digits(int number) {
    char buffer[MAXIMUM_STRING_SIZE];
    sprintf(buffer, "%d", number);
    return strlen(buffer);
}

void record_to_string(const Record * const record, char *buffer, const int expected_width) {
    char padding_string[MAXIMUM_STRING_SIZE];
    memset(padding_string, '.', MAXIMUM_STRING_SIZE - 1);
    padding_string[MAXIMUM_STRING_SIZE - 1] = '\0';
    const char format[] = "%s%*.*s%d";
    int padding_length = expected_width - strlen(record->name) - count_digits(record->score);
    sprintf(buffer, format, record->name, padding_length, padding_length, padding_string, record->score);
}

void top_scores(void) {
    if (COLS < 16) {
        return;
    }
    const int line_width = COLS - 6;
    Record records[MAXIMUM_RECORD_ARRAY_SIZE];
    int y = 2;
    const int line_count = LINES - 2 * y;
    size_t maximum_read_records;
    if (line_count > MAXIMUM_RECORD_ARRAY_SIZE) {
        maximum_read_records = MAXIMUM_RECORD_ARRAY_SIZE;
    } else {
        maximum_read_records = line_count;
    }
    size_t actually_read_records = read_records(records, maximum_read_records);
    char line[COLS];
    size_t i;
    clear();
    for (i = 0; i < actually_read_records; i++) {
        record_to_string(records + i, line, line_width);
        print_centered(y + i, line);
    }
    refresh();
    rest_for_seconds(2);
}

void write_menu(const Menu * const menu) {
    clear();
    const size_t entries = menu->option_count + 1;
    const unsigned int ENTRY_HEIGHT = 3;
    const unsigned int height = entries * ENTRY_HEIGHT;
    const int starting_y = (LINES - height) / 2;
    int y = starting_y + 1;
    print((COLS - strlen(menu->title)) / 2, y, menu->title);
    size_t i;
    for (i = 0; i < menu->option_count; i++) {
        char *string = menu->options[i];
        if (i == menu->selected_option) {
            char buffer[MAXIMUM_STRING_SIZE];
            sprintf(buffer, "> %s <", string);
            string = buffer;
        }
        const int x = (COLS - strlen(string)) / 2;
        y += ENTRY_HEIGHT;
        if (i == menu->selected_option) {
            attron(A_BOLD);
        }
        print(x, y, string);
        if (i == menu->selected_option) {
            attroff(A_BOLD);
        }
    }
}

int read_platforms(Platform *platforms) {
    const size_t INTEGER_ARRAY_SIZE = 1 + 2 * MAXIMUM_PLATFORM_COUNT;
    int input_integers[INTEGER_ARRAY_SIZE];
    size_t actually_read;
    size_t platform_count;
    char log_message_buffer[256];
    size_t i;
    int speed;
    int movement_type;

    log_message("Started reading platform data");
    actually_read = read_integers("assets/platforms.txt", input_integers, INTEGER_ARRAY_SIZE);

    sprintf(log_message_buffer, "Read %lu integers", (unsigned long) actually_read);
    log_message(log_message_buffer);

    if (actually_read > 0) {
        platform_count = input_integers[0] < MAXIMUM_PLATFORM_COUNT ? input_integers[0] : MAXIMUM_PLATFORM_COUNT;
    } else {
        platform_count = 0;
    }
    sprintf(log_message_buffer, "Platform count is %lu", (unsigned long) platform_count);
    log_message(log_message_buffer);

    for (i = 0; i < platform_count; i++) {
        Platform *platform = platforms + i;

        platform->width = input_integers[1 + 2 * i];

        platform->x = random_integer(1, COLS - 1);
        platform->y = random_integer(4, LINES - 4);

        platform->speed_x = 0;
        platform->speed_y = 0;
        speed = input_integers[1 + 2 * i + 1] * PLATFORM_BASE_SPEED;
        movement_type = random_integer(0, 4);
        if (movement_type < 2) { /* 40% */
            platform->speed_x = speed;
        } else if (movement_type < 4) { /* 40% */
            platform->speed_x = -speed;
        } else { /* 20% */
            platform->speed_y = -speed;
        }
    }

    return platform_count;
}

int game(void) {
    char name[PLAYER_NAME_MAXIMUM_SIZE];
    size_t platform_count;
    Player player;
    Platform platforms[MAXIMUM_PLATFORM_COUNT];
    BoundingBox box;
    Game game;

    read_player_name(name, PLAYER_NAME_MAXIMUM_SIZE);

    player = make_player(name);
    player.x = COLS / 2;
    player.y = LINES / 2;

    box = bounding_box_from_screen();

    platform_count = read_platforms(platforms);

    game = create_game(&player, platforms, platform_count, &box);

    run_game(&game);
    return 0;
}

int main_menu(void) {
    Menu menu;
    char title[MAXIMUM_STRING_SIZE];
    sprintf(title, "%s version %s", "Walls of Doom", WALLS_OF_DOOM_VERSION);
    menu.title = title;
    char *options[] = {"Play", "Top Scores", "Info", "Quit"};
    menu.options = options;
    menu.option_count = 4;
    menu.selected_option = 0;

    int got_quit = 0;
    while (!got_quit) {
        write_menu(&menu);
        Command command = wait_for_next_command();
        if (command == COMMAND_UP) {
            if (menu.selected_option > 0) {
                menu.selected_option--;
            }
        } else if (command == COMMAND_DOWN) {
            if (menu.selected_option + 1 < menu.option_count) {
                menu.selected_option++;
            }
        } else if (command == COMMAND_ENTER || command == COMMAND_CENTER) {
            if (menu.selected_option == 0) {
                game();
            } else if (menu.selected_option == 1) {
                top_scores();
            } else if (menu.selected_option == 2) {
                info();
            } else if (menu.selected_option == 3) {
                got_quit = 1;
            }
        }
    }
    return 0;
}

