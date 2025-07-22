#include "headers/particles.h"
#include "headers/drawing.h"
#include "headers/movement.h"

#include <windows.h>
#include <stdio.h>

#define COLOR_COUNT 8
#define COOK_TIME 2.0
#define COOK_ERROR 5.0

typedef struct grenade {
    float cook_time, alive_time;
    SpriteGroup *obj;
    COLORREF color;
    float forces[2];
    int thrown;
    struct grenade* next;
} grenade;

COLORREF grenade_colors[COLOR_COUNT] = {RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255), RGB(255, 255, 0), RGB(255, 0, 255),
                                        RGB(0, 255, 255), RGB(255, 255, 255), RGB(0, 0, 0)};

grenade *grenadeObj = NULL;
SIZE labelSize = {10, 7};

COLORREF canColor = RGB(128, 128, 128);
SIZE grenadeSize = {10, 20};
float throwSpeed = 800.0f;

int label_margin_top = 5;

void startGrenade(){
    grenade *currentGrenade = grenadeObj;
    while (currentGrenade != NULL){
        if (currentGrenade->thrown == FALSE){
            return;
        }
        currentGrenade = currentGrenade->next;
    }
    grenade *currGrenade = (grenade *)malloc(sizeof(grenade));
    if (currGrenade == NULL){
        printf("Error allocating space for grenade!\n");
        return;
    }
    
    currGrenade->thrown = FALSE;
    currGrenade->forces[0] = 0.0f;
    currGrenade->forces[1] = 0.0f;

    // Set cook time
    currGrenade->alive_time = 0.0f;
    float cook_error = ( ( rand() % ( (int)COOK_ERROR * 2 ) ) - COOK_ERROR )/ (COOK_ERROR*2);
    currGrenade->cook_time = COOK_TIME + cook_error;

    // Set grenade color
    currGrenade->color = grenade_colors[rand() % COLOR_COUNT];

    currGrenade->next = NULL;

    Sprite *labelSprite = (Sprite *)malloc(sizeof(Sprite));
    if (labelSprite == NULL) {
        printf("Error allocating space for grenade object!\n");
        return;
    }

    int yPos = GetPlayerPos().y + (int)( (float)GetPlayerSize().cy / 2.0);
    int xPos = GetPlayerPos().x;

    // Put the grenade in front of the character
    char *direction = getDirectionSprite(GetPlayerPtr());
    if (strcmp(direction, "_right") == 0) xPos += GetPlayerSize().cx - grenadeSize.cy;
    free(direction);

    SpriteGroup *canGroup = CreateSpriteGroup(xPos, yPos, grenadeSize.cx, grenadeSize.cy, canColor);
    CreateSprite(labelSprite, xPos, yPos + label_margin_top, labelSize.cx, labelSize.cy, currGrenade->color);
    AddSpriteToGroup(canGroup, labelSprite);
    currGrenade->obj = canGroup;

    if (grenadeObj == NULL){
        grenadeObj = currGrenade;
    }else{
        grenade *curr = grenadeObj;
        while (curr->next != NULL) curr = curr->next;
        curr->next = currGrenade;
    }
}

void drawGrenades(HDC hdc){
    grenade *curr = grenadeObj;
    while (curr != NULL) {
        PaintSpriteGroup(hdc, curr->obj);
        curr = curr->next;
    }
}

void throwGrenade(){
    grenade *curr = grenadeObj;
    int found = FALSE;
    while (curr != NULL && !found) {
        if (curr->thrown == FALSE){
            curr->thrown = TRUE;
            curr->forces[1] = -throwSpeed;
            char *direction = getDirectionSprite(GetPlayerPtr());
            if (strcmp(direction, "_right") == 0) {
                curr->forces[0] = throwSpeed;
            }else{
                curr->forces[0] = -throwSpeed;
            }
            free(direction);
            found = TRUE;
        }
        curr = curr->next;
    }
}

void updateGrenades(float dt, SpriteGroup* collisions){
    grenade *curr = grenadeObj;
    grenade *prev = NULL;
    while (curr != NULL) {
        curr->alive_time+=dt;
        if (curr->alive_time >= curr->cook_time){
            grenade *tmp_del = curr;
            SIZE explodeSize = {50, 50};
            createParticles(curr->obj->sprite->pos, explodeSize, 0.2, 0.1, 10, 50, FALSE, 500, curr->color, 50);
            curr = curr->next;
            DeleteSpriteGroup(tmp_del->obj);
            free(tmp_del);
            if (prev == NULL){
                grenadeObj = curr;
            }else{
                prev->next = curr;
            }
            continue;
        }else{
            SIZE smallSpray = {5, 5};
            int density = 1;
            createParticles(curr->obj->sprite->pos, smallSpray, 0.2, 0.1, 5, density, FALSE, TRUE, curr->color, 50);
        }
        if (curr->thrown == FALSE){
            // If player turns and grenade hasn't been tossed
            int xPos = GetPlayerPos().x;
            char *direction = getDirectionSprite(GetPlayerPtr());
            if (strcmp(direction, "_right") == 0)  xPos += GetPlayerSize().cx - grenadeSize.cy;
            free(direction);
            int yPos = GetPlayerPos().y + (int)( (float)GetPlayerSize().cy / 2.0);
            curr->obj->sprite->pos.y = yPos;
            curr->obj->sprite->pos.x = xPos;
            curr->obj->next->sprite->pos.y = yPos + label_margin_top;
            curr->obj->next->sprite->pos.x = xPos;
        }else{
            curr->obj->sprite->pos.x += curr->forces[0]*dt;
            curr->obj->sprite->pos.y += curr->forces[1]*dt;
            curr->obj->next->sprite->pos.y += curr->forces[0]*dt;
            curr->obj->next->sprite->pos.x += curr->forces[1]*dt;

            if (curr->forces[0] != 0){ // """Air resistance"""
                int prevForce = (int) (curr->forces[0]);
                curr->forces[0] += curr->forces[0] > 0 ? -1 : 1;
                if (curr->forces[0] > 0 && prevForce < 0 || curr->forces[0] < 0 && prevForce > 0) curr->forces[0] = 0;
            }

            int grounded = FALSE;
            POINT new_pos = get_transform_due(collisions, curr->obj->sprite, &grounded);

            if (!grounded){
                curr->forces[1] += gravity*dt;
            }else{
                curr->forces[1] = - (curr->forces[1] * 0.4); // grenade bounce
                if (abs(curr->forces[1]) < 10 && curr->forces[1] < 0) curr->forces[1] = 0;
            }

            if (new_pos.x != curr->obj->sprite->pos.x) curr->forces[0] = - (curr->forces[0] * 0.4); // bounce off walls
            curr->obj->sprite->pos.x = new_pos.x;
            curr->obj->sprite->pos.y = new_pos.y;
            curr->obj->next->sprite->pos.x = new_pos.x;
            curr->obj->next->sprite->pos.y = new_pos.y + label_margin_top;
        }
        prev = curr;
        curr = curr->next;
    }
}