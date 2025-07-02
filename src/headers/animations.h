#include <windows.h>

#ifndef animations
#define animations

// Structures
typedef struct Animation {
    HBITMAP base_image;
    int curr_image, image_count, fps;
    SIZE frame_dimensions;
    float frame_counter;
    char *animation_name;
}Animation;

typedef struct AnimationGroup {
    Animation *animation;
    struct AnimationGroup *playing;
    struct AnimationGroup *next;
}AnimationGroup;

// ======= INIT FUNCTIONS =======
Animation *LoadSpriteSheet(char *image_name, char *animation_name, size_t FrameWidth, size_t FrameHeight, int fps, float upscale);
AnimationGroup *CreateAnimatedGroup(Animation *first_anim);
AnimationGroup *LoadPNGIntoSprite(char *file_name, char *anim_name, size_t width, size_t height, int fps, int upscale);

// ======== GETTERS =================
char *GetCurrentAnimationName(AnimationGroup *group);

// ======= LIST FUNCTIONS =========
void AppendAnimation(AnimationGroup *group, Animation *animation);
void AddToAnimationGroup(AnimationGroup *group, char *file_name, char *anim_name, size_t width, size_t height, int fps, int upscale);

// ======= DELETE FUNCTIONS =========
void DeleteAnimation(Animation *anim);
void DeleteAnimationGroup(AnimationGroup *anim_group);

// DRAW FUNCTION
void DrawAnimatedSprite(HDC hdc, AnimationGroup *animated_sprite, int x, int y);

// GET CURRENT FRAME
int GetFrame(AnimationGroup *animated_sprite);

// FRAME UPDATE
void UpdateAnimatedSprite(AnimationGroup *animated_sprite, float dt);

// Changes animation playing for an animated sprite
void ChangeCurrentAnimation(AnimationGroup *animated_sprite, char *animation_name);

#endif