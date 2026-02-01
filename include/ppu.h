//Copyright (C) 2026  Giuseppe Caruso
#pragma once

#include <cstdint>

class Gb;

enum Mode {
    MODE0_HBLANK,
    MODE1_VBLANK,
    MODE2_OAM,
    MODE3_DRAW
};

class Ppu {
    public:
        uint32_t palette[4] = {0xE0F8D0FF, 0x88C070FF, 0x346856FF, 0x081820FF};
        uint32_t frame_buffer[160 * 144];

        uint8_t LCDC; //FF40
        uint8_t STAT; //FF41
        uint8_t LY; //FF44

        uint8_t SCX, SCY; //FF43, FF42

        int dots;
        Mode current_mode;

        Ppu(Gb* parent);
        void loadBackGround(uint8_t row);
        void tick(int cycles);
        uint8_t readMemory(uint16_t addr);
        void writeMemory(uint16_t addr, uint8_t value);

        Gb* gb;
};