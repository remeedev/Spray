#include "generalvars.h"

#ifndef bicycle
#define bicycle

double game_time;
int max_time;

void startDayCycle(HWND hwnd);

void endDayCycle();

void updateDayCycle(float dt);

void drawDayCycle(HDC hdc);

#endif