//Copyright (C) 2026  Giuseppe Caruso
#pragma once

#include <cstdint>

class Ppu {
    public:
        uint8_t LCDC; //FF40
        uint8_t STAT; //FF41
        uint8_t LY; //FF44

        Ppu();
        void tick(int cycles);
};