#include "drawing.h"

#include <windows.h>

#ifndef throwables
#define throwables

int grenade_count;

void startGrenade();

void drawGrenades(HDC hdc);

void throwGrenade();

void updateGrenades(float dt, SpriteGroup* collisions);

#endif