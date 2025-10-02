#include "generalvars.h"

#ifndef credits
#define credits

int showingCredits;

void init_credits();
void build_credits();
void draw_credits(HDC hdc);
void update_credits(float dt);

void switch_credit_speed(int value);
void hide_show_credits();
#endif