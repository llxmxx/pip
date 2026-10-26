#include "vm.h"

struct vm m;

void vm_reset(void) {
    memset(&m, 0, sizeof m);
    m.registers[7] = 0;
}

void vm_load_byte(int addr, int b){
    if(addr > CODE_END || addr < CODE_START) return;
    m.memory[addr] = b;
}

void vm_step(void) {
    if(!m.halted) step(&m);
}

int vm_reg(int i){
    if(i > 7 || i < 0) return 0;
    return m.registers[i];
}

int vm_pc(void){
    return m.counter;
}
int vm_flags(void){
    return m.flag;
}
int vm_halted(void){
    return m.halted;
}

int vm_mem(int addr){
    if(addr < 0 || addr > STACK_END) return 0;
    return m.memory[addr];
}

void vm_poke(int addr, int b){
    if(addr < DATA_START || addr > DATA_END) return;
    m.memory[addr] = b;
}