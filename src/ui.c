#include "headers/drawing.h"
#include "headers/level_loader.h"
#include "headers/generalvars.h"

#include <windows.h>
#include <stdio.h>

Sprite *healthJar = NULL;
Sprite *sprayCan = NULL;
RECT background_rect = {0, 0, 0, 0};

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
    SIZE AssetSize = {64, 64};
    int margin = 25;
    int upscale = 4;
    CreateImgSprite(healthJar, margin, game_res[1] - margin - AssetSize.cy, AssetSize.cx, AssetSize.cy, "./assets/ui/health_jar.png", upscale);
    background_rect.left = healthJar->pos.x + (2 * upscale);
    background_rect.top = healthJar->pos.y + (2 * upscale);
    background_rect.bottom = background_rect.top + AssetSize.cy - (3 * upscale);
    background_rect.right = background_rect.left + AssetSize.cx - (4 * upscale);
    CreateImgSprite(sprayCan, game_res[0] - margin - AssetSize.cx, margin, AssetSize.cx, AssetSize.cy, "./assets/ui/spray_can.png", 4);
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
    PaintSprite(hdc, sprayCan);
}

void endUI(){
    if (sprayCan) EraseSprite(sprayCan);
    if (healthJar) EraseSprite(healthJar);
}