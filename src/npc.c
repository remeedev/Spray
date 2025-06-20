#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "headers/animations.h"
#include "headers/drawing.h"
#include "headers/movement.h"
#include "headers/generalvars.h"

float NPC_speed = 750.0;

float conv_top = 3.0/5.0;

// Conversation definition
typedef struct conversation {
    wchar_t *line;
    Sprite* image;
    size_t end;
    struct conversation *next;
} conversation;

typedef struct NPC {
    Sprite* npcSprite;
    int friendly;
    float forces[2];
    float still;
    float stillTimer;
    int talking;
    conversation *conv;
} NPC;

// NPC Storage
typedef struct NPCGroup{
    NPC *npc;
    struct NPCGroup *next;
} NPCGroup;

NPCGroup *loaded_NPCs = NULL;

Sprite *chat_bubble = NULL;

conversation *conv_playing = NULL;

char *addPath(char *path, char *file){
    char *out = (char *)malloc(strlen(path) + strlen(file) + 1);
    strcpy(out, path);
    strcat(out, file);
    return out;
}

int min_conv_dist = 200;
int conv_padd = 50;

void loadConvoToLastSprite(char *file_name){
    if (chat_bubble == NULL){
        chat_bubble = (Sprite *)malloc(sizeof(Sprite));
        CreateImgSprite(chat_bubble, 0, 0, 48*2, 32*2, "./assets/ui/chat_bubble.png", 2);
    }
    FILE *file = fopen(file_name, "r");
    if (!file) return;
    char buffer[512];
    int AddingCharacters = TRUE;
    conversation *out = NULL;
    SpriteGroup *characters = NULL;
    int addedPlayer = FALSE;
    while (fgets(buffer, 512, file)){
        if (AddingCharacters && buffer[0] == '-'){
            AddingCharacters = FALSE;
            continue;
        }
        if (AddingCharacters){
            Sprite *image = (Sprite *)malloc(sizeof(Sprite));
            char *img_name = (char *)malloc(strlen(buffer)-1);
            for (size_t i = 0; buffer[i] != '\n'; i++)img_name[i] = buffer[i];
            img_name[strlen(buffer)-1] = '\0';
            int height = WindowHeight*conv_top-64*4;
            int left = addedPlayer ? WindowWidth-48*4-10 : 10;
            addedPlayer = TRUE;
            CreateImgSprite(image, left, height, 48*4, 64*4, img_name, 4); // Conversation images will be 48x64
            free(img_name);
            SpriteGroup *dum = (SpriteGroup *)malloc(sizeof(SpriteGroup));
            dum->sprite = image;
            dum->next = NULL;
            if (characters == NULL){
                characters = dum;
            }else{
                SpriteGroup *curr = characters;
                while (curr->next != NULL) curr = curr->next;
                curr->next = dum;
            }
        }else{
            conversation *convo = (conversation *)malloc(sizeof(conversation));
            size_t num_length = 0;
            for (size_t i = 0; buffer[i] != ':'; i++)num_length++;
            char *num = (char *)malloc(num_length+1);
            for (size_t i = 0; buffer[i] != ':'; i++)num[i] = buffer[i];
            num[num_length] = '\0';
            num_length++;
            size_t text_length = 0;
            while (buffer[text_length + num_length] != '\n' && buffer[num_length+text_length] != '\0') text_length++;
            char *line = (char *)malloc(text_length+1);
            for (size_t i = 0; i < text_length; i++) line[i] = buffer[num_length+i];
            line[text_length] = '\0';
            int len = MultiByteToWideChar(CP_UTF8, 0, line, -1, NULL, 0);
            wchar_t *processed_line = (wchar_t *)malloc(sizeof(wchar_t) * len);
            MultiByteToWideChar(CP_UTF8, 0, line, -1, processed_line, len);
            int index = atoi(num);
            free(line);
            free(num);
            int currIndex = 1;
            SpriteGroup *currChar = characters;
            while (currIndex != index){
                currChar = currChar->next;
                currIndex+=1;
            }
            convo->line = processed_line;
            convo->end = strlen(line);
            convo->next = NULL;
            convo->image = currChar->sprite;
            if (out){
                conversation *curr = out;
                while (curr->next != NULL) curr = curr->next;
                curr->next = convo;
            }else{
                out = convo;
            }
        }
    }
    SpriteGroup *groupDel = characters;
    while (groupDel != NULL){
        SpriteGroup *ll = groupDel;
        groupDel=groupDel->next;
        free(ll);
    }
    NPCGroup *curr_npc = loaded_NPCs;
    while (curr_npc->next != NULL) curr_npc = curr_npc->next;
    curr_npc->npc->conv = out;
    fclose(file);
}

int distToPlayer(Sprite *sprite);

void conversationsNext(){
    if (conv_playing != NULL){
        conv_playing = conv_playing->next;
        if (conv_playing) return;
        NPCGroup *curr = loaded_NPCs;
        while (curr->npc->talking == FALSE)curr = curr->next;
        curr->npc->talking = FALSE;
        talking = FALSE;
        return;
    }
    NPCGroup *curr = loaded_NPCs;
    while (curr != NULL){
        if (curr->npc->conv && distToPlayer(curr->npc->npcSprite) < min_conv_dist){
            conv_playing = curr->npc->conv;
            curr->npc->talking = TRUE;
            talking = TRUE;
        }
        curr = curr->next;
    }
}

void unloadConversation(conversation *conv){
    conversation *curr = conv;
    while (curr) {
        free(curr->line);
        conversation *prev = curr;
        curr = curr->next;
        free(prev);
    }
}

void clearSprites(){
    if (chat_bubble) EraseSprite(chat_bubble);
    chat_bubble = NULL;
    if (loaded_NPCs == NULL) return;
    NPCGroup *curr = loaded_NPCs;
    while (curr != NULL){
        NPCGroup *prev = curr;
        curr = curr->next;
        EraseSprite(prev->npc->npcSprite);
        if (prev->npc->conv != NULL) unloadConversation(prev->npc->conv);
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
    newNPC->conv = NULL;
    newNPC->talking = FALSE;
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
    newNPC->forces[0] = 0;
    newNPC->forces[1] = 0;
    newNPC->still = 0.0f;
    newNPC->stillTimer = 0.0f;
    newNPC->friendly = TRUE;
    newNPC->npcSprite = sprite;
    newNPC->conv = NULL;
    newNPC->talking = FALSE;
    addToLoadedSprites(newNPC);
}

int distToPlayer(Sprite *sprite){
    int dy = GetPlayerPos().y - sprite->pos.y;
    int dx = GetPlayerPos().x - sprite->pos.x;
    return sqrt(pow(dy, 2.0) + pow(dx, 2.0));
}

void drawAllNPCs(HDC hdc){
    NPCGroup *curr = loaded_NPCs;
    while (curr != NULL){
        if (curr->npc->conv != NULL && distToPlayer(curr->npc->npcSprite) < 200){
            int dist = distToPlayer(curr->npc->npcSprite);
            chat_bubble->pos.x = curr->npc->npcSprite->pos.x;
            dist-=50;
            dist = dist < 0 ? 0 : dist;
            float offsetY = (float)dist/150.0f;
            offsetY = 1.0f - offsetY;
            offsetY *= 20;
            chat_bubble->pos.y = curr->npc->npcSprite->pos.y + 10 - chat_bubble->size.cy - offsetY;
            PaintSprite(hdc, chat_bubble);
        }
        DrawAnimatedSprite(hdc, curr->npc->npcSprite->brush->anim_group, curr->npc->npcSprite->pos.x, curr->npc->npcSprite->pos.y);
        curr = curr->next;
    }
    if (conv_playing != NULL){
        // Background Black Box
        RECT boundingBox;
        boundingBox.top = WindowHeight*conv_top;
        boundingBox.left = 0;
        boundingBox.bottom = WindowHeight;
        boundingBox.right = WindowWidth;
        FillRect(hdc, &boundingBox, CreateNewColorBrush(RGB(0, 0, 0))->brush);

        // Character to show
        PaintSprite(hdc, conv_playing->image);

        // Text to be shown
        int midHeight = boundingBox.bottom - ((float)(boundingBox.bottom-boundingBox.top)/2.0f);
        int midWidth = boundingBox.right - ((float)(boundingBox.right-boundingBox.left)/2.0f);
        SelectObject(hdc, GameFont);
        SetTextAlign(hdc, TA_CENTER);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        TextOutW(hdc, midWidth, midHeight, conv_playing->line, conv_playing->end);
        char *text = "Press \"Space\" to continue...";
        SetTextAlign(hdc, TA_BASELINE);
        TextOut(hdc, 0, WindowHeight-10, text, strlen(text));
    }
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
        // GRAVITY ON NPC
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

        if (curr->npc->talking){
            ChangeAnimationNoDir("still", curr->npc->npcSprite);
            if (selfObj->pos.x >= GetPlayerPos().x) ChangeAnimationDirection("left", curr->npc->npcSprite);
            if (selfObj->pos.x < GetPlayerPos().x) ChangeAnimationDirection("right", curr->npc->npcSprite);
            curr = curr->next;
            continue;
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