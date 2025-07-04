#include <windows.h>

#ifndef handler
#define handler

void openOptions();

void showCredits();

void ForceGameMenu();

void startGameSystem(HWND hWnd, int screen_width, int screen_height);

void drawEvent(HWND hWnd);

void updateEvent(float dt);

void handleKEYDOWN(UINT key);
void handleKEYUP(UINT key);
void handleCHAR(UINT key);
#endif