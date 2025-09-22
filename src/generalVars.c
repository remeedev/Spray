#include <windows.h>

typedef struct textNode {
    char *value;
    struct textNode *next;
    struct textNode *child;
}textNode;

// Recursive search inside tree
textNode *find_text_node(textNode *structure, char *value){
    // Check self for value
    if (structure == NULL) return NULL;
    if (strcmp(structure->value, value) == 0) return structure;

    int found = FALSE;
    textNode *tmp_resp;
    tmp_resp = find_text_node(structure->child, value);
    found = tmp_resp != NULL;
    if (found){
        goto return_tmp;
    }
    tmp_resp = find_text_node(structure->next, value);
    found = found || tmp_resp != NULL;

    return_tmp:
    return tmp_resp;
}

int WindowWidth;
int WindowHeight;

HFONT GameFont;
HFONT SmallFont;
HFONT TitleFont;
HFONT ConsoleFont;
HFONT SmallBig;

COLORREF regular_text_color = RGB(255, 255, 255);
COLORREF undermined_text_color = RGB(150, 150, 150);
COLORREF ignore_text_color = RGB(50, 50, 50);
COLORREF highlight_text_color = RGB(250, 250, 50);
int is_fullscreen = FALSE;

HWND mainWindow;

char *gameVersion = "Beta 0.0.1";

RECT memorySize;

void toggle_fullscreen(){
    if (!is_fullscreen){
        RECT wR;
        GetWindowRect(mainWindow, &memorySize);
        GetClientRect(GetDesktopWindow(), &wR);
        SetWindowLong(mainWindow, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(mainWindow, HWND_TOP, 0, 0, wR.right, wR.bottom, SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        is_fullscreen = TRUE;
    }else{
        SetWindowLong(mainWindow, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
        int x = memorySize.left;
        int y = memorySize.top;
        int cx = memorySize.right - x;
        int cy = memorySize.bottom - y;
        SetWindowPos(mainWindow, HWND_TOPMOST, x, y, cx, cy, SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        is_fullscreen = FALSE;
    }
}

HFONT createSubFont(int size, int weight, char *name){
    return CreateFont(
        size, // height
        0, // width
        0, // escapement
        0, // orientation
        weight, // weight
        FALSE, // italic
        FALSE, // underline
        FALSE, // strikeout
        ANSI_CHARSET, //charset
        OUT_TT_PRECIS, //out precision
        CLIP_DEFAULT_PRECIS, // clipping
        NONANTIALIASED_QUALITY, // quality
        FIXED_PITCH | FF_MODERN, // pitch
        name == NULL ? "Pixelify Sans" : name // font name
    );
}

void openGameFont(){
    GameFont = createSubFont(32, FW_BOLD, NULL);
    TitleFont = createSubFont(72, FW_BOLD, NULL);
    SmallFont = createSubFont(24, FW_NORMAL, NULL);
    ConsoleFont = createSubFont(24, FW_NORMAL, "Consolas");
    SmallBig = createSubFont(16, FW_BOLD, "Consolas");
}

void deleteFont(){
    DeleteObject(GameFont);
    DeleteObject(SmallFont);
    DeleteObject(TitleFont);
    DeleteObject(ConsoleFont);
    DeleteObject(SmallBig);
}