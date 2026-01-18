#include <curses.h>
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

const char *NUMS[] = {

    "##########\n"
    "##      ##\n"
    "##      ##\n"
    "##      ##\n"
    "##      ##\n"
    "##      ##\n"
    "##      ##\n"
    "##      ##\n"
    "##########\0"

    ,

    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\0"

    ,

    "##########\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "##########\n"
    "##        \n"
    "##        \n"
    "##        \n"
    "##########\0"

    ,

    "##########\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "##########\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "##########\0"

    ,

    "##      ##\n"
    "##      ##\n"
    "##      ##\n"
    "##      ##\n"
    "##########\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\0"

    ,

    "##########\n"
    "##        \n"
    "##        \n"
    "##        \n"
    "##########\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "##########\0"

    ,

    "##########\n"
    "##        \n"
    "##        \n"
    "##        \n"
    "##########\n"
    "##      ##\n"
    "##      ##\n"
    "##      ##\n"
    "##########\0"

    ,

    "##########\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "        ##\0"

    ,

    "##########\n"
    "##      ##\n"
    "##      ##\n"
    "##      ##\n"
    "##########\n"
    "##      ##\n"
    "##      ##\n"
    "##      ##\n"
    "##########\0"

    ,

    "##########\n"
    "##      ##\n"
    "##      ##\n"
    "##      ##\n"
    "##########\n"
    "        ##\n"
    "        ##\n"
    "        ##\n"
    "##########\0"

};

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

struct time {
  int minutes, seconds;

} typedef t_time;

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
    return;
  }

  timer->paused = 1;
  timer->timeLeft =
      timer->waitTimeSeconds - difftime(time(NULL), timer->initTime);
}

void nextSession() {
  flash();
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

t_time decodeTime(const float secondsLeft) {
  int minutes, seconds;

  minutes = (int)secondsLeft / 60;
  seconds = secondsLeft - minutes * 60;
  printw("%d\n", minutes);
  printw("%d\n", seconds);

  return (t_time){minutes, seconds};
}

void drawTimer() {
  t_time timeLeftTimer;
  if (currentTimer->paused) {
    timeLeftTimer = decodeTime(currentTimer->timeLeft);
  } else {
    timeLeftTimer = decodeTime(currentTimer->timeLeft -
                               difftime(time(NULL), currentTimer->initTime));
  }

  // MINUTES
  int Fmin = 0, Smin = 0;
  Fmin = timeLeftTimer.minutes / 10;
  Smin = timeLeftTimer.minutes - Fmin * 10;
  // SECONDS
  int Fsec = 0, Ssec = 0;
  Fsec = timeLeftTimer.seconds / 10;
  Ssec = timeLeftTimer.seconds - Fsec * 10;

  move(LINES * 0.10, COLS * 0.);
  printw("%s", NUMS[Smin]);
  move(LINES * 0.45, COLS * 0.);
  printw("%s", NUMS[Fsec]);
  move(LINES * 0.75, COLS * 0.);
  printw("%s", NUMS[Ssec]);
}

void drawCLI() { drawTimer(); };

int main(int argc, char *argv[]) {
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, true);

  timers[s_work] = createTimer(3 * 60);
  timers[s_break] = createTimer(2 * 60);
  timers[s_longbreak] = createTimer(1 * 60);

  setTimer(timers[s_work]);

  while (!EXIT) {
    getInput();
    countTimer();
    drawCLI();
    refresh();
    clear();
    sleep_ms(5);
  }

  endwin();

  return 0;
}
