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
#include "headers/animations.h"

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
    CreateAnimatedSprite(mainMenu, 0, 0, game_res[0], game_res[1], "./assets/ui/main_menu.png", "mm", 12, upscale, NULL);
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
    CreateAnimatedSprite(watermark, x, y, cx, cy, "./assets/watermark_anim.png", "wm", 12, upscale, NULL);
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

int curr_save_opt = 1;

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
    if (save_count == 0){
        curr_save_opt = 1;
        current_save = 0;
    }else{
        curr_save_opt = 0;
        current_save = 0;
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
    save_int("bag_count", &weed_bags);
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
    end_save(FALSE);
}

// GAME MENU INFORMATION
char *menu_opts[] = {"Start Game", "Options", "Credits", "Quit Game", NULL};
void (*menu_funcs[])() = {&selectSave, &openOptions, &showCredits, &forceExit};
int currMenuOpt = 0;

char *funText = "The cat's name is max!";

void load_save_opt(){
    startGame();
    read_save(save_files[current_save]);
    selecting_save = FALSE;
}

void create_new_save_opt(){
    char *save_name = create_brand_new_save();
    startGame();
    read_save(save_name);
    selecting_save = FALSE;
}
int delete_confirmation = FALSE;

void delete_save_opt(){
    if (delete_confirmation){
        if (save_count > 0){
            delete_save(save_files[current_save]);
            selectSave();
            if (save_count == 0) curr_save_opt = 1;
        }
        return;
    }
    delete_confirmation = TRUE;
}

char *save_options[] = {"[] Load Save", "+ Create New Save", "- Delete Save", NULL};
const int save_opt_count = 3;
void (*save_funcs[])() = {&load_save_opt, &create_new_save_opt, &delete_save_opt};

void drawGameMenu(){
    if (showingCredits) return;
    PaintSprite(hdcMem, mainMenu);
    SetTextAlign(hdcMem, TA_TOP);
    SetBkMode(hdcMem, TRANSPARENT);
    SelectObject(hdcMem, GameFont);
    TEXTMETRIC tm;
    GetTextMetrics(hdcMem, &tm);

    if (selecting_save){
        SetTextAlign(hdcMem, TA_CENTER | TA_BASELINE);

        // Save colors
        COLORREF border_color = RGB(190, 151, 106);
        COLORREF outer_bg_color = RGB(146, 105, 59);
        COLORREF inner_bg_color = RGB(121, 85, 43);

        // Draw background
        POINT actual_mid = {(long)((double)game_res[0]/2.0), (long)((double)game_res[1]/2.0)};
        SIZE bg_size = {1016, 572};
        RECT border_bg = {actual_mid.x-(bg_size.cx/2), actual_mid.y-(bg_size.cy/2), actual_mid.x+(bg_size.cx/2), actual_mid.y+(bg_size.cy/2)};
        RECT save_bg = {border_bg.left+8, border_bg.top+8, border_bg.right-8, border_bg.bottom - 8};
        
        // DRAW BG
        FillRect(hdcMem, &border_bg, CreateNewColorBrush(border_color)->brush);
        FillRect(hdcMem, &save_bg, CreateNewColorBrush(outer_bg_color)->brush);

        POINT mid_pos;
        mid_pos.x = actual_mid.x;
        for (int i = 0; i < save_count; i++){
            int box_height = 175;
            int top = save_bg.top + 5 + (box_height + 10)*i;
            // Get Save border
            RECT save_box = {save_bg.left+25, top, save_bg.right-25, top + box_height};
            mid_pos.y = save_box.top + (box_height - tm.tmHeight + tm.tmAscent)/2;
            
            // Get Save background
            RECT save_box_bg = {
                save_box.left + 5,
                save_box.top + 5,
                save_box.right - 5,
                save_box.bottom - 5
            };

            // DRAW THE BOX
            FillRect(hdcMem, &save_box, CreateNewColorBrush(border_color)->brush);
            FillRect(hdcMem, &save_box_bg, CreateNewColorBrush(inner_bg_color)->brush);

            if (i == current_save){
                SetTextColor(hdcMem, highlight_text_color);
            }else{
                SetTextColor(hdcMem, regular_text_color);
            }

            TextOut(hdcMem, mid_pos.x, mid_pos.y, save_names[i], strlen(save_names[i]));
        }
        if (save_count == 0){
            char *no_saves_text = "There are no saves currently!";
            TextOut(hdcMem, mid_pos.x, save_bg.top + 5 + tm.tmHeight, no_saves_text, strlen(no_saves_text));
        }

        // DRAW Button options
        int buttons_width = 20 + (strlen(save_options[2]) + strlen(save_options[1]) + strlen(save_options[0]))*tm.tmAveCharWidth;
        int buttons_height = tm.tmHeight + tm.tmAscent;
        RECT buttons_bg = {border_bg.right-buttons_width, border_bg.bottom, border_bg.right, border_bg.bottom + buttons_height};
        FillRect(hdcMem, &buttons_bg, CreateNewColorBrush(border_color)->brush);
        SetTextAlign(hdcMem, TA_LEFT | TA_BOTTOM);
        int curr_x = buttons_bg.left + 5;
        for (int i = 0; i < 3; i++){
            if (i == curr_save_opt){
                SetTextColor(hdcMem, highlight_text_color);
            }else{
                if (save_count != 0){
                    SetTextColor(hdcMem, regular_text_color);
                }else{
                    SetTextColor(hdcMem, undermined_text_color);
                }
            }
            TextOut(hdcMem, curr_x, buttons_bg.bottom, save_options[i], strlen(save_options[i]));
            curr_x += strlen(save_options[i])*tm.tmAveCharWidth + 5;
        }

    }else{
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
            if (current_save < save_count - 1) current_save++;
        }
        if (key == VK_RIGHT){
            if (save_count == 0) {
                curr_save_opt = 1;
                return;
            }
            if (curr_save_opt < save_opt_count - 1) curr_save_opt++;
        }
        if (key == VK_LEFT){
            if (save_count == 0) {
                curr_save_opt = 1;
                return;
            }
            if (curr_save_opt > 0) curr_save_opt--;
        }
        if (key == VK_RETURN){
            if (curr_save_opt < save_opt_count){
                save_funcs[curr_save_opt]();
            }
            return;
        }
        if (delete_confirmation) delete_confirmation = FALSE;
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
    if (showingCredits){
        draw_credits(hdcMem);
        return;
    }
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