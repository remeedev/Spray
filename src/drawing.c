#include <windows.h>
#include <stdio.h>
#include "headers/images.h"
#include <string.h>
#include <wingdi.h>

typedef struct BrushPalette {
    HBRUSH brush;
    COLORREF color;
    char *name;
    HBITMAP bmp;
    struct BrushPalette *next;
} BrushPalette;

typedef struct Sprite {
    POINT pos;
    SIZE size;
    BrushPalette* brush;
} Sprite;

typedef struct SpriteGroup {
    Sprite* sprite;
    struct SpriteGroup* next;
} SpriteGroup;

BrushPalette *brushes = NULL;

BrushPalette* SearchBrushesColor(COLORREF color){
    if (brushes == NULL)return NULL;
    BrushPalette *curr = brushes;
    int found = FALSE;
    while (curr != NULL && !found){
        if (curr->color == color) {
            found = TRUE;
        }else{
            curr = curr->next;
        }
    }
    return curr;
}

BrushPalette *CreateNewColorBrush(COLORREF color){
    if (SearchBrushesColor(color) != NULL){
        return SearchBrushesColor(color);
    }
    HBRUSH brush = CreateSolidBrush(color);
    BrushPalette *holder = (BrushPalette *)malloc(sizeof(BrushPalette));
    holder->brush = brush;
    holder->color = color;
    holder->name = "";
    holder->next = NULL;
    if (brushes == NULL){
        brushes = holder;
    }else{
        BrushPalette *curr = brushes;
        while (curr->next != NULL)curr = curr->next;
        curr->next = holder;
    }
}

BrushPalette* SearchBrushesName(char *str){
    if (brushes == NULL)return NULL;
    BrushPalette *curr = brushes;
    int found = FALSE;
    while (curr != NULL && !found){
        if (strcmp(curr->name, str)) {
            found = TRUE;
        }else{
            curr = curr->next;
        }
    }
    return curr;
}

BrushPalette * LoadImageAsBrush(char *name){
    if (SearchBrushesName(name) != NULL){
        return SearchBrushesName(name);
    }
    BrushPalette *holder = (BrushPalette *)malloc(sizeof(BrushPalette));
    holder->brush = NULL;
    holder->color = (COLORREF)NULL;
    holder->name = name;
    holder->bmp = LoadPNGAsBmp(name);
    holder->next = NULL;
    if (brushes == NULL){
        brushes = holder;
    }else{
        BrushPalette *curr = brushes;
        while (curr->next != NULL)curr = curr->next;
        curr->next = holder;
    }
}

void LoadBrushes(){
    CreateNewColorBrush(RGB(255, 0, 0));
    CreateNewColorBrush(RGB(0, 255, 0));
    CreateNewColorBrush(RGB(0, 0, 255));
}

void deleteBrushes(){
    BrushPalette *curr = brushes;
    while (curr != NULL){
        if (curr->brush) DeleteObject(curr->brush);
        if (curr->bmp) DeleteObject(curr->bmp);
        BrushPalette *prev = curr;
        curr = curr->next;
        free(prev);
    }
}

void EraseSprite(Sprite *sprite){
    free(sprite);
}

// Function to create sprites
void CreateSprite(Sprite* sprite, int x, int y, int cx, int cy, COLORREF color){
    // Create the brush material
    sprite->brush = CreateNewColorBrush(color);
    POINT pos;
    pos.x = x;
    pos.y = y;
    SIZE size;
    size.cx = cx;
    size.cy = cy;
    sprite->pos = pos;
    sprite->size = size;
}

void CreateImgSprite(Sprite* sprite, int x, int y, int cx, int cy, char *name){
    sprite->brush = LoadImageAsBrush(name);
    POINT pos;
    pos.x = x;
    pos.y = y;
    SIZE size;
    size.cx = cx;
    size.cy = cy;
    sprite->pos = pos;
    sprite->size = size;
}

// Creating the main sprite
Sprite *player = NULL;

// ====================== Sprite Group Logic =======================

void AddSpriteToGroup(SpriteGroup *group, Sprite *sprite){
    SpriteGroup *curr_elem = group;
    while (curr_elem->next != NULL){
        curr_elem = curr_elem->next;
    }
    SpriteGroup *new_sprite = (SpriteGroup *)malloc(sizeof(SpriteGroup));
    new_sprite->sprite = sprite;
    new_sprite->next = NULL;
    curr_elem->next = new_sprite;
}

void CreateSpriteInGroup(SpriteGroup *group, int x, int y, int cx, int cy, COLORREF color){
    Sprite *elem = (Sprite *)malloc(sizeof(Sprite));
    CreateSprite(elem, x, y, cx, cy, color);
    AddSpriteToGroup(group, elem);
}

SpriteGroup *CreateSpriteGroup(int x, int y, int cx, int cy, COLORREF color){
    SpriteGroup *out = (SpriteGroup *)malloc(sizeof(SpriteGroup));
    Sprite *first_elem = (Sprite *)malloc(sizeof(Sprite));
    CreateSprite(first_elem, x, y, cx, cy, color);
    out->sprite = first_elem;
    out->next = NULL;
    return out;
}

void DeleteSpriteGroup(SpriteGroup *group){
    SpriteGroup *curr_elem = group;
    while (curr_elem != NULL){
        EraseSprite(curr_elem->sprite);
        curr_elem->sprite = NULL;
        SpriteGroup *prev = curr_elem;
        curr_elem = curr_elem->next;
        free(prev);
    }
}

// Basic code for map boundaries
SpriteGroup *CreateMapBoundaries(int floor_level, int screen_width, int screen_height){
    SpriteGroup *collisions = CreateSpriteGroup(0, screen_height-floor_level, screen_width, floor_level, RGB(0, 255, 0));
    CreateSpriteInGroup(collisions, -10, -10, 10, screen_height+20, RGB(0, 0, 255));
    CreateSpriteInGroup(collisions, screen_width, -10, 10, screen_height+20, RGB(0, 0, 255));
    CreateSpriteInGroup(collisions, 0, -10, screen_width, 10, RGB(0, 0, 255));
    return collisions;
}

void UpdateMapBoundaries(SpriteGroup *boundaries, int screen_width, int screen_height){ // Function will expect there to be at least 4 (four boundaries)
    SpriteGroup *curr_elem = boundaries; // floor
    curr_elem->sprite->size.cx = screen_width;
    curr_elem->sprite->pos.y = screen_height-curr_elem->sprite->size.cy;
    curr_elem = curr_elem->next; // Left wall
    curr_elem->sprite->size.cy = screen_height+20;
    curr_elem = curr_elem->next; // Right wall
    curr_elem->sprite->pos.x = screen_width;
    curr_elem->sprite->size.cy = screen_height+20;
    curr_elem = curr_elem->next; // Roof
    curr_elem->sprite->size.cx = screen_width;
}

void AppendToGroup(SpriteGroup *base_group, SpriteGroup *target){
    SpriteGroup *curr_elem = base_group;
    while (curr_elem->next != NULL) curr_elem = curr_elem->next;
    curr_elem->next = target;
}

int max_step_height = 45;

SpriteGroup *CreateStairCase(POINT start_point, POINT end_point){ // Because why not
    if (end_point.x < start_point.x){
        return CreateStairCase(end_point, start_point);
    }
    int step_height = max_step_height;
    int dx = end_point.x-start_point.x; //x2-x1
    int dy = end_point.y-start_point.y; //y2-y1
    while (dy%step_height != 0){
        step_height--;
    }
    step_height = (step_height<=0) ? 1 : step_height;
    int step_count = abs(dy/step_height); // good steps
    int m = dy>0?1:-1;
    int step_width = (int)((float)(dx)/(float)(step_count));
    SpriteGroup *out = CreateSpriteGroup(start_point.x, start_point.y+step_height*m, step_width, m==-1 ? step_height : step_height*step_count, RGB(0, 0, 255)); // First step
    for (int i = 1; i < step_count; i++){
        CreateSpriteInGroup(out, start_point.x+step_width*i, start_point.y+step_height*(i+1)*m, step_width, m==-1 ? step_height*(i+1) : step_height*(step_count-i), RGB(0, 0, 255));
    }
    return out;
}

SpriteGroup *CreateStairsWithCoords(int x1, int y1, int x2, int y2){
    POINT p1, p2;
    p1.x = x1;
    p2.x = x2;
    p1.y = y1;
    p2.y = y2;
    return CreateStairCase(p1, p2);
}

// ==================== Sprite Group Logic End =====================

void InitPlayer(){
    if (player == NULL){
        player = (Sprite *)malloc(sizeof(Sprite));
        CreateImgSprite(player, 0, 0, 64, 64, "./assets/sub.png");
    }
}

Sprite *GetPlayerPtr(){
    InitPlayer();
    return player;
}

POINT GetPlayerPos(){
    return player->pos;
}

SIZE GetPlayerSize(){
    return player->size;
}

void EndPlayer(){
    EraseSprite(player);
}

void SetPlayerPos(POINT new_pos){
    player->pos = new_pos;
}

void PaintSprite(HDC hdc, Sprite* sprite){
    RECT spriteInfo;
    spriteInfo.left = sprite->pos.x;
    spriteInfo.right = sprite->pos.x + sprite->size.cx;
    spriteInfo.top = sprite->pos.y;
    spriteInfo.bottom = sprite->pos.y + sprite->size.cy;
    if (sprite->brush->brush) {
        FillRect(hdc, &spriteInfo, sprite->brush->brush);
    }else{
        HDC hdcMem = CreateCompatibleDC(hdc);
        if (hdcMem){
            HBITMAP hbmOld = SelectObject(hdcMem, sprite->brush->bmp);
            BITMAP bm;
            GetObject(sprite->brush->bmp, sizeof(bm), &bm);
            BLENDFUNCTION bf = {0};
            bf.BlendOp = AC_SRC_OVER;
            bf.BlendFlags = 0;
            bf.SourceConstantAlpha = 255; // 0-255: use 255 for full alpha support from the bitmap
            bf.AlphaFormat = AC_SRC_ALPHA; // Important: uses per-pixel alpha!
            AlphaBlend(hdc, sprite->pos.x, sprite->pos.y, sprite->size.cx, sprite->size.cy, hdcMem, 0, 0, sprite->size.cx, sprite->size.cy, bf);
            // BitBlt(hdc, sprite->pos.x, sprite->pos.y, sprite->size.cx, sprite->size.cy, hdcMem, 0, 0, SRCCOPY);
            SelectObject(hdcMem, hbmOld);
            DeleteDC(hdcMem);
        }
    }
}

void PaintSpriteGroup(HDC hdc, SpriteGroup* group){
    SpriteGroup *curr_elem = group;
    while (curr_elem != NULL){
        PaintSprite(hdc, curr_elem->sprite);
        curr_elem = curr_elem->next;
    }
}