/*
SAVE FILE LOGIC

Hash Table for quick lookup

DATATYPES (denoted by single char of ascii):
1 - int
2 - float
3 - double
4 - string
*/

#include <math.h>
#include <string.h>
#include <windows.h>

// Time srand was already set
#include <stdlib.h>
#include <stdio.h>

char *save_folder = "./save";

typedef struct save_data{
    char datatype;
    char *key;
    void *obj;
}save_data;

typedef struct data_lookup{
    save_data **searches;
    int size;
    int capacity;
    double k;
} data_lookup;

data_lookup* curr_save = NULL;

int is_prime(int n){
    if (n < 2 || n % 2 == 0) return FALSE;
    if (n < 4) return TRUE;
    for (int i = 5; i < sqrt((double)n); i += 2){
        if (n % i == 0)return FALSE;
    }
    return TRUE;
}

int find_prime(int n){
    int curr_n = n + 1;
    if (curr_n % 2 == 0) curr_n += 1;
    while (!is_prime(curr_n)) {
        curr_n+=2;
    }
    return curr_n;
}

void delete_save_lookup(data_lookup *save){
    if (save == NULL) return;
    for (int i = 0; i < save->capacity; i++){
        if (save->searches[i] == NULL) {
            continue;
        }
        if (save->searches[i]->key) free(save->searches[i]->key);
        free(save->searches[i]);
    }
    free(save);
    free(save->searches);
}

data_lookup *create_empty_save(int capacity){
    data_lookup* out_lookup = (data_lookup *)malloc(sizeof(data_lookup));
    if (out_lookup == NULL){
        printf("Unable to allocate space for current save!\n");
        return NULL;
    }
    out_lookup->size = 0;
    out_lookup->capacity = find_prime(capacity);
    out_lookup->k = rand();
    out_lookup->searches = (save_data **)malloc(sizeof(save_data *)*out_lookup->capacity);
    if (out_lookup->searches == NULL){
        printf("Unable to allocate space for save table!\n");
        free(out_lookup);
        return NULL;
    }
    for (int i = 0; i < out_lookup->capacity; i++){
        out_lookup->searches[i] = (save_data *)malloc(sizeof(save_data));
        if (out_lookup->searches[i] == NULL){
            printf("Error initializing hash table!\n");
            delete_save_lookup(out_lookup);
            return NULL;
        }
        out_lookup->searches[i]->key = NULL;
        out_lookup->searches[i]->obj = NULL;
        out_lookup->searches[i]->datatype = (char) 0;
    }
    return out_lookup;
}

void init_save(){
    curr_save = create_empty_save(7);
}

int hash_key(char *key_name){
    if (key_name == NULL) return -1;
    if (curr_save == NULL) curr_save = create_empty_save(7);
    // fractional part of [weighted sum * k] times capacity
    double curr_value = 0.0;
    int curr = 0;
    while (key_name[curr] != '\0'){
        curr_value += (curr + 1)*(key_name[curr]);
        curr++;
    }
    curr_value *= curr_save->k;
    curr_value = fmod(curr_value, 1.0);
    curr_value *= curr_save->capacity;
    curr_value = floor(curr_value);
    int pos = (int) curr_value;
    if (curr_save->searches[pos]->key != NULL){
        while (curr_save->searches[pos]->key != NULL && strcmp(key_name, curr_save->searches[pos]->key) != 0) pos = (pos + 1)%curr_save->capacity; // Find available pos or actual pos
    }
    return pos;
}

data_lookup* expand_lookup(data_lookup* l_u);

void add_to_lookup(data_lookup *l_u, char *name, void *assigned_value, char value_type){
    int add_position = hash_key(name);
    if (l_u->searches[add_position]->key != NULL){
        printf("Double assignment to same key name!\n");
        printf("Tried to write %s but found %s\n", name, l_u->searches[add_position]->key);
        return;
    }
    save_data *set_pos = l_u->searches[add_position];
    set_pos->key = (char *)malloc(strlen(name) + 1);
    strcpy(set_pos->key, name);
    set_pos->obj = assigned_value;
    set_pos->datatype = value_type;
    l_u->size++;
    if (l_u->size == l_u->capacity){
        l_u = expand_lookup(l_u);
    }
}

void remove_from_lookup(data_lookup *l_u, char *key){
    int add_position = hash_key(key);
    if (l_u->searches[add_position]->key != NULL){
        free(l_u->searches[add_position]->key);
        l_u->searches[add_position]->key = NULL;
        l_u->searches[add_position]->obj = NULL;
        l_u->searches[add_position]->datatype = (char) 0;
    }
}

save_data* get_from_lookup(data_lookup *l_u, char *key){
    int add_position = hash_key(key);
    if (l_u->searches[add_position]->key != NULL){
        return l_u->searches[add_position];
    }
    return NULL;
}

data_lookup* expand_lookup(data_lookup* l_u){
    if (l_u == NULL) return NULL;
    data_lookup *tmp = create_empty_save(l_u->capacity * 2);
    int i = 0;
    while (i < l_u->capacity){
        if (l_u->searches[i]->key != NULL){
            add_to_lookup(tmp, l_u->searches[i]->key, l_u->searches[i]->obj, l_u->searches[i]->datatype);
        }
    }
    delete_save_lookup(l_u);
    return tmp;
}

void save_int(char *name, int *value){
    add_to_lookup(curr_save, name, value, (char) 1);
}

void save_float(char *name, float *value){
    add_to_lookup(curr_save, name, value, (char) 2);
}

void save_double(char *name, double *value){
    add_to_lookup(curr_save, name, value, (char) 3);
}

void save_str(char *name, char **value){
    add_to_lookup(curr_save, name, value, (char) 4);
}

void save_long(char *name, long *value){
    add_to_lookup(curr_save, name, value, (char) 5);
}

size_t get_data_size(char a){
    switch (a) {
        case 0:
            return -1;
        case 1:
            return sizeof(int);
        case 2:
            return sizeof(float);
        case 3:
            return sizeof(double);
        case 4:
            return sizeof(char *);
        case 5:
            return sizeof(long);
        default:
            return -1;
    }
}

unsigned char *raw_int(int a){
    unsigned char *out = (unsigned char *)malloc( sizeof( int ) );
    memcpy(out, &a, sizeof(int));
    return out;
}

unsigned char *raw_float(float a){
    unsigned char *out = (unsigned char *)malloc( sizeof( float ) );
    memcpy(out, &a, sizeof(float));
    return out;
}

unsigned char *raw_double(double a){
    unsigned char *out = (unsigned char *)malloc( sizeof( double ) );
    memcpy(out, &a, sizeof(double));
    return out;
}

unsigned char *raw_long(long a){
    unsigned char *out = (unsigned char *)malloc( sizeof( long ) );
    memcpy(out, &a, sizeof(long));
    return out;
}

void write_save(){
    char *save_name = "/test_save.dat";
    char *file_name = (char *)malloc(strlen(save_folder) + strlen(save_name) + 1);
    if (file_name == NULL){
        printf("There was an error allocating space for save folder!\n");
        return;
    }
    strcpy(file_name, save_folder);
    strcat(file_name, save_name);
    FILE *ptr = fopen(file_name, "wb");

    unsigned char *raw_data = NULL;
    size_t data_length = 0;

    int i = 0;
    while (i < curr_save->capacity){
        if (curr_save->searches[i]->key != NULL){
            save_data *elem = curr_save->searches[i];
            size_t data_size = get_data_size(elem->datatype);
            size_t value_len = 0;
            if (elem->datatype == (char)4){
                char *elem_value = *(char **)(elem->obj);
                value_len = strlen(elem_value);
            }else{
                value_len = data_size;
            }
            size_t write_length = sizeof(int) + strlen(elem->key) + sizeof(char) + sizeof(int) + value_len; // Size of name, name, datatype, value_len, value
            unsigned char *write_data = (unsigned char *)malloc(write_length);
            if (write_data == NULL){
                printf("Error creating the save file!\n");
                return;
            }
            int write_pos = 0;
            
            // Write the name size
            unsigned char *raw_name_size = raw_int(strlen(elem->key));
            for (int i = 0; i < 4; i++){
                write_data[write_pos] = raw_name_size[write_pos];
                write_pos++;
            }
            free(raw_name_size);
            // Write the name
            for (int i = 0; i < strlen(elem->key); i++){
                write_data[write_pos] = elem->key[i];
                write_pos++;
            }
            // Write the data type
            write_data[write_pos++] = elem->datatype;

            // Write the value len
            unsigned char *raw_data_size = raw_int(value_len);
            for (int i = 0; i < 4; i++){
                write_data[write_pos] = raw_data_size[i];
                write_pos++;
            }
            free(raw_data_size);
            unsigned char *raw_data_value = NULL;
            switch (elem->datatype){
                case 1: ;
                    int int_value_ptr = *(int *)elem->obj;
                    raw_data_value = raw_int(int_value_ptr);
                    break;
                case 2: ;
                    float float_value_ptr = *(float *)elem->obj;
                    raw_data_value = raw_float(float_value_ptr);
                    break;
                case 3: ;
                    double double_value_ptr = *(double *)elem->obj;
                    raw_data_value = raw_double(double_value_ptr);
                    break;
                case 4: ;
                    char *value_str = *(char **)elem->obj;
                    raw_data_value = (char *)value_str;
                    break;
                case 5: ;
                    long long_value_ptr = *(long *)elem->obj;
                    raw_data_value = raw_long(long_value_ptr);
                    break;
                default: ;
                    printf("Found an unknown datatype in the save file!\n");
                    return;
            }
            // Write the value
            for (int i = 0; i < value_len; i++){
                write_data[write_pos] = raw_data_value[i];
                write_pos++;
            }
            if (elem->datatype != (char) 4){
                free(raw_data_value);
            }

            // Write the data to the out char
            if (raw_data == NULL){
                raw_data = (unsigned char *)malloc(write_length);
                if (raw_data == NULL){
                    printf("There was an error allocating memory for raw data!\n");
                    return;
                }
                for (int i = 0; i < write_length; i++){
                    raw_data[data_length] = write_data[i];
                    data_length++;
                }
            }else{
                unsigned char *tmp = (unsigned char *)realloc(raw_data, data_length + write_length);
                if (tmp == NULL){
                    printf("There was an error reallocating memory for raw data!\n");
                    return;
                }
                raw_data = tmp;
                for (int i = 0; i < write_length; i++){
                    raw_data[data_length] = write_data[i];
                    data_length++;
                }
            }
        }
        i++;
    }
    fwrite(raw_data, sizeof(unsigned char), data_length, ptr);

    fclose(ptr);
}

void read_save(char *save_name){
    char *file_name = (char *)malloc(strlen(save_folder) + strlen(save_name) + 1);
    if (file_name == NULL){
        printf("There was an error allocating space for save folder!\n");
        return;
    }
    strcpy(file_name, save_folder);
    strcat(file_name, save_name);
    FILE *ptr = fopen(file_name, "rb");
    if (ptr == NULL){
        printf("Error opening save file!\n");
        return;
    }
    int name_size = 0;
    while (fread(&name_size, sizeof(int), 1, ptr)){
        char *name = (char *)malloc(name_size + 1);
        for (int i = 0; i < name_size; i++){
            name[i] = fgetc(ptr);
        }
        name[name_size] = '\0';

        char datatype = fgetc(ptr);
        save_data *elem = get_from_lookup(curr_save, name);
        if (elem == NULL){
            printf("Didn't find corresponding variable to %s\n", name);
            free(name);
            fclose(ptr);
            return;
        }
        int value_size = 0;
        fread(&value_size, sizeof(int), 1, ptr);
        if (datatype == 4) {
            char **to_write = (char **)elem->obj;
            *to_write = (char *)malloc(value_size + 1);
            if (*to_write == NULL){
                printf("There was an error unpacking the values from the save!\n");
                return;
            }
            for (int i = 0; i < value_size; i++){
                (*to_write)[i] = fgetc(ptr);
            }
            (*to_write)[value_size] = '\0';
        }else{
            fread(elem->obj, value_size, 1, ptr);
        }
        free(name);
    }
    fclose(ptr);
}