#include "command.hpp"
#include "clock.hpp"
#include "joystick.hpp"
#include "settings.hpp"
#include <stdexcept>
#include <vector>

CommandTable::CommandTable()
    : status(get_commands().size()), last_issued(get_commands().size()), last_modified(get_commands().size()) {
}

CommandTableSnapshot CommandTable::get_snapshot() const {
  std::vector<CommandValue> values;
  for (const auto command : get_commands()) {
    if (status[command] != 0.0) {
      values.emplace_back(command, status[command]);
    }
  }
  return CommandTableSnapshot(values);
}

CommandValue::CommandValue(Command command, F64 value) : command(command), value(value) {
}

Command CommandValue::get_command() const {
  return command;
}

F64 CommandValue::get_value() const {
  return value;
}

/**
 * Returns the Command value corresponding to the provided key combination.
 */
static Command command_from_key(const SDL_Keysym keysym) {
  const SDL_Keycode sym = keysym.sym;
  if (sym == SDLK_KP_8 || sym == SDLK_UP) {
    return COMMAND_UP;
  }
  if (sym == SDLK_KP_4 || sym == SDLK_LEFT) {
    return COMMAND_LEFT;
  }
  if (sym == SDLK_KP_5) {
    return COMMAND_CENTER;
  }
  if (sym == SDLK_KP_6 || sym == SDLK_RIGHT) {
    return COMMAND_RIGHT;
  }
  if (sym == SDLK_KP_2 || sym == SDLK_DOWN) {
    return COMMAND_DOWN;
  }
  if (sym == SDLK_SPACE) {
    return COMMAND_JUMP;
  }
  if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
    return COMMAND_ENTER;
  }
  if (sym == SDLK_c) {
    return COMMAND_CONVERT;
  }
  if (sym == SDLK_p) {
    return COMMAND_PAUSE;
  }
  if (sym == SDLK_F12) {
    return COMMAND_DEBUG;
  }
  if (sym == SDLK_q) {
    return COMMAND_QUIT;
  }
  return COMMAND_NONE;
}

static void set_command_table(CommandTable *table, Command command, double value, Milliseconds time) {
  table->status[command] = value;
  table->last_modified[command] = time;
}

static void digest_joystick_event(const Settings &settings, CommandTable *table, SDL_Event event) {
  const Milliseconds time = get_milliseconds();
  if (table == nullptr) {
    return;
  }
  if (event.type == SDL_JOYBUTTONDOWN) {
    set_command_table(table, command_from_joystick_event(settings, event), 1.0, time);
  } else if (event.type == SDL_JOYBUTTONUP) {
    set_command_table(table, command_from_joystick_event(settings, event), 0.0, time);
  } else if (event.type == SDL_JOYAXISMOTION) {
    if (abs(event.jaxis.value) > JOYSTICK_DEAD_ZONE) {
      double magnitude = event.jaxis.value / static_cast<double>(MAXIMUM_JOYSTICK_AXIS_VALUE);
      if (event.jaxis.axis == 0) {
        set_command_table(table, COMMAND_RIGHT, 0.0, time);
        set_command_table(table, COMMAND_LEFT, 0.0, time);
        if (event.jaxis.value > 0) {
          set_command_table(table, COMMAND_RIGHT, magnitude, time);
        } else if (event.jaxis.value < 0) {
          set_command_table(table, COMMAND_LEFT, -magnitude, time);
        }
      } else {
        set_command_table(table, COMMAND_DOWN, 0.0, time);
        set_command_table(table, COMMAND_UP, 0.0, time);
        if (event.jaxis.value > 0) {
          set_command_table(table, COMMAND_DOWN, magnitude, time);
        } else if (event.jaxis.value < 0) {
          set_command_table(table, COMMAND_UP, -magnitude, time);
        }
      }
    } else {
      if (event.jaxis.axis == 0) {
        set_command_table(table, COMMAND_RIGHT, 0.0, time);
        set_command_table(table, COMMAND_LEFT, 0.0, time);
      } else {
        set_command_table(table, COMMAND_DOWN, 0.0, time);
        set_command_table(table, COMMAND_UP, 0.0, time);
      }
    }
  }
}

static void digest_event(const Settings &settings, CommandTable *table, const SDL_Event event) {
  const Milliseconds time = get_milliseconds();
  if (table == nullptr) {
    return;
  }
  if (event.type == SDL_QUIT) {
    set_command_table(table, COMMAND_QUIT, 1.0, time);
  } else if (event.type == SDL_KEYDOWN) {
    set_command_table(table, command_from_key(event.key.keysym), 1.0, time);
  } else if (event.type == SDL_KEYUP) {
    set_command_table(table, command_from_key(event.key.keysym), 0.0, time);
  } else if (event.type == SDL_JOYAXISMOTION || event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) {
    digest_joystick_event(settings, table, event);
  }
}

void initialize_command_table(CommandTable *table) {
  const auto time = get_milliseconds();
  for (size_t i = 0; i < get_commands().size(); i++) {
    table->status[i] = 0.0;
    table->last_issued[i] = time;
    table->last_modified[i] = time;
  }
}

void read_commands(const Settings &settings, CommandTable *table) {
  SDL_Event event{};
  while (SDL_PollEvent(&event) != 0) {
    digest_event(settings, table, event);
  }
}

bool test_command_table(CommandTable *table, Command command, Milliseconds repetition_delay) {
  const Milliseconds time = get_milliseconds();
  if (table->status[command] == 0.0) {
    return false;
  }
  if (time - table->last_issued[command] < repetition_delay) {
    return false;
  }
  table->last_issued[command] = time;
  return true;
}

/**
 * Waits for any user input, blocking indefinitely.
 */
Code wait_for_input(const Settings &settings, CommandTable *table) {
  SDL_Event event{};
  while (true) {
    if (SDL_WaitEvent(&event) != 0) {
      digest_event(settings, table, event);
      if (event.type == SDL_QUIT) {
        return CODE_QUIT;
      }
      /* Mark this command as read. This prevents a key press from being carried to another screen. */
      if (event.type == SDL_KEYDOWN) {
        test_command_table(table, command_from_key(event.key.keysym), 0);
        return CODE_OK;
      }
      if (event.type == SDL_JOYBUTTONDOWN) {
        test_command_table(table, command_from_joystick_event(settings, event), 0);
        return CODE_OK;
      }
    } else {
      /* WaitEvent returns 0 to indicate errors. */
      return CODE_ERROR;
    }
  }
}

std::string command_to_string(Command command) {
  switch (command) {
  case COMMAND_NONE:
    return "None";
  case COMMAND_UP:
    return "Up";
  case COMMAND_LEFT:
    return "Left";
  case COMMAND_CENTER:
    return "Center";
  case COMMAND_RIGHT:
    return "Right";
  case COMMAND_DOWN:
    return "Down";
  case COMMAND_JUMP:
    return "Jump";
  case COMMAND_ENTER:
    return "Enter";
  case COMMAND_CONVERT:
    return "Convert";
  case COMMAND_PAUSE:
    return "Pause";
  case COMMAND_DEBUG:
    return "Debug";
  case COMMAND_QUIT:
    return "Quit";
  case COMMAND_CLOSE:
    return "Close";
  }
  throw std::runtime_error("Should never happen.");
}

CommandTableSnapshot::CommandTableSnapshot(const std::vector<CommandValue> &values) : values(values) {
}
