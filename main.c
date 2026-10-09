#include <stdint.h>
#include <stdio.h>

enum opcodes{HALT, ADD, SUB, MUL, DIV, LOADI};

struct vm{
    uint16_t registers[8];
    uint16_t counter;
    uint8_t flag;
    uint8_t halted;
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

void execute(struct vm *m, uint16_t inst){
    switch(opcode(inst)){
        case ADD:
            m->registers[dest(inst)] = m->registers[src1(inst)]+m->registers[src2(inst)];
            break;
        case SUB:
            m->registers[dest(inst)] = m->registers[src1(inst)]-m->registers[src2(inst)];
            break;
        case MUL:
            m->registers[dest(inst)] = m->registers[src1(inst)]*m->registers[src2(inst)];
            break;
        case DIV:
            m->registers[dest(inst)] = m->registers[src1(inst)]/m->registers[src2(inst)];
            break;
        case HALT:
            m->halted = 1;
            break;
        case LOADI:
            m->registers[dest(inst)] = fetch(m);
            break;
    }
}

void run(struct vm *m) {
    while(!m->halted){
        uint16_t inst = fetch(m);
        execute(m, inst);
    }
}

int main() {
    struct vm m = {0};
    m.memory[0] = 0x28; m.memory[3] = 0x02; m.memory[4] = 0x29; m.memory[7] = 0x03; m.memory[8] = 0x12; m.memory[9] = 0x20;
    run(&m);
    printf("%d\n", m.registers[2]);
    return 0;
}