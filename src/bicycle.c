/*

THIS FILE IS THE DAYTIME CYCLE
BUT TIME.H HAS A VARIABLE CALLED DAYTIME
SO NOW ITS CALLED BICYCLE

*/

#include "headers/level_loader.h"
#include "headers/drawing.h"
#include "headers/console.h"

#include <windows.h>
#include <stdio.h>

int set_gradient = FALSE;
HDC hdc_gradient;
HBITMAP gradient;

double game_time = 0;
int max_time = 900; // This represents a from morning to sunset
int gradient_height;

void startDayCycle(HWND hwnd){
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

void updateDayCycle(float dt){
    game_time += dt;
    if (game_time >= max_time)game_time -= max_time;
}

void drawDayCycle(HDC hdc){
    int shownHeight = gradient_height*(game_time / max_time);
    BitBlt(hdc, 0, 0, game_res[0], game_res[1], hdc_gradient, 0, shownHeight, SRCCOPY);
    if (gradient_height-shownHeight < game_res[1]){
        BitBlt(hdc, 0, gradient_height - shownHeight, game_res[0], game_res[1], hdc_gradient, 0, 0, SRCCOPY);
    }
}