#include <windows.h>

#ifndef console
#define console

int console_on;

void PrintToConsole(char *text);

void InitConsole();
void ClearConsole();
void DrawConsoleIfNeeded(HDC hdcMem);
void handleKeyConsole();
void handleCharConsole();

void DeleteConsoleIfNeeded();

#endif