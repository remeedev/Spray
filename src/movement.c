#include "headers/drawing.h"
#include <stdio.h>

float player_speed = 1000.0f;
float jump_force = 1500.0f;

// Gravity
float gravity = 5000.0f;
float friction = 3000.0f;
float air_loss = 2500.0f;

// Forgiveness Period
float time_since = 0.0f;
float forgive_period = 0.1f;
float stun_time = 0.0f;
float max_stun = 0.3f;

int w = 0;
int a = 0;
int s = 0;
int d = 0;

int lock_input = 0;

float player_forces[] = {0, 750};

int jump_direction = 0;
int can_jump = FALSE;
int grounded = FALSE;

int debug = FALSE;

void HandleKeyDown(UINT key){
    if (key == 'W') w = 1;
    if (key == 'A') a = 1;
    if (key == 'S') s = 1;
    if (key == 'D') d = 1;
    if (key == 'R') {
        POINT a;
        a.x = 0;
        a.y = 0;
        SetPlayerPos(a);
    };
    if (key == VK_SPACE){
        if ((lock_input == FALSE && (can_jump || time_since < forgive_period)) || debug){
            player_forces[1] = -jump_force;
            if (!grounded) {
                player_forces[0] = jump_direction*jump_force;
                lock_input = TRUE;
            }
            jump_direction=0;
            can_jump = FALSE;
            grounded = FALSE;
        }
    }
}

void HandleKeyUp(UINT key){
    if (key == 'W') w = 0;
    if (key == 'A') a = 0;
    if (key == 'S') s = 0;
    if (key == 'D') d = 0;
    if (key == 'X'){
        debug = debug ? FALSE : TRUE;
    }
}

int PointInSprite(POINT point, Sprite sprite){
    return (point.x > sprite.pos.x && point.x < sprite.pos.x + sprite.size.cx && point.y > sprite.pos.y && point.y < sprite.pos.y + sprite.size.cy) ? 1 : 0;
}

int LightInSprite(POINT point, Sprite sprite){ // Checks for equal (Helps with grounded variable)
    return (point.x >= sprite.pos.x && point.x <= sprite.pos.x + sprite.size.cx && point.y >= sprite.pos.y && point.y <= sprite.pos.y + sprite.size.cy) ? 1 : 0;
}

// Helps in the function below
typedef struct four_points{
    POINT top_right;
    POINT top_left;
    POINT bot_right;
    POINT bot_left;
} four_points;

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
    return move_points(in_points, transform); // Retornea nuevos puntos
}

POINT get_transform_due(SpriteGroup* collisions){ // Function takes the collision boxes and processes player go down
    // Gets the four edges of the player, then checks for collisions and returns final position
    four_points points = calculate_points(GetPlayerPos(), GetPlayerSize());
    SpriteGroup* curr_elem = collisions;
    
    // Reset information variables
    grounded = FALSE;
    can_jump = FALSE;

    // Setup output
    POINT out;
    out.x = 0;
    out.y = 0;
    while (curr_elem != NULL){
        four_points collision_points = calculate_points(curr_elem->sprite->pos, curr_elem->sprite->size);
        int tl = (LightInSprite(points.top_left, *curr_elem->sprite)==1) ? 1 : 0; // top left, right, etc.
        int tr = (LightInSprite(points.top_right, *curr_elem->sprite)==1) ? 1 : 0;
        int br = (LightInSprite(points.bot_right, *curr_elem->sprite)==1) ? 1 : 0;
        int bl = (LightInSprite(points.bot_left, *curr_elem->sprite)==1) ? 1 : 0;
        int total = tl + tr + br + bl;
        int lower_range_x = collision_points.bot_left.x-2;
        int upper_range_x = collision_points.bot_left.x+2;
        int lower_range_xx = collision_points.bot_right.x-2;
        int upper_range_xx = collision_points.bot_right.x+2;
        
        int shares_border_right = (points.bot_right.x >= lower_range_x && points.bot_right.x <= upper_range_x);
        int shares_border_left = (points.bot_left.x >= lower_range_xx && points.bot_left.x <= upper_range_xx);
        int shares_border = (shares_border_right || shares_border_left);
        if (shares_border && total > 0){
            jump_direction = shares_border_right ? -1 : 1;
            can_jump = TRUE;
            lock_input = FALSE;
        }
        if (points.bot_left.y >= collision_points.top_left.y-2 && points.bot_left.y <= collision_points.top_left.y+2 && (br || bl)){
            grounded = TRUE;
            can_jump = TRUE;
        }
        if (points.top_left.y >= collision_points.bot_left.y-2 && points.top_left.y <= collision_points.bot_left.y+2 && (tr || tl)){
            player_forces[1] = -player_forces[1];
        }
        
        if (total == 1){
            if (tl == 1) points = equalize_points(points, points.top_left, collision_points.bot_right, grounded);
            if (tr == 1) points = equalize_points(points, points.top_right, collision_points.bot_left, grounded);
            if (bl == 1) points = equalize_points(points, points.bot_left, collision_points.top_right, grounded);
            if (br == 1) points = equalize_points(points, points.bot_right, collision_points.top_left, grounded);
        }
        if (total == 2){
            POINT transform;
            transform.x = 0;
            transform.y = 0;
            if (tl == TRUE && tr == TRUE){ // Top
                transform.y = collision_points.bot_left.y-points.top_left.y; // Make bottom of collider top of collided
            }
            if (bl == TRUE && br == TRUE){ // Bottom
                transform.y = collision_points.top_left.y-points.bot_left.y;
            }
            if (tl == TRUE && bl == TRUE){ // Left side
                transform.x = collision_points.top_right.x - points.bot_left.x;
            }
            if (tr == TRUE && br == TRUE){ // Right side
                transform.x = collision_points.top_left.x - points.bot_right.x;
            }
            points = move_points(points, transform);
        }
        curr_elem = curr_elem->next;
    }
    return points.top_left;
}

void UpdatePositionOnResize(int screen_width, int screen_height){
    POINT curr_pos = GetPlayerPos();
    if (curr_pos.x > screen_width-GetPlayerSize().cx){
        curr_pos.x = screen_width-GetPlayerSize().cx;
    }
    if (curr_pos.y > screen_height-GetPlayerSize().cx){
        curr_pos.y = 0;
    }
    SetPlayerPos(curr_pos);
}

void UpdatePosition(float dt, SpriteGroup* collisions){
    dt = (dt > 0.05f) ? 0.05f : dt;
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
    POINT curr_pos = GetPlayerPos();
    if (grounded == TRUE && player_forces[1] > 0){
        can_jump = TRUE;
        player_forces[1]=0;
        lock_input = 0;
    }else{
        player_forces[1]+=gravity*dt;
    }
    if (player_forces[0] != 0){
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
    curr_pos = get_transform_due(collisions);
    SetPlayerPos(curr_pos);
    if (debug) printf("(%d, %d)\n", curr_pos.x, curr_pos.y);
}