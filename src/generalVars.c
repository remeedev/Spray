#include <windows.h>


int WindowWidth;
int WindowHeight;

HFONT GameFont;
HFONT SmallFont;
HFONT TitleFont;

char *gameVersion = "Beta 0.0.1";

HFONT createSubFont(int size, int weight){
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
        "Pixelify Sans" // font name
    );
}

void openGameFont(){
    GameFont = createSubFont(32, FW_BOLD);
    TitleFont = createSubFont(72, FW_BOLD);
    SmallFont = createSubFont(24, FW_NORMAL);
}

void deleteFont(){
    DeleteObject(GameFont);
    DeleteObject(SmallFont);
    DeleteObject(TitleFont);
}