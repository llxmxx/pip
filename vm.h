#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FLAG_OVERFLOW 1
#define FLAG_ZERO 2
#define FLAG_LESS 4
#define CODE_START 0x0000
#define CODE_END 0x5FFF
#define DATA_START 0x6000
#define DATA_END 0xBFFF
#define STACK_START 0xC000
#define STACK_END 0xFFFF

enum opcodes{
    HALT,
    ADD, SUB, MUL, DIV,
    LOADI,
    CMP,
    JMP, JZ, JNZ, JEQ, JNE, JLT, JGT, JLE, JGE,
    LOAD, STORE, MOV,
    CALL, RET,
    AND, OR, XOR, NOT, SHL, SHR,
    PRINT
};

#endif