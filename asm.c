#include <stdio.h>
#include <string.h>
#include "vm.h"

int main(int argc, char *argv[]){
    if(argc < 3){
        printf("usage: asm input.asm output.bin\n");
        return 1;
    }
    FILE *in = fopen(argv[1], "r");
    if(in == NULL){
        printf("%s could not be opened\n", argv[1]);
        return 1;
    }
    FILE *out = fopen(argv[2], "wb");
    if(out == NULL){
        printf("%s could not be opened\n", argv[2]);
        fclose(in);
        return 1;
    }

    char line[256]; int lineno = 0;
    while(fgets(line, sizeof line, in) != NULL){
        lineno++;

        char *nl = strchr(line, '\n');
        if(nl != NULL) *nl = '\0';
        char *cm = strchr(line, ';');
        if(cm != NULL) *cm = '\0';

        char *words[4];
        int count = 0;

        char *w = strtok(line, " ,\t");
        while(w != NULL && count < 4){
            words[count] = w;
            count++;
            w = strtok(NULL, " ,\t");
        }
        if(count == 0) continue;

        printf("%d: ", lineno);
        for(int i = 0; i < count; i++){
            printf("[%s] ", words[i]);
        }
        printf("\n");
    }

    fclose(in);
    fclose(out);
    return 0;
}