#include "headers/drawing.h"
#include "headers/animations.h"
#include "headers/npc.h"
#include "headers/console.h"
#include "headers/generalvars.h"

#include <stdio.h>

// VARIABLE DEFINITION
float player_speed = 1000.0f;
float jump_force = 1350.0f;

// Gravity
float gravity = 5000.0f;
float friction = 4000.0f;
float air_loss = 800.0f;

// Forgiveness Period
float time_since = 0.0f;
float forgive_period = 0.1f;
float stun_time = 0.0f;
float max_stun = 0.3f;

// Ground Forgivance
int GroundForgive = 5;

int w = 0;
int a = 0;
int s = 0;
int d = 0;

int lock_input = 0;
int talking = FALSE;

float player_forces[] = {0, 750};

int jump_direction = 0;
int can_jump = FALSE;
int grounded = FALSE;

int debug = FALSE;

// ======== controls ===========
void HandleKeyDown(UINT key){
    if (console_on) return;
    if (key == VK_SPACE && talking) conversationsNext();
    if (talking) return;
    if (key == 'F') conversationsNext();
    if (key == 'W') w = 1;
    if (key == 'A') a = 1;
    if (key == 'S') s = 1;
    if (key == 'D') d = 1;
    if (key == VK_SPACE){
        if ((lock_input == FALSE && (can_jump || time_since < forgive_period)) || debug){
            if (!grounded) {
                player_forces[1] = -(jump_force-air_loss);
                player_forces[0] = jump_direction*jump_force;
                lock_input = TRUE;
            }else{
                player_forces[1] = -jump_force;
            }
            jump_direction=0;
            can_jump = FALSE;
            grounded = FALSE;
        }
    }
}

void HandleKeyUp(UINT key){
    if (console_on) return;
    if (key == 'W') w = 0;
    if (key == 'A') a = 0;
    if (key == 'S') s = 0;
    if (key == 'D') d = 0;
}
// ======== controls end ===========

// ========= Logic Functions ========
int PointInSprite(POINT point, Sprite sprite){
    return (point.x > sprite.pos.x && point.x < sprite.pos.x + sprite.size.cx && point.y > sprite.pos.y && point.y < sprite.pos.y + sprite.size.cy) ? 1 : 0;
}

int LightInSprite(POINT point, Sprite sprite){ // Checks for equal (Helps with grounded variable)
    return (point.x >= sprite.pos.x && point.x <= sprite.pos.x + sprite.size.cx && point.y >= sprite.pos.y && point.y <= sprite.pos.y + sprite.size.cy) ? 1 : 0;
}
// ========== Logic End ===========

// Four corners definition
typedef struct four_points{
    POINT top_right;
    POINT top_left;
    POINT bot_right;
    POINT bot_left;
} four_points;

// Setters
four_points move_points(four_points og, POINT transform){
    og.top_right.x += transform.x;
    og.top_right.y += transform.y;
    og.top_left.x += transform.x;
    og.top_left.y += transform.y;
    og.bot_right.x += transform.x;
    og.bot_right.y += transform.y;
    og.bot_left.x += transform.x;
    og.bot_left.y += transform.y;
    return og;
}

// Init funct
four_points calculate_points(POINT pos, SIZE size){
    four_points out;
    out.top_left.x = pos.x;
    out.top_left.y = pos.y;
    out.top_right.x = pos.x + size.cx;
    out.top_right.y = pos.y;
    out.bot_left.x = pos.x;
    out.bot_left.y = pos.y+size.cy;
    out.bot_right.x = pos.x + size.cx;
    out.bot_right.y = pos.y+size.cy;
    return out;
}

// Approximation move (causes stairs to be a thing)
int counter = 0;
four_points equalize_points(four_points in_points, POINT edge, POINT target_edge, int grounded){
    POINT transform;
    if ((abs(edge.x - target_edge.x) >= abs(edge.y - target_edge.y)) || (abs(edge.y - target_edge.y) <= 45 && grounded)){
        transform.y = target_edge.y - edge.y;
        transform.x = 0;
    }else{
        transform.y = 0;
        transform.x = target_edge.x-edge.x;
    }
    return move_points(in_points, transform); // Returns the new points
}



int GetCollision(Sprite *sprite, int *out, Sprite *srcSprite){
    // Literally code from get transform due
    four_points points = calculate_points(srcSprite->pos, srcSprite->size);

    four_points collision_points = calculate_points(sprite->pos, sprite->size);
    int tl = LightInSprite(points.top_left, *sprite);
    int tr = LightInSprite(points.top_right, *sprite);
    int br = LightInSprite(points.bot_right, *sprite);
    int bl = LightInSprite(points.bot_left, *sprite);

    int itl = LightInSprite(collision_points.top_left, *srcSprite);
    int itr = LightInSprite(collision_points.top_right, *srcSprite);
    int ibr = LightInSprite(collision_points.bot_right, *srcSprite);
    int ibl = LightInSprite(collision_points.bot_left, *srcSprite);
    if (out != NULL){
        out[0] = tl;
        out[1] = tr;
        out[2] = bl;
        out[3] = br;
        out[4] = itl;
        out[5] = itr;
        out[6] = ibl;
        out[7] = ibr;
    }
    return tl+tr+br+bl+itl+itr+ibr+ibl;
}

// COLLISION DETECTION
POINT get_transform_due(SpriteGroup* collisions, Sprite *sprite, int *grounded){ // Function takes the collision boxes and processes player go down
    /*
        Function returns the new position a Sprite should have considering the collisions that were passed to it.
        Arguments:
            SpriteGroup : the collisions to be applied
            Sprite : The sprite that is currently moving
            *int : 2nd output -> determines whether the sprite is grounded or not
    */
    int player = sprite == GetPlayerPtr() ? TRUE : FALSE;
    // Gets the four edges of the player, then checks for collisions and returns final position
    four_points points = calculate_points(sprite->pos, sprite->size);
    SpriteGroup* curr_elem = collisions;

    *grounded = FALSE;
    if (player){
        // Reset information variables
        can_jump = FALSE;
    }

    while (curr_elem != NULL){
        int *coll_info = (int *)malloc(sizeof(int)*8);
        int assumedHeight = curr_elem->sprite->size.cy < 75 ? curr_elem->sprite->size.cy : 0;
        GetCollision(curr_elem->sprite, coll_info, sprite);
        int tl = coll_info[0];
        int tr = coll_info[1];
        int bl = coll_info[2];
        int br = coll_info[3];
        int itl = coll_info[4];
        int itr = coll_info[5];
        int ibl = coll_info[6];
        int ibr = coll_info[7];
        free(coll_info);
        four_points collision_points = calculate_points(curr_elem->sprite->pos, curr_elem->sprite->size);

        // EXTRA SPECIAL CASES goddamn :/
        if ((points.top_left.y <= collision_points.top_left.y && points.bot_left.y >= collision_points.bot_left.y) &&
            (points.bot_left.x >= collision_points.bot_left.x && points.bot_right.x <= collision_points.bot_right.x)){
            bl = TRUE;
            br = TRUE;
        }
        int total = tl+tr+br+bl;
        int intTotal = itl+itr+ibr+ibl;

        if (total == 0 && intTotal == 0){
            // Player is not affected by obj
            curr_elem = curr_elem->next;
            continue;
        }

        // DETERMINES WHETHER SPRITE IS COLLIDING WITH FLOOR
        int tY = bl || br ? collision_points.top_left.y - points.bot_left.y : collision_points.bot_left.y - points.top_left.y + 1;
        int tX = bl || ibr ? collision_points.top_right.x - points.top_left.x : collision_points.top_left.x - points.top_right.x;
        int c75 = abs(tY) < abs(tX);
        if (((bl && tl) || (br && tr) || (itl && ibl) || (itr && ibr)) || (((bl && itr) || (br && itl)) && !c75)){
            points.bot_left.x += tX;
            points.bot_right.x += tX;
            points.top_left.x += tX;
            points.top_right.x += tX;
        }
        if (((bl && br) || (itr && itl) || (ibr && ibl) || (tl && tr)) ||
        (((bl && itr) || (br && itl) || (tl && ibr) || (tr && ibl)) && c75)){
            points.bot_left.y += tY;
            points.bot_right.y += tY;
            points.top_left.y += tY;
            points.top_right.y += tY;
            *grounded = TRUE;
        }
        
        curr_elem = curr_elem->next;
    }
    return points.top_left;
}

void ChangeAnimationNoDir(char *new_animation_name, Sprite *sprite){
    char *anim_name = GetCurrentAnimationName(sprite->brush->anim_group);
    if (anim_name == NULL) anim_name = "walking_right";
    size_t direction_size = 0;
    size_t pos = 0;
    while (anim_name[pos] != '_')pos++;
    while (anim_name[pos+direction_size] != '\0')direction_size++;
    if (direction_size <= 0) return;
    char *direction = (char *)malloc(direction_size+1);
    if (direction == NULL) {
        printf("Couldn't set direction in animation!\n");
        return;
    }
    for (size_t i = 0; i < direction_size; i++)direction[i] = anim_name[pos+i];
    direction[direction_size] = '\0';
    char *out = (char *)malloc(strlen(new_animation_name)+direction_size+2);
    if (out == NULL) {
        printf("Couldn't create animation out name!\n");
        return;
    }
    sprintf(out, "%s%s", new_animation_name, direction);
    ChangeCurrentAnimation(sprite->brush->anim_group, out);
    free(out);
    free(direction);
}

void ChangeAnimationDirection(char *direction, Sprite *sprite){
    char *anim_name = GetCurrentAnimationName(sprite->brush->anim_group);
    if (anim_name == NULL) anim_name = "walking_right";
    size_t anim_size = 0;
    while (anim_name[anim_size] != '_')anim_size++;
    char *base_name = (char *)malloc(anim_size+1);
    if (base_name == NULL){
        printf("Error allocating space!\n");
        return;
    }
    for (int i = 0; i < anim_size; i++)base_name[i]=anim_name[i];
    base_name[anim_size] = '\0';
    char *out = (char *)malloc(anim_size + strlen(direction) + 2);
    if (out == NULL){
        printf("Error allocating space!\n");
        return;
    }
    sprintf(out, "%s_%s", base_name, direction);
    ChangeCurrentAnimation(sprite->brush->anim_group, out);
    free(out);
    free(base_name);
}

// Basic Update Function (player movement)
void UpdatePosition(float dt, SpriteGroup* collisions){
    dt = (dt > 0.05f) ? 0.05f : dt;
    UpdateAnimatedSprite(GetPlayerPtr()->brush->anim_group, dt);
    if (lock_input){
        stun_time+=dt;
        if (stun_time >= max_stun){
            lock_input = FALSE;
            stun_time = 0;
        }
    }
    time_since+=dt;
    if (grounded || can_jump){
        time_since=0.0f;
    }
    if (lock_input == FALSE){
        if (a || d){
            if (abs(player_forces[0]) <= player_speed) player_forces[0] += 20*dt*player_speed*(d ? 1 : -1);
        }
    }
    if (grounded == TRUE && player_forces[1] > 0){
        can_jump = TRUE;
        player_forces[1]=0;
        lock_input = 0;
    }else{
        player_forces[1]+=(int)(gravity*dt);
    }
    POINT curr_pos = GetPlayerPos();
    if (player_forces[0] != 0){
        if (!grounded){
        }
        int m = (player_forces[0] > 0) ? 1 : -1;
        curr_pos.x += (int)(player_forces[0]*dt);
        player_forces[0] = (float)(abs(player_forces[0])-(grounded ? friction : friction-air_loss)*dt)*m;
        if ((m == -1 && player_forces[0] > 0) || (m == 1 && player_forces[0] < 0)){
            player_forces[0] = 0;
        }
    }
    if (player_forces[1] != 0){
        curr_pos.y += (int)(player_forces[1]*dt);
    }

    SetPlayerPos(curr_pos);
    curr_pos = get_transform_due(collisions, GetPlayerPtr(), &grounded);
    if (grounded){
        player_forces[1] = 0;
        if (a || d){
            ChangeAnimationNoDir("walking", GetPlayerPtr());
        }else{
            ChangeAnimationNoDir("still", GetPlayerPtr());
        }
    }else{
        if (player_forces[1] > 0){
            ChangeAnimationNoDir("falling", GetPlayerPtr());
        }
    }
    if (a)ChangeAnimationDirection("left", GetPlayerPtr());
    if (d)ChangeAnimationDirection("right", GetPlayerPtr());
    SetPlayerPos(curr_pos);
}