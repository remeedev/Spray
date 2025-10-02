#include "generalvars.h"

#ifndef the_chronic
#define the_chronic

int weed_bags;
int convert_to_weed_smoker(NPC *npc);
void create_weed_sale(conversation *add_location, SpriteGroup* characters);
conversation * process_weed_sale(conversation *curr_conv, NPC *npc);
int amure(NPC *npc);
void start_weed_convos();
void check_weed();
void lock_weed_smoker(NPC *npc);

#endif