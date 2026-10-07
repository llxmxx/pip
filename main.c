#include <stdint.h>

enum opcodes{HALT, ADD, SUB, MUL, DIV, LOADI};

struct vm{
    uint16_t registers[8];
    uint16_t counter;
    uint8_t flag;
    uint8_t memory[65536];
};

uint16_t fetch(struct vm *m) {
    uint16_t inst = ((m->memory[m->counter]) << 8) | (m->memory[m->counter+1]);
    m->counter += 2;
    return inst;
}

uint8_t opcode(uint16_t inst){
    return inst >> 11;
}

uint8_t dest(uint16_t inst){
    return (inst >> 8) & 0x7;
}

uint8_t src1(uint16_t inst){
    return (inst >> 5) & 0x7;
}

uint8_t src2(uint16_t inst){
    return (inst >> 2) & 0x7;
}

int main() {
    return 0;
} 