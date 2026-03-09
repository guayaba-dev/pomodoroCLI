#ifndef __POMODORO_CONFIG__
#define __POMODORO_CONFIG__

#include <stdbool.h>
#define _true 1
#define _false 1

struct option {
  int needsArgument;
  int argc;
  char shortName;
  char nameParam[25];
};

struct option long_options[] = {{_true, 1, 'w', "work"},
                                {_true, 1, 'b', "break"},
                                {_true, 1, 'l', "longBreak"},
                                {_true, 1, 'r', "rounds"},
                                {_true, 1, 'c', "config"},
                                {_true, 1, 's', "sound"},
                                {_false, 0, 'h', "help"},
                                {_false, 0, 'v', "version"}

};

typedef struct {
  int work_duration;
  int break_duration;
  int longBreak_duration;
  int roundsTillLongBreak;
} PomodoroConfig;

PomodoroConfig config = {.work_duration = 30,
                         .break_duration = 10,
                         .longBreak_duration = 15,
                         .roundsTillLongBreak = 4};

void loadConfig(int argc, char *argv[]) {

  typedef struct {

  } configParser;
}

#endif /* ifndef __POMODORO_CONFIG */
