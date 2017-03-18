#include "joystick.h"
#include "command.h"
#include "constants.h"
#include "logger.h"

#include <SDL.h>

#define JOYSTICK_DEAD_ZONE 4096

/* DualShock constants. L3 and R3 are the analog sticks pressed down. */
#define DUALSHOCK_TRIANGLE 0
#define DUALSHOCK_CIRCLE 1
#define DUALSHOCK_CROSS 2
#define DUALSHOCK_SQUARE 3
#define DUALSHOCK_L1 4
#define DUALSHOCK_L2 6
#define DUALSHOCK_L3 10
#define DUALSHOCK_R1 5
#define DUALSHOCK_R2 7
#define DUALSHOCK_R3 11
#define DUALSHOCK_SELECT 8
#define DUALSHOCK_START 9

static SDL_Joystick *shared_joystick = NULL;

static void log_joystick_count() {
  char log_buffer[MAXIMUM_STRING_SIZE];
  sprintf(log_buffer, "Joystick count: %d", SDL_NumJoysticks());
  log_message(log_buffer);
}

SDL_Joystick *get_joystick() { return shared_joystick; }

static void set_joystick(SDL_Joystick *joystick) { shared_joystick = joystick; }

void initialize_joystick() {
  log_joystick_count();
  if (SDL_NumJoysticks()) {
    set_joystick(SDL_JoystickOpen(0));
    if (get_joystick() == NULL) {
      log_message("Failed to open controller!");
    }
  }
}

Command command_from_joystick_event(const SDL_Event event) {
  if (event.type == SDL_JOYBUTTONDOWN) {
    if (event.jbutton.button == DUALSHOCK_TRIANGLE) {
      return COMMAND_INVEST;
    } else if (event.jbutton.button == DUALSHOCK_CIRCLE) {
      return COMMAND_CONVERT;
    } else if (event.jbutton.button == DUALSHOCK_CROSS) {
      return COMMAND_JUMP;
    } else if (event.jbutton.button == DUALSHOCK_START) {
      return COMMAND_ENTER;
    } else if (event.jbutton.button == DUALSHOCK_SELECT) {
      return COMMAND_PAUSE;
    }
  } else if (event.type == SDL_JOYAXISMOTION) {
    if (abs(event.jaxis.value) > JOYSTICK_DEAD_ZONE) {
      if (event.jaxis.axis == 0) {
        if (event.jaxis.value > 0) {
          return COMMAND_RIGHT;
        } else {
          return COMMAND_LEFT;
        }
      } else {
        if (event.jaxis.value > 0) {
          return COMMAND_DOWN;
        } else {
          return COMMAND_UP;
        }
      }
    }
  }
  return COMMAND_NONE;
}

void finalize_joystick() {
  if (get_joystick() != NULL) {
    SDL_JoystickClose(get_joystick());
    set_joystick(NULL);
  }
}
