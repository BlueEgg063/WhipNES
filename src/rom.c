#include "cpu.h"
#include "rom.h"
int load_nes_rom(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open %s\n", filename);
        return 0;
    }

    unsigned char header[16];
    if (fread(header, 1, 16, f) != 16) {
        fprintf(stderr, "Error: Failed to read iNES header\n");
        fclose(f);
        return 0;
    }

    if (header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1A) {
        fprintf(stderr, "Error: Not a valid NES file\n");
        fclose(f);
        return 0;
    }

    int prg_banks = header[4];
    int prg_size = prg_banks * 16384; // 16KB per bank

    if (fread(&cpu.memory[0x8000], 1, prg_size, f) != prg_size) {
        fprintf(stderr, "Error: Failed to read PRG-ROM data\n");
        fclose(f);
        return 0;
    }
    fclose(f);

    // Mirror 16KB PRG-ROM if only one bank (NROM-128)
    if (prg_banks == 1) {
        for (int i = 0; i < 0x4000; i++) {
            cpu.memory[0xC000 + i] = cpu.memory[0x8000 + i];
        }
    }

    return 1; // Success
}
