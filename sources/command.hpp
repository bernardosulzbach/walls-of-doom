#ifndef COMMAND_H
#define COMMAND_H

#include "clock.hpp"
#include "code.hpp"
#include "settings.hpp"
#include <vector>

using CommandIntegralType = U16;

/**
 * The Command enumeration represents the different commands the user may issue.
 */
enum Command : CommandIntegralType {
  COMMAND_NONE,
  COMMAND_UP,
  COMMAND_LEFT,
  COMMAND_CENTER,
  COMMAND_RIGHT,
  COMMAND_DOWN,
  COMMAND_JUMP,
  COMMAND_ENTER,
  COMMAND_CONVERT,
  COMMAND_PAUSE,
  COMMAND_DEBUG,
  COMMAND_QUIT,
  COMMAND_CLOSE
};

inline std::vector<Command> get_commands() {
  return {COMMAND_NONE,  COMMAND_UP,      COMMAND_LEFT,  COMMAND_CENTER, COMMAND_RIGHT, COMMAND_DOWN, COMMAND_JUMP,
          COMMAND_ENTER, COMMAND_CONVERT, COMMAND_PAUSE, COMMAND_DEBUG,  COMMAND_QUIT,  COMMAND_CLOSE};
}

class CommandValue {
public:
  CommandValue(Command command, F64 value);

  Command get_command() const;
  F64 get_value() const;

private:
  Command command;
  F64 value;
};

class CommandTableSnapshot {
public:
  explicit CommandTableSnapshot(const std::vector<CommandValue> &values);

  inline std::vector<CommandValue>::const_iterator begin() const {
    return values.begin();
  }

  inline std::vector<CommandValue>::const_iterator end() const {
    return values.end();
  }

private:
  std::vector<CommandValue> values;
};

class CommandTableHistory {
public:
  inline void append(CommandTableSnapshot snapshot) {
    return snapshots.push_back(snapshot);
  }

  inline std::vector<CommandTableSnapshot>::const_iterator begin() const {
    return snapshots.begin();
  }

  inline std::vector<CommandTableSnapshot>::const_iterator end() const {
    return snapshots.end();
  }

private:
  std::vector<CommandTableSnapshot> snapshots;
};

class CommandTable {
public:
  std::vector<F64> status;
  std::vector<Milliseconds> last_issued;
  std::vector<Milliseconds> last_modified;

  CommandTable();

  CommandTableSnapshot get_snapshot() const;
};

std::string command_to_string(Command command);

void initialize_command_table(CommandTable *table);

bool test_command_table(CommandTable *table, enum Command command, Milliseconds repetition_delay);

void read_commands(const Settings &settings, CommandTable *table);

/**
 * Waits for any user input, blocking indefinitely.
 */
Code wait_for_input(const Settings &settings, CommandTable *table);

#endif
