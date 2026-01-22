#include <stdio.h>
enum configNames { WORK_TIME, BREAK_TIME, LONG_BREAKTIME, NOTIF, SOUND };

int config[5] = {0, 0, 0, 0, 0};

const char *configKeys[5] = {"WORK_TIME", "BREAK_TIME", "LONG_BREAKTIME",
                             "NOTIF", "SOUND"};

void loadFile(const char *dir) {
  FILE *file = fopen(dir, "r");

  if (file == NULL)
    return;

  char data[50];

  while (fgets(data, 50, file) != NULL) {

    // HASHMAP get config name
  }
}
