#include <stdlib.h>
#include <time.h>

// Versión correcta
void sleep_ms(int ms) {
#ifdef _WIN32
#include <windows.h>
  Sleep(ms);
#else
#include <unistd.h>
  // ms * 1000 = microsegundos
  usleep(ms * 1000);
#endif
}

struct timer {
  float waitTimeSeconds;
  float timeLeft;
  _Bool paused;
  time_t initTime;
} typedef t_timer;

_Bool EXIT = 0;
t_timer *currentTimer;

void setCurrentTimer(t_timer *timer) { currentTimer = timer; }

void *createTimer(float waitTimeSeconds) {
  t_timer *timer = (struct timer *)malloc(sizeof(struct timer));
  timer->waitTimeSeconds = waitTimeSeconds;
  timer->timeLeft = 0.0;
  timer->paused = 0;
  timer->initTime = time(NULL);
  return timer;
}

void setTimer(t_timer *timer) { currentTimer = timer; }

void countTimer() {

  time_t now = time(NULL);
  if (difftime(now, currentTimer->initTime) < currentTimer->waitTimeSeconds) {
    return;
  }

  EXIT = 1;
};

int main(int argc, char *argv[]) {
  t_timer *fiveSecTimer = createTimer(5.);
  setTimer(fiveSecTimer);

  while (!EXIT) {
    countTimer();
    sleep_ms(5);
  }

  return 0;
}
