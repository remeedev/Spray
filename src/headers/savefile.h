#include "generalvars.h"

#ifndef savefile
#define savefile

void init_save();

int save_int(char *name, int *value);
int save_float(char *name, float *value);
int save_double(char *name, double *value);
int save_str(char *name, char **value);
int save_long(char *name, long *value);

void write_save();

size_t get_data_size(char a);

void read_save(char *save_name);
int get_saves(char ***save_names, char ***file_names);
char * create_brand_new_save();
void* get_from_save(char *key);
int add_to_save(char *name, void *assigned_value, char value_type, int freeReady);
void remove_from_save(char *name);
int check_pending_overwrite(char *key);
void overwrite(char *key, void *value, int freeReady);
void add_static_save(char *key, void *value, char datatype);

void end_save(int final);

#endif