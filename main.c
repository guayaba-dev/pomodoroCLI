#include <stdio.h>
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

// Data structures
enum pomodoroSessions { s_work, s_break, s_longbreak };

struct pomodoroSession {
  enum pomodoroSessions session;
  enum pomodoroSessions nextSession;
} typedef p_session;

struct timer {
  float waitTimeSeconds;
  float timeLeft;
  _Bool paused;
  time_t initTime;
} typedef t_timer;

// Global values
_Bool EXIT = 0;
p_session p_sessions[3] = {
    {s_work, s_break}, {s_break, s_longbreak}, {s_longbreak, s_work}};
enum pomodoroSessions currentSession = s_work;

t_timer *currentTimer;
t_timer *timers[3];

void setCurrentTimer(t_timer *timer) { currentTimer = timer; }

void *createTimer(float waitTimeSeconds) {
  t_timer *timer = (struct timer *)malloc(sizeof(struct timer));
  timer->waitTimeSeconds = waitTimeSeconds;
  timer->timeLeft = waitTimeSeconds;
  timer->paused = 0;
  return timer;
}

void setTimer(t_timer *timer) {
  currentTimer = timer;
  currentTimer->initTime = time(NULL);
}

void pauseTimer(t_timer *timer) {
  timer->paused = 1;
  timer->timeLeft =
      timer->waitTimeSeconds - difftime(time(NULL), timer->initTime);
}

void nextSession() {
  currentSession = p_sessions[currentSession].nextSession;
  setTimer(timers[currentSession]);
};

void countTimer() {

  if (currentTimer->paused)
    return;

  time_t now = time(NULL);
  if (difftime(now, currentTimer->initTime) < currentTimer->timeLeft) {
    printf("currentSession: %d\n", currentSession);
    printf("currentSession: %f\n", currentTimer->waitTimeSeconds -
                                       difftime(now, currentTimer->initTime));
    return;
  }

  nextSession();
};

int main(int argc, char *argv[]) {
  timers[s_work] = createTimer(25);
  timers[s_break] = createTimer(5);
  timers[s_longbreak] = createTimer(15);

  setTimer(timers[s_work]);

  while (!EXIT) {
    countTimer();
    sleep_ms(5);
  }

  return 0;
}
