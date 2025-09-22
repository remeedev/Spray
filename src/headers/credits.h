#include <windows.h>

#ifndef credits
#define credits

int showingCredits;

void init_credits();
void build_credits();
void draw_credits(HDC hdc);
void update_credits(float dt);

#endif