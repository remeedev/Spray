#include <windows.h>
#include <stdio.h>

#include "headers/drawing.h"
#include "headers/movement.h"
#include "headers/level_loader.h"
#include "headers/console.h"
#include "headers/generalvars.h"
#include "headers/particles.h"

// WATERMARK NECESSARY DATA
int watermarkShow = TRUE;

float wm_time = 0.0f;
float wm_duration = 5.0f;

Sprite *watermark = NULL;
void endWatermark(){
    if (watermark)EraseSprite(watermark);
    watermark = NULL;
}
void loadWatermark(){
    endWatermark();
    watermark = (Sprite *)malloc(sizeof(Sprite));
    int cx = game_res[0]*0.6;
    cx = cx - cx % 96;
    int cy = cx/2;
    int x = (game_res[0] - cx) / 2;
    int y = (game_res[1] - cy) / 2;
    int upscale = cx/96;
    CreateAnimatedSprite(watermark, x, y, cx, cy, "./assets/watermark_anim.png", "wm", 12, upscale);
}

int in_level = FALSE;

void startGameSystem(HWND hWnd, int screen_width, int screen_height){
    StartGraphics(hWnd);
    Resize(hWnd, screen_width, screen_height);
    LoadBrushes();

    if (watermarkShow){
        loadWatermark();
    }
}

void startGame(){
    InitPlayer();
    POINT zero_pos;
    zero_pos.x = 0;
    zero_pos.y = 0;
    SetPlayerPos(zero_pos);
    player_forces[0] = 0;
    player_forces[1] = 0;
    loadLevel("./levels/particles.txt");
    paused = FALSE;
    in_level = TRUE;
}

void openOptions(){

}

void showCredits(){

}

void ForceGameMenu(){
    EndLastLevel();
    in_level = FALSE;
}

// GAME MENU INFORMATION
char *menu_opts[] = {"Start Game", "Options", "Credits", "Quit Game", NULL};
void (*menu_funcs[])() = {&startGame, &openOptions, &forceExit};
int currMenuOpt = 0;

void drawGameMenu(){
    SelectObject(hdcMem, TitleFont);
    SetTextAlign(hdcMem, TA_TOP);
    SetTextColor(hdcMem,regular_text_color);
    SetBkMode(hdcMem, TRANSPARENT);
    TextOut(hdcMem, 0, 0, "Sprayz", 6);
    SelectObject(hdcMem, GameFont);

    TEXTMETRIC tm;
    GetTextMetrics(hdcMem, &tm);
    int curr = 0;
    int startPadding = 80;
    while (menu_opts[curr] != NULL){
        int x = 0;
        SetTextColor(hdcMem, undermined_text_color);
        if (currMenuOpt == curr){
            x = 20;
            SetTextColor(hdcMem, highlight_text_color);
        }
        TextOut(hdcMem, x, startPadding + curr*(tm.tmHeight+tm.tmAscent), menu_opts[curr], strlen(menu_opts[curr]));
        curr++;
    }
    SetTextAlign(hdcMem, TA_BOTTOM | TA_RIGHT);
    SetTextColor(hdcMem, ignore_text_color);
    TextOut(hdcMem, game_res[0], game_res[1], gameVersion, strlen(gameVersion));
}

void handleKEYDOWN(UINT key){
    if (in_level){
        HandleKeyDown(key);
        UIKeyDown(key);
        return;
    }
    if (watermark != NULL && key == VK_SPACE){
        endWatermark();
        watermarkShow = FALSE;
        return;
    }
    switch(key){
        case VK_UP: ;
            if (currMenuOpt > 0) currMenuOpt--;
            break;
        case VK_DOWN: ;
            size_t opt_count = 0;
            while (menu_opts[opt_count] != NULL)opt_count++;
            if (currMenuOpt < opt_count-1) currMenuOpt++;
            break;
        case VK_RETURN: ;
            menu_funcs[currMenuOpt]();
            break;
    }
}

void handleKEYUP(UINT key){
    if (in_level){
        HandleKeyUp(key);
        return;
    }
}

void handleCHAR(UINT key){
    if (in_level){
        handleCharConsole(key);
        return;
    }
}

void taskDraws(){
    if (in_level){
        DrawGame();
        return;
    }
    if (watermarkShow && wm_time < wm_duration && watermark != NULL){
        // DRAW WATERMARK
        PaintSprite(hdcMem, watermark);
        return;
    }
    drawGameMenu();
}


void drawEvent(HWND hWnd){
    PAINTSTRUCT ps;
    HDC hdcW = BeginPaint(hWnd, &ps);
    RECT rcPaint;
    rcPaint.top = 0;
    rcPaint.left = 0;
    rcPaint.bottom = game_res[1];
    rcPaint.right = game_res[0];
    FillRect(hdcMem, &rcPaint, CreateNewColorBrush(RGB(0, 0, 0))->brush);
    
    // BLACK BARS
    if (resized_ticks > 0.01){
        resized_ticks++;
        if (resized_ticks > 5){
            HBRUSH black_brush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdcW, &ps.rcPaint, black_brush);
            DeleteObject(black_brush);
            resized_ticks = 0;
        }
    }

    taskDraws();

    // FIX FOR ARTIFACTS
    SetStretchBltMode(hdcW, HALFTONE);
    SetBrushOrgEx(hdcW, 0, 0, NULL);
    
    StretchBlt(hdcW, offsetX, offsetY, closest_width, closest_height, hdcMem, 0, 0, game_res[0], game_res[1], SRCCOPY);
    EndPaint(hWnd, &ps);
}

void updateEvent(float dt){
    if (in_level){
        if (!paused) Update(dt);
        return;
    }
    if (watermarkShow && wm_time < wm_duration && watermark != NULL){
        if (GetFrame(watermark->brush->anim_group) < 31) UpdateAnimatedSprite(watermark->brush->anim_group, dt);
        wm_time += dt;
        if (wm_time >= wm_duration) {
            endWatermark();
        }
        return;
    }
}