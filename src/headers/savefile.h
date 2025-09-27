#ifndef savefile
#define savefile

void init_save();

void save_int(char *name, int *value);
void save_float(char *name, float *value);
void save_double(char *name, double *value);
void save_str(char *name, char **value);
void save_long(char *name, long *value);

void write_save();

void read_save(char *save_name);

#endif