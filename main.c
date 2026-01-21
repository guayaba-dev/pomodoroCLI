#include <curses.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "timerOutSound.h"

#define NUMS_LINES 9

#define NOTIF_TITLE "CPomodoro"

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

#ifdef _WIN32
#include <windows.h>
void play_sound(void) {
  PlaySound((LPCSTR)timerOut_wav, NULL, SND_MEMORY | SND_ASYNC);
}
#else
void play_sound(void) {
  // Guardar temporalmente y reproducir
  FILE *tmp = tmpfile();
  if (tmp) {
    fwrite(timerOut_wav, 1, timerOut_wav_len, tmp);
    fflush(tmp);

#ifdef __APPLE__
    char cmd[256];
    sprintf(cmd, "afplay /dev/fd/%d 2>/dev/null &", fileno(tmp));
#else
    char cmd[256];
    sprintf(cmd, "aplay -q /dev/fd/%d 2>/dev/null &", fileno(tmp));
#endif

    system(cmd);
    // No cerrar tmp - se cerrará al salir del programa
  }
}
#endif

void sendNotif(const char *title, const char *message) {
#ifdef _WIN32

  MessageBoxA(NULL, message, title, MB_OK | MB_ICONINFORMATION);

#elif __APPLE__
  char comand[512];
  snprintf(comand, sizeof(comand),
           "osascript -e 'display notification \"%s\" with title \"%s\"'",
           message, title);
  system(comand);

#else
  char comand[512];
  snprintf(comand, sizeof(comand), "notify-send \"%s\" \"%s\"", title, message);
  system(comand);

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

const char *RESUME[7] = {
    "#      \0", //
    "###    \0", //
    "#####  \0", //
    "#######\0", //
    "#####  \0", //
    "###    \0", //
    "#      \0"  //
};

const char *PAUSE[7] = {
    "##  ##\0", //
    "##  ##\0", //
    "##  ##\0", //
    "##  ##\0", //
    "##  ##\0", //
    "##  ##\0", //
    "##  ##\0", //

};

const char *loremIpsum = {"loremIpsum"};

// Data structures
enum pomodoroStates { s_work, s_break, s_longbreak };
const char *stateNames[3] = {"WORK", "BREAK", "LONG BREAK"};
const char *stateMessages[3] = // Messages displayed when a sessionEnded
    {"WORK SESSSION ENDED, Take a break!!", "BREAK TIME IS OVER, Let's work",
     "LONG BREAK is over, let's work"};

struct pomodoroState {
  enum pomodoroStates state;
  enum pomodoroStates nextState;
  _Bool countsAsSession;
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
    {s_work, s_break}, {s_break, s_work, 1}, {s_longbreak, s_work, 1}};
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
  currentTimer->timeLeft = currentTimer->waitTimeSeconds;
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
  play_sound();
  sendNotif(NOTIF_TITLE, stateMessages[currentState]);
  if (p_states[currentState].countsAsSession)
    sessionNumber++;

  if (sessionNumber % 4 == 0 && sessionNumber != 0 &&
      currentState != s_longbreak)
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

  return (t_time){minutes, seconds};
}

void drawASCII(const char *text[], int sizeLines, int x, int y) {

  for (int i = 0; i < sizeLines; i++) {
    move(y, x);

    printw("%s", text[i]);

    y++;
  }
}

void drawCenteredInCords(int x, int y, const char *text, ...) {
  int textLengt = strlen(text);

  int newx = x - (textLengt / 2);

  move(y, newx);

  va_list args;

  va_start(args, text);

  vw_printw(stdscr, text, args);

  va_end(args);
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

  drawASCII(NUMS[Fmin], NUMS_LINES, COLS * 0.30, LINES * 0.25);
  drawASCII(NUMS[Smin], NUMS_LINES, COLS * 0.40, LINES * 0.25);
  drawASCII(NUMS[Fsec], NUMS_LINES, COLS * 0.54, LINES * 0.25);
  drawASCII(NUMS[Ssec], NUMS_LINES, COLS * 0.64, LINES * 0.25);
  drawASCII(colon, NUMS_LINES, COLS * 0.5, LINES * 0.25);
}

void drawPauseSimbols() {

  if (currentTimer->paused == 1) {
    drawASCII(RESUME, 7, COLS * 0.5 - 2, LINES * .6);
    return;
  }

  drawASCII(PAUSE, 7, COLS * 0.5 - 2, LINES * .6);
};

void drawCLI() {
  drawTimer();
  drawPauseSimbols();

  drawCenteredInCords(COLS * 0.5, LINES * 0.2 - 1, "%s  #%d",
                      stateNames[currentState], sessionNumber);

  drawCenteredInCords(COLS * 0.5, LINES - 5, "press P to pause");
};

int main(int argc, char *argv[]) {
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, true);

  timers[s_work] = createTimer(10);
  timers[s_break] = createTimer(2);
  timers[s_longbreak] = createTimer(1);

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
