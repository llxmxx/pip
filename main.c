#include <stdio.h>
#include "vm.h"

struct vm{
    uint16_t registers[8];
    uint16_t counter;
    uint8_t flag;
    uint8_t halted;
    uint8_t memory[65536];
};

uint16_t fetch(struct vm *m) {
    if(m->counter >= CODE_END){
        printf("pc left code region: %d\n", m->counter);
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
            if(addr == STACK_END){
                printf("address out of bounds: %d\n", addr);
                m->halted = 1;
                return;
            }
            m->registers[dest(inst)] = (m->memory[addr] << 8) | m->memory[addr+1];
            break;
        }
        case STORE:{
            uint16_t addr = m->registers[dest(inst)];
            if(addr < DATA_START){
                printf("address not in data: %d\n", addr);
                m->halted = 1;
                return;
            }
            else if(addr == STACK_END){
                printf("address out of bounds: %d\n", addr);
                m->halted = 1;
                return;
            }
            uint16_t value = m->registers[src1(inst)];
            m->memory[addr] = (value >> 8);
            m->memory[addr + 1] = (value & 0xFF);
            break;
        }
        case MOV:
            m->registers[dest(inst)] = m->registers[src1(inst)];
            break;
        case CALL:
            if((uint16_t)(m->registers[7]-2) < STACK_START){
                printf("stack overflow: %d\n", m->counter-2);
                m->halted = 1;
                return;
            }
            m->registers[7] -= 2;
            m->memory[m->registers[7]] = m->counter>>8;
            m->memory[m->registers[7]+1] = m->counter&0xFF;
            m->counter = m->registers[dest(inst)];
            break;
        case RET:
            if(!m->registers[7]){
                printf("RET with empty stack: %d\n", m->counter-2);
                m->halted = 1;
                return;
            }
            m->counter = (m->memory[m->registers[7]] << 8) | m->memory[m->registers[7]+1];
            m->registers[7] += 2;
            break;
        case AND:
            m->registers[dest(inst)] = m->registers[src1(inst)] & m->registers[src2(inst)];
            break;
        case OR:
            m->registers[dest(inst)] = m->registers[src1(inst)] | m->registers[src2(inst)];
            break;
        case XOR:
            m->registers[dest(inst)] = m->registers[src1(inst)] ^ m->registers[src2(inst)];
            break;
        case NOT:
            m->registers[dest(inst)] = ~m->registers[src1(inst)];
            break;
        case SHL:
            if(m->registers[src2(inst)] >= 16) m->registers[dest(inst)] = 0;
            else m->registers[dest(inst)] = m->registers[src1(inst)] << m->registers[src2(inst)];
            break;
        case SHR:
            if(m->registers[src2(inst)] >= 16) m->registers[dest(inst)] = 0;
            else m->registers[dest(inst)] = m->registers[src1(inst)] >> m->registers[src2(inst)];
            break;
        case PRINT:
            printf("%d\n", m->registers[src1(inst)]);
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

int main(int argc, char *argv[]) {
    struct vm m = {0};
    if(argc < 2){
        printf("usage: vm program.bin\n");
        return 1;
    }
    FILE *f = fopen(argv[1], "rb");
    if(f == NULL){
        printf("file couldnt be opened\n");
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
    return 0;
}