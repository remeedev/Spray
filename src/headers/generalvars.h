#include <windows.h>

#ifndef generalvars
#define generalvars


// Structures
typedef struct Animation {
    HBITMAP base_image;
    int curr_image, image_count, fps, upscale;
    SIZE frame_dimensions;
    float frame_counter;
    char *animation_name;
}Animation;

typedef struct AnimationGroup {
    Animation *animation;
    struct AnimationGroup *playing;
    struct AnimationGroup *next;
}AnimationGroup;

// Basic structures and typedefs

typedef struct BrushPalette {
    unsigned int type; // 0 Color 1 Animation
    HBRUSH brush;
    COLORREF color;
    char *name;
    AnimationGroup *anim_group;
    struct BrushPalette *next;
} BrushPalette;

typedef struct Sprite {
    POINT pos;
    SIZE size;
    BrushPalette* brush;
    int health, damage, maxHealth, alr_dead;
    char *name;
} Sprite;

typedef struct SpriteGroup {
    Sprite* sprite;
    struct SpriteGroup* next;
} SpriteGroup;

typedef struct NPC NPC;

typedef struct conversation {
    wchar_t *line;
    Sprite* image;
    size_t end;
    char **options;
    size_t option_count;
    int option_selected;
    int (*skip_check)(void);
    struct conversation *(*option_handler)(struct conversation *, NPC* npc);
    struct conversation *next;
} conversation;

typedef struct textNode {
    char *value;
    struct textNode *next;
    struct textNode *child;
}textNode;

// NPC Structs
typedef struct NPC {
    Sprite* npcSprite;
    int friendly, targetX, talking, talked, didDamage;
    float forces[2];
    float still, stillTimer;
    float hitTime, hitTimeMax;
    int weed_smoker, smoking;
    float weed_amount, smoke_inhaled;
    conversation *conv;
} NPC;

// NPC Storage
typedef struct NPCGroup{
    NPC *npc;
    struct NPCGroup *next;
} NPCGroup;

// Recursive search inside tree
textNode *find_text_node(textNode *structure, char *value);

int WindowWidth;
int WindowHeight;

char *gameVersion;
char *user_name;

HWND mainWindow;

HFONT GameFont;
HFONT SmallFont;
HFONT TitleFont;
HFONT ConsoleFont;
HFONT SmallBig;

COLORREF regular_text_color;
COLORREF undermined_text_color;
COLORREF highlight_text_color;
COLORREF ignore_text_color;
int is_fullscreen;

void toggle_fullscreen();

void openGameFont();
void deleteFont();

#endif