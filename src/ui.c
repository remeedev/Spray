#include "headers/drawing.h"
#include "headers/level_loader.h"
#include "headers/generalvars.h"
#include "headers/throwables.h"
#include "headers/the_chronic.h"

#include <windows.h>
#include <stdio.h>

Sprite *healthJar = NULL;
Sprite *sprayCan = NULL;
Sprite *weedBag = NULL;
RECT background_rect = {0, 0, 0, 0};
POINT grenade_text;
POINT weed_text;

int artistOptions = FALSE;

void startUI(){
    healthJar = (Sprite *)malloc(sizeof( Sprite ));
    if (healthJar == NULL){
        printf("There was an error allocating space for the health jar!\n");
        return;
    }
    sprayCan = (Sprite *)malloc(sizeof( Sprite ));
    if (sprayCan == NULL){
        printf("There was an error allocating space for the ui spray can!\n");
        return;
    }
    weedBag = (Sprite *)malloc(sizeof( Sprite ));
    if (weedBag == NULL){
        printf("There was an error allocating space for the ui spray can!\n");
        return;
    }
    SIZE AssetSize = {64, 64};
    int margin = 25;
    int upscale = 4;
    CreateImgSprite(healthJar, margin, game_res[1] - margin - AssetSize.cy, AssetSize.cx, AssetSize.cy, "./assets/ui/health_jar.png", upscale);
    background_rect.left = healthJar->pos.x + (2 * upscale);
    background_rect.top = healthJar->pos.y + (2 * upscale);
    background_rect.bottom = background_rect.top + AssetSize.cy - (3 * upscale);
    background_rect.right = background_rect.left + AssetSize.cx - (4 * upscale);

    CreateImgSprite(sprayCan, game_res[0] - margin - AssetSize.cx, margin, AssetSize.cx, AssetSize.cy, "./assets/ui/spray_can.png", 4);
    grenade_text.x = game_res[0] - margin - AssetSize.cx;
    grenade_text.y = margin + (int)(((float)AssetSize.cy)*(3.0/4.0));
    CreateImgSprite(weedBag, game_res[0] - margin - AssetSize.cx, margin + sprayCan->size.cy + 15, AssetSize.cx, AssetSize.cy, "./assets/ui/weed_bag.png", 4);
    weed_text.x = game_res[0] - margin - AssetSize.cx - 10;
    weed_text.y = margin + (int)(((float)AssetSize.cy)*(3.0/4.0)) + sprayCan->size.cy + 15;
}

void drawPrompt(HDC hdc, char *prompt){
    SelectObject(hdc, GameFont);
    SetTextAlign(hdc, TA_TOP | TA_LEFT);
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    int padding = 5;
    int margin = 20;
    int borderWeight = 4;
    POINT startPoint = {background_rect.right + margin, background_rect.top};

    COLORREF border_color = RGB(94, 61, 20);
    COLORREF bg_color = RGB(173, 135, 87);
    int innerHeight = tm.tmHeight + padding*2;
    int innerWidth = tm.tmAveCharWidth*strlen(prompt) + padding*2;

    RECT border_left = {
        startPoint.x,
        startPoint.y + borderWeight,
        startPoint.x + borderWeight,
        startPoint.y + innerHeight + borderWeight
    };
    RECT border_top = {
        startPoint.x + borderWeight,
        startPoint.y,
        startPoint.x + borderWeight + innerWidth,
        startPoint.y + borderWeight
    };
    RECT border_bottom = {
        startPoint.x + borderWeight,
        startPoint.y + borderWeight + innerHeight,
        startPoint.x + borderWeight + innerWidth,
        startPoint.y + borderWeight*2 + innerHeight
    };
    RECT border_right = {
        startPoint.x + innerWidth + borderWeight,
        startPoint.y + borderWeight,
        startPoint.x + innerWidth + borderWeight*2,
        startPoint.y + borderWeight + innerHeight
    };

    RECT prompt_holder = {
        startPoint.x + borderWeight,
        startPoint.y + borderWeight,
        startPoint.x + borderWeight + innerWidth,
        startPoint.y + borderWeight + innerHeight
    };
    FillRect(hdc, &border_left, CreateNewColorBrush(border_color)->brush);
    FillRect(hdc, &border_right, CreateNewColorBrush(border_color)->brush);
    FillRect(hdc, &border_top, CreateNewColorBrush(border_color)->brush);
    FillRect(hdc, &border_bottom, CreateNewColorBrush(border_color)->brush);
    
    FillRect(hdc, &prompt_holder, CreateNewColorBrush(bg_color)->brush);

    TextOut(hdc, startPoint.x + borderWeight + padding, startPoint.y + borderWeight + padding, prompt, strlen(prompt));
}

void drawUI(HDC hdc){
    int playerHealth = GetPlayerPtr()->health;
    int maxHealth = GetPlayerPtr()->maxHealth;
    float currHealthPerc = ((float) playerHealth)/((float) maxHealth);
    int healthHeight = (int)(currHealthPerc*(background_rect.bottom - background_rect.top));
    FillRect(hdc, &background_rect, CreateNewColorBrush(RGB(128, 128, 128))->brush);
    RECT healthRect = {background_rect.left, background_rect.bottom - healthHeight, background_rect.right, background_rect.bottom};
    int red_tone = 150;
    int blood_brightness = (255-red_tone) * (-currHealthPerc + 1);
    COLORREF health_color = RGB(150+blood_brightness, blood_brightness, blood_brightness);
    FillRect(hdc, &healthRect, CreateNewColorBrush(health_color)->brush);
    PaintSprite(hdc, healthJar);
    char count_text[5];
    sprintf(count_text, "%d", grenade_count);
    SelectObject(hdc, GameFont);
    SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
    SetTextColor(hdc, regular_text_color);
    SetBkMode(hdc, TRANSPARENT);
    TextOut(hdc, grenade_text.x, grenade_text.y, count_text, strlen(count_text));
    PaintSprite(hdc, sprayCan);
    PaintSprite(hdc, weedBag);
    char weed_count_text[5];
    sprintf(weed_count_text, "%d", weed_bags);
    TextOut(hdc, weed_text.x, weed_text.y, weed_count_text, strlen(weed_count_text));
    if (artistOptions && current_level_name) {
        POINT mouse;
        GetCursorPos(&mouse);
        ScreenToClient(mainWindow, &mouse);

        mouse.x -= offsetX;
        mouse.y -= offsetY;
        mouse.x = mouse.x < 0 ? 0 : mouse.x;
        mouse.y = mouse.y < 0 ? 0 : mouse.y;
        int pixelSize = 8;
        POINT as_coords = {(mouse.x*game_res[0])/closest_width, (mouse.y*game_res[1])/closest_height};
        as_coords.x /= pixelSize;
        as_coords.y /= pixelSize;
        as_coords.x *= pixelSize;
        as_coords.y *= pixelSize;

        RECT outPixel = {as_coords.x, as_coords.y, (as_coords.x + pixelSize), (as_coords.y + pixelSize)};
        FillRect(hdc, &outPixel, CreateNewColorBrush(RGB(255, 255, 255))->brush);
        
        char as_text[100];
        sprintf(as_text, "(%d, %d)", as_coords.x, as_coords.y);
        SetTextAlign(hdc, TA_LEFT | TA_TOP);
        TextOut(hdc, 0, 0, current_level_name, strlen(current_level_name));
        TextOut(hdc, 0, 48, as_text, strlen(as_text));
    }

    // Draw key prompt
    if (searchRedirect(FALSE)){
        drawPrompt(hdc, "F");
    } 
}

void endUI(){
    if (sprayCan) EraseSprite(sprayCan);
    if (healthJar) EraseSprite(healthJar);
}