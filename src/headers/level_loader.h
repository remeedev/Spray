#include "drawing.h"
#include <windows.h>

#ifndef LevelLoader
#define LevelLoader

// ADMIN VARS

int showCollisions;
int showDebug;
COLORREF collisionColor;
COLORREF characterColor;

// GAME RES PUBLIC
size_t game_res[2];

// FOR HANDLER
HBITMAP hbmMem;
HDC hdcMem;
HBITMAP hbmOld;
int resized_ticks;
int closest_width, closest_height;
int offsetX, offsetY;

// ADMIN VAR END

void loadLevel(char *file_name);

void forceExit();
void UIKeyDown(UINT key);
int paused;

void DrawGame();
void DrawExclusiveWM(HWND hWnd, Sprite *watermark);

void Resize(HWND hWnd, int screen_width, int screen_height);

void StartGraphics(HWND hwnd);

void EndLastLevel();

void Update(float dt);

void onEnd();

#endif