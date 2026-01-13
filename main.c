#include <curses.h>
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
enum pomodoroStates { s_work, s_break, s_longbreak };

struct pomodoroState {
  enum pomodoroStates state;
  enum pomodoroStates nextState;
  _Bool countsAsWorkSession;
} typedef p_state;

struct timer {
  float waitTimeSeconds;
  float timeLeft;
  _Bool paused;
  time_t initTime;
} typedef t_timer;

// Global values
_Bool EXIT = 0;
p_state p_states[3] = {
    {s_work, s_break, 1}, {s_break, s_work}, {s_longbreak, s_work}};
enum pomodoroStates currentState = s_work;
int sessionNumber = 0;

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

void togglePauseTimer(t_timer *timer) {
  if (timer->paused == 1) {
    timer->paused = 0;
    timer->initTime = time(NULL);
  }

  timer->paused = 1;
  timer->timeLeft =
      timer->waitTimeSeconds - difftime(time(NULL), timer->initTime);
}

void nextSession() {
  if (p_states[currentState].countsAsWorkSession)
    sessionNumber++;

  if (sessionNumber % 4 == 0 && sessionNumber != 0)
    currentState = s_longbreak;
  else
    currentState = p_states[currentState].nextState;

  setTimer(timers[currentState]);
};

void countTimer() {
  time_t now = time(NULL);
  printw("currentSession: %d\n", currentState);
  printw("currentSession: %f\n",
         currentTimer->timeLeft - difftime(now, currentTimer->initTime));

  if (currentTimer->paused)
    return;

  if (difftime(now, currentTimer->initTime) < currentTimer->timeLeft) {
    return;
  }

  nextSession();
};

void getInput() {

  int tecla = getch();

  if (tecla == ERR)
    return;

  if (tecla == 27)
    EXIT = 1;

  if (tecla == 112 || tecla == 80)
    togglePauseTimer(currentTimer);
};

int main(int argc, char *argv[]) {
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, true);

  timers[s_work] = createTimer(3);
  timers[s_break] = createTimer(2);
  timers[s_longbreak] = createTimer(15);

  setTimer(timers[s_work]);

  while (!EXIT) {
    getInput();
    countTimer();
    refresh();
    clear();
    sleep_ms(5);
  }

  return 0;
}
