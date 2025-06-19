#include <windows.h>

#ifndef npc
#define npc

void loadEnemyNPC(char *path, int x, int y, int cx, int cy, int upscale);
void loadFriendlyNPC(char *path, int x, int y, int cx, int cy, int upscale);

void drawAllNPCs(HDC hdc);

void clearSprites();

void updateNPCs(SpriteGroup *collisions);
#endif