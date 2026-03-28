#ifndef __POMODORO_CONFIG__
#define __POMODORO_CONFIG__

#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#define _true 1
#define _false 0

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

  int opt;

  while ((opt = getopt(argc, argv, "w:b:l:r:")) != -1) {

    switch (opt) {

    case 'w':
      config.work_duration = atoi(optarg);
      break;

    case 'b':
      config.break_duration = atoi(optarg);
      break;

    case 'l':
      config.longBreak_duration = atoi(optarg);
      break;

    case 'r':
      config.roundsTillLongBreak = atoi(optarg);
      break;
    }
  }
}

#endif /* ifndef __POMODORO_CONFIG */
