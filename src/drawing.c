/*
DRAWING.C
Handles brushes/images/spritesheets
Handles Sprites and Spritegroups
Has the handle to the main player
*/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wingdi.h>

#include "headers/images.h"
#include "headers/animations.h"

// Basic structures and typedefs

typedef struct BrushPalette {
    unsigned int type; // 0 Color 1 Animation
    HBRUSH brush;
    COLORREF color;
    char *name;
    AnimationGroup *anim_group;
    struct BrushPalette *next;
} BrushPalette;

typedef struct Sprite {
    POINT pos;
    SIZE size;
    BrushPalette* brush;
    int health, damage, maxHealth;
} Sprite;

typedef struct SpriteGroup {
    Sprite* sprite;
    struct SpriteGroup* next;
} SpriteGroup;

// ======== BRUSH MEMORY SECTION =============
BrushPalette *brushes = NULL;

BrushPalette* SearchBrushesColor(COLORREF color){
    if (brushes == NULL)return NULL;
    BrushPalette *curr = brushes;
    int found = FALSE;
    int sr = GetRValue(color);//Self Red
    int sg = GetGValue(color);//Self Green
    int sb = GetBValue(color);//Self Blue
    while (curr != NULL && !found){
        if (curr->type == 0){
            int r = GetRValue(curr->color);
            int g = GetGValue(curr->color);
            int b = GetBValue(curr->color);
            if (r == sr && g == sg && b == sb) {
                found = TRUE;
            }
        }
        if (!found) curr = curr->next;
    }
    return curr;
}

BrushPalette *CreateNewColorBrush(COLORREF color){
    if (SearchBrushesColor(color) != NULL){
        return SearchBrushesColor(color);
    }
    HBRUSH brush = CreateSolidBrush(color);
    BrushPalette *holder = (BrushPalette *)malloc(sizeof(BrushPalette));
    if (holder == NULL){
        printf("Unable to create Brush Palette!\n");
        return NULL;
    }
    holder->brush = brush;
    holder->type = 0;
    holder->color = color;
    holder->anim_group = NULL;
    holder->name = "";
    holder->next = NULL;
    if (brushes == NULL){
        brushes = holder;
    }else{
        BrushPalette *curr = brushes;
        while (curr->next != NULL)curr = curr->next;
        curr->next = holder;
    }
    return holder;
}

BrushPalette* SearchBrushesName(char *str){
    if (brushes == NULL)return NULL;
    BrushPalette *curr = brushes;
    int found = FALSE;
    while (curr != NULL && !found){
        if (strcmp(curr->name, str)==0 && curr->type == 0) {
            found = TRUE;
        }else{
            curr = curr->next;
        }
    }
    return curr;
}

BrushPalette *LoadAnimationAsBrush(char *name, char *animation_name, size_t width, size_t height, int fps, int upscale){
    if (SearchBrushesName(name)){
        return SearchBrushesName(name);
    }
    BrushPalette *out = (BrushPalette *)malloc(sizeof(BrushPalette));
    if (out == NULL){
        printf("Unable to create Brush Palette from animation!\n");
        return NULL;
    }
    AnimationGroup *anim_group = LoadPNGIntoSprite(name, animation_name, width, height, fps, upscale);
    if (anim_group == NULL){
        printf("Unable to create animation group!\n");
        free(out);
        return NULL;
    }
    out->anim_group = anim_group;
    out->color = (COLORREF)NULL;
    out->name = name;
    out->type = 1;
    out->next = NULL;
    if (brushes == NULL){
        brushes = out;
    }else{
        BrushPalette *curr = brushes;
        while (curr->next != NULL)curr = curr->next;
        curr->next = out;
    }
    return out;
}

void LoadBrushes(){
    CreateNewColorBrush(RGB(255, 0, 0));
    CreateNewColorBrush(RGB(0, 255, 0));
    CreateNewColorBrush(RGB(0, 0, 255));
    CreateNewColorBrush(RGB(0, 0, 0));
}

void deleteBrushes(){
    BrushPalette *curr = brushes;
    while (curr != NULL){
        if (curr->brush == 0) DeleteObject(curr->brush);
        if (curr->anim_group) DeleteAnimationGroup(curr->anim_group);
        BrushPalette *prev = curr;
        curr = curr->next;
        free(prev);
    }
}

// ======== BRUSH MEMORY SECTION END =============

// ======== Sprites ===================
void EraseSprite(Sprite *sprite){ // Deletes sprites
    free(sprite);
}

void CreateSprite(Sprite* sprite, int x, int y, int cx, int cy, COLORREF color){ // Creates simple color sprite
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
    sprite->health = 5;
    sprite->maxHealth = 5;
    sprite->damage = 1;
}

void CreateImgSprite(Sprite* sprite, int x, int y, int cx, int cy, char *name, int upscale){ // Create strictly single image sprite
    sprite->brush = LoadAnimationAsBrush(name, "basic", (size_t)cx, (size_t)cy, 0, upscale);
    POINT pos;
    pos.x = x;
    pos.y = y;
    SIZE size;
    size.cx = cx;
    size.cy = cy;
    sprite->pos = pos;
    sprite->size = size;
    sprite->health = 5;
    sprite->maxHealth = 5;
    sprite->damage = 1;
}

void CreateAnimatedSprite(Sprite* sprite, int x, int y, int cx, int cy, char *name, char *animation_name, int fps, int upscale){ // Creates animating sprite from spritesheet
    sprite->brush = LoadAnimationAsBrush(name, animation_name, (size_t)cx, (size_t)cy, fps, upscale);
    POINT pos;
    pos.x = x;
    pos.y = y;
    SIZE size;
    size.cx = cx;
    size.cy = cy;
    sprite->pos = pos;
    sprite->size = size;
    sprite->health = 5;
    sprite->maxHealth = 5;
    sprite->damage = 1;
}

void UpdateAnimatedSprites(SpriteGroup *group, float dt){
    SpriteGroup *curr = group;
    while (curr != NULL){
        if (curr->sprite->brush->type == 1){
            UpdateAnimatedSprite(curr->sprite->brush->anim_group, dt);
        }
        curr = curr->next;
    }
}

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

void AppendToGroup(SpriteGroup *base_group, SpriteGroup *target){
    SpriteGroup *curr_elem = base_group;
    while (curr_elem->next != NULL) curr_elem = curr_elem->next;
    curr_elem->next = target;
}

// ==================== Sprite Group Logic End =====================

// MAP BOUNDARIES
// Basic code for map boundaries
SpriteGroup *CreateMapBoundaries(int floor_level, int screen_width, int screen_height){
    SpriteGroup *collisions = CreateSpriteGroup(0, screen_height-floor_level, screen_width, floor_level+100, RGB(0, 255, 0));
    CreateSpriteInGroup(collisions, -100, -20, 100, screen_height+40, RGB(0, 0, 255));
    CreateSpriteInGroup(collisions, screen_width, -20, 100, screen_height+40, RGB(0, 0, 255));
    CreateSpriteInGroup(collisions, -20, -100, screen_width+40, 100, RGB(0, 0, 255));
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
// MAP BOUNDARIES END

// $$$$$$$$$$$$$$$$$$$ STAIRS $$$$$$$$$$$$$$$$$$$$$$
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

// $$$$$$$$$$$$$$$$$$$ STAIRS NO MORE $$$$$$$$$$$$$$$$$$$$$$

// =================== MAIN SPRITE CODE ====================
// Creating the main sprite
Sprite *player = NULL;

void InitPlayer(){
    if (player == NULL){
        player = (Sprite *)malloc(sizeof(Sprite));
        CreateAnimatedSprite(player, 0, 0, 48*2, 64*2, "./assets/player/walking_right.png", "walking_right", 10, 8);
        AddToAnimationGroup(player->brush->anim_group, "./assets/player/walking_left.png", "walking_left", 48*2, 64*2, 10, 8);
        AddToAnimationGroup(player->brush->anim_group, "./assets/player/attacking_right.png", "attacking_right", 48*2, 64*2, 10, 8);
        AddToAnimationGroup(player->brush->anim_group, "./assets/player/attacking_left.png", "attacking_left", 48*2, 64*2, 10, 8);
        AddToAnimationGroup(player->brush->anim_group, "./assets/player/still_right.png", "still_right", 48*2, 64*2, 0, 8);
        AddToAnimationGroup(player->brush->anim_group, "./assets/player/still_left.png", "still_left", 48*2, 64*2, 0, 8);
        AddToAnimationGroup(player->brush->anim_group, "./assets/player/falling_right.png", "falling_right", 48*2, 64*2, 0, 8);
        AddToAnimationGroup(player->brush->anim_group, "./assets/player/falling_left.png", "falling_left", 48*2, 64*2, 0, 8);
        player->maxHealth = 10;
        player->health = 10;
        player->damage = 2;
    }
}

Sprite *GetPlayerPtr(){
    InitPlayer();
    return player;
}

POINT GetPlayerPos(){
    InitPlayer();
    return player->pos;
}

SIZE GetPlayerSize(){
    InitPlayer();
    return player->size;
}

void EndPlayer(){
    if (player) EraseSprite(player);
}

void SetPlayerPos(POINT new_pos){
    if (player) player->pos = new_pos;
}

// =================== MAIN SPRITE CODE END ====================

// =================== GRAPHICS LOGIC =======================
void PaintSprite(HDC hdc, Sprite* sprite){
    RECT spriteInfo;
    spriteInfo.left = sprite->pos.x;
    spriteInfo.right = sprite->pos.x + sprite->size.cx;
    spriteInfo.top = sprite->pos.y;
    spriteInfo.bottom = sprite->pos.y + sprite->size.cy;
    if (sprite->brush->type == 0) {
        FillRect(hdc, &spriteInfo, sprite->brush->brush);
    }else{
        DrawAnimatedSprite(hdc, sprite->brush->anim_group, sprite->pos.x, sprite->pos.y);
    }
}

void PaintSpriteGroup(HDC hdc, SpriteGroup* group){
    SpriteGroup *curr_elem = group;
    while (curr_elem != NULL){
        PaintSprite(hdc, curr_elem->sprite);
        curr_elem = curr_elem->next;
    }
}
// =================== GRAPHICS LOGIC END =======================