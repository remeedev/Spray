#include <stdio.h>

#include "headers/drawing.h"
#include "headers/savefile.h"

void lockCharacterAttr(char *name, char *attribute, char datatype, void *write_from){
    char *attr_name = (char *)malloc(strlen(name) + strlen(attribute) + 2);
    sprintf(attr_name, "%s_%s", name, attribute);
    void *attr = get_from_save(attr_name);
    add_static_save(attr_name, write_from, datatype);
    free(attr_name);
}

void loadCharacterAttr(char *name, char *attribute, char datatype, void *write_to){
    char *attr_name = (char *)malloc(strlen(name) + strlen(attribute) + 2);
    if (attr_name == NULL){
        printf("Error allocating space for attr_name\n");
        return;
    }
    sprintf(attr_name, "%s_%s", name, attribute);
    void *attr = get_from_save(attr_name);
    if (attr == NULL){
        add_to_save(attr_name, write_to, datatype, FALSE);
        free(attr_name);
        return;
    }
    switch (datatype){
        case 1: ;
            *(int *)write_to = *(int *)attr;
            break;
        case 2: ;
            *(float *)write_to = *(float *)attr;
            break;
        case 3: ;
            *(double *)write_to = *(double *)attr;
            break;
        case 4: ;
            *(char **)write_to = *(char **)attr;
            break;
        case 5: ;
            *(long *)write_to = *(long *)attr;
            break;
    }
    if (check_pending_overwrite(attr_name)){
        overwrite(attr_name, write_to, FALSE);
    }
    free(attr_name);
}

void loadSpriteCharacter(Sprite *sprite){
    // if (sprite->name == NULL) return;
    // loadCharacterAttr(sprite->name, "health", 1, &sprite->health);
    // if (sprite->health <= 0){
    //     sprite->health = 0;
    // }
}

void lockSpriteDeath(Sprite *sprite){
    // if (sprite->name == NULL) return;
    // int zero_health = 0;
    // lockCharacterAttr(sprite->name, "health", 1, &zero_health);
}