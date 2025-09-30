#include <windows.h>
#include "drawing.h"
#include "generalvars.h"

#ifndef NPC_H
#define NPC_H

typedef struct NPC {
    Sprite* npcSprite;
    int friendly, targetX, talking, talked, didDamage;
    float forces[2];
    float still, stillTimer;
    float hitTime, hitTimeMax;
    conversation *conv;
} NPC;

// NPC Storage
typedef struct NPCGroup{
    NPC *npc;
    struct NPCGroup *next;
} NPCGroup;

// CONVERSATION LOAD
void loadConvoToLastSprite(char *file_name);
void handle_key_down_conv(UINT key);
void conversationsNext();

// Conversation end
int skip_conversation();

void loadEnemyNPC(char *path, int x, int y, int cx, int cy, int upscale, char *name);
void loadFriendlyNPC(char *path, int x, int y, int cx, int cy, int upscale, char *name);

void drawAllNPCs(HDC hdc);
void drawConversationIfNeeded(HDC hdc);

void clearSprites();

void updateNPCs(SpriteGroup *collisions);

// for grenades
int distToPlayer(Sprite *sprite);

// DAMAGE SECTION
void doShortAttack(Sprite *HurtBox, int damage);
void doExplosiveDamage(POINT center, int radius, int damage);

int changeHarmLevel(int indexFrom1, int value);

#endif