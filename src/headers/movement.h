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
POINT get_transform_due(SpriteGroup* collisions, Sprite *sprite, int *grounded);

char *getRawAnimND(Sprite *sprite);
char *getDirectionSprite(Sprite *sprite);

float gravity;
float friction;
int GroundForgive;
int talking;
float player_forces[2];
float jump_force;

#endif