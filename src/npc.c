#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "headers/animations.h"
#include "headers/drawing.h"
#include "headers/movement.h"
#include "headers/generalvars.h"
#include "headers/level_loader.h"
#include "headers/the_chronic.h"
#include "headers/characters.h"

float NPC_speed = 750.0;

float conv_top = 3.0/5.0;

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

NPCGroup *loaded_NPCs = NULL;

Sprite *chat_bubble = NULL;

conversation *conv_playing = NULL;
SpriteGroup *dead_npcs = NULL;

char *addPath(char *path, char *file){
    char *out = (char *)malloc(strlen(path) + strlen(file) + 1);
    strcpy(out, path);
    strcat(out, file);
    return out;
}

int min_conv_dist = 200;
int conv_padd = 50;

typedef void (*conversation_creator)(conversation *, SpriteGroup *);

char *option_assignment[] = {"weed_sale", NULL};
conversation_creator option_funcs[] = {&create_weed_sale};

void loadConvoToLastSprite(char *file_name){
    if (chat_bubble == NULL){
        chat_bubble = (Sprite *)malloc(sizeof(Sprite));
        CreateImgSprite(chat_bubble, 0, 0, 88, 56, "./assets/ui/chat_bubble.png", 8);
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
            if (buffer[0] == '$' && buffer[1] == '('){
                char *saved = NULL;
                size_t var_size = 0;
                int curr_pos = 2;
                while (buffer[curr_pos] != ')'){
                    curr_pos++;
                    var_size++;
                }
                saved = (char *)malloc(var_size + 1);
                if (saved == NULL){
                    printf("There was an error allocating for var name!\n");
                    return;
                }
                curr_pos = 2;
                int save_pos = 0;
                while (buffer[curr_pos] != ')'){
                    saved[save_pos] = buffer[curr_pos];
                    save_pos++;
                    curr_pos++;
                }
                saved[var_size] = '\0';
                int option_loading = 0;
                while (option_assignment[option_loading] != NULL){
                    if (strcmp(option_assignment[option_loading], saved)==0){
                        conversation_creator creator_func = option_funcs[option_loading];
                        creator_func(out, characters);
                    }
                    option_loading++;
                }
                free(saved);
                continue;
            }
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
            convo->option_count = 0;
            convo->option_handler = NULL;
            convo->option_selected = 0;
            convo->options = NULL;
            convo->skip_check = NULL;
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
    curr_npc->npc->talked = FALSE;
    fclose(file);
}

int distToPlayer(Sprite *sprite);

void conversationsNext(){
    if (conv_playing != NULL){
        conv_playing = conv_playing->next;
        if (conv_playing) {
            if (conv_playing->skip_check != NULL){
                if (conv_playing->skip_check()){
                    conv_playing = conv_playing->next;
                }
            }
            return;
        }
        NPCGroup *curr = loaded_NPCs;
        while (curr->npc->talking == FALSE) curr = curr->next;
        curr->npc->talking = FALSE;
        curr->npc->talked = TRUE;
        talking = FALSE;
        return;
    }else{
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
}

void handle_key_down_conv(UINT key){
    if (key == VK_SPACE && conv_playing->options == NULL){
        conversationsNext();
        return;
    }
    if (conv_playing->options){
        if (key == VK_SPACE || key == VK_RETURN){
            conv_playing = conv_playing->option_handler(conv_playing);
        }
        if (key == VK_UP){
            if (conv_playing->option_selected > 0){
                conv_playing->option_selected--;
            }
        }
        if (key == VK_DOWN){
            if (conv_playing->option_selected < conv_playing->option_count - 1){
                conv_playing->option_selected++;
            }
        }
    }
}

// Tries to skip on esc played
int skip_conversation(){
    int steps = 0;
    if (conv_playing == NULL) return 0;
    while (conv_playing != NULL){
        conversationsNext();
        steps++;
    }
    return steps;
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
    if (loaded_NPCs == NULL && dead_npcs == NULL) return;
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

    // LITERAL DEAD BODY CLEANUP
    SpriteGroup *tmp = dead_npcs;
    while (tmp != NULL){
        SpriteGroup *copy = tmp;
        tmp = tmp->next;
        EraseSprite(copy->sprite);
        free(copy);
        copy = NULL;
    }
    dead_npcs = NULL;
}

void addToLoadedSprites(NPC *sprite){
    NPCGroup *new = (NPCGroup *)malloc(sizeof(NPCGroup));
    if (new == NULL){
        printf("Error allocating space for npc!\n");
        return;
    }
    new->npc = sprite;
    new->next = NULL;
    if (loaded_NPCs == NULL){
        loaded_NPCs = new;
    }else{
        NPCGroup *curr = loaded_NPCs;
        while (curr->next != NULL) curr = curr->next;
        curr->next = new;
    }
}

void loadEnemyNPC(char *path, int x, int y, int cx, int cy, int upscale, char *name){
    char *still_left = addPath(path, "/still_left.png");
    FILE *file = fopen(still_left, "rb");
    if (file == NULL){
        printf("Error npc path not loaded!\n");
        return;
    }
    fclose(file);
    Sprite *sprite = (Sprite *)malloc(sizeof(Sprite));

    if (sprite == NULL){
        printf("Error allocating space for enemy enemy!\n");
        return;
    }

    CreateAnimatedSprite(sprite, x, y, cx, cy, still_left, "still_left", 10, upscale, name);
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

    if (newNPC == NULL){
        printf("Error allocating space for the npc!\n");
        return;
    }

    newNPC->forces[0] = 0;
    newNPC->forces[1] = 0;
    newNPC->still = 0.0f;
    newNPC->stillTimer = 0.0f;
    newNPC->friendly = FALSE;
    newNPC->npcSprite = sprite;
    newNPC->conv = NULL;
    newNPC->talking = FALSE;
    newNPC->targetX = sprite->pos.x;
    newNPC->talked = TRUE;
    newNPC->hitTime = 1.5f;
    newNPC->hitTimeMax = 1.5f;
    newNPC->didDamage = FALSE;
    addToLoadedSprites(newNPC);
}

void loadFriendlyNPC(char *path, int x, int y, int cx, int cy, int upscale, char *name){
    char *still_left = addPath(path, "/still_left.png");
    FILE *file = fopen(still_left, "rb");
    if (file == NULL){
        printf("Error npc path not loaded!\n");
        return;
    }
    fclose(file);
    Sprite *sprite = (Sprite *)malloc(sizeof(Sprite));

    // Friendly has different animations
    CreateAnimatedSprite(sprite, x, y, cx, cy, still_left, "still_left", 10, upscale, name);
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
    newNPC->targetX = sprite->pos.x;
    newNPC->talked = TRUE;
    newNPC->hitTime = 1.5f;
    newNPC->hitTimeMax = 1.5f;
    newNPC->didDamage = FALSE;
    addToLoadedSprites(newNPC);
}

int distToPlayer(Sprite *sprite){
    int dy = GetPlayerPos().y - sprite->pos.y;
    int dx = GetPlayerPos().x - sprite->pos.x;
    return sqrt(pow(dy, 2.0) + pow(dx, 2.0));
}

void drawTextHUD(HDC hdc, int x, int y, char **texts){
    int pos = 0;
    int currY = y;
    int currX = x;
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    while (texts[pos] != NULL){
        currY -= tm.tmHeight;
        int drawnY = currY;
        currY += tm.tmAscent - tm.tmHeight;
        TextOut(hdc, currX, drawnY, texts[pos], strlen(texts[pos]));
        pos++;
    }

}

NPCGroup *killNPC(NPCGroup *prev, NPCGroup *curr){
    if (curr == NULL){
        curr = prev->next;
    }
    if (curr->npc->npcSprite->name)lockSpriteDeath(curr->npc->npcSprite);
    // Create the dead body
    Sprite *dead_npc = (Sprite *)malloc(sizeof(Sprite));
    POINT pos = curr->npc->npcSprite->pos;
    SIZE size = curr->npc->npcSprite->size;
    CreateAnimatedSprite(dead_npc, pos.x, pos.y, size.cx, size.cy, "./assets/dead_spray.png", "basic", 24, 8, NULL);
    SpriteGroup *npcWrapper = (SpriteGroup *)malloc(sizeof(SpriteGroup));
    npcWrapper->sprite = dead_npc;
    npcWrapper->next = NULL;
    if (dead_npcs == NULL){
        dead_npcs = npcWrapper;
    }else{
        SpriteGroup *tmp_sg = dead_npcs;
        while (tmp_sg->next != NULL)tmp_sg = tmp_sg->next;
        tmp_sg->next = npcWrapper;
    }

    NPCGroup *tmp = curr->next;
    EraseSprite(curr->npc->npcSprite);
    if (curr->npc->conv != NULL) unloadConversation(curr->npc->conv);
    free(curr->npc);
    free(curr);
    curr = tmp;
    if (prev == NULL) {
        loaded_NPCs = curr;
    }else{
        prev->next = tmp;
    }
    return curr;
}

void doShortAttack(Sprite *HurtBox, int damage){
    NPCGroup *curr = loaded_NPCs;
    NPCGroup *prev = NULL;
    while(curr != NULL){
        Sprite *sprite = curr->npc->npcSprite;
        if (curr->npc->friendly == TRUE) {
            curr = curr->next;
            continue;
        }
        if (GetCollision(sprite, NULL, HurtBox) > 0){
            curr->npc->npcSprite->health -= damage;
            if (curr->npc->npcSprite->health <= 0){
                curr = killNPC(prev, curr);
                continue;
            }
            curr->npc->forces[1] = -(jump_force - 200);
            char *direction = getDirectionSprite(GetPlayerPtr());
            curr->npc->forces[0] = strcmp(direction, "left") == 0 ? curr->npc->forces[1] : -curr->npc->forces[1];
            free(direction);
            curr->npc->hitTime = 0;
            if (curr->npc->friendly == FALSE && curr->npc->talked == FALSE){
                curr->npc->talked = TRUE;
            }
        }
        prev = curr;
        curr = curr->next;
    }
}

void doExplosiveDamage(POINT center, int radius, int damage){
    // Do damage to NPCs
    NPCGroup *curr = loaded_NPCs;
    NPCGroup *prev = NULL;
    while(curr != NULL){
        Sprite *sprite = curr->npc->npcSprite;
        if (curr->npc->friendly == TRUE) {
            curr = curr->next;
            continue;
        }
        float distanceToPoint = sqrt(pow(center.x-curr->npc->npcSprite->pos.x, 2.0) + pow(center.y-curr->npc->npcSprite->pos.y, 2.0));
        if (distanceToPoint <= radius){
            // damage calculated on a sin curve
            double distance_as_percentage = distanceToPoint/radius;
            int total_damage = (int) round(cos(distance_as_percentage * (3.14159265/2.0)) * damage);
            curr->npc->npcSprite->health -= total_damage;
            curr->npc->hitTime = 0;
            if (curr->npc->npcSprite->health <= 0){
                curr = killNPC(prev, curr);
                continue;
            }
        }
        prev = curr;
        curr = curr->next;
    }
}

// VARIABLES FOR SHOWING HEALTH BAR
int health_bar_width = 100;
int health_bar_height = 15;
int health_bar_padding = 1;

void drawAllNPCs(HDC hdc){
    // DEAD BODIES GO FIRST
    if (dead_npcs) PaintSpriteGroup(hdc, dead_npcs);

    // NOW THE ALIVE PEOPLE
    NPCGroup *curr = loaded_NPCs;
    int count = 0;
    while (curr != NULL){
        if (curr->npc->hitTime < curr->npc->hitTimeMax && !showDebug){
            RECT healthArea;
            healthArea.top = curr->npc->npcSprite->pos.y + curr->npc->npcSprite->size.cy;
            healthArea.left = curr->npc->npcSprite->pos.x + (curr->npc->npcSprite->size.cx - health_bar_width) / 2;
            healthArea.right = healthArea.left + health_bar_width;
            healthArea.bottom = healthArea.top + health_bar_height;

            RECT healthBar;
            healthBar.top = healthArea.top + health_bar_padding;
            healthBar.left = healthArea.left + health_bar_padding;
            healthBar.right = healthArea.right - health_bar_padding;
            healthBar.bottom = healthArea.bottom - health_bar_padding;

            RECT healthValue = healthBar;
            int total_width = healthBar.right - healthBar.left;
            float health = ((float)curr->npc->npcSprite->health)/((float)curr->npc->npcSprite->maxHealth);
            int perc_width = (int)(health*((float)total_width));
            healthValue.right = healthValue.left+perc_width;

            FillRect(hdc, &healthArea, CreateNewColorBrush(RGB(101, 67, 33))->brush);
            FillRect(hdc, &healthBar, CreateNewColorBrush(RGB(150, 100, 50))->brush);
            FillRect(hdc, &healthValue, CreateNewColorBrush(RGB(0, 150, 0))->brush);
        }
        int conversative = curr->npc->conv != NULL && distToPlayer(curr->npc->npcSprite) < 200;
        if (conversative && !showDebug && curr->npc->talked == FALSE){
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
        if (showCollisions){
            RECT boundingBox;
            boundingBox.left = curr->npc->npcSprite->pos.x;
            boundingBox.top = curr->npc->npcSprite->pos.y;
            boundingBox.right = boundingBox.left + curr->npc->npcSprite->size.cx;
            boundingBox.bottom = boundingBox.top + curr->npc->npcSprite->size.cy;
            FillRect(hdc, &boundingBox, CreateNewColorBrush(characterColor)->brush);
        }
        if (showDebug){
            int currY = curr->npc->npcSprite->pos.y;
            int midX = curr->npc->npcSprite->pos.x + (curr->npc->npcSprite->size.cx / 2);
            SelectObject(hdc, SmallBig);
            SetBkMode(hdc, TRANSPARENT);
            SetTextAlign(hdc, TA_BASELINE | TA_CENTER);
            
            // GET POS
            char posTxt[100];
            sprintf(posTxt, "(%d, %d)", curr->npc->npcSprite->pos.x, curr->npc->npcSprite->pos.y);

            // GET FORCES
            char forcesTxt[100];
            sprintf(forcesTxt, "%10f  %10f", curr->npc->forces[0], curr->npc->forces[1]);

            // Still Timer
            char timerTxt[100];
            sprintf(timerTxt, "Timer State: %5f/%5f", curr->npc->still, curr->npc->stillTimer);
            
            // targetX
            char targetTxt[100];
            sprintf(targetTxt, "Target: %d", curr->npc->targetX);

            // GET TALK STATE
            char *talk_state = conversative ? "Ready to talk" : "Conversation available";

            char *lvl = curr->npc->friendly ? "Friendly" : curr->npc->talked ? "Aggressive" : "Aggressive (Passive State)";
            char idTxt[100];
            count++;
            sprintf(idTxt, "ID %d", count);

            char healthTxt[100];
            sprintf(healthTxt, "%d/%d", curr->npc->npcSprite->health, curr->npc->npcSprite->maxHealth);

            char *displayed_info[] = {posTxt, forcesTxt, timerTxt, targetTxt, talk_state, lvl, healthTxt, idTxt, NULL};
            drawTextHUD(hdc, midX, currY, displayed_info);
        }
        DrawAnimatedSprite(hdc, curr->npc->npcSprite->brush->anim_group, curr->npc->npcSprite->pos.x, curr->npc->npcSprite->pos.y);
        curr = curr->next;
    }
    
}

void drawConversationIfNeeded(HDC hdc){
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
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        if (conv_playing->options == NULL){
            SetTextAlign(hdc, TA_CENTER);
            TextOutW(hdc, midWidth, midHeight, conv_playing->line, conv_playing->end);
            char *text = "Press \"Space\" to continue...";
            SetTextAlign(hdc, TA_BASELINE);
            SelectObject(hdc, SmallFont);
            TextOut(hdc, 0, WindowHeight-10, text, strlen(text));
        }else{
            SetTextAlign(hdc, TA_TOP | TA_LEFT);
            int left_align = boundingBox.left + 5;
            int top_line = boundingBox.top + 5;
            TextOutW(hdc, left_align, top_line, conv_playing->line, conv_playing->end);

            TEXTMETRICW tm;
            GetTextMetricsW(hdc, &tm);
            left_align += 15;
            for (int i = 0; i < conv_playing->option_count; i++){
                top_line += tm.tmHeight + tm.tmAscent;
                if (conv_playing->option_selected == i){
                    SetTextColor(hdc, highlight_text_color);
                }else{
                    SetTextColor(hdc, regular_text_color);
                }
                TextOut(hdc, left_align, top_line, ">", 1);
                TextOut(hdc, left_align + 2*tm.tmAveCharWidth, top_line, conv_playing->options[i], strlen(conv_playing->options[i]));
            }


        }
    }
}

int changeHarmLevel(int indexFrom1, int value){
    int pos = 1;
    NPCGroup *curr = loaded_NPCs;
    int found = FALSE;
    while (curr != NULL && !found){
        if (pos == indexFrom1){
            found = TRUE;
            curr->npc->friendly = value;
        }
        pos++;
        curr = curr->next;
    }
    return found;
}

void updateNPCs(SpriteGroup *collisions, float dt){
    dt = (dt > 0.05f) ? 0.05f : dt;
    NPCGroup *curr = loaded_NPCs;
    NPCGroup *prev = NULL;
    int playerY = GetPlayerPos().y;
    while (curr != NULL){
        if (curr->npc->npcSprite->health <= 0){
            curr = killNPC(prev, curr);
            continue;
        }
        prev = curr;
        if (curr->npc->hitTime < curr->npc->hitTimeMax){
            curr->npc->hitTime += dt;
        }
        UpdateAnimatedSprite(curr->npc->npcSprite->brush->anim_group, dt);
        Sprite *selfObj = curr->npc->npcSprite;
        SpriteGroup *curr_coll = collisions;
        int grounded = FALSE;
        int prevX = selfObj->pos.x;
        selfObj->pos = get_transform_due(collisions, selfObj, &grounded);
        // GRAVITY ON NPC
        if (!grounded){
            curr->npc->forces[1] += gravity*dt;
        }else{
            curr->npc->forces[1] = 0;
        }

        if (selfObj->pos.x != prevX && grounded){
            curr->npc->forces[1] = -jump_force;
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
        int dist = distToPlayer(curr->npc->npcSprite);
        if (((curr->npc->npcSprite->pos.y >= GetPlayerPos().y && curr->npc->npcSprite->pos.y <= GetPlayerPos().y + GetPlayerSize().cy)
        && curr->npc->friendly == FALSE || (dist < 400 && dist > 300))){
            curr->npc->targetX = GetPlayerPos().x;
            if (GetPlayerPos().y > selfObj->pos.y && grounded){
                curr->npc->forces[1] = -jump_force;
            }
        }
        if (curr->npc->friendly == TRUE){
            curr->npc->targetX = selfObj->pos.x;
        }
        
        if (curr->npc->friendly == FALSE && curr->npc->npcSprite->pos.x != curr->npc->targetX && curr->npc->talked){
            int prev = curr->npc->forces[0];
            curr->npc->forces[0] = ((curr->npc->targetX - selfObj->pos.x) > 0) ? NPC_speed : -NPC_speed;
            if (((prev > 0 && curr->npc->forces[0] < 0) || (prev < 0 && curr->npc->forces[0] > 0)) && prev != 0){
                curr->npc->forces[0] = 0;
                curr->npc->targetX = curr->npc->npcSprite->pos.x;
            }
        }

        if (distToPlayer(selfObj) < 50 && (!curr->npc->talked || curr->npc->friendly == FALSE)){
            ChangeAnimationDirection(GetPlayerPos().x < selfObj->pos.x ? "left" : "right", selfObj);
            if (curr->npc->talked){
                ChangeAnimationNoDir("attack", selfObj);
                if (GetFrame(selfObj->brush->anim_group) == 0) curr->npc->didDamage = FALSE;
                if (GetFrame(selfObj->brush->anim_group) == 2 && curr->npc->didDamage == FALSE){
                    curr->npc->didDamage = TRUE;
                    GetPlayerPtr()->health -= selfObj->damage;
                    if (GetPlayerPtr()->health <= 0){
                        GetPlayerPtr()->health = GetPlayerPtr()->maxHealth;
                    }
                }
            }
        }else{
            // RANDOMIZED MOVEMENT
            if (curr->npc->forces[0] == 0 && grounded){
                curr->npc->still += dt;
                if (curr->npc->still > curr->npc->stillTimer){
                    curr->npc->stillTimer = (float)(rand()%100)/10.0;
                    curr->npc->still = 0;
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

    // Update dead bodies
    SpriteGroup *deadBodies = dead_npcs;
    while(deadBodies != NULL){
        if (deadBodies->sprite->brush->type == 1){
            int currFrame = GetFrame(deadBodies->sprite->brush->anim_group);
            if (currFrame < 12){
                UpdateAnimatedSprite(deadBodies->sprite->brush->anim_group, dt);
            }
        }
        deadBodies = deadBodies->next;
    }
}