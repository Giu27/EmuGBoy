//Copyright (C) 2026  Giuseppe Caruso
#pragma once

class Ppu {
    uint8_t LCDC; //FF40
    uint8_t STAT; //FF41
    uint8_t LY; //FF44

    void tick(int cycles);
};