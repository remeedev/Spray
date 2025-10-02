#include "generalvars.h"

#ifndef CHARACTERS_H
#define CHARACTERS_H

void loadSpriteCharacter(Sprite *sprite);
void lockSpriteStatic(Sprite *sprite);
void loadCharacterAttr(char *name, char *attribute, char datatype, void *write_to);
void lockCharacterAttr(char *name, char *attribute, char datatype, void *write_from);

#endif