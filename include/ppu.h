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

struct Object {
    uint8_t Y;
    uint8_t X;
    uint8_t tile_idx;
    uint8_t flags;
};

class Ppu {
    public:
        uint32_t palette[4] = {0xE0F8D0FF, 0x88C070FF, 0x346856FF, 0x081820FF};
        uint32_t frame_buffer[160 * 144];
        std::vector<Object> objects_buffer; //Stores up to 10 sprites

        uint8_t LCDC; //FF40
        uint8_t STAT; //FF41
        uint8_t LY; //FF44
        uint8_t WLY; //Internal Window Line counter

        uint8_t SCX, SCY; //FF43, FF42
        uint8_t WX, WY; //FF4A, FF4B

        bool window_triggered;
        bool prev_LCDC_b7;

        int dots;
        Mode current_mode;

        Ppu(Gb* parent);
        void loadBackGround();
        void loadObjects();
        void loadFromOAM();
        void tick(int cycles);
        uint8_t readMemory(uint16_t addr);
        void writeMemory(uint16_t addr, uint8_t value);
        void checkSTAT();
        void checkLYC();

        Gb* gb;
};