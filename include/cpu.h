#ifndef CPU_H
#define CPU_H
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
typedef void(*opcode)(void);
typedef void(*addrmode)(void);
typedef struct{
const char* mnemonic;
opcode op;
addrmode addr;
uint8_t cycles;
} Instruction;
typedef struct{
    uint16_t pc; //program counter
    uint8_t a, x, y; //registers
    uint8_t status; //status flags
    uint8_t fetched; //byte fetched from memory
    int8_t addr_rel; //addr for branch instructions
    uint16_t addr_abs; //absolute unsigned addr for other instructions
    int cycles; //the main decrementing cycle count
    int extracycles;//extra cycles added in main loop
    bool ACU; //Is current instruction operating directly on A?
    uint8_t sp; //stack pointer
    uint8_t memory[65536]; //64KB memory in a flat array. No mmio here, but it will be handled by memread/memwrite
} CPU;
extern CPU cpu;
void init();
void cpu_clock();
uint8_t read(uint16_t addr);
void reset_cpu();
#endif