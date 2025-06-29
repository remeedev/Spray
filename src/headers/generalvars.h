#include <windows.h>

#ifndef generalvars
#define generalvars

int WindowWidth;
int WindowHeight;

char *gameVersion;

HWND mainWindow;

HFONT GameFont;
HFONT SmallFont;
HFONT TitleFont;
HFONT ConsoleFont;
HFONT SmallBig;

void openGameFont();
void deleteFont();

#endif