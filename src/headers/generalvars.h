#include <windows.h>
#include "drawing.h"

#ifndef generalvars
#define generalvars

typedef struct conversation {
    wchar_t *line;
    Sprite* image;
    size_t end;
    char **options;
    size_t option_count;
    int option_selected;
    int (*skip_check)(void);
    struct conversation *(*option_handler)(struct conversation *);
    struct conversation *next;
} conversation;

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
char *user_name;

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