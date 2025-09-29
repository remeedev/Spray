#include <windows.h>
#include <stdio.h>

#include "headers/drawing.h"
#include "headers/movement.h"
#include "headers/level_loader.h"
#include "headers/console.h"
#include "headers/generalvars.h"
#include "headers/particles.h"
#include "headers/credits.h"
#include "headers/savefile.h"
#include "headers/throwables.h"
#include "headers/the_chronic.h"

// WATERMARK NECESSARY DATA
int watermarkShow = TRUE;

float wm_time = 0.0f;
float wm_duration = 5.0f;

Sprite *mainMenu = NULL;
float time_since_anim = 0.0f;
int time_between_anim = 5;

Sprite *watermark = NULL;

void endMainMenu(){
    if (mainMenu) EraseSprite(mainMenu);
    mainMenu = NULL;
}

void startMainMenu(){
    endMainMenu();
    mainMenu = (Sprite *)malloc(sizeof(Sprite));
    int upscale = game_res[0]/160.0;
    CreateAnimatedSprite(mainMenu, 0, 0, game_res[0], game_res[1], "./assets/ui/main_menu.png", "mm", 12, upscale);
}

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
    InitPlayer();
    init_save();
    start_weed_convos();
    StartGraphics(hWnd);
    Resize(hWnd, screen_width, screen_height);
    LoadBrushes();
    startMainMenu();
    init_credits();
    build_credits();
    save_console_allow();

    if (watermarkShow){
        loadWatermark();
    }
}

char **save_names = NULL;
char **save_files = NULL;
int save_count;
int selecting_save = FALSE;
int current_save = 0;

void startGame();

void selectSave(){
    if (save_names){
        for (int i = 0; i < save_count; i++){
            free(save_names[i]);
            free(save_files[i]);
        }
        free(save_names);
        free(save_files);
    }
    save_count = get_saves(&save_names, &save_files);
    if (save_count == 0) {
        char *save_name = create_brand_new_save();
        startGame();
        read_save(save_name);
        selecting_save = FALSE;
        return;
    }
    selecting_save = TRUE;
}

int started_game = FALSE;

void startGame(){
    endMainMenu();
    POINT zero_pos;
    loadLevel("./levels/level_0.txt");
    zero_pos.x = 0;
    zero_pos.y = 0;
    SetPlayerPos(zero_pos);
    player_forces[0] = 0;
    player_forces[1] = 0;
    paused = FALSE;
    in_level = TRUE;
    if (started_game) return;
    started_game = TRUE;
    save_int("health", &GetPlayerPtr()->health);
    save_int("grenades", &grenade_count);
    save_str("name", &user_name);
    save_long("x", &GetPlayerPtr()->pos.x);
    save_long("y", &GetPlayerPtr()->pos.y);
}

void openOptions(){

}

void showCredits(){
    hide_show_credits();
}

void ForceGameMenu(){
    EndLastLevel();
    in_level = FALSE;
    startMainMenu();
}

// GAME MENU INFORMATION
char *menu_opts[] = {"Start Game", "Options", "Credits", "Quit Game", NULL};
void (*menu_funcs[])() = {&selectSave, &openOptions, &showCredits, &forceExit};
int currMenuOpt = 0;

char *funText = "The cat's name is max!";

void drawGameMenu(){
    PaintSprite(hdcMem, mainMenu);
    SetTextAlign(hdcMem, TA_TOP);
    SetBkMode(hdcMem, TRANSPARENT);
    SelectObject(hdcMem, GameFont);

    if (selecting_save){
        SetTextAlign(hdcMem, TA_CENTER | VTA_CENTER);
        int mid_x = (int)((float)game_res[0]/2.0);
        int v_padding = 5;
        int h_padding = 80;
        RECT background_box = {h_padding-25, v_padding, game_res[0] - h_padding + 25, game_res[1]-v_padding};
        int box_height = (int)((float)(background_box.bottom - background_box.top)/3.0);
        FillRect(hdcMem, &background_box, CreateNewColorBrush(RGB(150, 123, 75))->brush);
        for (int i = 0; i < 3; i++){
            RECT save_rect = {h_padding, box_height*i + v_padding + background_box.top, game_res[0]-h_padding, box_height*(i + 1)-(v_padding*2)};
            HBRUSH brush = i == current_save ? CreateNewColorBrush(RGB(120, 100, 60))->brush : CreateNewColorBrush(RGB(173, 135, 87))->brush;
            FillRect(hdcMem, &save_rect, brush);
            POINT mid_pos = {(save_rect.right-save_rect.left)/2, (save_rect.bottom - save_rect.top)/2};
            mid_pos.y += i*box_height;
            if (i < save_count){
                TextOut(hdcMem, mid_pos.x, mid_pos.y, save_names[i], strlen(save_names[i]));
            }else{
                char *create_text = "+ Create Save";
                TextOut(hdcMem, mid_pos.x, mid_pos.y, create_text, strlen(create_text));
            }
        }
    }else{
        TEXTMETRIC tm;
        GetTextMetrics(hdcMem, &tm);
        int curr = 0;
        int startPadding = 225;
        int leftMargin = 20;
        while (menu_opts[curr] != NULL){
            int x = 0;
            SetTextColor(hdcMem, regular_text_color);
            if (currMenuOpt == curr){
                x = 20;
                SetTextColor(hdcMem, highlight_text_color);
            }
            TextOut(hdcMem, x + leftMargin, startPadding + curr*(tm.tmHeight+tm.tmAscent), menu_opts[curr], strlen(menu_opts[curr]));
            curr++;
        }
        SetTextAlign(hdcMem, TA_BOTTOM | TA_RIGHT);
        SetTextColor(hdcMem, ignore_text_color);
        TextOut(hdcMem, game_res[0], game_res[1], gameVersion, strlen(gameVersion));
    }
}

void handleKEYDOWN(UINT key){
    if (showingCredits){
        if (key == VK_SPACE){
            switch_credit_speed(TRUE);
        }
        if (key == VK_ESCAPE){
            hide_show_credits();
        }
        return;
    }
    if (in_level){
        HandleKeyDown(key);
        UIKeyDown(key);
        return;
    }
    if (watermark != NULL){
        if (key == VK_SPACE){
            endWatermark();
            watermarkShow = FALSE;
        }
        return;
    }
    if (selecting_save){
        if (key == VK_ESCAPE){
            selecting_save = FALSE;
        }
        if (key == VK_UP){
            if (current_save > 0) current_save--;
        }
        if (key == VK_DOWN){
            if (current_save < save_count && current_save < 2) current_save++;
        }
        if (key == VK_RETURN){
            if (current_save < save_count){
                startGame();
                read_save(save_files[current_save]);
            }else{
                char *save_name = create_brand_new_save();
                startGame();
                read_save(save_name);
            }
            selecting_save = FALSE;
        }
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
    if (showingCredits){
        if (key == VK_SPACE){
            switch_credit_speed(FALSE);
        }
        return;
    }
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
    // Needs update for special feature
    draw_credits(hdcMem);
    if (in_level){
        DrawGame();
        return;
    }
    if (watermarkShow && wm_time < wm_duration && watermark != NULL){
        // DRAW WATERMARK
        PaintSprite(hdcMem, watermark);
        SetTextColor(hdcMem, regular_text_color);
        SetBkMode(hdcMem, TRANSPARENT);
        SetTextAlign(hdcMem, TA_BOTTOM | TA_LEFT);
        SelectObject(hdcMem, GameFont);
        char *hintText = "Press space bar to skip cutscene";
        TextOut(hdcMem, 0, game_res[1], hintText, strlen(hintText));
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
    update_credits(dt);
    if (in_level){
        if (!paused) Update(dt);
        if (paused) {
            if (GetFrame(pause_menu_anim->brush->anim_group) != 15) {
                UpdateAnimatedSprite(pause_menu_anim->brush->anim_group, dt);
            }else{
                time_since_anim += dt;
                if (time_since_anim > time_between_anim){
                    time_since_anim = 0.0f;
                    while (GetFrame(pause_menu_anim->brush->anim_group) != 0) UpdateAnimatedSprite(pause_menu_anim->brush->anim_group, dt);
                    time_between_anim = 5 + rand()%5;
                }
            }
        }
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
    if (GetFrame(mainMenu->brush->anim_group) != 23) {
        UpdateAnimatedSprite(mainMenu->brush->anim_group, dt);
    }else {
        time_since_anim += dt;
        if (time_since_anim > time_between_anim){
            time_since_anim = 0.0f;
            while (GetFrame(mainMenu->brush->anim_group) != 0){
                UpdateAnimatedSprite(mainMenu->brush->anim_group, dt);
            }
            time_between_anim = 5 + rand()%5;
        }
    }
}