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

static struct inst table[] = {
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

static struct label labels[256];
static int nlabels = 0;

static int parse_reg(const char *w){
    if(strlen(w) == 2 && w[0] == 'R' && w[1] >= '0' && w[1] < '8'){
        return (w[1] - '0');
    }
    else return -1;
}

static long parse_num(const char *w){
    char *end;
    long v = strtol(w, &end, 0);
    if(*end == '\0' && end != w && v >= 0 && v <= 65535){
        return v;
    }
    else return -1;
}

#define OUTCAP 24576
#define MAPCAP 65536

static uint8_t outbuf[OUTCAP];
static int outlen = 0;
static char mapbuf[MAPCAP];
static int maplen = 0;

static const char *src_text;
static int src_pos = 0;

static char *get_line(char *buf, int n){
    if(src_text[src_pos] == '\0') return NULL;
    int i = 0;
    while(src_text[src_pos] != '\0' && src_text[src_pos] != '\n' && i < n-2){
        buf[i++] = src_text[src_pos++];
    }
    if(src_text[src_pos] == '\n'){
        buf[i++] = '\n';
        src_pos++;
    }
    buf[i] = '\0';
    return buf;
}

static int emit(int b){
    if(outlen >= OUTCAP) return 0;
    outbuf[outlen++] = (uint8_t)b;
    return -1;
}

int web_assemble(const char *text){
    src_text = text;
    nlabels = 0;
    outlen = 0;
    maplen = 0;
    mapbuf[0] = '\0';

    char line[256]; int lineno = 0, addr = 0;
    for(int pass = 1; pass <= 2; pass++){
        src_pos = 0;
        lineno = 0;
        addr = 0;
        while(get_line(line, sizeof line) != NULL){
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
                if(pass == 1){
                    if(strlen(words[0]) >= 32){
                        printf("label too long on line %d\n", lineno);
                        return -1;
                    }
                    if(nlabels >= 256){
                        printf("too many labels\n");
                        return -1;
                    }
                    char *cl = strchr(words[0], ':');
                    *cl = '\0';
                    strcpy(labels[nlabels].name, words[0]);
                    labels[nlabels].addr = addr;
                    nlabels += 1;    
                }
                continue;
            }

            int found = -1;
            for(int i = 0; i < sizeof table/sizeof table[0]; i++){
                if(!strcmp(words[0], table[i].name)){
                    found = i;
                    break;
                }
            }

            if(found == -1){
                printf("unknown instruction %s on line %d\n", words[0], lineno);
                return -1;
            }
            if(pass == 1){
                if(table[found].shape == DI) addr += 4;
                else addr += 2;
                continue;
            }

            int d = 0, s1 = 0, s2 = 0;
            long num = -1;

            switch(table[found].shape){
                case DSS:
                    if(count != 4){
                        printf("line %d: %s needs 3 operands\n", lineno, words[0]);
                        return -1;
                    }
                    d = parse_reg(words[1]);
                    s1 = parse_reg(words[2]);
                    s2 = parse_reg(words[3]);
                    break;
                case NONE:
                    if(count != 1){
                        printf("line %d: %s needs 0 operands\n", lineno, words[0]);
                        return -1;
                    }
                    break;
                case S:
                    if(count != 2){
                        printf("line %d: %s needs 1 operand\n", lineno, words[0]);
                        return -1;
                    }
                    s1 = parse_reg(words[1]);
                    break;
                case DI:
                    if(count != 3){
                        printf("line %d: %s needs 2 operands\n", lineno, words[0]);
                        return -1;
                    }
                    d = parse_reg(words[1]);
                    num = parse_num(words[2]);
                    if(num == -1){
                        for(int i = 0; i < nlabels; i++){
                            if(!strcmp(words[2], labels[i].name)){
                                num = labels[i].addr;
                                break;
                            }
                        }
                    }
                    break;
                case D:
                    if(count != 2){
                        printf("line %d: %s needs 1 operand\n", lineno, words[0]);
                        return -1;
                    }
                    d = parse_reg(words[1]);
                    break;
                case DS:
                    if(count != 3){
                        printf("line %d: %s needs 2 operands\n", lineno, words[0]);
                        return -1;
                    }
                    d = parse_reg(words[1]);
                    s1 = parse_reg(words[2]);
                    break;
                case SS:
                    if(count != 3){
                        printf("line %d: %s needs 2 operands\n", lineno, words[0]);
                        return -1;
                    }
                    s1 = parse_reg(words[1]);
                    s2 = parse_reg(words[2]);
                    break;
            }

            if(d < 0 || s1 < 0 || s2 < 0 || (table[found].shape == DI && num < 0)){
                printf("line %d: bad operand\n", lineno);
                return -1;
            }

            if(maplen < MAPCAP-32){
                maplen += snprintf(mapbuf + maplen, MAPCAP-maplen, "%d %d\n", addr, lineno);
            }

            uint16_t word = (table[found].opcode << 11) | (d << 8) | (s1 << 5) | (s2 << 2);
            int ok = emit(word >> 8) && emit(word & 0xFF);
            if(table[found].shape == DI){
                ok = ok && emit(num >> 8) && emit(num & 0xFF);
            }
            if(!ok){
                printf("program too big (code memory is 24576bytes)\n");
                return -1;
            }
            addr += (table[found].shape == DI) ? 4 : 2;
        }
    }
    return outlen;
}

int web_byte(int i){
    if(i < 0 || i >= outlen) return 0;
    return outbuf[i];
}

const char *web_map(void){
    return mapbuf;
}