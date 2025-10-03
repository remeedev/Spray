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
#include "headers/lang.h"

// WATERMARK NECESSARY DATA
int watermarkShow = TRUE;

float wm_time = 0.0f;
float wm_duration = 5.0f;

Sprite *mainMenu = NULL;
float time_since_anim = 0.0f;
int time_between_anim = 5;

Sprite *watermark = NULL;

// Options Variables
int options_menu_visible = FALSE;

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
    save_int_conf("lang", &lang);
    save_int_conf("wm_s", &watermarkShow);
    read_save_conf();
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
    options_menu_visible = TRUE;
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
// Texts has gone to lang.c, ain't gonna add translation to each file that puts text inside the screen, only one
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

const int save_opt_count = 3;
void (*save_funcs[])() = {&load_save_opt, &create_new_save_opt, &delete_save_opt};

void drawGameMenu(){
    if (showingCredits || options_menu_visible) return;
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
        int buttons_width = 20 + (strlen(get_save_option_text(2)) + strlen(get_save_option_text(1)) + strlen(get_save_option_text(0)))*tm.tmAveCharWidth;
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
            TextOut(hdcMem, curr_x, buttons_bg.bottom, get_save_option_text(i), strlen(get_save_option_text(i)));
            curr_x += strlen(get_save_option_text(i))*tm.tmAveCharWidth + 5;
        }

    }else {
        // Draw Main Menu
        int curr = 0;
        int startPadding = 225;
        int leftMargin = 20;
        while (get_mm_option_text(curr) != NULL){
            int x = 0;
            SetTextColor(hdcMem, regular_text_color);
            if (currMenuOpt == curr){
                x = 20;
                SetTextColor(hdcMem, highlight_text_color);
            }
            TextOut(hdcMem, x + leftMargin, startPadding + curr*(tm.tmHeight+tm.tmAscent), get_mm_option_text(curr), strlen(get_mm_option_text(curr)));
            curr++;
        }
        SetTextAlign(hdcMem, TA_BOTTOM | TA_RIGHT);
        SetTextColor(hdcMem, ignore_text_color);
        TextOut(hdcMem, game_res[0], game_res[1], gameVersion, strlen(gameVersion));
    }
}

void back_from_options_menu(UINT key){
    if (key != VK_RETURN) return;
    options_menu_visible = FALSE;
}

void change_watermark_show(UINT key){
    if (key == VK_RETURN) watermarkShow = !watermarkShow;
}

void change_language(UINT key){
    if (key == VK_RIGHT) change_lang_next();
    if (key == VK_LEFT) change_lang_prev();
}

int option_key_selected = 0;
void (*option_key_handle[])(UINT) = {&change_language, &change_watermark_show, &back_from_options_menu, NULL};

void drawOptionsMenu(){
    if (!options_menu_visible) return;
    TEXTMETRICW tm;
    GetTextMetricsW(hdcMem, &tm);
    
    PaintSprite(hdcMem, pause_menu_anim);

    SetTextAlign(hdcMem, TA_CENTER | TA_TOP);
    SelectObject(hdcMem, GameFont);
    SetTextColor(hdcMem, regular_text_color);
    TextOutW(hdcMem, (int)((double)game_res[0]/2.0), 0, get_options_option_text(0), wcslen(get_options_option_text(0)));

    SetTextAlign(hdcMem, TA_LEFT | TA_TOP);
    int curr_opt_added = 0;
    int x = 15;
    int right_margin = game_res[0] - 10;
    int y = tm.tmHeight;
    TextOutW(hdcMem, x, y, get_options_option_text(1), wcslen(get_options_option_text(1)));

    // < lang >
    SetTextAlign(hdcMem, TA_RIGHT | TA_TOP);
    TextOut(hdcMem, right_margin, y, ">", 1);
    right_margin -= tm.tmAveCharWidth*2;
    if (curr_opt_added == option_key_selected){
        SetTextColor(hdcMem, highlight_text_color);
    }else{
        SetTextColor(hdcMem, regular_text_color);
    }
    TextOutW(hdcMem, right_margin, y, get_options_option_text(2 + get_lang()), wcslen(get_options_option_text(2 + get_lang())));
    right_margin -= tm.tmAveCharWidth*( wcslen(get_options_option_text(2 + get_lang())) + 1);
    SetTextColor(hdcMem, regular_text_color);
    TextOut(hdcMem, right_margin, y, "<", 1);
    curr_opt_added++;

    SetTextAlign(hdcMem, TA_LEFT | TA_TOP);
    y += tm.tmHeight + 10;
    TextOutW( hdcMem, x, y, get_options_option_text(4), wcslen( get_options_option_text(4) ) );
    
    // On/Off
    right_margin = game_res[0] - 10;
    SetTextAlign(hdcMem, TA_RIGHT | TA_TOP);
    if (curr_opt_added == option_key_selected){
        SetTextColor(hdcMem, highlight_text_color);
    }else{
        SetTextColor(hdcMem, regular_text_color);
    }
    TextOutW(hdcMem, right_margin, y, watermarkShow ? get_options_option_text(7) : get_options_option_text(6), wcslen(watermarkShow ? get_options_option_text(7) : get_options_option_text(6)));
    SetTextColor(hdcMem, regular_text_color);
    curr_opt_added++;

    SetTextAlign(hdcMem, TA_BOTTOM | TA_LEFT);

    if (curr_opt_added == option_key_selected){
        SetTextColor(hdcMem, highlight_text_color);
    }else{
        SetTextColor(hdcMem, regular_text_color);
    }
    TextOutW(hdcMem, x, game_res[1] - 15, get_options_option_text(5), wcslen(get_options_option_text(5)));
}

void handleKEYDOWN(UINT key){
    if (watermarkShow == FALSE && watermark != NULL) endWatermark();
    if (options_menu_visible){
        if (key == VK_RIGHT || key == VK_LEFT || key == VK_RETURN){
            option_key_handle[option_key_selected](key);
            write_conf();
        }
        if (key == VK_ESCAPE){
            options_menu_visible = FALSE;
        }
        if (key == VK_UP){
            if (option_key_selected > 0) option_key_selected--;
        }
        if (key == VK_DOWN){
            size_t len = 0;
            while (option_key_handle[len] != NULL) len++;
            if (option_key_selected < len - 1) option_key_selected++;
        }
        return;
    }
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
            while (get_mm_option_text(opt_count) != NULL)opt_count++;
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
    if (options_menu_visible){
        drawOptionsMenu();
        return;
    }
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
        char *hintText = skip_text(VK_SPACE);
        TextOut(hdcMem, 0, game_res[1], hintText, strlen(hintText));
        free(hintText);
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
    if (paused || options_menu_visible) {
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