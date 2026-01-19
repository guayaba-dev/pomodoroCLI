#include <curses.h>
#include <stdlib.h>
#include <time.h>

#define NUMS_LINES 9

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

const char *NUMS[10][NUMS_LINES] = {

    "##########\0",
    "##      ##\0",
    "##      ##\0",
    "##      ##\0",
    "##      ##\0",
    "##      ##\0",
    "##      ##\0",
    "##      ##\0",
    "##########\0"

    ,

    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0"

    ,

    "##########\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "##########\0",
    "##        \0",
    "##        \0",
    "##        \0",
    "##########\0"

    ,

    "##########\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "##########\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "##########\0"

    ,

    "##      ##\0",
    "##      ##\0",
    "##      ##\0",
    "##      ##\0",
    "##########\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0"

    ,

    "##########\0",
    "##        \0",
    "##        \0",
    "##        \0",
    "##########\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "##########\0"

    ,

    "##########\0",
    "##        \0",
    "##        \0",
    "##        \0",
    "##########\0",
    "##      ##\0",
    "##      ##\0",
    "##      ##\0",
    "##########\0"

    ,

    "##########\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "        ##\0"

    ,

    "##########\0",
    "##      ##\0",
    "##      ##\0",
    "##      ##\0",
    "##########\0",
    "##      ##\0",
    "##      ##\0",
    "##      ##\0",
    "##########\0"

    ,

    "##########\0",
    "##      ##\0",
    "##      ##\0",
    "##      ##\0",
    "##########\0",
    "        ##\0",
    "        ##\0",
    "        ##\0",
    "##########\0"

};

const char *colon[9] = {

    " \0", //
    "#\0", //
    "#\0", //
    " \0", //
    " \0", //
    " \0", //
    "#\0", //
    "#\0", //
    " \0"  //

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
  timer->timeLeft = timer->timeLeft - difftime(time(NULL), timer->initTime);
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
  move(0, 0);
  printw("timeLeft: \n%f", currentTimer->timeLeft);

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

  return (t_time){minutes, seconds};
}

void drawText(const char *text[], int sizeLines, int x, int y) {

  for (int i = 0; i < sizeLines; i++) {
    move(y, x);

    printw("%s", text[i]);

    y++;
  }
}

void drawTimer() {
  t_time timeLeftTimer;
  if (currentTimer->paused) {
    timeLeftTimer = decodeTime(currentTimer->timeLeft);
  } else {
    timeLeftTimer = decodeTime(currentTimer->timeLeft -
                               difftime(time(NULL), currentTimer->initTime));
  }

  // MINUTES || FirstMinutes || SecondMinutes
  int Fmin = 0, Smin = 0;
  Fmin = timeLeftTimer.minutes / 10;
  Smin = timeLeftTimer.minutes - Fmin * 10;
  // SECONDS
  int Fsec = 0, Ssec = 0;
  Fsec = timeLeftTimer.seconds / 10;
  Ssec = timeLeftTimer.seconds - Fsec * 10;

  drawText(NUMS[Fmin], NUMS_LINES, COLS * 0.30, LINES * 0.25);
  drawText(NUMS[Smin], NUMS_LINES, COLS * 0.40, LINES * 0.25);
  drawText(NUMS[Fsec], NUMS_LINES, COLS * 0.54, LINES * 0.25);
  drawText(NUMS[Ssec], NUMS_LINES, COLS * 0.64, LINES * 0.25);
  drawText(colon, NUMS_LINES, COLS * 0.5, LINES * 0.25);
}

void drawCLI() { drawTimer(); };

int main(int argc, char *argv[]) {
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, true);

  timers[s_work] = createTimer(10 * 60);
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
