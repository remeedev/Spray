#include "drawing.h"
#include "generalvars.h"

#ifndef throwables
#define throwables

int grenade_count;

void startGrenade();

void drawGrenades(HDC hdc);

void throwGrenade();

void updateGrenades(float dt, SpriteGroup* collisions);

void endGrenades();

#endif