#include <windows.h>

#ifndef generalvars
#define generalvars

typedef struct textNode {
    char *value;
    struct textNode *next;
    struct textNode *child;
}textNode;

// Recursive search inside tree
textNode *find_text_node(textNode *structure, char *value);

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