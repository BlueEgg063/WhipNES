#ifndef PPU_H
#define PPU_H
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
enum PPU_REGS{
    PPUCTRL = 0,
    PPUMASK = 1,
    PPUSTATUS = 2,
    OAMADDR = 3,
    OAMDATA = 4,
    PPUSCROLL = 5,
    PPUADDR = 6,
    PPUDATA = 7
};
typedef struct {
    uint8_t registers[8];
    uint8_t OAMDMA;
    uint16_t v;
    uint16_t t;
    uint8_t x;
    bool w;
} PPU;
extern PPU ppu;
void ppu_write(uint16_t addr, uint8_t value);
uint8_t ppu_read(uint16_t addr);
#endif