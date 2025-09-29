/*

THIS FILE IS THE DAYTIME CYCLE
BUT TIME.H HAS A VARIABLE CALLED DAYTIME
SO NOW ITS CALLED BICYCLE

*/

#include "headers/level_loader.h"
#include "headers/drawing.h"
#include "headers/console.h"
#include "headers/savefile.h"

#include <windows.h>
#include <stdio.h>

int set_gradient = FALSE;
HDC hdc_gradient;
HBITMAP gradient;

double game_time = 0;
int max_time = 900; // This represents a from morning to sunset
int gradient_height;

Sprite *mountains;
int prevX = 0;
int mountainX = 0;
int assumed_level = 0;

SpriteGroup *clouds = NULL;
double time_between_clouds = 3.0;
double time_since_cloud = 3.0;

int cloud_count = 5;
int saved_X = FALSE;

void startDayCycle(HWND hwnd){
    if (!saved_X){
        if (save_int("mountain_x", &mountainX) == FALSE){
            printf("Unable to save mountain_x\n");
        }
        if (save_int("assumed_level", &assumed_level) == FALSE){
            printf("Unable to save assumed_level!\n");
        }
    }
    mountains = (Sprite *)malloc( sizeof(Sprite) );
    if (mountains == NULL){
        printf("Error loading the image of the mountains!\n");
        return;
    }
    CreateImgSprite(mountains, 0, 0, game_res[0]*2, game_res[1], "./assets/sky/mountains.png", 8);
    if (set_gradient) return;
    HDC main_hdc = GetDC(hwnd);
    hdc_gradient = CreateCompatibleDC(main_hdc);
    gradient_height = game_res[1]*4;
    gradient = CreateCompatibleBitmap(main_hdc, game_res[0], gradient_height);
    int gradient_steps = 100 * 2;
    int r1, g1, b1;
    r1 = 230;
    g1 = 163;
    b1 = 87;
    int r2, g2, b2;
    r2 = 11;
    g2 = 6;
    b2 = 15;
    HGDIOBJ old = SelectObject(hdc_gradient, gradient);
    if (old == NULL || old == HGDI_ERROR){
        printf("There was an error selecting the gradient obj\n");
        DeleteDC(hdc_gradient);
        DeleteObject(gradient);
        ReleaseDC(hwnd, main_hdc);
        return;
    }
    int y_step = gradient_height/gradient_steps;
    for (int i = 0; i < gradient_steps + 3; i++){
        int step_red, step_green, step_blue;
        int c = gradient_steps / 2;
        if (i < c){
            step_red   = r1 + ((r2 - r1) * i)*2 / gradient_steps;
            step_green = g1 + ((g2 - g1) * i)*2 / gradient_steps;
            step_blue  = b1 + ((b2 - b1) * i)*2 / gradient_steps;
        }else{
            step_red   = r2 + ((r1 - r2) * (i - c))*2 / gradient_steps;
            step_green = g2 + ((g1 - g2) * (i - c))*2 / gradient_steps;
            step_blue  = b2 + ((b1 - b2) * (i - c))*2 / gradient_steps;
        }

        COLORREF rect_color = RGB(step_red, step_green, step_blue);

        RECT rect_space = {0, y_step*i, game_res[0], y_step * (i + 1)};
        FillRect(hdc_gradient, &rect_space, CreateNewColorBrush(rect_color)->brush);
    }
    ReleaseDC(hwnd, main_hdc);
    set_gradient = TRUE;
}

void endDayCycle(){
    if (!set_gradient) return;
    DeleteObject(gradient);
    DeleteDC(hdc_gradient);
}

double relative_movement(){
    return (( (double)(GetPlayerPos().x) / (double)game_res[1] )*0.01)*(double)game_res[0];
}

void updateDayCycle(float dt){
    game_time += dt;
    if (game_time >= max_time)game_time -= max_time;
    if (abs(prevX - GetPlayerPos().x) > 500){
        if ((int)GetPlayerPos().x < (int)(game_res[0]/2.0)){
            assumed_level ++;
            mountainX = mountains->pos.x;
        }else{
            assumed_level --;
            mountainX = mountains->pos.x + relative_movement();
        }
    }
    prevX = GetPlayerPos().x;
    SpriteGroup *currCloud = clouds;
    SpriteGroup *prev = NULL;
    int cloudN = 0;
    int clouds_spawned = 0;
    while (currCloud != NULL) {
        clouds_spawned++;
        currCloud->sprite->pos.x += 1;
        if ((long)currCloud->sprite->pos.x > (long)game_res[0]){
            SpriteGroup *next = currCloud->next;
            EraseSprite(currCloud->sprite);
            free(currCloud);
            if (prev != NULL){
                prev->next = next;
            }else{
                clouds = next;
            }
            currCloud = next;
            continue;
        }
        prev = currCloud;
        currCloud = currCloud->next;
    }
    if (clouds_spawned < cloud_count){
        time_since_cloud += dt;
        if (time_since_cloud > time_between_clouds){
            time_since_cloud = 0.0;
            Sprite *cloud = (Sprite *)malloc(sizeof(Sprite));
            if (cloud == NULL){
                printf("Error loading a new cloud!\n");
                return;
            }
            int upscale = 8;
            int x = -80*upscale - rand()%500;
            int y = rand()%400 - 250;
            CreateImgSprite(cloud, x, y, 80*upscale, 45*upscale, "./assets/sky/cloud0.png", upscale);
            if (clouds == NULL){
                clouds = (SpriteGroup *)malloc(sizeof(SpriteGroup));
                clouds->next = NULL;
                clouds->sprite = cloud;
            }else{
                AddSpriteToGroup(clouds, cloud);
            }
        }
    }
}


void drawDayCycle(HDC hdc){
    int shownHeight = gradient_height*(game_time / max_time);
    BitBlt(hdc, 0, 0, game_res[0], game_res[1], hdc_gradient, 0, shownHeight, SRCCOPY);
    if (gradient_height-shownHeight < game_res[1]){
        BitBlt(hdc, 0, gradient_height - shownHeight, game_res[0], game_res[1], hdc_gradient, 0, 0, SRCCOPY);
    }
    mountains->pos.x = mountainX - relative_movement();
    if (mountains->pos.x > 0)mountains->pos.x = 0;
    if (mountains->pos.x + mountains->size.cx < game_res[1]) mountains->pos.x = game_res[1] - mountains->size.cx;
    if (clouds) PaintSpriteGroup(hdc, clouds);
    PaintSprite(hdc, mountains);
}
