//Copyright (C) 2026  Giuseppe Caruso
#include <iostream>
#include <gb.h>
#include <utils.h>
#include <ppu.h>

Ppu::Ppu(Gb* parent) : gb(parent) {
    LCDC = 0x91;
    STAT = 0x85;
    current_mode = MODE2_OAM;
}

void Ppu::loadBackGround(uint8_t row) {
    uint16_t base_pointer = getBit(LCDC, 4) ? 0x8000 : 0x9000;
    uint16_t tile_map = getBit(LCDC, 3) ? 0x9C00 : 0x9800;
    uint8_t addr;
    uint8_t colour;

    SCY = gb->readMemory(0xFF42);
    SCX = gb->readMemory(0xFF43);

    uint8_t pal = gb->readMemory(0xFF47); //BGP
    for (int i = 0; i < 256; i++) {
        uint8_t offY = row + SCY;
        uint8_t offX = row + SCX;

        uint8_t tile_idx = gb->readMemory(tile_map + ((offY / 8 * 32) + (offX / 8))); 

        if (base_pointer == 0x8000) {
            addr = base_pointer + (tile_idx * 16) + (offY % 8 * 2);
            colour = (getBit(gb->readMemory(addr), 7- (offX % 8))) + (getBit(gb->readMemory(addr + 1), 7- (offX % 8))) * 2;
        } else {
            addr = base_pointer + ((int8_t) tile_idx * 16) + (offY % 8 * 2);
            colour = (getBit(gb->readMemory(addr), 7- (offX % 8))) + (getBit(gb->readMemory(addr + 1), 7- (offX % 8))) * 2;
        }
        std::cout<<(int) colour <<"\n";
    }
}

void Ppu::tick(int cycles) {
    dots += cycles;
    
    switch(current_mode) {
        case MODE0_HBLANK:
            LY++;
            if (LY == 144) {
                current_mode = MODE1_VBLANK;
            } else {
                current_mode = MODE2_OAM;
            }     
            break;

        case MODE1_VBLANK:
            LY++;
            if (LY == 154) {
                current_mode = MODE2_OAM;
                LY = 0;
            }
            gb->memory[0xFF44] = LY;
            break;

        case MODE2_OAM:
            current_mode = MODE3_DRAW;
            break;

        case MODE3_DRAW:
            current_mode = MODE0_HBLANK;
            break;
    }
}