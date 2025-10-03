#ifndef language
#define language

int lang;

void change_lang_next();
void change_lang_prev();

int get_lang();

char *get_key(UINT key);
char *skip_text(UINT key);

char * get_mm_option_text(int index);
char * get_save_option_text(int index);
char * get_pause_option_text(int index);
wchar_t * get_options_option_text(int index);

#endif