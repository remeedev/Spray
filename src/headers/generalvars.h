#include <windows.h>

#ifndef generalvars
#define generalvars

int WindowWidth;
int WindowHeight;

char *gameVersion;

HFONT GameFont;
HFONT SmallFont;
HFONT TitleFont;

void openGameFont();
void deleteFont();

#endif