#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "headers/animations.h"
#include "headers/drawing.h"
#include "headers/movement.h"

float NPC_speed = 750.0;

typedef struct NPC {
    Sprite* npcSprite;
    int friendly;
    float forces[2];
    float still;
    float stillTimer;
} NPC;

// NPC Storage
typedef struct NPCGroup{
    NPC *npc;
    struct NPCGroup *next;
} NPCGroup;

NPCGroup *loaded_NPCs = NULL;

char *addPath(char *path, char *file){
    char *out = (char *)malloc(strlen(path) + strlen(file) + 1);
    strcpy(out, path);
    strcat(out, file);
    return out;
}

void clearSprites(){
    if (loaded_NPCs == NULL) return;
    NPCGroup *curr = loaded_NPCs;
    while (curr != NULL){
        NPCGroup *prev = curr;
        curr = curr->next;
        EraseSprite(prev->npc->npcSprite);
        free(prev->npc);
        free(prev);
    }
    loaded_NPCs = NULL;
}

void addToLoadedSprites(NPC *sprite){
    NPCGroup *new = (NPCGroup *)malloc(sizeof(NPCGroup));
    new->npc = sprite;
    new->next = NULL;
    if (loaded_NPCs == NULL){
        loaded_NPCs = new;
        srand(time(NULL));
    }else{
        NPCGroup *curr = loaded_NPCs;
        while (curr->next != NULL) curr = curr->next;
        curr->next = new;
    }
}

void loadEnemyNPC(char *path, int x, int y, int cx, int cy, int upscale){
    char *still_left = addPath(path, "/still_left.png");
    FILE *file = fopen(still_left, "rb");
    if (file == NULL){
        printf("Error npc path not loaded!\n");
        return;
    }
    fclose(file);
    Sprite *sprite = (Sprite *)malloc(sizeof(Sprite));

    CreateAnimatedSprite(sprite, x, y, cx, cy, still_left, "still_left", 10, upscale);
    free(still_left);
    char *still_right = addPath(path, "/still_right.png");
    AddToAnimationGroup(sprite->brush->anim_group, still_right, "still_right", cx, cy, 10, upscale);
    free(still_right);
    char *walking_left = addPath(path, "/walking_left.png");
    AddToAnimationGroup(sprite->brush->anim_group, walking_left, "walking_left", cx, cy, 10, upscale);
    free(walking_left);
    char *walking_right = addPath(path, "/walking_right.png");
    AddToAnimationGroup(sprite->brush->anim_group, walking_right, "walking_right", cx, cy, 10, upscale);
    free(walking_right);
    
    // UNIQUE FOR ENEMIES
    char *attack_right = addPath(path, "/attack_right.png");
    AddToAnimationGroup(sprite->brush->anim_group, attack_right, "attack_right", cx, cy, 10, upscale);
    free(attack_right);
    char *attack_left = addPath(path, "/attack_left.png");
    AddToAnimationGroup(sprite->brush->anim_group, attack_left, "attack_left", cx, cy, 10, upscale);
    free(attack_left);

    NPC *newNPC = (NPC *)malloc(sizeof(NPC));
    newNPC->forces[0] = 0;
    newNPC->forces[1] = 0;
    newNPC->still = 0.0f;
    newNPC->stillTimer = 0.0f;
    newNPC->friendly = FALSE;
    newNPC->npcSprite = sprite;
    addToLoadedSprites(newNPC);
}

void loadFriendlyNPC(char *path, int x, int y, int cx, int cy, int upscale){
    char *still_left = addPath(path, "/still_left.png");
    FILE *file = fopen(still_left, "rb");
    if (file == NULL){
        printf("Error npc path not loaded!\n");
        return;
    }
    fclose(file);
    Sprite *sprite = (Sprite *)malloc(sizeof(Sprite));

    // Friendly has different animations
    CreateAnimatedSprite(sprite, x, y, cx, cy, still_left, "still_left", 10, upscale);
    free(still_left);
    char *still_right = addPath(path, "/still_right.png");
    AddToAnimationGroup(sprite->brush->anim_group, still_right, "still_right", cx, cy, 10, upscale);
    free(still_right);
    char *walking_left = addPath(path, "/walking_left.png");
    AddToAnimationGroup(sprite->brush->anim_group, walking_left, "walking_left", cx, cy, 10, upscale);
    free(walking_left);
    char *walking_right = addPath(path, "/walking_right.png");
    AddToAnimationGroup(sprite->brush->anim_group, walking_right, "walking_right", cx, cy, 10, upscale);
    free(walking_right);

    NPC *newNPC = (NPC *)malloc(sizeof(NPC));
    newNPC->npcSprite = sprite;
    newNPC->forces[0] = 0;
    newNPC->forces[1] = 0;
    newNPC->friendly = TRUE;
    addToLoadedSprites(newNPC);
}

void drawAllNPCs(HDC hdc){
    NPCGroup *curr = loaded_NPCs;
    while (curr != NULL){
        DrawAnimatedSprite(hdc, curr->npc->npcSprite->brush->anim_group, curr->npc->npcSprite->pos.x, curr->npc->npcSprite->pos.y);
        curr = curr->next;
    }
}

int distToPlayer(Sprite *sprite){
    int dy = GetPlayerPos().y - sprite->pos.y;
    int dx = GetPlayerPos().x - sprite->pos.x;
    return sqrt(pow(dy, 2.0) + pow(dx, 2.0));
}

void updateNPCs(SpriteGroup *collisions, float dt){
    dt = (dt > 0.05f) ? 0.05f : dt;
    NPCGroup *curr = loaded_NPCs;
    while (curr != NULL){
        UpdateAnimatedSprite(curr->npc->npcSprite->brush->anim_group, dt);
        Sprite *selfObj = curr->npc->npcSprite;
        SpriteGroup *curr_coll = collisions;
        int grounded = FALSE;
        int prevX = selfObj->pos.x;
        selfObj->pos = get_transform_due(collisions, selfObj, FALSE, &grounded, curr->npc->forces);

        if (!grounded){
            curr->npc->forces[1] += gravity*dt;
        }else{
            curr->npc->forces[1] = 0;
        }

        if (selfObj->pos.x != prevX){
            // curr->npc->forces[0] = 0;
            curr->npc->forces[1] = -600.0;
        }
        

        if (curr->npc->forces[1] != 0){
            selfObj->pos.y += curr->npc->forces[1]*dt;
        }
        
        // GOTO PLAYER
        if (selfObj->pos.y >= GetPlayerPos().y && selfObj->pos.y <= GetPlayerPos().y + GetPlayerSize().cy && curr->npc->friendly == FALSE){
            curr->npc->forces[0] = ((GetPlayerPos().x - selfObj->pos.x) > 0) ? NPC_speed : -NPC_speed;
        }

        if (distToPlayer(selfObj) < 50 && curr->npc->friendly == FALSE){
            ChangeAnimationNoDir("attack", selfObj);
        }else{
            // RANDOMIZED MOVEMENT
            if (curr->npc->forces[0] == 0 && grounded){
                curr->npc->still += dt;
                if (curr->npc->still > curr->npc->stillTimer){
                    curr->npc->stillTimer = (float)(rand()%100)/10.0;
                    curr->npc->forces[0] = rand()%750;
                    curr->npc->forces[0] = rand()%2 ? -curr->npc->forces[0]-500 : curr->npc->forces[0]+500;
                }
            }else{
                selfObj->pos.x += curr->npc->forces[0]*dt;
                int neg = curr->npc->forces[0] > 0 ? 1 : -1;
                curr->npc->forces[0] -=friction*neg*dt;
                if ((curr->npc->forces[0] > 0 && neg < 0) || (curr->npc->forces[0] < 0 && neg > 0)) curr->npc->forces[0] = 0;
            }

            // Animation playing
            if (curr->npc->forces[0] != 0){
                ChangeAnimationNoDir("walking", selfObj);
                if (curr->npc->forces[0] > 0)ChangeAnimationDirection("right", selfObj);
                if (curr->npc->forces[0] < 0)ChangeAnimationDirection("left", selfObj);
            }else{
                ChangeAnimationNoDir("still", selfObj);
            }
        }
        curr = curr->next;
    }
}