#include <windows.h>

int WindowWidth;
int WindowHeight;

HFONT GameFont;

void openGameFont(){
    GameFont = CreateFont(
        32, // height
        0, // width
        0, // escapement
        0, // orientation
        FW_BOLD, // weight
        FALSE, // italic
        FALSE, // underline
        FALSE, // strikeout
        ANSI_CHARSET, //charset
        OUT_DEFAULT_PRECIS, //out precision
        CLIP_MASK, // clipping
        ANTIALIASED_QUALITY, // quality
        DEFAULT_PITCH, // pitch
        "Arial" // font name
    );
}

void deleteFont(){
    DeleteObject(GameFont);
}