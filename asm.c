#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vm.h"

enum shape {NONE, D, S, DS, SS, DSS, DI};

struct inst{
    const char *name;
    int opcode;
    enum shape shape;
};

struct label {
    char name[32];
    int addr;
};

struct inst table[] = {
    {"HALT", HALT, NONE},
    {"RET", RET, NONE},
    {"ADD", ADD, DSS},
    {"SUB", SUB, DSS},
    {"MUL", MUL, DSS},
    {"DIV", DIV, DSS},
    {"AND", AND, DSS},
    {"OR", OR, DSS},
    {"XOR", XOR, DSS},
    {"SHL", SHL, DSS},
    {"SHR", SHR, DSS},
    {"LOADI", LOADI, DI},
    {"JMP", JMP, D},
    {"JLT", JLT, D},
    {"JEQ", JEQ, D},
    {"JNE", JNE, D},
    {"JGT", JGT, D},
    {"JLE", JLE, D},
    {"JGE", JGE, D},
    {"CALL", CALL, D},
    {"NOT", NOT, DS},
    {"LOAD", LOAD, DS},
    {"STORE", STORE, DS},
    {"MOV", MOV, DS},
    {"JZ", JZ, DS},
    {"JNZ", JNZ, DS},
    {"CMP", CMP, SS},
    {"PRINT", PRINT, S}
};

struct label labels[256];
int nlabels = 0;

int parse_reg(const char *w){
    if(strlen(w) == 2 && w[0] == 'R' && w[1] >= '0' && w[1] < '8'){
        return (w[1] - '0');
    }
    else return -1;
}

long parse_num(const char *w){
    char *end;
    long v = strtol(w, &end, 0);
    if(*end == '\0' && end != w && v >= 0 && v <= 65535){
        return v;
    }
    else return -1;
}

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

    char line[256]; int lineno = 0, addr = 0;
    while(fgets(line, sizeof line, in) != NULL){
        lineno++;

        char *nl = strchr(line, '\n');
        if(nl != NULL) *nl = '\0';
        char *cm = strchr(line, ';');
        if(cm != NULL) *cm = '\0';

        char *words[4];
        int count = 0;

        char *w = strtok(line, " ,\t[]");
        while(w != NULL && count < 4){
            words[count] = w;
            count++;
            w = strtok(NULL, " ,\t[]");
        }
        if(count == 0) continue;
        else if(count == 1 && words[0][strlen(words[0]) - 1] == ':'){
            if(strlen(words[0]) >= 32){
                printf("label too long on line %d\n", lineno);
                fclose(in); fclose(out);
                return 1;
            }
            if(nlabels >= 256){
                printf("too many labels\n");
                fclose(in); fclose(out);
                return 1;
            }
            char *cl = strchr(words[0], ':');
            *cl = '\0';
            strcpy(labels[nlabels].name, words[0]);
            labels[nlabels].addr = addr;
            nlabels += 1;
            continue;
        }

        int found = -1;
        for(int i = 0; i < sizeof table; i++){
            if(!strcmp(words[0], table[i].name)){
                found = i;
                break;
            }
        }

        if(found == -1){
            printf("unknown instruction %s on line %d\n", words[0], lineno);
        }

        int d = 0, s1 = 0, s2 = 0;
        long num = -1;

        switch(table[found].shape){
            case DSS:
                if(count != 4){
                    printf("line %d: %s needs 3 operands\n", lineno, words[0]);
                    fclose(in); fclose(out);
                    return 1;
                }
                d = parse_reg(words[1]);
                s1 = parse_reg(words[2]);
                s2 = parse_reg(words[3]);
                break;
            case NONE:
                if(count != 1){
                    printf("line %d: %s needs 0 operands\n", lineno, words[0]);
                    fclose(in); fclose(out);
                    return 1;
                }
                break;
            case S:
                if(count != 2){
                    printf("line %d: %s needs 1 operand\n", lineno, words[0]);
                    fclose(in); fclose(out);
                    return 1;
                }
                s1 = parse_reg(words[1]);
                break;
            case DI:
                if(count != 3){
                    printf("line %d: %s needs 2 operands\n", lineno, words[0]);
                    fclose(in); fclose(out);
                    return 1;
                }
                d = parse_reg(words[1]);
                num = parse_num(words[2]);
                break;
            case D:
                if(count != 2){
                    printf("line %d: %s needs 1 operand\n", lineno, words[0]);
                    fclose(in); fclose(out);
                    return 1;
                }
                d = parse_reg(words[1]);
                break;
            case DS:
                if(count != 3){
                    printf("line %d: %s needs 2 operands\n", lineno, words[0]);
                    fclose(in); fclose(out);
                    return 1;
                }
                d = parse_reg(words[1]);
                s1 = parse_reg(words[2]);
                break;
            case SS:
                if(count != 3){
                    printf("line %d: %s needs 2 operands\n", lineno, words[0]);
                    fclose(in); fclose(out);
                    return 1;
                }
                s1 = parse_reg(words[1]);
                s2 = parse_reg(words[2]);
                break;
        }

        if(d < 0 || s1 < 0 || s2 < 0 || (table[found].shape == DI && num < 0)){
            printf("line %d: bad operand\n", lineno);
            fclose(in); fclose(out);
            return 1;
        }

        uint16_t word = (table[found].opcode << 11) | (d << 8) | (s1 << 5) | (s2 << 2);
        fputc(word >> 8, out);
        fputc(word & 0xFF, out);
        if(table[found].shape == DI){
            fputc(num >> 8, out);
            fputc(num & 0xFF, out);
        }

        if(table[found].shape == DI) addr += 4;
        else addr += 2;
    }

    fclose(in);
    fclose(out);
    return 0;
}