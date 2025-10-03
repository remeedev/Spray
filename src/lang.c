#include <stdio.h>

#include "headers/generalvars.h"

int lang = 1; // 0 - en | 1 - es
int lang_len = 2; // How many languages implemented

int get_lang(){
    return lang;
}

void change_lang_next(){
    lang++;
    if (lang > lang_len - 1){
        lang = lang - lang_len;
    }
}
void change_lang_prev(){
    lang--;
    if (lang < 0){
        lang = lang + lang_len;
    }
}

char *load_str(char *str){
    char *out = (char *)malloc(strlen(str));
    strcpy(out, str);
    return out;
}

UINT special_keybinds[] = {
    VK_SPACE,
    VK_RETURN,
    VK_ESCAPE
};

char *special_keys_en[] = {
    "space",
    "return",
    "escape",
    NULL
};

char *special_keys_es[] = {
    "Espacio",
    "Enter",
    "Escape",
    NULL
};


char *get_key(UINT key){
    char *out = NULL;
    if ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
        out = malloc(2);
        if (out == NULL){
            printf("There was an error allocating space for key name!\n");
            return NULL;
        }
        out[0] = (char) key;
        out[1] = '\0';
    }
    int i = 0;
    while (special_keys_en[i] != NULL && special_keybinds[i] != key) i++;
    if (special_keys_en[i] == NULL) return NULL;
    switch(lang){
        case 0: ;
            return special_keys_en[i];
        case 1: ;
            return special_keys_es[i];
        default: ;
            printf("Language not set up!\n");
            return NULL;
    }
}

char *skip_text(UINT key){
    char *key_text = get_key(key);
    char *text;
    switch(lang){
        case 0: ;
            text = "Press %s to skip!";
            break;
        case 1: ;
            text = "Presiona %s para saltar!";
            break;
        default: ;
            printf("Language not set up!\n");
            return NULL;
    }
    char *out = (char *)malloc(strlen(text) - 1 + strlen(key_text));
    if (out == NULL){
        printf("Error allocating memory for skip text!\n");
        return NULL;
    }
    sprintf(out, text, key_text);
    free(key_text);
    return out;
}

char *main_menu_options_en[] = {
    "Start Game",
    "Options",
    "Credits",
    "Quit Game",
    NULL
};

char *main_menu_options_es[] = {
    "Comenzar Juego",
    "Opciones",
    "Creditos",
    "Cerrar Juego",
    NULL
};

char *save_menu_options_en[] = {
    "[] Load Save",
    "+ Create New Save",
    "- Delete Save",
    NULL
};

char *save_menu_options_es[] = {
    "[] Cargar Juego",
    "+ Crear Nuevo Juego",
    "- Borrar Juego",
    NULL
};

char *pause_menu_options_en[] = {
    "Resume Game",
    "Options",
    "Credits",
    "Back to Main Menu",
    "Quit Game",
    NULL
};

char *pause_menu_options_es[] = {
    "Resumir",
    "Opciones",
    "Creditos",
    "Volver a Menu Principal",
    "Cerrar Juego",
    NULL
};

wchar_t *option_menu_options_en[] = {
    L"Options",
    L"Language",
    L"English",
    L"Spanish",
    L"Show Intro",
    L"Back",
    L"Off",
    L"On",
    NULL
};

wchar_t *option_menu_options_es[] = {
    L"Opciones",
    L"Lenguaje",
    L"Inglés",
    L"Español",
    L"Mostrar Intro",
    L"Volver",
    L"No",
    L"Sí",
    NULL
};

char * get_list_at(int index, char *list[]){
    if ( index < 0 )return NULL;
    int i = 0;
    while (list[i] != NULL && i < index){
        i++;
    }
    return list[i];
}

wchar_t * get_list_at_es(int index, wchar_t *list[]){
    if ( index < 0 )return NULL;
    int i = 0;
    while (list[i] != NULL && i < index){
        i++;
    }
    return list[i];
}

char * get_mm_option_text(int index){
    switch(lang){
        case 0: ;
            return get_list_at(index, main_menu_options_en);
        case 1: ;
            return get_list_at(index, main_menu_options_es);
        default: ;
            printf("Language not set up!\n");
            return NULL;
    }
}

char * get_save_option_text(int index){
    if (index < 0) return NULL;
    int i = 0;
    switch(lang){
        case 0: ;
            return get_list_at(index, save_menu_options_en);
        case 1: ;
            return get_list_at(index, save_menu_options_es);
        default: ;
            printf("Language not set up!\n");
            return NULL;
    }
}

char * get_pause_option_text(int index){
    if (index < 0) return NULL;
    int i = 0;
    switch(lang){
        case 0: ;
            return get_list_at(index, pause_menu_options_en);
        case 1: ;
            return get_list_at(index, pause_menu_options_es);
        default: ;
            printf("Language not set up!\n");
            return NULL;
    }
}

wchar_t * get_options_option_text(int index){
    if (index < 0) return NULL;
    int i = 0;
    switch(lang){
        case 0: ;
            return get_list_at_es(index, option_menu_options_en);
        case 1: ;
            return get_list_at_es(index, option_menu_options_es);
        default: ;
            printf("Language not set up!\n");
            return NULL;
    }
}