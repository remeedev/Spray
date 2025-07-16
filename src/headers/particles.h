#include <windows.h>

#ifndef particles
#define particles

void createParticles(POINT pos, SIZE size, float lifespan, float particleLifespan, int spawnRadius, int density, int gravityApplied, int moveAway, COLORREF color, int smudge);

void endParticles();

void updateParticles(float dt);

void drawParticles(HDC hdc);

#endif