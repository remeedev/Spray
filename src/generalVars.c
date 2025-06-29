#include <windows.h>


int WindowWidth;
int WindowHeight;

HFONT GameFont;
HFONT SmallFont;
HFONT TitleFont;
HFONT ConsoleFont;
HFONT SmallBig;

HWND mainWindow;

char *gameVersion = "Beta 0.0.1";

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