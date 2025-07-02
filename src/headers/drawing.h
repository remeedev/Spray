#include <windows.h>
#include <wingdi.h>
#include "animations.h"

#ifndef moving
#define moving

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

// DIRECT BRUSH FUNCTIONS HIDDEN
void LoadBrushes();
void deleteBrushes();
BrushPalette *CreateNewColorBrush(COLORREF color); // In case draw comes in from another file and a brush is needed

// SPRITE FUNCTIONS

// SPRITE CREATION
void CreateSprite(Sprite* sprite, int x, int y, int cx, int cy, COLORREF color);
void CreateImgSprite(Sprite* sprite, int x, int y, int cx, int cy, char *name, int upscale);
void CreateAnimatedSprite(Sprite* sprite, int x, int y, int cx, int cy, char *name, char *animation_name, int fps, int upscale);

void EraseSprite(Sprite *sprite);

// SPRITE GROUPS
void AddSpriteToGroup(SpriteGroup *group, Sprite *sprite);
void CreateSpriteInGroup(SpriteGroup *group, int x, int y, int cx, int cy, COLORREF color);
SpriteGroup *CreateSpriteGroup(int x, int y, int cx, int cy, COLORREF color);
void DeleteSpriteGroup(SpriteGroup *group);
void AppendToGroup(SpriteGroup *base_group, SpriteGroup *target);

// ANIMATED SPRITES
void UpdateAnimatedSprites(SpriteGroup *group, float dt);

// MAP BOUNDARIES
SpriteGroup *CreateMapBoundaries(int floor_level, int screen_width, int screen_height);
void UpdateMapBoundaries(SpriteGroup *boundaries, int screen_width, int screen_height);

// STAIRS
SpriteGroup *CreateStairCase(POINT start_point, POINT end_point);
SpriteGroup *CreateStairsWithCoords(int x1, int y1, int x2, int y2);

// PLAYER FUNCTIONS
void InitPlayer();
Sprite *GetPlayerPtr();
POINT GetPlayerPos();
SIZE GetPlayerSize();
void EndPlayer();
void SetPlayerPos(POINT new_pos);

// PAINTING FUNCTIONS
void PaintSprite(HDC hdc, Sprite* sprite);
void PaintSpriteGroup(HDC hdc, SpriteGroup* group);
#endif