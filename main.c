#include <stdio.h>
#include "vm.h"

int main(int argc, char *argv[]) {
    struct vm m = {0};
    if(argc < 2){
        printf("usage: vm program.bin\n");
        return 1;
    }
    FILE *f = fopen(argv[1], "rb");
    if(f == NULL){
        printf("file could not be opened\n");
        return 1;
    }
    size_t n = fread(m.memory, 1, CODE_END+1, f);
    fclose(f);
    if(!n){
        printf("file is empty\n");
        return 1;
    }
    m.registers[7] = 0;
    run(&m);

    if(argc > 2 && !strcmp(argv[2], "--grid")){
        for(int y = 0; y < 64; y++){
            for(int x = 0; x < 64; x++){
                int addr = DATA_START + (y*64 + x)*2;
                if(m.memory[addr+1]) putchar('#');
                else putchar('.');
            }
            putchar('\n');
        }    
    }
    

    return 0;
}