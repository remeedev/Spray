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

COLORREF regular_text_color;
COLORREF undermined_text_color;
COLORREF highlight_text_color;
COLORREF ignore_text_color;
int is_fullscreen;

void toggle_fullscreen();

void openGameFont();
void deleteFont();

#endif