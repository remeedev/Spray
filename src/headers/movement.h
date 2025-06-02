#include "drawing.h"

#ifndef movement
#define movement

void HandleKeyDown(UINT key);
void HandleKeyUp(UINT key);

void UpdatePosition(float dt, SpriteGroup* collisions);

void UpdatePositionOnResize(int screen_width, int screen_height);

#endif