#ifndef savefile
#define savefile

void init_save();

int save_int(char *name, int *value);
int save_float(char *name, float *value);
int save_double(char *name, double *value);
int save_str(char *name, char **value);
int save_long(char *name, long *value);

void write_save();

void read_save(char *save_name);
int get_saves(char ***save_names, char ***file_names);
char * create_brand_new_save();

#endif