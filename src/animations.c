/*
ANIMATIONS.C
Handles Spritesheet animation and sprite animations
(AnimatedSprite)
*/

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "headers/images.h"
#include "headers/generalvars.h"

// ======= INIT FUNCTIONS =======
Animation *LoadSpriteSheet(char *image_name, char *animation_name, size_t FrameWidth, size_t FrameHeight, int fps, float upscale){
    // Load the sprite sheet anim
    HBITMAP image = LoadPNGAsBmp(image_name);

    // Create the object
    Animation *out_anim = (Animation *)malloc(sizeof(Animation));
    if (out_anim == NULL){
        printf("Error: Failed to alloc space for Animation!\n");
        return NULL;
    }
    // Set the animation information
    out_anim->base_image = image;
    BITMAP bm;
    GetObject(image, sizeof(bm), &bm);
    out_anim->image_count = (int)((float)bm.bmWidth/((float)FrameWidth/upscale)) * (int)((float)bm.bmHeight/((float)FrameHeight/upscale));
    out_anim->upscale = upscale;

    // Save the dimensions of frame
    SIZE frame_dimensions;
    frame_dimensions.cx = FrameWidth;
    frame_dimensions.cy = FrameHeight;
    out_anim->frame_dimensions = frame_dimensions;

    // Extras
    out_anim->curr_image = 0;
    out_anim->fps = (fps > 0) ? fps : 24;
    out_anim->animation_name = animation_name;
    out_anim->frame_counter = 0.0f;
    return out_anim;
}

AnimationGroup *CreateAnimatedGroup(Animation *first_anim){
    AnimationGroup *out = (AnimationGroup *)malloc(sizeof(AnimationGroup));

    if (out == NULL){
        printf("Error: Failed to alloc space for animation group!\n");
        return NULL;
    }

    // Set the variables
    out->animation = first_anim;
    out->next = NULL;
    out->playing = NULL;
    return out;
}

// Init function from name
AnimationGroup *LoadPNGIntoSprite(char *file_name, char *anim_name, size_t width, size_t height, int fps, int upscale){
    Animation *anim = LoadSpriteSheet(file_name, anim_name, width, height, fps, upscale);
    AnimationGroup *anim_group = CreateAnimatedGroup(anim);
    if (anim == NULL || anim_group == NULL)printf("Failed to load animation!\n");
    return anim_group;
}
// ======= INIT FUNCTIONS END =======

// ======== GETTERS =================
char *GetCurrentAnimationName(AnimationGroup *group){
    Animation *curr_anim = group->playing ? group->playing->animation : group->animation;
    if (curr_anim)return curr_anim->animation_name;
    return NULL; // Normally caused by error
}
// ======== GETTERS END =========

// ======= LIST FUNCTIONS =========
void AppendAnimation(AnimationGroup *group, Animation *animation){
    AnimationGroup *curr = group;
    while (curr->next != NULL)curr = curr->next;
    AnimationGroup *to_append = CreateAnimatedGroup(animation);
    curr->next = to_append;
}

void AddToAnimationGroup(AnimationGroup *group, char *file_name, char *anim_name, size_t width, size_t height, int fps, int upscale){
    Animation *anim = LoadSpriteSheet(file_name, anim_name, width, height, fps, upscale);
    AppendAnimation(group, anim);
}
// ======= LIST FUNCTIONS END =========

// ======= DELETE FUNCTIONS =========
void DeleteAnimation(Animation *anim){
    DeleteObject(anim->base_image);
    free(anim);
}

void DeleteAnimationGroup(AnimationGroup *anim_group){
    AnimationGroup *curr = anim_group;
    while (curr!=NULL){
        AnimationGroup *prep = curr->next;
        DeleteAnimation(curr->animation);
        curr = prep;
    }
    free(anim_group);
}

// ======= DELETE FUNCTIONS END =========

// ======= DRAW FUNCTIONS =============
BLENDFUNCTION bf = {0};
int bf_init = FALSE;

// Draw current animation
// CODE BROUGHT FROM drawing.c in older version
void DrawAnimatedSprite(HDC hdc, AnimationGroup *animated_sprite, int x, int y){
    Animation *curr_anim = animated_sprite->playing ? animated_sprite->playing->animation : animated_sprite->animation;
    HBITMAP curr_img = curr_anim->base_image;
    if (curr_anim == NULL) return;
    HDC hdcMem = CreateCompatibleDC(hdc);
    if (hdcMem){
        HBITMAP hbmOld = SelectObject(hdcMem, curr_img);
        BITMAP bm;
        GetObject(curr_img, sizeof(bm), &bm);
        if(!bf_init){
            bf.BlendOp = AC_SRC_OVER;
            bf.BlendFlags = 0;
            bf.SourceConstantAlpha = 255;
            bf.AlphaFormat = AC_SRC_ALPHA;
            bf_init = TRUE;
        }
        SIZE frame = curr_anim->frame_dimensions;
        int srcX = ((frame.cx/curr_anim->upscale)*curr_anim->curr_image)%bm.bmWidth;
        int srcY = (int)(((frame.cx/curr_anim->upscale)*curr_anim->curr_image)/bm.bmWidth)*(frame.cy/curr_anim->upscale);
        if (!AlphaBlend(hdc, x, y, frame.cx, frame.cy, hdcMem, srcX, srcY, (int)((float)frame.cx/(float)curr_anim->upscale), (int)((float)frame.cy/(float)curr_anim->upscale), bf))printf("");
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
    }
}

// Changes frame through time
void UpdateAnimatedSprite(AnimationGroup *animated_sprite, float dt){
    Animation *curr_anim = animated_sprite->playing ? animated_sprite->playing->animation : animated_sprite->animation;
    if (curr_anim == NULL)return;
    if (curr_anim->fps == 0) return;
    curr_anim->frame_counter += dt;
    if (curr_anim->frame_counter > 1.0/((float) curr_anim->fps)){
        curr_anim->frame_counter = 0;
        curr_anim->curr_image = (curr_anim->curr_image + 1)%curr_anim->image_count;
    }
}

// Get current frame
int GetFrame(AnimationGroup *animated_sprite){
    Animation *curr_anim = animated_sprite->playing ? animated_sprite->playing->animation : animated_sprite->animation;
    if (curr_anim == NULL)return 0;
    if (curr_anim->fps == 0) return 0;
    return curr_anim->curr_image;
}

// Changes animation playing for a group
int ChangeCurrentAnimation(AnimationGroup *animated_sprite, char *animation_name){
    Animation *curr_anim = animated_sprite->playing ? animated_sprite->playing->animation : animated_sprite->animation;
    if (strcmp(animation_name, curr_anim->animation_name) == 0)return 0; // Nothing to change
    int changed = FALSE;
    AnimationGroup *start = animated_sprite;
    while (start != NULL && !changed){
        if (strcmp(start->animation->animation_name, animation_name) == 0){
            animated_sprite->playing = start;
            changed = TRUE;
        }else{
            start = start->next;
        }
    }
    if (changed){
        curr_anim->curr_image = 0;
        curr_anim->frame_counter = 0.0;
        return 1;
    }
    return -1;
}