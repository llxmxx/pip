#include <stdint.h>

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