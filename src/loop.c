#include "cpu.h"
#include "rom.h"
// Assume cpu, read(), write(), IMM(), ADC(), SBC(), LDA(), znflags() etc. are defined properly

void print_cpu_state(uint16_t pc) {
    printf("PC: %04X  A: %02X  Status: N:%d V:%d - B:%d D:%d I:%d Z:%d C:%d\n",
        pc,
        cpu.a,
        (cpu.status & 0x80) != 0,  // N flag
        (cpu.status & 0x40) != 0,  // V flag
        (cpu.status & 0x10) != 0,  // B flag (ignored here)
        (cpu.status & 0x08) != 0,  // D flag (decimal mode)
        (cpu.status & 0x04) != 0,  // I flag (interrupt disable)
        (cpu.status & 0x02) != 0,  // Z flag
        (cpu.status & 0x01) != 0   // C flag
    );
}
void load_prg(uint8_t prg[], size_t size,uint16_t mem_start){
    for (int i = 0; i < size; i++){
        cpu.memory[mem_start + i] = prg[i];
    }
}
int main() {
    init();

    if (!load_nes_rom("nestest.nes")) {
        printf("ERROR: FILE DOES NOT EXIST\n");
        return 1;
    }

    reset_cpu();

    int max_instructions = 50000;
    int instructions_executed = 0;
while (instructions_executed < max_instructions) {
    cpu_clock();

    if (cpu.cycles == 0) {
        // Instruction just finished, print state and check conditions
        uint16_t current_pc = cpu.pc;
        uint8_t opcode = read(current_pc);

        if (opcode == 0x00) {  // BRK - stop
            printf("BRK encountered. Stopping test.\n");
            break;
        }


        instructions_executed++;
    }
}

if (cpu.memory[0x6000] != 0xFF) {
            if (cpu.memory[0x6000] == 0x00) {
                printf("Nestest PASS ✅\n");
            } else {
                printf("Nestest FAIL ❌ Code: 0x%02X\n", cpu.memory[0x6000]);
            }
        }
printf("Final PC: %04X\n", cpu.pc);

    return 0;
}
