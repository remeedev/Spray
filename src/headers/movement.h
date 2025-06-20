#include "drawing.h"

#ifndef movement
#define movement

// Key public functions
void HandleKeyDown(UINT key);
void HandleKeyUp(UINT key);

// For level loader
int GetCollision(Sprite *sprite, int *out, Sprite *srcSprite);

// Update Function (must run on every update)
void UpdatePosition(float dt, SpriteGroup* collisions);

void ChangeAnimationDirection(char *direction, Sprite *sprite);
void ChangeAnimationNoDir(char *new_animation_name, Sprite *sprite);
POINT get_transform_due(SpriteGroup* collisions, Sprite *sprite, int player, int *grounded, float forces[2]);

float gravity;
float friction;
int GroundForgive;
int talking;

#endif