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

    char line[256]; int lineno = 1;
    while(fgets(line, sizeof line, in) != NULL){
        char *nl = strchr(line, '\n');
        if(nl != NULL) *nl = '\0';
        char *cm = strchr(line, ';');
        if(cm != NULL) *cm = '\0';
        printf("%d: %s\n", lineno, line);
        lineno++;
    }

    fclose(in);
    fclose(out);
    return 0;
}