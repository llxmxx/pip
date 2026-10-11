#include <stdint.h>
#include <stdio.h>

#define FLAG_OVERFLOW 1
#define FLAG_ZERO 2
#define FLAG_LESS 4

enum opcodes{HALT, ADD, SUB, MUL, DIV, LOADI, CMP, JMP, JZ, JNZ, JEQ, JNE, JLT, JGT, JLE, JGE, LOAD, STORE, MOV};

struct vm{
    uint16_t registers[8];
    uint16_t counter;
    uint8_t flag;
    uint8_t halted;
    uint8_t memory[65536];
};

uint16_t fetch(struct vm *m) {
    if(m->counter >= 65535){
        printf("exceeds memory: %d\n", m->counter);
        m->halted = 1;
        return 0;
    }
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
        case ADD:{
            uint32_t result = m->registers[src1(inst)]+m->registers[src2(inst)];
            if(result > 65535) m->flag |= FLAG_OVERFLOW;
            else m->flag &= ~FLAG_OVERFLOW;
            m->registers[dest(inst)] = result;
            break;
        }
        case SUB:
            m->registers[dest(inst)] = m->registers[src1(inst)]-m->registers[src2(inst)];
            break;
        case MUL:{
            uint32_t result = (uint32_t)m->registers[src1(inst)]*m->registers[src2(inst)];
            if(result > 65535) m->flag |= FLAG_OVERFLOW;
            else m->flag &= ~FLAG_OVERFLOW;
            m->registers[dest(inst)] = result;
            break;
        }
        case DIV:{
            uint16_t src_2 = m->registers[src2(inst)];
            if(!src_2){
                printf("ERR: divide by zero: %d\n", m->counter-2);
                m->halted = 1;
            }
            else m->registers[dest(inst)] = m->registers[src1(inst)]/src_2;
            break;
        }
        case HALT:
            m->halted = 1;
            break;
        case LOADI:
            m->registers[dest(inst)] = fetch(m);
            break;
        case CMP:{
            m->flag &= ~FLAG_ZERO;
            m->flag &= ~FLAG_LESS;
            if(m->registers[src1(inst)] == m->registers[src2(inst)]) m->flag |= FLAG_ZERO;
            else if(m->registers[src1(inst)] < m->registers[src2(inst)]) m->flag |= FLAG_LESS;
            break;
        }
        case JMP:
            m->counter = m->registers[dest(inst)];
            break;
        case JZ:
            if(!m->registers[src1(inst)]) m->counter = m->registers[dest(inst)];
            break;
        case JNZ:
            if(m->registers[src1(inst)]) m->counter = m->registers[dest(inst)];
            break;
        case JEQ:
            if((m->flag & FLAG_ZERO) == FLAG_ZERO) m->counter = m->registers[dest(inst)];
            break;
        case JNE:
            if((m->flag & FLAG_ZERO) != FLAG_ZERO) m->counter = m->registers[dest(inst)];
            break;
        case JLT:
            if((m->flag & FLAG_LESS) == FLAG_LESS) m->counter = m->registers[dest(inst)];
            break;
        case JGT:
            if((m->flag & FLAG_ZERO) != FLAG_ZERO && (m->flag & FLAG_LESS) != FLAG_LESS) m->counter = m->registers[dest(inst)];
            break;
        case JLE:
            if((m->flag & FLAG_LESS) == FLAG_LESS || (m->flag & FLAG_ZERO) == FLAG_ZERO) m->counter = m->registers[dest(inst)];
            break;
        case JGE:
            if((m->flag & FLAG_LESS) != FLAG_LESS) m->counter = m->registers[dest(inst)];
            break;
        case LOAD:{
            uint16_t addr = m->registers[src1(inst)];
            m->registers[dest(inst)] = (m->memory[addr] << 8) | m->memory[addr+1];
            break;
        }
        case STORE:{
            uint16_t addr = m->registers[dest(inst)];
            uint16_t value = m->registers[src1(inst)];
            m->memory[addr] = (value >> 8);
            m->memory[addr + 1] = (value & 0xFF);
            break;
        }
        case MOV:
            m->registers[dest(inst)] = m->registers[src1(inst)];
            break;
        default:
            printf("undefined opcode %d at %d\n", opcode(inst), m->counter-2);
            m->halted = 1;
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
    m.memory[0] = 0x28; m.memory[4] = 0x29; m.memory[7] = 0x01; m.memory[8] = 0x2A; m.memory[11] = 0x0A; m.memory[12] = 0x2B; m.memory[15] = 0x10; m.memory[16] = 0x08; m.memory[17] = 0x04; m.memory[18] = 0x30; m.memory[19] = 0x08; m.memory[20] = 0x63; m.memory[22] = 0x00;
    run(&m);
    printf("%d\n", m.registers[0]);
    return 0;
}