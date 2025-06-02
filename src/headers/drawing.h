#include <windows.h>
#include <wingdi.h>

#ifndef moving
#define moving

typedef struct BrushPalette {
    HBRUSH brush;
    COLORREF color;
    char *name;
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

void CreateSprite(Sprite *sprite, int x, int y, int cx, int cy, COLORREF color);

Sprite *GetPlayerPtr();

POINT GetPlayerPos();

SIZE GetPlayerSize();

void EndPlayer();

void SetPlayerPos(POINT new_pos);

void PaintSprite(HDC hdc, Sprite *sprite);

void EraseSprite(Sprite sprite);

void AddSpriteToGroup(SpriteGroup *group, Sprite *sprite);

void CreateSpriteInGroup(SpriteGroup *group, int x, int y, int cx, int cy, COLORREF color);

SpriteGroup *CreateSpriteGroup(int x, int y, int cx, int cy, COLORREF color);

void DeleteSpriteGroup(SpriteGroup *group);

void PaintSpriteGroup(HDC hdc, SpriteGroup* group);

void InitPlayer();

SpriteGroup *CreateStairCase(POINT start_point, POINT end_point);

SpriteGroup *CreateStairsWithCoords(int x1, int y1, int x2, int y2);

void AppendToGroup(SpriteGroup *base_group, SpriteGroup *target);

SpriteGroup *CreateMapBoundaries(int floor_level, int screen_width, int screen_height);
void UpdateMapBoundaries(SpriteGroup *boundaries, int screen_width, int screen_height);

void deleteBrushes();

void LoadBrushes();

#endif