#include "drawing.h"

#ifndef movement
#define movement

// Key public functions
void HandleKeyDown(UINT key);
void HandleKeyUp(UINT key);

// Update Function (must run on every update)
void UpdatePosition(float dt, SpriteGroup* collisions);

// Update on resize
void UpdatePositionOnResize(int screen_width, int screen_height);

#endif