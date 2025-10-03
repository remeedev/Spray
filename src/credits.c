#include <windows.h>
#include <stdio.h>

#include "headers/generalvars.h"
#include "headers/level_loader.h"
#include "headers/drawing.h"

int showingCredits = FALSE;

float credit_pos = 0;

textNode *credit_saved = NULL;
float credit_speed = 50.0;
float min_credit_speed = 50.0;
float max_credit_speed = 150.0;
int sped_up = FALSE;
Sprite *game_logo = NULL;

void switch_credit_speed(int value){
    sped_up = value;
}

void hide_show_credits(){
    showingCredits = !showingCredits;
    credit_pos = game_res[1];
    switch_credit_speed(FALSE);
}

void add_to_credits(char *role, char *name){
    textNode *parent_node = find_text_node(credit_saved, role);
    if (find_text_node(parent_node, name) != NULL) {
        printf("Tried to add same name to role!\n");
        return;
    }
    textNode *name_node = find_text_node(credit_saved, name);
    textNode *new_node = (textNode *) malloc(sizeof(textNode));
    if (new_node == NULL){
        printf("There was an issue creating the new node in credits!\n");
        return;
    }
    new_node->child = NULL;
    new_node->next = NULL;
    if (name_node){
        new_node->value = name_node->value;
    }else{
        new_node->value = (char *)malloc(strlen(name) + 1);
        if (new_node->value == NULL){
            printf("There was an error allocating space for name!\n");
            return;
        }
        strcpy(new_node->value, name);
    }
    if (parent_node){
        textNode *credited = parent_node->child;
        if (credited == NULL){
            parent_node->child = new_node;
            return;
        }
        while (credited->next){
            credited = credited->next;
        }
        credited->next = new_node;
        return;
    }else{
        textNode *fixed_parent = (textNode *) malloc(sizeof(textNode));
        if (fixed_parent == NULL){
            printf("Error allocating space for local variable!\n");
            return;
        }
        fixed_parent->value = (char *)malloc(strlen(role) + 1);
        if (fixed_parent->value == NULL){
            printf("There was an error creating parent node value in credits!\n");
            return;
        }
        strcpy(fixed_parent->value, role);
        fixed_parent->child = new_node;
        fixed_parent->next = NULL;
        textNode *curr_pos = credit_saved;
        if (curr_pos == NULL){
            credit_saved = fixed_parent;
        }else{
            while (curr_pos->next){
                curr_pos = curr_pos->next;
            }
            curr_pos->next = fixed_parent;
        }
    }
}

typedef struct textDraw{
    char *text;
    HFONT font_type;
    struct textDraw *next;
} textDraw;

textDraw* saved_credits = NULL;
size_t credit_len = 0;

void load_game_logo(){
    if (game_logo != NULL) return;
    game_logo = (Sprite *)malloc(sizeof(Sprite));
    int upscale = 8;
    int cx = 60*upscale;
    int x = (int)(((double)game_res[0]-(double)cx)/2.0);
    CreateImgSprite(game_logo, x, 0, cx, 30*upscale, "./assets/ui/game_logo.png", upscale);
}

void init_credits(){
    credit_pos = game_res[1];
    load_game_logo();
    add_to_credits("Programmer", "Remeedev");
    add_to_credits("Visual Artist", "Remeedev");
    // add_to_credits("Music Producer", "Pistola de Guaro");
}

void build_credits_raw(textNode *structure){
    textDraw *new_val = (textDraw *)malloc(sizeof(textDraw));
    if (new_val == NULL){
        printf("There was an error allocating space for saved credits!\n");
        return;
    }
    new_val->text = structure->value;
    new_val->font_type = structure->child == NULL ? SmallFont : GameFont;
    if (structure->child != NULL) credit_len++;
    new_val->next = NULL;
    if (saved_credits == NULL){
        saved_credits = new_val;
    }else{
        textDraw *curr = saved_credits;
        while (curr->next) curr = curr->next;
        curr->next = new_val;
    }
    credit_len++;
    if (structure->child) build_credits_raw(structure->child);
    if (structure->next) build_credits_raw(structure->next);
}

void build_credits(){
    build_credits_raw(credit_saved);
}


void draw_credits(HDC hdc){
    if (showingCredits == FALSE) return;
    TEXTMETRIC smallFontMetrics;
    SelectObject(hdc, SmallFont);
    GetTextMetrics(hdc, &smallFontMetrics);

    TEXTMETRIC gameFontMetrics;
    SelectObject(hdc, GameFont);
    GetTextMetrics(hdc, &gameFontMetrics);

    int yLevel = (int)credit_pos;
    SetTextAlign(hdc, TA_TOP | TA_CENTER);
    int xLevel = game_res[0] / 2;
    game_logo->pos.y = yLevel;
    yLevel += game_logo->size.cy + 15;
    PaintSprite(hdc, game_logo);

    textDraw *curr = saved_credits;
    while (curr){
        SelectObject(hdc, curr->font_type);
        if (curr->font_type == GameFont) yLevel += gameFontMetrics.tmHeight;
        TextOut(hdc, xLevel, yLevel, curr->text, strlen(curr->text));
        if (curr->font_type == GameFont){
            SetTextColor(hdc, undermined_text_color);
            yLevel += gameFontMetrics.tmHeight;
        }else{
            SetTextColor(hdc, regular_text_color);
            yLevel += smallFontMetrics.tmHeight;
        }
        curr = curr->next;
    }
    
    if (yLevel < 0){
        showingCredits = FALSE;
        credit_pos = game_res[1];
    }
}

void update_credits(float dt){
    if (sped_up && credit_speed < max_credit_speed){
        credit_speed += dt*(credit_speed);
        if (credit_speed > max_credit_speed){
            credit_speed = max_credit_speed;
        }
    }
    if (!sped_up && credit_speed > min_credit_speed){
        credit_speed = min_credit_speed;
    }
    if (showingCredits == FALSE) return;
    credit_pos -= credit_speed*dt;
}