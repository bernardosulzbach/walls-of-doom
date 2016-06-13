#include "io.h"

#include "constants.h"
#include "game.h"
#include "logger.h"
#include "math.h"
#include "physics.h"
#include "player.h"
#include "rest.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <locale.h>
#include <curses.h>

#define MAXIMUM_LINE_WIDTH 80

/**
 * Initializes the color schemes used to render the game.
 */
int initialize_color_schemes(void) {
    init_pair(COLOR_TOP_BAR, COLOR_CYAN, COLOR_MAGENTA);
    init_pair(COLOR_BOTTOM_BAR, COLOR_BLACK, COLOR_YELLOW);
    init_pair(COLOR_PLATFORMS, COLOR_WHITE, COLOR_WHITE);
    init_pair(COLOR_PLAYER, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_INVINCIBILITY, COLOR_BLACK, COLOR_YELLOW);
    init_pair(COLOR_LEVITATION, COLOR_BLACK, COLOR_WHITE); /* Requested was grey */
    init_pair(COLOR_LOW_GRAVITY, COLOR_BLACK, COLOR_BLUE); /* Requested was purple */
    init_pair(COLOR_SUPER_JUMP, COLOR_BLACK, COLOR_CYAN);
    init_pair(COLOR_TIME_STOP, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(COLOR_EXTRA_LIFE, COLOR_BLACK, COLOR_GREEN);
    init_pair(COLOR_EXTRA_POINTS, COLOR_BLACK, COLOR_RED); /* Requested was brown */
    return 0;
}

void log_terminal_color_support(void) {
    char log_buffer[MAXIMUM_STRING_SIZE];
    sprintf(log_buffer, "Current terminal supports %d colors", COLORS);
    log_message(log_buffer);
    sprintf(log_buffer, "Current terminal supports %d color pairs", COLOR_PAIRS);
    log_message(log_buffer);
}

/**
 * Initializes the required resources.
 *
 * Should only be called once, right after starting.
 */
void initialize(void) {
    initialize_logger();
    /* Enable wide characters. */
    setlocale(LC_ALL, "");
    /* Initialize the screen. */
    initscr();
    /* Prevent terminal echo. */
    noecho();
    /* Prevent delay from getch(). */
    nodelay(stdscr, TRUE);
    /* Enable function keys such as F1 and the arrow keys */
    keypad(stdscr, TRUE);
    /* Do not display the cursor. */
    curs_set(FALSE);
    /* Initialize the coloring functionality. */
    start_color();
    log_terminal_color_support();
    initialize_color_schemes();
}

void enable_string_input(void) {
    fflush(stdin);
    echo();
    /* Display the cursor. */
    curs_set(TRUE);
    nodelay(stdscr, FALSE);
}

void disable_string_input(void) {
    noecho();
    /* Do not display the cursor. */
    curs_set(FALSE);
    nodelay(stdscr, TRUE);
}

/**
 * Finalizes the acquired resources.
 *
 * Should only be called once, right before exiting.
 */
void finalize(void) {
    finalize_logger();
    endwin();
}

/**
 * Enables echo and reads a string from the user.
 *
 * Returns 0 in case of success.
 */
int read_string(char *destination, const size_t maximum_size) {
    int result;
    enable_string_input();
    result = getnstr(destination, maximum_size - 1);
    disable_string_input();
    if (result == ERR) { /* Got curses error code. */
        log_message("Got an error when reading string");
        return 1;
    } else {
        return 0;
    }
}

/**
 * Returns a pointer to the start of the text of the string.
 *
 * This is either the first character which is not a space or '\0'.
 */
char *find_start_of_text(char *string) {
    while (*string != '\0' && isspace(*string)) {
        string++;
    }
    return string;
}

/**
 * Returns a pointer to the end of the text of the string.
 *
 * This is either the first trailing space or '\0'.
 */
char *find_end_of_text(char *string) {
    char *last_not_space = string;
    while (*string != '\0') {
        if (!isspace(*string)) {
            last_not_space = string;
        }
        string++;
    }
    if (*last_not_space != '\0') {
        last_not_space++;
    }
    return last_not_space;
}

/**
 * Trims a string by removing all leading and trailing spaces.
 */
void trim_string(char *string) {
    int copying = 0;
    char *write = string;
    char *read = string;
    /* Copy everthing from the first not space up to the end. */
    while (*read != '\0') {
        if (!copying) {
            copying = !isspace(*read);
        }
        if (copying) {
            *write = *read;
            write++;
        }
        read++;
    }
    *write = '\0';
    /* Replace all trailing spaces by the null character. */
    if (write != string) { /* At the first position there is not a space. */
        write--;
        while (isspace(*write)) {
            *write = '\0';
            write--;
        }
    }
}

/**
 * Evaluates whether or not a Player name is a valid name.
 *
 * A name is considered to be valid if it has at least two characters after being trimmed.
 */
int is_valid_player_name(const char *player_name) {
    char buffer[MAXIMUM_PLAYER_NAME_SIZE];
    strcpy(buffer, player_name);
    trim_string(buffer);
    return strlen(buffer) >= 2;
}

void read_player_name(char *destination, const size_t maximum_size) {
    int read_error = 0;
    int valid_name = 0;
    const char message[] = "Name your character: ";
    const int message_size = strlen(message);
    const int maximum_width = message_size + maximum_size;
    char log_buffer[MAXIMUM_STRING_SIZE];
    /* While there is not a read error or a valid name. */
    while (!read_error && !valid_name) {
        clear();
        if (maximum_width <= COLS) {
            print((COLS - maximum_width) / 2, LINES / 2, message);
        } else {
            print(0, LINES / 2, message);
        }
        refresh();
        read_error = read_string(destination, maximum_size);
        if (read_error) {
            log_message("Failed to read player name");
            /* Cope with it by providing a name for the player. */
            strcpy(destination, "ERROR READING PLAYER NAME");
        } else {
            sprintf(log_buffer, "Read '%s' from the user", destination);
            log_message(log_buffer);
            /* Trim the name the user entered. */
            trim_string(destination);
            sprintf(log_buffer, "Trimmed the input to '%s'", destination);
            log_message(log_buffer);
            valid_name = is_valid_player_name(destination);
        }
    }
}

/**
 * Prints the provided string on the screen starting at (x, y).
 */
void print(const int x, const int y, const char *string) {
    /* Validate that x and y are nonnegative. */
    if (x < 0 || y < 0) {
        return;
    }
    mvprintw(y, x, string);
}

/**
 * Prints the provided string centered on the screen at the provided line.
 */
void print_centered(const int y, const char *string) {
    const int x = (COLS - strlen(string)) / 2;
    print(x, y, string);
}

/**
 * Replaces all newlines, carriage returns, and tabs by the ASCII space.
 */
void normalize_whitespaces(char *string) {
    char c;
    size_t i;
    for (i = 0; string[i] != '\0'; i++) {
        c = string[i];
        if (c == '\n' || c == '\t') {
            string[i] = ' ';
        }
    }
}

void wrap_at_right_margin(char *string, const size_t columns) {
    size_t last_line_start = 0;
    size_t string_length = strlen(string);
    while (string_length - last_line_start > columns) {
        size_t next_line_start = last_line_start + columns;
        while (string[next_line_start] != ' ') {
            next_line_start--;
            if (next_line_start == last_line_start) {
               /* There are no spaces in this line, so we can't do anything. */
               /* Abort, simply. */
               break;
            }
            next_line_start--;
        }
        string[next_line_start] = '\n';
        last_line_start = next_line_start;
    }
}

int count_lines(char *buffer) {
    size_t counter = 0;
    size_t i = 0;
    while (buffer[i] != '\0') {
        if (buffer[i] == '\n') {
            counter++;
        }
        i++;
    }
    return counter;
}

char *copy_first_line(char *source, char *destination) {
    while (*source != '\0' && *source != '\n') {
        *destination++ = *source++;
    }
    *destination = '\0';
    if (*source == '\0') {
        return source;
    } else {
        return source + 1;
    }
}

void pad_line_right(char *line, const size_t width) {
    size_t i;
    for (i = strlen(line); i < width; i++) {
        line[i] = ' ';
    }
    line[width] = '\0';
}

/**
 * Prints the provided string after formatting it to increase readability.
 */
void print_long_text(char *string) {
    size_t lines_copied = 0;
    char line[MAXIMUM_LINE_WIDTH];
    char *cursor;
    int width = MAXIMUM_LINE_WIDTH;
    int line_count;
    normalize_whitespaces(string);
    if (MAXIMUM_LINE_WIDTH > COLS - 2) {
        width = COLS - 2;
    }
    wrap_at_right_margin(string, width);
    line_count = count_lines(string);
    clear();
    /* Print each line. */
    cursor = string;
    while (*cursor != '\0') {
        cursor = copy_first_line(cursor, line);
        pad_line_right(line, width);
        print_centered((LINES - line_count) / 2 + lines_copied, line);
        lines_copied++;
    }
    refresh();
}

/**
 * Prints the provided Platform, respecting the BoundingBox.
 */
void print_platform(const Platform * const platform, const BoundingBox * const box) {
    int i;
    int x;
    int y;
    for (i = 0; i < platform->width; i++) {
        x = platform->x + i;
        y = platform->y;
        if (x >= box->min_x && x <= box->max_x && y >= box->min_y && y <= box->max_y) {
            print(x, y, " ");
        }
    }
}

/**
 * Draws the top status bar on the screen for a given Player.
 *
 * Returns 0 if successful.
 */
int draw_top_bar(const Player * const player) {
    char power_buffer[MAXIMUM_STRING_SIZE];
    char lives_buffer[MAXIMUM_STRING_SIZE];
    char score_buffer[MAXIMUM_STRING_SIZE];

    char *strings[TOP_BAR_STRING_COUNT];

    const int columns_per_string = COLS / TOP_BAR_STRING_COUNT;

    int begin_x;
    int after_x;
    int begin_text_x;
    int after_text_x;

    int string_length;

    int x;
    int i;
    
    if (player->perk != PERK_NONE) {
        sprintf(power_buffer, "%s", get_perk_name(player->perk));
    } else {
        sprintf(power_buffer, "No Power");
    }

    sprintf(lives_buffer, "Lives: %d", player->lives);

    sprintf(score_buffer, "Score: %d", player->score);

    strings[0] = GAME_NAME;
    strings[1] = power_buffer;
    strings[2] = lives_buffer;
    strings[3] = score_buffer;

    attron(COLOR_PAIR(COLOR_TOP_BAR));
    attron(A_BOLD);
    for (i = 0; i < TOP_BAR_STRING_COUNT; i++) {
        begin_x = i * columns_per_string;
        after_x = (i + 1) * columns_per_string;
        if (i + 1 == TOP_BAR_STRING_COUNT) {
            /*
             * If this is the last string on the top, make sure that we will
             * write enough colored spaces. Integer division truncates, which
             * can make 4 * (COLS / 4) != COLS. Therefore, some spaces may be
             * left uncolored at the end if we do not ensure that all columns
             * are painted.
             */
            after_x = COLS;
        }
        string_length = strlen(strings[i]);
        if (string_length < columns_per_string) {
            /*
             * Write the string because it fits.
             */
            begin_text_x = begin_x + (columns_per_string - string_length) / 2;
            after_text_x = begin_text_x + string_length;
            for (x = begin_x; x < begin_text_x; x++) {
                print(x, 0, " ");
            }
            print(begin_text_x, 0, strings[i]);
            for (x = after_text_x; x < after_x; x++) {
                print(x, 0, " ");
            }
        } else {
            /*
             * String does not fit, do not write it.
             */
            for (x = begin_x; x < after_x; x++) {
                print(x, 0, " ");
            }
        }
    }
    attroff(A_BOLD);
    attroff(COLOR_PAIR(COLOR_TOP_BAR));

    return 0;
}

/*
 * Draws the bottom status bar on the screen for a given Player.
 *
 * Returns 0 if successful.
 */
int draw_bottom_bar(void) {
    int i;
    attron(COLOR_PAIR(COLOR_BOTTOM_BAR));
    for (i = 0; i < COLS; i++) {
        print(i, LINES - 1, " ");
    }
    attroff(COLOR_PAIR(COLOR_BOTTOM_BAR));
    return 0;
}

int draw_borders(void) {
    int i;
    for (i = 1; i < COLS - 1; i++) {
        print(i, 1, "+");
    }
    for (i = 1; i < LINES - 1; i++) {
        print(0, i, "+");
    }
    for (i = 1; i < COLS - 1; i++) {
        print(i, LINES - 2, "+");
    }
    for (i = 1; i < LINES - 1; i++) {
        print(COLS - 1, i, "+");
    }
    return 0;
}

int draw_platforms(const Platform * platforms, const size_t platform_count, const BoundingBox * const box) {
    size_t i;
    attron(COLOR_PAIR(COLOR_PLATFORMS));
    for (i = 0; i < platform_count; i++) {
        print_platform(&platforms[i], box);
    }
    attroff(COLOR_PAIR(COLOR_PLATFORMS));
    return 0;
}

int has_active_perk(const Game * const game) {
    return game->perk != PERK_NONE;
}

ColorScheme get_perk_color(Perk perk) {
    if (perk == PERK_POWER_INVINCIBILITY) {
        return COLOR_INVINCIBILITY;
    } else if (perk == PERK_POWER_LEVITATION) {
        return COLOR_LEVITATION;
    } else if (perk == PERK_POWER_LOW_GRAVITY) {
        return COLOR_LOW_GRAVITY;
    } else if (perk == PERK_POWER_SUPER_JUMP) {
        return COLOR_SUPER_JUMP;
    } else if (perk == PERK_POWER_TIME_STOP) {
        return COLOR_TIME_STOP;
    } else if (perk == PERK_BONUS_EXTRA_POINTS) {
        return COLOR_EXTRA_POINTS;
    } else if (perk == PERK_BONUS_EXTRA_LIFE) {
        return COLOR_EXTRA_LIFE;
    } else {
        char buffer[MAXIMUM_STRING_SIZE];
        sprintf(buffer, "get_perk_color called with invalid value: %d", perk);
        log_message(buffer);
        return COLOR_PLAYER;
    }
}

int draw_perk(const Game * const game) {
    if (has_active_perk(game)) {
        attron(COLOR_PAIR(get_perk_color(game->perk)));
        attron(A_BOLD);
        print(game->perk_x, game->perk_y, get_perk_symbol());
        attroff(A_BOLD);
        attroff(COLOR_PAIR(get_perk_color(game->perk)));
    }
    return 0;
}

int draw_player(const Player * const player) {
    attron(COLOR_PAIR(COLOR_PLAYER));
    print(player->x, player->y, PLAYER_SYMBOL);
    attroff(COLOR_PAIR(COLOR_PLAYER));
    return 0;
}

/**
 * Draws a full game to the screen.
 */
int draw_game(const Game * const game) {
    clear();
  
    draw_top_bar(game->player);
    draw_bottom_bar();
    draw_borders();
    draw_platforms(game->platforms, game->platform_count, game->box);
    draw_perk(game);
    draw_player(game->player);

    refresh();

    return 0;
}

void print_game_result(const char *name, const unsigned int score, const int position) {
    char first_line[MAXIMUM_STRING_SIZE];
    char second_line[MAXIMUM_STRING_SIZE];
    sprintf(first_line, "%s died after making %d points.", name, score);
    if (position > 0) {
        sprintf(second_line, "%s got to position %d!", name, position);
    } else {
        sprintf(second_line, "%s didn't make it to the top scores.", name);
    }
    clear();
    print_centered(LINES / 2 - 1, first_line);
    print_centered(LINES / 2 + 1, second_line);
    refresh();
}

/**
 * Returns a BoundingBox that represents the playable area after removing bars
 * and margins.
 */
BoundingBox bounding_box_from_screen(void) {
    BoundingBox box;
    box.min_x = 1;
    box.min_y = 2; /* Top bar. */
    box.max_x = COLS - 2;
    box.max_y = LINES - 3; /* Bottom bar. */
    return box;
}

/**
 * Returns the Command value corresponding to the provided input code.
 */
Command command_from_input(const int input) {
    if (input == '8' || input == KEY_UP) {
        return COMMAND_UP;
    } else if (input == '4' || input == KEY_LEFT) {
        return COMMAND_LEFT;
    } else if (input == '5') {
        return COMMAND_CENTER;
    } else if (input == '6' || input == KEY_RIGHT) {
        return COMMAND_RIGHT;
    } else if (input == '2' || input == KEY_DOWN) {
        return COMMAND_DOWN;
    } else if (input == 'q' || input == 'Q') {
        return COMMAND_QUIT;
    } else if (input == ' ') {
        return COMMAND_JUMP;
    } else if (input == '\n') {
        return COMMAND_ENTER;
    } else {
        return COMMAND_NONE;
    }
}

/**
 * Reads the next command that needs to be processed. This is the last command
 * on the input buffer.
 *
 * This function consumes the whole input buffer and returns either
 * COMMAND_NONE (if no other Command could be produced by what was in the input
 * buffer) or the last Command different than COMMAND_NONE that could be
 * produced by what was in the input buffer.
 */
Command read_next_command(void) {
    Command last_valid_command = COMMAND_NONE;
    Command current;
    int input;
    do {
        input = getch();
        current = command_from_input(input);
        if (current != COMMAND_NONE) {
            last_valid_command = current;
        }
    } while (input != ERR);
    return last_valid_command;
}

/**
 * Waits for user input, indefinitely.
 */
Command wait_for_next_command(void) {
    Command command = COMMAND_NONE;
    while (command == COMMAND_NONE) {
        rest_for_second_fraction(FPS);
        command = read_next_command();
    }
    return command;
}

