//Copyright (C) 2026  Giuseppe Caruso
#pragma once

#include <string>
#include <sm83.h>

#define MEMORY_SIZE 0x10000

/* GB Memory map
0000 to 3FFF: 16 Kib ROM Bank 00
4000 to 7FFF: 16 Kib ROM Bank 01-NN
8000 to 9FFF: 8 Kib VRAM
A000 to BFFF: 8 Kib External RAM
C000 to CFFF: 4 Kib WRAM
D000 to DFFF: 4 Kib WRAM
E000 to FDFF: Echo RAM (Nintendo says use of this RAM area is prohibited. It's a mirror of C000 - DDFF)
FE00 to FE9F: OAM
FEA0 to FEFF: Not usable (Once again, Nintendo says this area is prohibited)
FF00 to FF7F: IO Registers
FF80 to FFFE: HRAM
FFFF to FFFF: IE Register
*/

class Gb {
    public:
        Cpu cpu;
        uint8_t memory [MEMORY_SIZE] = {0};

        uint16_t internal_counter;

        void loadRom(std::string path);
        uint8_t readMemory(uint16_t addr);
        void writeMemory(uint16_t addr, uint8_t value);

        Gb();
};