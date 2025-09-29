#include "headers/generalvars.h"

#include <windows.h>
#include <stdio.h>
#include <wchar.h>

int weed_bags = 5;

wchar_t *weed_convos = L"Do you have any weed, I could buy from you?";
char *options[] = {"Yes, I have some on me!", "I'm currently not selling.", NULL};

size_t get_strlist_length(char **in){
    size_t len = 0;
    while (in[len] != NULL) len++;
    return len;
}

conversation *have_weed = NULL;
conversation *no_weed = NULL;

conversation *create_text(char *line){
    conversation *out = (conversation *)malloc(sizeof(conversation));
    if (out == NULL){
        printf("There was an error allocating space for text!\n");
        return NULL;
    }
    out->image = NULL;
    out->next = NULL;
    out->option_count = 0;
    out->option_handler = NULL;
    out->option_selected = 0;
    out->options = NULL;
    out->skip_check = NULL;
    wchar_t *out_line = (wchar_t *)malloc((strlen(line)+1)*sizeof(wchar_t));
    if (out_line == NULL){
        printf("There was an error allocating space for out_line!\n");
        return NULL;
    }
    for (int i = 0; i < strlen(line); i++){
        out_line[i] = (wchar_t)line[i];
    }
    out_line[strlen(line)] = L'\0';
    out->end = strlen(line);
    out->line = out_line;
    return out;
}

int check_weed(){
    return weed_bags == 0;
}

void start_weed_convos(){
    have_weed = create_text(options[0]);
    no_weed = create_text(options[1]);
}

conversation * process_weed_sale(conversation *curr_conv){
    if (curr_conv->option_selected == 0) {
        weed_bags--;
        return have_weed;
    }
    if (curr_conv->option_selected == 1) {
        return no_weed;
    }
}

void create_weed_sale(conversation *add_location, SpriteGroup *characters){
    conversation *new_add = (conversation *)malloc(sizeof(conversation));
    if (new_add == NULL){
        printf("There was an error allocating space for weed sale!\n");
        return;
    }
    new_add->image = NULL;
    size_t str_len = wcslen(weed_convos);
    wchar_t *line = (wchar_t *)malloc((str_len + 1)*sizeof(wchar_t));
    wcscpy(line, weed_convos);
    SpriteGroup* curr_char = characters;
    new_add->image = curr_char->next->sprite; // second
    new_add->line = line;
    new_add->end = str_len;
    new_add->next = NULL;
    new_add->option_count = get_strlist_length(options);
    new_add->options = options;
    new_add->option_handler = &process_weed_sale;
    new_add->option_selected = 0;
    new_add->skip_check = &check_weed;
    conversation *available_space = add_location;
    while (available_space->next) available_space = available_space->next;
    available_space->next = new_add;
}