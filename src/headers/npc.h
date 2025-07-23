#include <windows.h>

#ifndef npc
#define npc

// CONVERSATION LOAD
void loadConvoToLastSprite(char *file_name);
void conversationsNext();

// Conversation end
int skip_conversation();

void loadEnemyNPC(char *path, int x, int y, int cx, int cy, int upscale);
void loadFriendlyNPC(char *path, int x, int y, int cx, int cy, int upscale);

void drawAllNPCs(HDC hdc);

void clearSprites();

void updateNPCs(SpriteGroup *collisions);

// for grenades
int distToPlayer(Sprite *sprite);

// DAMAGE SECTION
void doShortAttack(Sprite *HurtBox, int damage);
void doExplosiveDamage(POINT center, int radius, int damage);

int changeHarmLevel(int indexFrom1, int value);

#endif