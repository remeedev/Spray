#include <math.h>

// Time srand was already set
#include <stdlib.h>

#define NULL 0
#define FALSE 0
#define TRUE 1

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
    double m, k;
} data_lookup;

data_lookup* curr_save = NULL;

int is_prime(int n){
    if (n < 2 || n % 2 == 0) return FALSE;
    if (n == 2) return TRUE;
    for (int i = 3; i < sqrt((double)n); i += 2){
        if (n % i == 0)return FALSE;
    }
    return TRUE;
}

int find_prime(int n){
    int curr_n = n;
    if (n % 2 == 0) curr_n += 1;
    while (!is_prime(curr_n)) curr_n+=2;
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

void create_empty_save(){
    if (curr_save != NULL) return;
    curr_save = (data_lookup *)malloc(sizeof(data_lookup));
    if (curr_save == NULL){
        printf("Unable to allocate space for current save!\n");
        return;
    }
    curr_save->size = 0;
    curr_save->capacity = find_prime(7);
    curr_save->m = rand();
    curr_save->k = rand();
    curr_save->searches = (save_data **)malloc(sizeof(save_data *)*curr_save->capacity);
    if (curr_save->searches == NULL){
        printf("Unable to allocate space for save table!\n");
        free(curr_save);
        return;
    }
    for (int i = 0; i < curr_save->capacity; i++){
        curr_save->searches[i] = (save_data *)malloc(sizeof(save_data));
        if (curr_save->searches[i] == NULL){
            printf("Error initializing hash table!\n");
            delete_save_lookup(curr_save);
            return;
        }
        curr_save->searches[i]->key = NULL;
        curr_save->searches[i]->obj = NULL;
        curr_save->searches[i]->datatype = (char) 0;
    }
}

void hash_key(char *key_name){
    if (curr_save == NULL) create_empty_save();
}

void add_to_lookup(char *name, void *assigned_value){

}

void save_int(char *name, int *value){

}

void save_float(char *name, float *value){

}

void save_double(char *name, double *value){

}

void save_str(char *name, char *value){

}

void read_save(char *file_name){

}

void write_save(){

}