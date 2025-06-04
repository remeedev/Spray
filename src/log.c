#include <stdio.h>

int WriteToLogStr(char *content){
    FILE *debugFile = fopen("./debug", "w");
    if (debugFile == NULL){
        printf("Error opening the debug file!\n");
    }

    fprintf(debugFile, content);
    fclose(debugFile);
}

int WriteToLogBits(unsigned char *content, size_t len){
    FILE *debugFile = fopen("./debug", "w");
    if (debugFile == NULL){
        printf("Error opening the debug file!\n");
        return 0;
    }
    if(fwrite(content, 1, len, debugFile) != len){
        printf("Unable to write to file!\n");
        fclose(debugFile);
        return 0;
    }
    fclose(debugFile);
    return len;
}